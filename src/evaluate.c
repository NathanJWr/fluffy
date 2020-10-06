static object_null NullObject = {
    .Base.Type = FLUFF_OBJECT_NULL,
    .Base.Size = 0,
};

static environment BuiltinEnv;
static executing_block *HeadExecBlock;
static environment *RootEnv;

object *builtinPrint(object **Args) {
  if (ArraySize(Args) == 1) {
    PrintObject(Args[0]);
    printf("\n");
  }
  return (object *)&NullObject;
}

object *builtinType(object **Args) {
  if (ArraySize(Args) == 1) {
    return NewStringCopy(FluffObjectType[Args[0]->Type]);
  }
  return NewError("invalid number of arguments. expected 1 and received %d",
                  ArraySize(Args));
}

STATIC_BUILTIN_FUNCTION_VARIABLE(BuiltinPrint, builtinPrint);
STATIC_BUILTIN_FUNCTION_VARIABLE(BuiltinType, builtinType);

/* TODO: switch statements default to NULL. Implement some kind of error
 * messages */
object *evalProgram(ast_program *Program, environment *Env,
                    executing_block *ExecBlock);
object *evalNumber(ast_base *Node);
object *evalBool(ast_base *Node);
object *evalString(ast_base *Node);
object *evalArrayLiteral(ast_base *Node, environment *Env,
                         executing_block *ExecBlock);
object *evalBlock(ast_block_statement *Block, environment *Env,
                  executing_block *ExecBlock);
object **evalExpressions(ast_base **Exprs, environment *Env,
                         executing_block *ExecBlock);
object ***evalArrayItems(ast_base **Items, environment *Env,
                         executing_block *ExecBlock);
object *evalPrefix(ast_base *Node, environment *Env,
                   executing_block *ExecBlock);
object *evalPrefixExpression(fluff_token_type Op, object *Obj);
object *evalIfExpression(ast_base *Node, environment *Env,
                         executing_block *ExecBlock);
object *evalReturnStatement(ast_base *Node, environment *Env,
                            executing_block *ExecBlock);
object *evalVarStatement(ast_base *Node, environment *Env,
                         executing_block *ExecBlock);
object *evalIdentifier(ast_base *Node, environment *Env);
object *evalFunctionLiteral(ast_base *Node, environment *Env,
                            executing_block *ExecBlock);
object *evalFunction(ast_base *Node, environment *Env,
                     executing_block *ExecBlock);
object *evalIndexExpression(ast_base *Node, environment *Env,
                            executing_block *ExecBlock);
object *evalClassStatement(ast_base *Node, environment *Env,
                           executing_block *ExecBlock);
object *evalNewExpression(ast_base *Node, environment *Env,
                          executing_block *ExecBlock);
object *evalBangOperatorExpression(object *Ojb);
object *evalMinusPrefixOperatorExpression(object *Obj);
object *evalInfix(ast_base *Node, environment *Env, executing_block *ExecBlock);
object *evalInfixExpression(fluff_token_type Op, object *Left, object *Right);
object *evalDotOperator(ast_base *Left, ast_base *Right, environment *Env,
                        executing_block *ExecBlock);
object *evalFunctionCall(object *Fn, object **Exprs,
                         executing_block *ExecBlock);
object *evalInfixAssignExpression(ast_base *Left, ast_base *Right,
                                  environment *Env, executing_block *ExecBlock);
object *evalNumberInfixExpression(fluff_token_type Op, object_number *Left,
                                  object_number *Right);
object *evalStringInfixExpression(fluff_token_type Op, object_string *Left,
                                  object_string *Right);
object *evalBooleanInfixExpression(fluff_token_type Op, object_boolean *Left,
                                   object_boolean *Right);
bool isTruthy(object *Obj);
object *applyFunction(object *Fn, object **Args, executing_block *ExecBlock);
environment *extendFnEnv(object_function *Fn, object **Args,
                         unsigned int ArgsLength);
object *unwrapReturnValue(object *Obj);
char *DuplicateStringWithGC(char *Str);

void EvalInit(environment *Root) {
  /* Create a builtin function table */
  InitEnv(&BuiltinEnv, 16, malloc, free);
  AddToEnv(&BuiltinEnv, "print", (object *)&BuiltinPrint);
  AddToEnv(&BuiltinEnv, "type", (object *)&BuiltinType);

  InitObjectMethodEnvs();

  /* Remember the root env */
  RootEnv = Root;
}

object *wrapReturnVal(object *Ret, executing_block *ExecBlock) {
  if (Ret->Type != FLUFF_OBJECT_NULL)
    ArrayPush(ExecBlock->InFlightObjects, Ret);
  return Ret;
}

