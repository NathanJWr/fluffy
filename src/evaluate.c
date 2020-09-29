static object_null NullObject = {
    .Base.Type = FLUFF_OBJECT_NULL,
    .Base.Size = 0,
};

static environment BuiltinEnv;
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

static object_builtin BuiltinPrint = {
    .Base.Type = FLUFF_OBJECT_BUILTIN,
    .Base.Size = sizeof(object_builtin),
    .Fn = builtinPrint,
};

static object_builtin BuiltinType = {
    .Base.Type = FLUFF_OBJECT_BUILTIN,
    .Base.Size = sizeof(object_builtin),
    .Fn = builtinType,
};

/* TODO: switch statements default to NULL. Implement some kind of error
 * messages */
object *evalProgram(ast_program *Program, environment *Env);
object *evalBlock(ast_block_statement *Block, environment *Env);
object **evalExpressions(ast_base **Exprs, environment *Env);
object ***evalArrayItems(ast_base **Items, environment *Env);
object *evalPrefixExpression(fluff_token_type Op, object *Obj);
object *evalBangOperatorExpression(object *Ojb);
object *evalMinusPrefixOperatorExpression(object *Obj);
object *evalInfixExpression(fluff_token_type Op, object *Left, object *Right);
object *evalNumberInfixExpression(fluff_token_type Op, object_number *Left,
                                  object_number *Right);
object *evalStringInfixExpression(fluff_token_type Op, object_string *Left,
                                  object_string *Right);
object *evalBooleanInfixExpression(fluff_token_type Op, object_boolean *Left,
                                   object_boolean *Right);
bool isTruthy(object *Obj);
object *applyFunction(object *Fn, object **Args);
environment *extendFnEnv(object_function *Fn, object **Args,
                         unsigned int ArgsLength);
object *unwrapReturnValue(object *Obj);
char *DuplicateStringWithGC(char *Str);

void EvalInit(environment *Root) {
  /* Create a builtin function table */
  InitEnv(&BuiltinEnv, 16, malloc);
  AddToEnv(&BuiltinEnv, "print", (object *)&BuiltinPrint);
  AddToEnv(&BuiltinEnv, "type", (object *)&BuiltinType);

  /* Remember the root env */
  RootEnv = Root;
}

object *Eval(ast_base *Node, environment *Env) {
  switch (Node->Type) {

  case AST_PROGRAM: {
    return evalProgram((ast_program *)Node, Env);
  } break;

  case AST_NUMBER: {
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
  } break;

  case AST_BOOLEAN: {
    object_boolean *Bool = NewBoolean();
    Bool->Value = ((ast_boolean *)Node)->Value;
    return (object *)Bool;
  } break;

  case AST_STRING: {
    char *AstStr = ((ast_string *)Node)->Value;
    object_string *Str = (object_string *)NewObject(
        FLUFF_OBJECT_STRING, sizeof(object_string) + strlen(AstStr) + 1);
    strcpy(Str->Value, AstStr);
    return (object *)Str;
  } break;

  case AST_ARRAY_LITERAL: {
    ast_array_literal *Arr = (ast_array_literal *)Node;
    object_array *ArrObject =
        (object_array *)NewObject(FLUFF_OBJECT_ARRAY, sizeof(object_array));
    ArrObject->Items = evalArrayItems(Arr->Items, Env);
    return (object *)ArrObject;
  } break;

  case AST_PREFIX_EXPRESSION: {
    ast_prefix_expression *Prefix = (ast_prefix_expression *)Node;
    object *Right = Eval(Prefix->Right, Env);
    if (Right->Type == FLUFF_OBJECT_ERROR) {
      return Right;
    }
    return evalPrefixExpression(Prefix->Operation, Right);
  } break;

  case AST_INFIX_EXPRESSION: {
    ast_infix_expression *Infix = (ast_infix_expression *)Node;
    object *Left = Eval(Infix->Left, Env);
    object *Right = Eval(Infix->Right, Env);
    if (Left->Type == FLUFF_OBJECT_ERROR) {
      return Left;
    }
    if (Right->Type == FLUFF_OBJECT_ERROR) {
      return Right;
    }
    return evalInfixExpression(Infix->Operation, Left, Right);
  } break;

  case AST_BLOCK_STATEMENT: {
    return evalBlock((ast_block_statement *)Node, Env);
  } break;

  case AST_IF_EXPRESSION: {
    ast_if_expression *IfExpr = (ast_if_expression *)Node;
    object *Cond = Eval(IfExpr->Condition, Env);
    if (Cond->Type == FLUFF_OBJECT_ERROR) {
      return Cond;
    }
    if (isTruthy(Cond)) {
      return Eval(IfExpr->Consequence, Env);
    } else if (IfExpr->Alternative) {
      return Eval(IfExpr->Alternative, Env);
    } else {
      return (object *)&NullObject;
    }
  } break;

  case AST_RETURN_STATEMENT: {
    object_return *Return = NewReturn();
    object *Retval = Eval(((ast_return_statement *)Node)->Expr, Env);
    Return->Retval = Retval;
    return (object *)Return;
  } break;

  case AST_VAR_STATEMENT: {
    ast_var_statement *Stmt = (ast_var_statement *)Node;
    object *Val = Eval(Stmt->Value, Env);
    if (Val->Type == FLUFF_OBJECT_ERROR) {
      return Val;
    }

    AddToEnv(Env, (char *)Stmt->Name->Value, Val);
    return (object *)&NullObject;
  } break;

  case AST_IDENTIFIER: {
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
  } break;

  case AST_FUNCTION_LITERAL: {
    ast_function_literal *Fn = (ast_function_literal *)Node;
    object_function *FnObj = NewFunction();
    FnObj->Parameters = (ast_identifier **)Fn->Parameters;
    FnObj->Body = (ast_block_statement *)Fn->Body;
    FnObj->Env = Env;
    FnObj->RecurEnvs = NULL;
    return (object *)FnObj;
  } break;

  case AST_FUNCTION_CALL: {
    ast_function_call *Call = (ast_function_call *)Node;
    object *Fn = Eval(Call->FunctionName, Env);

    object **Exprs = evalExpressions(Call->Arguments, Env);
    object *RetObject = NULL;

    if (Exprs[ArraySize(Exprs) - 1]->Type == FLUFF_OBJECT_ERROR) {
      return Exprs[ArraySize(Exprs) - 1];
    }

    switch (Fn->Type) {
    case FLUFF_OBJECT_FUNCTION: {
      RetObject = applyFunction(Fn, Exprs);
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
  } break;

  case AST_INDEX_EXPRESSION: {
    ast_index *IndexExpr = (ast_index *)Node;
    object *IndexEval = Eval(IndexExpr->Index, Env);
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
  } break;

  default:
    /* TODO: @Nathan want to throw an error here? */
    return (object *)&NullObject;
  }
}

object *applyFunction(object *Fn, object **Args) {
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
  EvaluatedObject = Eval((ast_base *)Function->Body, ExtendedEnv);
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

object **evalExpressions(ast_base **Exprs, environment *Env) {
  object **Result = NULL;
  unsigned int i;
  unsigned int ExprsLength = ArraySize(Exprs);

  for (i = 0; i < ExprsLength; i++) {
    object *Evaluated = Eval(Exprs[i], Env);
    ArrayPush(Result, Evaluated);
    if (Evaluated->Type == FLUFF_OBJECT_ERROR) {
      return Result;
    }
  }
  return Result;
}

object ***evalArrayItems(ast_base **Items, environment *Env) {
  object ***Result = NULL;
  unsigned int i;
  unsigned int ItemsLength = ArraySize(Items);

  for (i = 0; i < ItemsLength; i++) {
    object **EvaluatedPointer = GCMalloc(sizeof(object *));
    *EvaluatedPointer = Eval(Items[i], Env);

    GCArrayPush(Result, EvaluatedPointer);
    if ((*EvaluatedPointer)->Type == FLUFF_OBJECT_ERROR) {
      return Result;
    }
  }

  return Result;
}

object *evalProgram(ast_program *Program, environment *Env) {
  object *Result;
  ast_base **Statements = Program->Statements;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i], Env);

    if (GCNeedsCleanup()) {
      GCMarkAndSweep(RootEnv);
    }
    if (Result->Type == FLUFF_OBJECT_RETURN) {
      return ((object_return *)Result)->Retval;
    }
  }

