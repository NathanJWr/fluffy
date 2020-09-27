static object_null NullObject;

/* TODO: switch statements default to NULL. Implement some kind of error
 * messages */
object *evalProgram(ast_program *Program, environment *Env);
object *evalBlock(ast_block_statement *Block, environment *Env);
object **evalExpressions(ast_base **Exprs, environment *Env);
object *evalPrefixExpression(token_type Op, object *Obj);
object *evalBangOperatorExpression(object *Ojb);
object *evalMinusPrefixOperatorExpression(object *Obj);
object *evalInfixExpression(token_type Op, object *Left, object *Right);
object *evalNumberInfixExpression(token_type Op, object_number *Left,
                                  object_number *Right);
object *evalStringInfixExpression(token_type Op, object_string *Left,
                                  object_string *Right);
object *evalBooleanInfixExpression(token_type Op, object_boolean *Left,
                                   object_boolean *Right);
bool isTruthy(object *Obj);
object *applyFunction(object *Fn, object **Args);
environment *extendFnEnv(object_function *Fn, object **Args,
                         unsigned int ArgsLength);
object *unwrapReturnValue(object *Obj);
char *DuplicateStringWithGC(char *Str);

void EvalInit(void) {
  NullObject.Base.Type = OBJECT_NULL;
  NullObject.Base.Size = 0;
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
    case num_integer: {
      Num->Int = ((ast_number *)Node)->Int;
    } break;
    case num_double: {
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
        OBJECT_STRING, sizeof(object_string) + strlen(AstStr) + 1);
    strcpy(Str->Value, AstStr);
    return (object *)Str;
  } break;

  case AST_ARRAY_LITERAL: {
    ast_array_literal *Arr = (ast_array_literal *)Node;
    object_array *ArrObject =
        (object_array *)NewObject(OBJECT_ARRAY, sizeof(object_array));
    ArrObject->Items = evalExpressions(Arr->Items, Env);
    return (object *)ArrObject;
  } break;

  case AST_PREFIX_EXPRESSION: {
    ast_prefix_expression *Prefix = (ast_prefix_expression *)Node;
    object *Right = Eval(Prefix->Right, Env);
    if (Right->Type == OBJECT_ERROR) {
      return Right;
    }
    return evalPrefixExpression(Prefix->Operation, Right);
  } break;

  case AST_INFIX_EXPRESSION: {
    ast_infix_expression *Infix = (ast_infix_expression *)Node;
    object *Left = Eval(Infix->Left, Env);
    object *Right = Eval(Infix->Right, Env);
    if (Left->Type == OBJECT_ERROR) {
      return Left;
    }
    if (Right->Type == OBJECT_ERROR) {
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
    if (Cond->Type == OBJECT_ERROR) {
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
    if (Val->Type == OBJECT_ERROR) {
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
    return (object *)FnObj;
  } break;

  case AST_FUNCTION_CALL: {
    ast_function_call *Call = (ast_function_call *)Node;
    object *Fn = Eval(Call->FunctionName, Env);
    object **Exprs = evalExpressions(Call->Arguments, Env);
    object *RetObject = NULL;

    if (Exprs[ArraySize(Exprs) - 1]->Type == OBJECT_ERROR) {
      return Exprs[ArraySize(Exprs) - 1];
    }

    RetObject = applyFunction(Fn, Exprs);
    ArrayFree(Exprs);
    return RetObject;
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

  if (Fn->Type != OBJECT_FUNCTION) {
    return NewError("not a function: %s", ObjectType[Fn->Type]);
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
  EvaluatedObject = Eval((ast_base *)Function->Body, ExtendedEnv);
  EvaluatedObject = unwrapReturnValue(EvaluatedObject);

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
  if (Obj->Type == OBJECT_RETURN) {
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
    if (Evaluated->Type == OBJECT_ERROR) {
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

    if (Result->Type == OBJECT_RETURN) {
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

    if (Result->Type == OBJECT_RETURN) {
      return Result;
    }
  }

  return Result;
}

object *evalPrefixExpression(token_type Op, object *Obj) {
  switch (Op) {
  case TOKEN_BANG: {
    return evalBangOperatorExpression(Obj);
  } break;
  case TOKEN_MINUS: {
    return evalMinusPrefixOperatorExpression(Obj);
  }
  default: {
    return NewError("unknown operator: %s on %s", TokenType[Op],
                    ObjectType[Obj->Type]);
  }
  }
}

object *evalBangOperatorExpression(object *Obj) {
  switch (Obj->Type) {

  case OBJECT_BOOLEAN: {
    object_boolean *Bool = NewBoolean();
    Bool->Value = !((object_boolean *)Obj)->Value;
    return (object *)(Bool);
  } break;

  case OBJECT_NUMBER: {
    object_boolean *Bool = NewBoolean();
    switch (((object_number *)Obj)->Type) {
    case num_integer: {
      Bool->Value = !((object_number *)Obj)->Int;
    } break;
    case num_double: {
      Bool->Value = !((object_number *)Obj)->Dbl;
    } break;
    default: {
      assert(0);
    } break;
    }
    return (object *)(Bool);
  } break;

  default: {
    return NewError("unknown operator: !%s", ObjectType[Obj->Type]);
  } break;
  }
}

object *evalMinusPrefixOperatorExpression(object *Obj) {
  switch (Obj->Type) {
  case OBJECT_NUMBER: {
    object_number *Num = NewNumber();
    Num->Type = -((object_number *)Obj)->Type;
    switch (Num->Type) {
    case num_integer: {
      Num->Int = -((object_number *)Obj)->Int;
    } break;
    case num_double: {
      Num->Dbl = -((object_number *)Obj)->Dbl;
    } break;
    default: {
      assert(0);
    } break;
    }
    return (object *)Num;
  } break;

  default: {
    return NewError("unknown operator: -%s", ObjectType[Obj->Type]);
  }
  }
}

object *evalInfixExpression(token_type Op, object *Left, object *Right) {
  if (Left->Type == Right->Type) {
    switch (Left->Type) {

    case OBJECT_NUMBER: {
      return evalNumberInfixExpression(Op, (object_number *)Left,
                                       (object_number *)Right);
    } break;

    case OBJECT_BOOLEAN: {
      return evalBooleanInfixExpression(Op, (object_boolean *)Left,
                                        (object_boolean *)Right);
    } break;

    case OBJECT_STRING: {
      return evalStringInfixExpression(Op, (object_string *)Left,
                                       (object_string *)Right);
    }

    default: {
      return NewError("unsupported type %s in infix expression", TokenType[Op]);
    }
    }
  } else {
    return NewError("type mismatch: %s, %s", ObjectType[Left->Type],
                    ObjectType[Right->Type]);
  }
}

object *evalNumberInfixExpression(token_type Op, object_number *Left,
                                  object_number *Right) {
  /* We will be comparing/matching doubles here */
  double LeftVal, RightVal, Delta, Epsilon = __DBL_EPSILON__;
  switch (Left->Type) {
  case num_integer: {
    LeftVal = Left->Int;
  } break;
  case num_double: {
    LeftVal = Left->Dbl;
  } break;
  default: {
    assert(0);
  } break;
  }
  switch (Right->Type) {
  case num_integer: {
    RightVal = Right->Int;
  } break;
  case num_double: {
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
    case num_integer: {
      Result->Int = LeftVal + RightVal;
      return (object *)Result;
    } break;
    case num_double: {
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
    case num_integer: {
      Result->Int = LeftVal - RightVal;
      return (object *)Result;
    } break;
    case num_double: {
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
    case num_integer: {
      Result->Int = LeftVal / RightVal;
      return (object *)Result;
    } break;
    case num_double: {
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
    case num_integer: {
      Result->Int = LeftVal * RightVal;
      return (object *)Result;
    } break;
    case num_double: {
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
    return NewError("unknown operator: %s %s %s", ObjectType[Left->Base.Type],
                    TokenType[Op], ObjectType[Right->Base.Type]);
  }
  }
}

object *evalStringInfixExpression(token_type Op, object_string *Left,
                                  object_string *Right) {
  switch (Op) {

  case TOKEN_PLUS: {
    unsigned int LeftSize = strlen(Left->Value);
    unsigned int RightSize = strlen(Right->Value);
    unsigned int Size = LeftSize + RightSize + 1;

    object_string *StrObj =
        (object_string *)NewObject(OBJECT_STRING, sizeof(object_string) + Size);
    memcpy(StrObj->Value, Left->Value, LeftSize);
    memcpy(StrObj->Value + LeftSize, Right->Value, RightSize);
    StrObj->Value[Size-1] = '\0';
    return (object *)StrObj;
  } break;

  case TOKEN_EQ: {
    object_boolean *Eq =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Eq->Value = (0 == strcmp(Left->Value, Right->Value));
    return (object *)Eq;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *Eq =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Eq->Value = !(0 == strcmp(Left->Value, Right->Value));
    return (object *)Eq;
  } break;

  default: {
    return NewError("unknown operator: %s %s %s", ObjectType[Left->Base.Type],
                    TokenType[Op], ObjectType[Right->Base.Type]);
  } break;
  }
}

object *evalBooleanInfixExpression(token_type Op, object_boolean *Left,
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
  case OBJECT_NULL: {
    return false;
  } break;
  case OBJECT_BOOLEAN: {
    return ((object_boolean *)Obj)->Value;
  } break;
  case OBJECT_NUMBER: {
    switch (((object_number *)Obj)->Type) {
    case num_integer: {
      return ((object_number *)Obj)->Int != 0;
    } break;
    case num_double: {
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