object *Eval(ast_base *Node, environment *Env, executing_block *ExecBlock) {
  switch (Node->Type) {
  case AST_PROGRAM:
    return evalProgram((ast_program *)Node, Env, ExecBlock);
  case AST_NUMBER:
    return wrapReturnVal(evalNumber(Node), ExecBlock);
  case AST_BOOLEAN:
    return wrapReturnVal(evalBool(Node), ExecBlock);
  case AST_STRING:
    return wrapReturnVal(evalString(Node), ExecBlock);
  case AST_ARRAY_LITERAL:
    return wrapReturnVal(evalArrayLiteral(Node, Env, ExecBlock), ExecBlock);
  case AST_PREFIX_EXPRESSION:
    return wrapReturnVal(evalPrefix(Node, Env, ExecBlock), ExecBlock);
  case AST_INFIX_EXPRESSION:
    return wrapReturnVal(evalInfix(Node, Env, ExecBlock), ExecBlock);
  case AST_BLOCK_STATEMENT:
    return wrapReturnVal(evalBlock((ast_block_statement *)Node, Env, ExecBlock),
                         ExecBlock);
  case AST_IF_EXPRESSION:
    return evalIfExpression(Node, Env, ExecBlock);
  case AST_RETURN_STATEMENT:
    return wrapReturnVal(evalReturnStatement(Node, Env, ExecBlock), ExecBlock);
  case AST_VAR_STATEMENT:
    return evalVarStatement(Node, Env, ExecBlock);
  case AST_IDENTIFIER:
    return evalIdentifier(Node, Env);
  case AST_FUNCTION_LITERAL:
    return wrapReturnVal(evalFunctionLiteral(Node, Env, ExecBlock), ExecBlock);
  case AST_FUNCTION_CALL:
    return wrapReturnVal(evalFunction(Node, Env, ExecBlock), ExecBlock);
  case AST_INDEX_EXPRESSION:
    return evalIndexExpression(Node, Env, ExecBlock);
  case AST_CLASS_STATEMENT:
    return evalClassStatement(Node, Env, ExecBlock);
  case AST_NEW_EXPRESSION:
    return wrapReturnVal(evalNewExpression(Node, Env, ExecBlock), ExecBlock);
  default:
    /* TODO: @Nathan want to throw an error here? */
    return (object *)&NullObject;
  }
}

object *evalNumber(ast_base *Node) {
  object_number *Num = NewNumber();
  Num->Type = ((ast_number *)Node)->Type;
  switch (Num->Type) {
  case NUM_INTEGER: {
    Num->Int = ((ast_number *)Node)->Int;
  } break;
  case NUM_DOUBLE: {
    Num->Dbl = ((ast_number *)Node)->Dbl;
  } break;
  default: {
    assert(0);
  } break;
  }
  return (object *)Num;
}

object *evalBool(ast_base *Node) {
  object_boolean *Bool = NewBoolean();
  Bool->Value = ((ast_boolean *)Node)->Value;
  return (object *)Bool;
}

object *evalString(ast_base *Node) {
  char *AstStr = ((ast_string *)Node)->Value;
  object_string *Str = (object_string *)NewObject(
      FLUFF_OBJECT_STRING, sizeof(object_string) + strlen(AstStr) + 1);
  strcpy(Str->Value, AstStr);
  return (object *)Str;
}

object *evalArrayLiteral(ast_base *Node, environment *Env,
                         executing_block *ExecBlock) {
  ast_array_literal *Arr = (ast_array_literal *)Node;
  object_array *ArrObject =
      (object_array *)NewObject(FLUFF_OBJECT_ARRAY, sizeof(object_array));
  ArrObject->Items = evalArrayItems(Arr->Items, Env, ExecBlock);
  return (object *)ArrObject;
}

object *evalPrefix(ast_base *Node, environment *Env,
                   executing_block *ExecBlock) {
  ast_prefix_expression *Prefix = (ast_prefix_expression *)Node;
  object *Right = Eval(Prefix->Right, Env, ExecBlock);
  if (Right->Type == FLUFF_OBJECT_ERROR) {
    return Right;
  }
  return evalPrefixExpression(Prefix->Operation, Right);
}