  return Result;
}

object *evalBlock(ast_block_statement *Block, environment *Env) {
  object *Result;
  ast_base **Statements = Block->Statements;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i], Env);

    if (GCNeedsCleanup()) {
      GCMarkAndSweep(RootEnv);
    }
    if (Result->Type == FLUFF_OBJECT_RETURN) {
      return Result;
    }
  }

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
      return NewError("unsupported type %s in infix expression", FluffTokenType[Op]);
    }
    }
  } else {
    return NewError("type mismatch: %s, %s", FluffObjectType[Left->Type],
                    FluffObjectType[Right->Type]);
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
    return NewError("unknown operator: %s %s %s", FluffObjectType[Left->Base.Type],
                    FluffTokenType[Op], FluffObjectType[Right->Base.Type]);
  }
  }
}

object *evalStringInfixExpression(fluff_token_type Op, object_string *Left,
                                  object_string *Right) {
  switch (Op) {

  case TOKEN_PLUS: {
    unsigned int LeftSize = strlen(Left->Value);
    unsigned int RightSize = strlen(Right->Value);
    unsigned int Size = LeftSize + RightSize + 1;

    object_string *StrObj =
        (object_string *)NewObject(FLUFF_OBJECT_STRING, sizeof(object_string) + Size);
    memcpy(StrObj->Value, Left->Value, LeftSize);
    memcpy(StrObj->Value + LeftSize, Right->Value, RightSize);
    StrObj->Value[Size - 1] = '\0';
    return (object *)StrObj;
  } break;

  case TOKEN_EQ: {
    object_boolean *Eq =
        (object_boolean *)NewObject(FLUFF_OBJECT_BOOLEAN, sizeof(object_boolean));
    Eq->Value = (0 == strcmp(Left->Value, Right->Value));
    return (object *)Eq;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *Eq =
        (object_boolean *)NewObject(FLUFF_OBJECT_BOOLEAN, sizeof(object_boolean));
    Eq->Value = !(0 == strcmp(Left->Value, Right->Value));
    return (object *)Eq;
  } break;

  default: {
    return NewError("unknown operator: %s %s %s", FluffObjectType[Left->Base.Type],
                    FluffTokenType[Op], FluffObjectType[Right->Base.Type]);
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
