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
object *evalIntegerInfixExpression(token_type Op, object_integer *Left,
                                   object_integer *Right);
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

  case AST_INTEGER_LITERAL: {
    object_integer *Int =
        (object_integer *)NewObject(OBJECT_INTEGER, sizeof(object_integer));
    Int->Value = ((ast_integer_literal *)Node)->Integer;
    return (object *)Int;
  } break;

  case AST_BOOLEAN: {
    object_boolean *Bool =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Bool->Value = ((ast_boolean *)Node)->Value;
    return (object *)Bool;
  } break;

  case AST_STRING: {
    object_string *Str =
        (object_string *)NewObject(OBJECT_STRING, sizeof(object_string));
    Str->Value = DuplicateStringWithGC(((ast_string *)Node)->Value);
    return (object *)Str;
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
    object_return *Return =
        (object_return *)NewObject(OBJECT_RETURN, sizeof(object_return));
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
    object_function *FnObj =
        (object_function *)NewObject(OBJECT_FUNCTION, sizeof(object_function));
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
    object_boolean *Bool =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Bool->Value = !((object_boolean *)Obj)->Value;
    return (object *)(Bool);
  } break;

  case OBJECT_INTEGER: {
    object_boolean *Bool =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Bool->Value = !((object_integer *)Obj)->Value;
    return (object *)(Bool);
  } break;

  default: {
    return NewError("unknown operator: !%s", ObjectType[Obj->Type]);
  } break;
  }
}

object *evalMinusPrefixOperatorExpression(object *Obj) {
  switch (Obj->Type) {
  case OBJECT_INTEGER: {
    object_integer *Int =
        (object_integer *)NewObject(OBJECT_INTEGER, sizeof(object_integer));
    Int->Value = -((object_integer *)Obj)->Value;
    return (object *)Int;
  } break;

  default: {
    return NewError("unknown operator: -%s", ObjectType[Obj->Type]);
  }
  }
}

object *evalInfixExpression(token_type Op, object *Left, object *Right) {
  if (Left->Type == Right->Type) {
    switch (Left->Type) {

    case OBJECT_INTEGER: {
      return evalIntegerInfixExpression(Op, (object_integer *)Left,
                                        (object_integer *)Right);
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

object *evalStringInfixExpression(token_type Op, object_string *Left,
                                  object_string *Right) {
  switch (Op) {

  case TOKEN_PLUS: {
    unsigned int Size = strlen(Left->Value) + strlen(Right->Value) + 1;
    char *Str = GCMalloc(Size);
    strcat(Str, Left->Value);
    strcat(Str, Right->Value);

    object_string *StrObj =
        (object_string *)NewObject(OBJECT_STRING, sizeof(object_string));
    StrObj->Value = Str;
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

object *evalIntegerInfixExpression(token_type Op, object_integer *Left,
                                   object_integer *Right) {
  switch (Op) {
  case TOKEN_PLUS: {
    object_integer *Result =
        (object_integer *)NewObject(OBJECT_INTEGER, sizeof(object_integer));
    Result->Value = Left->Value + Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_MINUS: {
    object_integer *Result =
        (object_integer *)NewObject(OBJECT_INTEGER, sizeof(object_integer));
    Result->Value = Left->Value - Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_SLASH: {
    object_integer *Result =
        (object_integer *)NewObject(OBJECT_INTEGER, sizeof(object_integer));
    Result->Value = Left->Value / Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_ASTERISK: {
    object_integer *Result =
        (object_integer *)NewObject(OBJECT_INTEGER, sizeof(object_integer));
    Result->Value = Left->Value * Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_LT: {
    object_boolean *Result =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Result->Value = Left->Value < Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_GT: {
    object_boolean *Result =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Result->Value = Left->Value > Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_EQ: {
    object_boolean *Result =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Result->Value = Left->Value == Right->Value;
    return (object *)Result;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *Result =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Result->Value = Left->Value != Right->Value;
    return (object *)Result;
  } break;

  default: {
    return NewError("unknown operator: %s %s %s", ObjectType[Left->Base.Type],
                    TokenType[Op], ObjectType[Right->Base.Type]);
  }
  }
}

object *evalBooleanInfixExpression(token_type Op, object_boolean *Left,
                                   object_boolean *Right) {
  switch (Op) {

  case TOKEN_EQ: {
    object_boolean *Return =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Return->Value = Left->Value == Right->Value;
    return (object *)Return;
  } break;

  case TOKEN_NOT_EQ: {
    object_boolean *Return =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
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
  case OBJECT_INTEGER: {
    return ((object_integer *)Obj)->Value != 0;
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