object *evalInfix(ast_base *Node, environment *Env,
                  executing_block *ExecBlock) {
  ast_infix_expression *Infix = (ast_infix_expression *)Node;
  if (Infix->Left->Type == AST_IDENTIFIER && Infix->Operation == TOKEN_ASSIGN) {
    return evalInfixAssignExpression(Infix->Left, Infix->Right, Env, ExecBlock);
  } else if (Infix->Operation == TOKEN_DOT) {
    return evalDotOperator(Infix->Left, Infix->Right, Env, ExecBlock);
  }

  object *Left = Eval(Infix->Left, Env, ExecBlock);
  object *Right = Eval(Infix->Right, Env, ExecBlock);
  if (Left->Type == FLUFF_OBJECT_ERROR) {
    return Left;
  }
  if (Right->Type == FLUFF_OBJECT_ERROR) {
    return Right;
  }
  return evalInfixExpression(Infix->Operation, Left, Right);
}

object *evalIfExpression(ast_base *Node, environment *Env,
                         executing_block *ExecBlock) {
  ast_if_expression *IfExpr = (ast_if_expression *)Node;
  object *Cond = Eval(IfExpr->Condition, Env, ExecBlock);
  if (Cond->Type == FLUFF_OBJECT_ERROR) {
    return Cond;
  }
  if (isTruthy(Cond)) {
    return Eval(IfExpr->Consequence, Env, ExecBlock);
  } else if (IfExpr->Alternative) {
    return Eval(IfExpr->Alternative, Env, ExecBlock);
  } else {
    return (object *)&NullObject;
  }
}

object *evalReturnStatement(ast_base *Node, environment *Env,
                            executing_block *ExecBlock) {
  object_return *Return =
      (object_return *)wrapReturnVal((object *)NewReturn(), ExecBlock);

  object *Retval = Eval(((ast_return_statement *)Node)->Expr, Env, ExecBlock);
  Return->Retval = Retval;
  return (object *)Return;
}

object *evalVarStatement(ast_base *Node, environment *Env,
                         executing_block *ExecBlock) {
  ast_var_statement *Stmt = (ast_var_statement *)Node;
  object *Val = Eval(Stmt->Value, Env, ExecBlock);
  if (Val->Type == FLUFF_OBJECT_ERROR) {
    return Val;
  }

  if (!FindInEnv(Env, (char *)Stmt->Name->Value)) {
    AddToEnv(Env, (char *)Stmt->Name->Value, Val);
  } else {
    return NewError("variable %s already exists!", Stmt->Name->Value);
  }
  return (object *)&NullObject;
}

object *evalIdentifier(ast_base *Node, environment *Env) {
  ast_identifier *Ident = (ast_identifier *)Node;
  object *Obj = FindInEnv(Env, Ident->Value);
  if (Obj) {
    return Obj;
  }
  Obj = FindInEnv(&BuiltinEnv, Ident->Value);
  if (Obj) {
    return Obj;
  } else {
    return (object *)&NullObject;
  }
}

object *evalFunctionLiteral(ast_base *Node, environment *Env,
                            executing_block *ExecBlock) {
  ast_function_literal *Fn = (ast_function_literal *)Node;
  object_function *FnObj = NewFunction();
  FnObj->Parameters = (ast_identifier **)Fn->Parameters;
  FnObj->Body = (ast_block_statement *)Fn->Body;
  FnObj->Env = Env;
  FnObj->RecurEnvs = NULL;
  return (object *)FnObj;
}

object *evalFunction(ast_base *Node, environment *Env,
                     executing_block *ExecBlock) {
  ast_function_call *Call = (ast_function_call *)Node;
  object *Fn = Eval(Call->FunctionName, Env, ExecBlock);
  object **Exprs = evalExpressions(Call->Arguments, Env, ExecBlock);
  if (Exprs[ArraySize(Exprs) - 1]->Type == FLUFF_OBJECT_ERROR) {
    return Exprs[ArraySize(Exprs) - 1];
  }
  return evalFunctionCall(Fn, Exprs, ExecBlock);
}

object *evalIndexExpression(ast_base *Node, environment *Env,
                            executing_block *ExecBlock) {
  ast_index *IndexExpr = (ast_index *)Node;
  object *IndexEval = Eval(IndexExpr->Index, Env, ExecBlock);
  object_number *IndexNumber = NULL;
  object *LookupObject = NULL;

  if (IndexEval->Type == FLUFF_OBJECT_ERROR) {
    return IndexEval;
  }
  /* We really care that the index is actaully a number */
  if (IndexEval->Type != FLUFF_OBJECT_NUMBER) {
    return NewError("index is not a number, is %s",
                    FluffObjectType[IndexNumber->Type]);
  }
  IndexNumber = (object_number *)IndexEval;
  /* We shouldn't try to index with a double */
  if (IndexNumber->Type != NUM_INTEGER) {
    return NewError("index is not and integer");
  }

  /* Find the object we want to index into */
  LookupObject = FindInEnv(Env, IndexExpr->Var->Value);
  switch (LookupObject->Type) {
  case FLUFF_OBJECT_ARRAY: {
    object_array *Arr = (object_array *)LookupObject;
    /* Check the bounds of the array before accessing */
    if (IndexNumber->Int < 0 || IndexNumber->Int >= ArraySize(Arr->Items)) {
      return NewError(
          "attempting to access array elements out of bounds with index %d",
          IndexNumber->Int);
    }
    return *Arr->Items[IndexNumber->Int];
  } break;

  default: {
    return NewError("cannot index object of type %s",
                    FluffObjectType[LookupObject->Type]);
  }
  }
}

