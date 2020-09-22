static object_null NullObject;

/* TODO: switch statements default to NULL. Implement some kind of error
 * messages */
object *evalProgram(ast_program *Program);
object *evalBlock(ast_block_statement *Block);
object *evalPrefixExpression(token_type Op, object *Obj);
object *evalBangOperatorExpression(object *Ojb);
object *evalMinusPrefixOperatorExpression(object *Obj);
object *evalInfixExpression(token_type Op, object *Left, object *Right);
object *evalIntegerInfixExpression(token_type Op, object_integer *Left,
                                   object_integer *Right);
object *evalBooleanInfixExpression(token_type Op, object_boolean *Left,
                                   object_boolean *Right);
bool isTruthy(object *Obj);

void EvalInit(void) {
  NullObject.Base.Type = OBJECT_NULL;
  NullObject.Base.Size = 0;
}

object *Eval(ast_base *Node) {
  switch (Node->Type) {

  case AST_PROGRAM: {
    return evalProgram((ast_program *)Node);
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

  case AST_PREFIX_EXPRESSION: {
    ast_prefix_expression *Prefix = (ast_prefix_expression *)Node;
    object *Right = Eval(Prefix->Right);
    return evalPrefixExpression(Prefix->Operation, Right);
  } break;

  case AST_INFIX_EXPRESSION: {
    ast_infix_expression *Infix = (ast_infix_expression *)Node;
    object *Left = Eval(Infix->Left);
    object *Right = Eval(Infix->Right);
    return evalInfixExpression(Infix->Operation, Left, Right);
  } break;

  case AST_BLOCK_STATEMENT: {
    return evalBlock((ast_block_statement *)Node);
  } break;

  case AST_IF_EXPRESSION: {
    ast_if_expression *IfExpr = (ast_if_expression *)Node;
    object *Cond = Eval(IfExpr->Condition);
    if (isTruthy(Cond)) {
      return Eval(IfExpr->Consequence);
    } else if (IfExpr->Alternative) {
      return Eval(IfExpr->Alternative);
    } else {
      return (object *)&NullObject;
    }
  } break;

  case AST_RETURN_STATEMENT: {
    object_return *Return =
        (object_return *)NewObject(OBJECT_RETURN, sizeof(object_return));
    object *Retval = Eval(((ast_return_statement *)Node)->Expr);
    Return->Retval = Retval;
    return (object *)Return;
  } break;

  default:
    return (object *)&NullObject;
  }
}

object *evalProgram(ast_program *Program) {
  object *Result;
  ast_base **Statements = Program->Statements;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i]);

    if (Result->Type == OBJECT_RETURN) {
      return ((object_return *)Result)->Retval;
    }
  }

  return Result;
}

object *evalBlock(ast_block_statement *Block) {
  object *Result;
  ast_base **Statements = Block->Statements;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i]);

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
  }
}

object *evalBangOperatorExpression(object *Obj) {
  object_boolean *Bool =
      (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
  switch (Obj->Type) {

  case OBJECT_BOOLEAN: {
    Bool->Value = !((object_boolean *)Obj)->Value;
  } break;

  case OBJECT_INTEGER: {
    Bool->Value = !((object_integer *)Obj)->Value;
  } break;

  default: {
    return (object *)&NullObject;
  } break;
  }

  return (object *)(Bool);
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
    return (object *)&NullObject;
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
    }
    }
  } else {
    return (object *)&NullObject;
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
    return (object *)&NullObject;
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
