static environment BuiltinEnv;
static executing_block *HeadExecBlock;
static environment *RootEnv;

/* TODO: switch statements default to NULL. Implement some kind of error
 * messages */

void internalEval(ast_base * Node, environment * Env);

void evalProgram(ast_program *Program, environment *Env);
void evalNumber(ast_number *Number);
void evalBool(ast_boolean *Node);
void evalString(ast_string *String);
void evalArrayLiteral(ast_array_literal *Array, environment *Env);
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
  AddToEnv(&BuiltinEnv, "System", (object *)GetObjectSystemClass());

  InitObjectMethodEnvs();

  /* Remember the root env */
  RootEnv = Root;
}

#define OBJECT_STACK_SIZE 256
static object * ObjectStack[OBJECT_STACK_SIZE];
static int ObjectStackSize = 0;
void objectStackPush(void * obj) {
  if (ObjectStackSize < OBJECT_STACK_SIZE) {
    ObjectStack [ ObjectStackSize++ ] = (object *) obj;
  } else {
    fprintf(stderr, "Stack Overflow!\n");
    exit (EXIT_FAILURE);
  }
}

void * objectStackPop(void) {
  if (ObjectStackSize - 1 < 0) {
    fprintf(stderr, "Stack Underflow!\n");
    exit(EXIT_FAILURE);
  }

  return ObjectStack [ --ObjectStackSize ];
}

void * objectStackPeek(void) {
  if (ObjectStackSize - 1 < 0) {
    fprintf(stderr, "Stack Underflow!\n");
    exit(EXIT_FAILURE);
  }
  return ObjectStack [ ObjectStackSize - 1 ];
}


object *Eval(ast_base *Node, environment *Env) {
  internalEval(Node, Env);
  return objectStackPop();
}

void internalEval(ast_base * Node, environment * Env) {
  switch (Node->Type) {
  case AST_PROGRAM: {
    evalProgram((ast_program *) Node, Env);
  } break;
  case AST_NUMBER: {
    evalNumber((ast_number *) Node);
  } break;
  case AST_BOOLEAN: {
    evalBool((ast_boolean *) Node);
  } break;
  case AST_STRING: {
    evalString((ast_string *) Node);
  } break;
  case AST_ARRAY_LITERAL: {
    evalArrayLiteral((ast_array_literal *) Node, Env);
  } break;
  }
}

object *unwrapReturnValue(object *Obj) {
  if (Obj->Type == FLUFF_OBJECT_RETURN) {
    return ((object_return *)Obj)->Retval;
  }
  return Obj;
}

/* evaluates the statements inside a program ast node
 * 
 * @return: the last evaluated statement on the stack
 */
void evalProgram(ast_program *Program, environment *Env) {
  ast_base ** Statements = Program->Statements;
  unsigned int StatementsSize = ArraySize(Statements);

  for (unsigned int i = 0; i < StatementsSize; i++) {
    internalEval(Statements [ i ], Env);

    object * EvaluatedObject = objectStackPeek();
    if (EvaluatedObject->Type == FLUFF_OBJECT_ERROR) {
      /* early terminate on error */
      return;
    } else if (EvaluatedObject->Type == FLUFF_OBJECT_RETURN) {
      /* early terminate on return */
      objectStackPop();
      objectStackPush(unwrapReturnValue(EvaluatedObject));
      return;
    }
  }

  /* leave return value on stack */
}


/* creates a number object based on the ast node value
 * 
 * @return: number object on the stack
 */
void evalNumber(ast_number *Number) {
  object_number *Num = NewNumber();
  Num->Type = Number->Type;

  switch (Number->Type) {
  case NUM_INTEGER: {
    Num->Int = Number->Int;
  } break;
  case NUM_DOUBLE: {
    Num->Dbl = Number->Dbl;
  } break;
  }

  objectStackPush(Num);
}


/* creates a boolean object based on the ast node value
 * 
 * @return: boolean object on the stack
 */
void evalBool(ast_boolean *Bool) {
  object_boolean * EvaluatedBool = NewBoolean();
  Bool->Value = Bool->Value;
  objectStackPush(EvaluatedBool);
}

/* creates a string object based on the ast node value
 * 
 * @return: string object on the stack
 */
void evalString(ast_string *String) {
  object_string * EvaluatedString = NewString(strlen(String->Value) + 1);
  strcpy(EvaluatedString->Value, String->Value);
  objectStackPush(EvaluatedString);
}

/* creates an array object based on the ast node value
 * 
 * @return: array object on the stack
 */
void evalArrayLiteral(ast_array_literal * ArrayNode, environment *Env) {
  object_array * Array = NewArray();
  Array->Items = NULL;

  /* keep the array on the stack because we'll be leaving this
   * function to evaluate each item
   */
  objectStackPush(Array);

  ast_base ** Items = ArrayNode->Items;
  unsigned int ItemsLength = ArraySize(Items);

  for (unsigned int i = 0; i < ItemsLength; i++) {
    internalEval(Items [ i ], Env);

    object ** p = GCMalloc(sizeof(*p));
    *p = objectStackPop();
    ArrayPush(Array->Items, p);
  }

  /* leave array on stack as the return value */
}