object *evalClassStatement(ast_base *Node, environment *Env,
                           executing_block *ExecBlock) {
  ast_class *Class = (ast_class *)Node;
  object_class *ClassObj =
      (object_class *)NewObject(FLUFF_OBJECT_CLASS, sizeof(object_class));

  ClassObj->Variables = NULL;
  ClassObj->Base.MethodEnv = CreateEnvironment();
  size_t ClassStatementsSize = ArraySize(Class->Variables);

  /* Variables are split into two categories
   * First is the functions, which can be evaluated and stored as
   * methods right now. Second is the variables, which can be stored
   * as their ast representations and instantiated when an object
   * of this class is created */
  for (size_t i = 0; i < ClassStatementsSize; i++) {
    if (Class->Variables[i]->Value->Type == AST_FUNCTION_LITERAL) {
      object *Method = Eval(Class->Variables[i]->Value, Env, ExecBlock);
      AddToEnv(ClassObj->Base.MethodEnv, Class->Variables[i]->Name->Value,
               Method);
    } else {
      GCArrayPush(ClassObj->Variables, Class->Variables[i]);
    }
  }

  /* Insert the class name into the user defined class environemnt */
  AddToEnv(RootEnv, Class->Name->Value, (object *)ClassObj);
  return (object *)&NullObject;
}

object *evalNewExpression(ast_base *Node, environment *Env,
                          executing_block *ExecBlock) {
  ast_new_expression *Expr = (ast_new_expression *)Node;

  /* find the class inside our stored user defined class env */
  object *Obj = FindInEnv(RootEnv, Expr->Class->Value);
  object_class *Class = NULL;
  if (Obj->Type == FLUFF_OBJECT_CLASS) {
    Class = (object_class *)Obj;
  }
  if (Class) {
    /* Instantiate the class by creating an enclosed environemnt with
     * whatever variables have been defined */
    object_class_instantiation *Instance =
        (object_class_instantiation *)NewObject(
            FLUFF_OBJECT_CLASS_INSTANTIATION,
            sizeof(object_class_instantiation));
    Instance->Locals = CreateEnvironment();

    size_t VarSize = ArraySize(Class->Variables);
    for (size_t i = 0; i < VarSize; i++) {
      Eval((ast_base *)Class->Variables[i], Instance->Locals, ExecBlock);
    }

    Instance->Base.MethodEnv = Class->Base.MethodEnv;
    return (object *)Instance;
  } else {
    return NewError("class %s does not exist", Expr->Class->Value);
  }
}

/* Standard way to call a function. This should be used
 * for any kind of function object and it's object arguments.
 * This will free the array of object arguments Exprs */
object *evalFunctionCall(object *Fn, object **Exprs,
                         executing_block *ExecBlock) {
  object *RetObject = NULL;
  switch (Fn->Type) {
  case FLUFF_OBJECT_FUNCTION: {
    RetObject = applyFunction(Fn, Exprs, ExecBlock);
  } break;
  case FLUFF_OBJECT_BUILTIN: {
    object_builtin *Builtin = (object_builtin *)Fn;
    RetObject = Builtin->Fn(Exprs);
  } break;
  default: {
    break;
  }
  }
  ArrayFree(Exprs);
  return RetObject;
}

object *applyFunction(object *Fn, object **Args, executing_block *ExecBlock) {
  object_function *Function;
  unsigned int ArgsLength;
  unsigned int ParamsLength;

  environment *ExtendedEnv = NULL;
  object *EvaluatedObject = NULL;

  if (Fn->Type != FLUFF_OBJECT_FUNCTION) {
    return NewError("not a function: %s", FluffObjectType[Fn->Type]);
  }

  Function = (object_function *)Fn;
  ArgsLength = ArraySize(Args);
  ParamsLength = ArraySize(Function->Parameters);

  if (ArgsLength != ParamsLength) {
    return NewError(
        "invalid number of arguments to fn: expected %d, recieved %d",
        ParamsLength, ArgsLength);
  }

