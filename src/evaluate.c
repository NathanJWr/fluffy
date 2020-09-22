static object_null NullObject;

/* TODO: switch statements default to NULL. Implement some kind of error
 * messages */
object *evalStatements(ast_base **Statements);
object *evalPrefixExpression(token_type Op, object *Obj);
object *evalBangOperatorExpression(object *Ojb);
object *evalMinusPrefixOperatorExpression(object *Obj);

void EvalInit(void) {
  NullObject.Base.Type = OBJECT_NULL;
  NullObject.Base.Size = 0;
}

object *Eval(ast_base *Node) {
  switch (Node->Type) {

  case AST_PROGRAM: {
    return evalStatements(((ast_program *)Node)->Statements);
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

  default:
    return (object *)&NullObject;
  }
}

object *evalStatements(ast_base **Statements) {
  object *Result;
  unsigned int StatementsSize = ArraySize(Statements);
  unsigned int i;

  for (i = 0; i < StatementsSize; i++) {
    Result = Eval(Statements[i]);
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
  switch (Obj->Type) {

  case OBJECT_BOOLEAN: {
    object_boolean *Bool =
        (object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean));
    Bool->Value = !((object_boolean *)Obj)->Value;
    return (object *)(Bool);
  } break;

  default: {
    return NULL;
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
    return NULL;
  }
  }
}