  ExtendedEnv = extendFnEnv(Function, Args, ArgsLength);
  /* Save the new environment by inserting ExtendedEnv into a linked list */
  /* NOTE: This is to make the extended environment "accessible" if the
   * garbage collector were to run in the middle of the function's execution
   */
  environment_linked *StoredExtendedEnv = GCMalloc(sizeof(environment_linked));
  StoredExtendedEnv->Env = ExtendedEnv;
  StoredExtendedEnv->Next = NULL;
  StoredExtendedEnv->Prev = NULL;
  if (Function->RecurEnvs) {
    StoredExtendedEnv->Next = Function->RecurEnvs;
    Function->RecurEnvs->Prev = StoredExtendedEnv;
  }
  Function->RecurEnvs = StoredExtendedEnv;

  /* Evaluate the function */
  EvaluatedObject = Eval((ast_base *)Function->Body, ExtendedEnv, ExecBlock);
  EvaluatedObject = unwrapReturnValue(EvaluatedObject);

  /* Remove the Extended Environmenet from the linked list */
  if (StoredExtendedEnv == Function->RecurEnvs) {
    Function->RecurEnvs = StoredExtendedEnv->Next;
  }
  if (StoredExtendedEnv->Next != NULL) {
    StoredExtendedEnv->Next->Prev = StoredExtendedEnv->Prev;
  }
  if (StoredExtendedEnv->Prev != NULL) {
    StoredExtendedEnv->Prev->Next = StoredExtendedEnv->Next;
  }

  return EvaluatedObject;
}

environment *extendFnEnv(object_function *Fn, object **Args,
                         unsigned int ArgsLength) {
  environment *Env = CreateEnclosedEnvironment(Fn->Env);
  unsigned int i;

  for (i = 0; i < ArgsLength; i++) {
    const char *Var = Fn->Parameters[i]->Value;
    object *Obj = Args[i];

    AddToEnv(Env, Var, Obj);
  }

  return Env;
}

object *unwrapReturnValue(object *Obj) {
  if (Obj->Type == FLUFF_OBJECT_RETURN) {
    return ((object_return *)Obj)->Retval;
  }
  return Obj;
}

object **evalExpressions(ast_base **Exprs, environment *Env,
                         executing_block *ExecBlock) {
  object **Result = NULL;
  unsigned int i;
  unsigned int ExprsLength = ArraySize(Exprs);

  for (i = 0; i < ExprsLength; i++) {
    object *Evaluated = Eval(Exprs[i], Env, ExecBlock);
    ArrayPush(Result, Evaluated);
    if (Evaluated->Type == FLUFF_OBJECT_ERROR) {
      return Result;
    }
  }
  return Result;
}

object ***evalArrayItems(ast_base **Items, environment *Env,
                         executing_block *ExecBlock) {
  object ***Result = NULL;
  unsigned int i;
  unsigned int ItemsLength = ArraySize(Items);

  for (i = 0; i < ItemsLength; i++) {
    object **EvaluatedPointer = GCMalloc(sizeof(object *));
    *EvaluatedPointer = Eval(Items[i], Env, ExecBlock);

    GCArrayPush(Result, EvaluatedPointer);
    if ((*EvaluatedPointer)->Type == FLUFF_OBJECT_ERROR) {
      return Result;
    }
  }

  return Result;
}

void markAllExecBlocks() {
  executing_block *Head = HeadExecBlock;
  while (Head) {
    size_t ArrSize = ArraySize(Head->InFlightObjects);
    for (size_t i = 0; i < ArrSize; i++) {
      markObject(Head->InFlightObjects[i]);
    }

    Head = Head->Next;
  }
}

object *evalProgram(ast_program *Program, environment *Env,
                    executing_block *ExecBlock) {
  object *Result;
  ast_base **Statements = Program->Statements;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  /* Create a new exec block and link it with previous ones */
  executing_block *CurExecBlock = calloc(1, sizeof(executing_block));
  if (HeadExecBlock) {
    CurExecBlock->Next = HeadExecBlock;
    (HeadExecBlock)->Prev = CurExecBlock;
  }
  HeadExecBlock = CurExecBlock;

  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i], Env, CurExecBlock);

    if (Result->Type == FLUFF_OBJECT_RETURN) {
      return ((object_return *)Result)->Retval;
    }

    if (GCNeedsCleanup()) {
      markAllExecBlocks();
      GCMarkAndSweep(RootEnv);
    }
  }

  /* Remove our exec block */
  if (CurExecBlock == HeadExecBlock) {
    HeadExecBlock = CurExecBlock->Next;
  }
  if (CurExecBlock->Next != NULL) {
    CurExecBlock->Next->Prev = (HeadExecBlock)->Prev;
  }
  if (CurExecBlock->Prev != NULL) {
    CurExecBlock->Prev->Next = (HeadExecBlock)->Next;
  }
  ArrayFree(CurExecBlock->InFlightObjects);
  free(CurExecBlock);

  return Result;
}

object *evalBlock(ast_block_statement *Block, environment *Env,
                  executing_block *ExecBlock) {
  object *Result;
  ast_base **Statements = Block->Statements;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  /* Create a new exec block and link it with previous ones */
  executing_block *CurExecBlock = calloc(1, sizeof(executing_block));
  if (HeadExecBlock) {
    CurExecBlock->Next = HeadExecBlock;
    HeadExecBlock->Prev = CurExecBlock;
  }
  HeadExecBlock = CurExecBlock;

  /* Execute statements */
  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i], Env, HeadExecBlock);

    if (GCNeedsCleanup()) {
      markAllExecBlocks();
      GCMarkAndSweep(RootEnv);
    }

    if (Result->Type == FLUFF_OBJECT_RETURN) {
      return Result;
    }
  }

  /* Remove our exec block */
  if (CurExecBlock == HeadExecBlock) {
    HeadExecBlock = CurExecBlock->Next;
  }
  if (CurExecBlock->Next != NULL) {
    CurExecBlock->Next->Prev = (HeadExecBlock)->Prev;
  }
  if (CurExecBlock->Prev != NULL) {
    CurExecBlock->Prev->Next = (HeadExecBlock)->Next;
  }
  ArrayFree(CurExecBlock->InFlightObjects);
  free(CurExecBlock);
  return Result;
}

object *evalPrefixExpression(fluff_token_type Op, object *Obj) {
  switch (Op) {
  case TOKEN_BANG: {
    return evalBangOperatorExpression(Obj);
  } break;
  case TOKEN_MINUS: {
    return evalMinusPrefixOperatorExpression(Obj);
  }
  default: {
    return NewError("unknown operator: %s on %s", FluffTokenType[Op],
                    FluffObjectType[Obj->Type]);
  }
  }
}

object *evalBangOperatorExpression(object *Obj) {
  switch (Obj->Type) {

  case FLUFF_OBJECT_BOOLEAN: {
    object_boolean *Bool = NewBoolean();
    Bool->Value = !((object_boolean *)Obj)->Value;
    return (object *)(Bool);
  } break;

  case FLUFF_OBJECT_NUMBER: {
    object_boolean *Bool = NewBoolean();
    switch (((object_number *)Obj)->Type) {
    case NUM_INTEGER: {
      Bool->Value = !((object_number *)Obj)->Int;
    } break;
    case NUM_DOUBLE: {
      Bool->Value = !((object_number *)Obj)->Dbl;
    } break;
    default: {
      assert(0);
    } break;
    }
    return (object *)(Bool);
  } break;

  default: {
    return NewError("unknown operator: !%s", FluffObjectType[Obj->Type]);
  } break;
  }
}

object *evalMinusPrefixOperatorExpression(object *Obj) {
  switch (Obj->Type) {
  case FLUFF_OBJECT_NUMBER: {
    object_number *Num = NewNumber();
    Num->Type = -((object_number *)Obj)->Type;
    switch (Num->Type) {
    case NUM_INTEGER: {
      Num->Int = -((object_number *)Obj)->Int;
    } break;
    case NUM_DOUBLE: {
      Num->Dbl = -((object_number *)Obj)->Dbl;
    } break;
    default: {
      assert(0);
    } break;
    }
    return (object *)Num;
  } break;

  default: {
    return NewError("unknown operator: -%s", FluffObjectType[Obj->Type]);
  }
  }
}

object *evalInfixExpression(fluff_token_type Op, object *Left, object *Right) {
  if (Left->Type == Right->Type) {
    switch (Left->Type) {

    case FLUFF_OBJECT_NUMBER: {
      return evalNumberInfixExpression(Op, (object_number *)Left,
                                       (object_number *)Right);
    } break;

    case FLUFF_OBJECT_BOOLEAN: {
      return evalBooleanInfixExpression(Op, (object_boolean *)Left,
                                        (object_boolean *)Right);
    } break;

    case FLUFF_OBJECT_STRING: {
      return evalStringInfixExpression(Op, (object_string *)Left,
                                       (object_string *)Right);
    }

    default: {
      return NewError("unsupported type %s in infix expression",
                      FluffTokenType[Op]);
    }
    }
  } else {
    return NewError("type mismatch: %s, %s", FluffObjectType[Left->Type],
                    FluffObjectType[Right->Type]);
  }
}

object *evalInfixAssignExpression(ast_base *Left, ast_base *Right,
                                  environment *Env,
                                  executing_block *ExecBlock) {
  object *RightObj = Eval(Right, Env, ExecBlock);
  if (Left->Type == AST_IDENTIFIER) {
    const char *Var = ((ast_identifier *)Left)->Value;
    if (FindInEnv(Env, Var)) {
      ReplaceInEnv(Env, Var, RightObj);
    } else {
      return NewError("variable %s does not exist yet!", Var);
    }
  }
  return (object *)&NullObject;
}

object *evalDotOperator(ast_base *Left, ast_base *Right, environment *Env,
                        executing_block *ExecBlock) {
  if (Right->Type == AST_FUNCTION_CALL) {
    /* This is only slightly different than the normal handling of
     * AST_FUNCTION_CALL This is because we need to insert the implicit This
     * pointer to the calling object as well as lookup the functions in a
     * different way, since the function pointer's for
     * each object's methods are located within the SupportedFunctions inside
     * the root object structure */
    ast_function_call *Method = (ast_function_call *)Right;
    object *Caller = Eval(Left, Env, ExecBlock);
    object **Exprs = evalExpressions(Method->Arguments, Env, ExecBlock);

    /* Insert the Caller into the beginning of exprs as a **this** pointer */
    const char *LookupMethod = ((ast_identifier *)Method->FunctionName)->Value;
    object *Function = FindInEnv(Caller->MethodEnv, LookupMethod);
    if (Caller->Type == FLUFF_OBJECT_CLASS_INSTANTIATION) {
      ((object_function *)Function)->Env =
          ((object_class_instantiation *)Caller)->Locals;
    } else {
      ArrayPush(Exprs, Caller);
      if (ArraySize(Exprs) > 1) {
        /* put Caller at the front */
        size_t End = ArraySize(Exprs) - 1;
        object *Temp = Exprs[0];
        Exprs[0] = Exprs[End];
        Exprs[End] = Temp;
      }
    }
    if (Function) {
      return evalFunctionCall(Function, Exprs, ExecBlock);
    } else {
      return NewError("method %s not found for %s", LookupMethod,
                      FluffObjectType[Caller->Type]);
    }

  } else {
    return NewError("expected function call after dot operator");
  }
}

object *evalNumberInfixExpression(fluff_token_type Op, object_number *Left,
                                  object_number *Right) {
  /* We will be comparing/matching doubles here */
  double LeftVal, RightVal, Delta, Epsilon = DBL_EPSILON;
  switch (Left->Type) {
  case NUM_INTEGER: {
    LeftVal = Left->Int;
  } break;
  case NUM_DOUBLE: {
    LeftVal = Left->Dbl;
  } break;
  default: {
    assert(0);
  } break;
  }
  switch (Right->Type) {
  case NUM_INTEGER: {
    RightVal = Right->Int;
  } break;
  case NUM_DOUBLE: {
    RightVal = Right->Dbl;
  } break;
  default:
    break;
  }
  /* For some comparisons, find the delta between the two values */
  Delta = abs(LeftVal - RightVal);
  switch (Op) {
  case TOKEN_PLUS: {
    object_number *Result = NewNumber();
    /* Cast result to the largest type of LeftVal and RightVal */
    Result->Type = max(Left->Type, Right->Type);
    switch (Result->Type) {
    case NUM_INTEGER: {
      Result->Int = LeftVal + RightVal;
      return (object *)Result;
    } break;
    case NUM_DOUBLE: {
      Result->Dbl = LeftVal + RightVal;
      return (object *)Result;
    } break;
    default: {
      assert(0);
    } break;
    }
  } break;

  case TOKEN_MINUS: {
    object_number *Result = NewNumber();
    /* Cast result to the largest type of LeftVal and RightVal */
    Result->Type = max(Left->Type, Right->Type);
    switch (Result->Type) {
    case NUM_INTEGER: {
      Result->Int = LeftVal - RightVal;
      return (object *)Result;
    } break;
    case NUM_DOUBLE: {
      Result->Dbl = LeftVal - RightVal;
      return (object *)Result;
    } break;
    default: {
      assert(0);
    } break;
    }
  } break;

  case TOKEN_SLASH: {
    object_number *Result = NewNumber();
    /* Cast result to the largest type of LeftVal and RightVal */
    Result->Type = max(Left->Type, Right->Type);
    switch (Result->Type) {
    case NUM_INTEGER: {
      Result->Int = LeftVal / RightVal;
      return (object *)Result;
    } break;
    case NUM_DOUBLE: {
      Result->Dbl = LeftVal / RightVal;
      return (object *)Result;
    } break;
    default: {
      assert(0);
    } break;
    }
  } break;

  case TOKEN_ASTERISK: {
    object_number *Result = NewNumber();
    /* Cast result to the largest type of LeftVal and RightVal */
    Result->Type = max(Left->Type, Right->Type);
    switch (Result->Type) {
    case NUM_INTEGER: {
      Result->Int = LeftVal * RightVal;
      return (object *)Result;
    } break;
    case NUM_DOUBLE: {
      Result->Dbl = LeftVal * RightVal;
      return (object *)Result;
    } break;
    default: {
      assert(0);
    } break;
    }
  } break;

  case TOKEN_LT: {
    object_boolean *ResultBool = NewBoolean();
    ResultBool->Value = LeftVal < RightVal;
    return (object *)ResultBool;
  } break;

  case TOKEN_GT: {
    object_boolean *ResultBool = NewBoolean();
    ResultBool->Value = LeftVal > RightVal;
    return (object *)ResultBool;
  } break;

  case TOKEN_EQ: {
    object_boolean *ResultBool = NewBoolean();
    ResultBool->Value = Delta <= Epsilon;
    return (object *)ResultBool;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *ResultBool = NewBoolean();
    ResultBool->Value = Delta > Epsilon;
    return (object *)ResultBool;
  } break;

  default: {
    return NewError("unknown operator: %s %s %s",
                    FluffObjectType[Left->Base.Type], FluffTokenType[Op],
                    FluffObjectType[Right->Base.Type]);
  }
  }
  return NULL;
}

object *evalStringInfixExpression(fluff_token_type Op, object_string *Left,
                                  object_string *Right) {
  switch (Op) {

  case TOKEN_PLUS: {
    unsigned int LeftSize = strlen(Left->Value);
    unsigned int RightSize = strlen(Right->Value);
    unsigned int Size = LeftSize + RightSize + 1;

    object_string *StrObj = (object_string *)NewObject(
        FLUFF_OBJECT_STRING, sizeof(object_string) + Size);
    memcpy(StrObj->Value, Left->Value, LeftSize);
    memcpy(StrObj->Value + LeftSize, Right->Value, RightSize);
    StrObj->Value[Size - 1] = '\0';
    return (object *)StrObj;
  } break;

  case TOKEN_EQ: {
    object_boolean *Eq = (object_boolean *)NewObject(FLUFF_OBJECT_BOOLEAN,
                                                     sizeof(object_boolean));
    Eq->Value = (0 == strcmp(Left->Value, Right->Value));
    return (object *)Eq;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *Eq = (object_boolean *)NewObject(FLUFF_OBJECT_BOOLEAN,
                                                     sizeof(object_boolean));
    Eq->Value = !(0 == strcmp(Left->Value, Right->Value));
    return (object *)Eq;
  } break;

  default: {
    return NewError("unknown operator: %s %s %s",
                    FluffObjectType[Left->Base.Type], FluffTokenType[Op],
                    FluffObjectType[Right->Base.Type]);
  } break;
  }
}

object *evalBooleanInfixExpression(fluff_token_type Op, object_boolean *Left,
                                   object_boolean *Right) {
  switch (Op) {

  case TOKEN_EQ: {
    object_boolean *Return = NewBoolean();
    Return->Value = Left->Value == Right->Value;
    return (object *)Return;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *Return = NewBoolean();
    Return->Value = Left->Value != Right->Value;
    return (object *)Return;
  } break;

  default: {
    return (object *)&NullObject;
  } break;
  }
}

bool isTruthy(object *Obj) {
  switch (Obj->Type) {
  case FLUFF_OBJECT_NULL: {
    return false;
  } break;
  case FLUFF_OBJECT_BOOLEAN: {
    return ((object_boolean *)Obj)->Value;
  } break;
  case FLUFF_OBJECT_NUMBER: {
    switch (((object_number *)Obj)->Type) {
    case NUM_INTEGER: {
      return ((object_number *)Obj)->Int != 0;
    } break;
    case NUM_DOUBLE: {
      return ((object_number *)Obj)->Dbl != 0;
    } break;
    default: {
      assert(0);
    } break;
    }
  }
  default: {
    return true;
  } break;
  }
}

char *DuplicateStringWithGC(char *Str) {
  char *Dup = GCMalloc(strlen(Str) + 1);
  strcpy(Dup, Str);
  return Dup;
}
