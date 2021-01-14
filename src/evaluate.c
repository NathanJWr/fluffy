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
void evalBlock(ast_block_statement *Block, environment *Env);
object **evalExpressions(ast_base **Exprs, environment *Env,
                         executing_block *ExecBlock);
object ***evalArrayItems(ast_base **Items, environment *Env,
                         executing_block *ExecBlock);
void evalPrefix(ast_prefix_expression * Prefix, environment *Env);
object *evalPrefixExpression(fluff_token_type Op, object *Obj);
void evalIfExpression(ast_if_expression * IfExpr, environment *Env);
void evalReturnStatement(ast_return_statement * ReturnStatement, environment *Env);
void evalVarStatement(ast_var_statement * VarStatement, environment *Env);
void evalIdentifier(ast_identifier * Ident, environment *Env);
void evalFunctionLiteral(ast_function_literal * FunctionLiteral, environment *Env);
void evalFunction(ast_function_call * FunctionCall, environment *Env);
object *evalIndexExpression(ast_base *Node, environment *Env,
                            executing_block *ExecBlock);
object *evalClassStatement(ast_base *Node, environment *Env,
                           executing_block *ExecBlock);
object *evalNewExpression(ast_base *Node, environment *Env,
                          executing_block *ExecBlock);

void evalBangPrefixOperatorExpression();
void evalMinusPrefixOperatorExpression();

void evalInfix(ast_infix_expression * Infix, environment *Env);
object *evalInfixExpression(fluff_token_type Op, object *Left, object *Right);
object *evalDotOperator(ast_base *Left, ast_base *Right, environment *Env,
                        executing_block *ExecBlock);
object *evalFunctionCall(object *Fn, object **Exprs,
                         executing_block *ExecBlock);
object *evalInfixAssignExpression(ast_base *Left, ast_base *Right,
                                  environment *Env, executing_block *ExecBlock);
void evalNumberInfixExpression(fluff_token_type Op);
void evalStringInfixExpression(fluff_token_type Op);
void evalBooleanInfixExpression(fluff_token_type Op);
bool isTruthy(object *Obj);
void applyFunction(void);
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
  case AST_PREFIX_EXPRESSION: {
    evalPrefix((ast_prefix_expression *) Node, Env);
  } break;
  case AST_INFIX_EXPRESSION: {
    evalInfix((ast_infix_expression *) Node, Env);
  } break;
  case AST_BLOCK_STATEMENT: {
    evalBlock((ast_block_statement *) Node, Env);
  } break;
  case AST_IF_EXPRESSION: {
    evalIfExpression((ast_if_expression *) Node, Env);
  } break;
  case AST_RETURN_STATEMENT: {
    evalReturnStatement((ast_return_statement *) Node, Env);
  } break;
  case AST_VAR_STATEMENT: {
    evalVarStatement((ast_var_statement *) Node, Env);
  } break;
  case AST_IDENTIFIER: {
    evalIdentifier((ast_identifier *) Node, Env);
  } break;
  case AST_FUNCTION_LITERAL: {
    evalFunctionLiteral((ast_function_literal *) Node, Env);
  } break;
  case AST_FUNCTION_CALL: {
    evalFunction((ast_function_call *) Node, Env);
  } break;
  default: {
    objectStackPush(NewError("Cannot evaluate %s", AstType [ Node->Type ]));
  }
  }
}

/* calls internal eval and returns if the object at the top of the stack is an error */
#define internalEvalReturnIfError(Node, Env)  \
  internalEval(Node, Env); \
  if ( ((object *) objectStackPeek())->Type == FLUFF_OBJECT_ERROR ) \
    return;

/* calls internal eval and returns if the object at the top of the stack is an error 
 * or of type FLUFF_OBJECT_RETURN */
#define internalEvalReturnIfErrorOrReturn(Node, Env)  \
  internalEval(Node, Env); \
  if ( ((object *) objectStackPeek())->Type == FLUFF_OBJECT_ERROR ) \
    return; \
  else if ( ((object *) objectStackPeek())->Type == FLUFF_OBJECT_RETURN ) \
    return;

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
  size_t StatementsSize = ArraySize(Statements);

  for (unsigned int i = 0; i < StatementsSize; i++) {
    internalEvalReturnIfError(Statements [ i ], Env);
    
    object * EvaluatedObject = objectStackPeek();
    if (EvaluatedObject->Type == FLUFF_OBJECT_RETURN) {
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
  EvaluatedBool->Value = Bool->Value;
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
  size_t ItemsLength = ArraySize(Items);

  for (size_t i = 0; i < ItemsLength; i++) {
    internalEvalReturnIfError(Items [ i ], Env);

    object ** p = GCMalloc(sizeof(*p));
    *p = objectStackPop();
    ArrayPush(Array->Items, p);
  }

  /* leave array on stack as the return value */
}

/* evaluates whatever is on the right of the prefix expression and then applies the prefix operator
 * to the result 
 *
 * @return: object that contains the evaluated prefix expression on the stack
 */
void evalPrefix(ast_prefix_expression *Prefix, environment *Env) {
  internalEvalReturnIfError(Prefix->Right, Env);
  switch (Prefix->Operation) {
  case TOKEN_BANG: {
    evalBangPrefixOperatorExpression();
  } break;
  case TOKEN_MINUS: {
    evalMinusPrefixOperatorExpression();
  } break;
  default: {
    object * RightEval = objectStackPeek();
    objectStackPush(NewError("unknown operator: %s on %s", FluffTokenType [ Prefix->Operation ],
                             FluffObjectType [ RightEval->Type ]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* attempts to apply the bang operator to the object on the top of the stack
 *
 * @return: object that contains the result of the bang prefix operator or an error if the bang 
 *          operator cannot be applied to a particular object type
 */
void evalBangPrefixOperatorExpression() {
  object * Obj = objectStackPop();
  switch (Obj->Type) {
  case FLUFF_OBJECT_BOOLEAN: {
    object_boolean * Bool = (object_boolean *) Obj;
    object_boolean * EvalBool = NewBoolean();
    EvalBool->Value = !Bool->Value;
    objectStackPush(EvalBool);
  } break;
  case FLUFF_OBJECT_NUMBER: {
    object_number * Number = (object_number *) Obj;
    object_boolean * EvalBool = NewBoolean();
    switch (Number->Type) {
    case NUM_INTEGER: {
      EvalBool->Value = !Number->Int;
    } break;
    case NUM_DOUBLE: {
      EvalBool->Value = !Number->Dbl;
    } break;
    }
    objectStackPush(EvalBool);
  } break;
  default: {
    objectStackPush(NewError("unknown operator: !%s", FluffObjectType[Obj->Type]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* Attempts to apply the minux prefix operator to the object on the top of the stack.
 * 
 * @return: object that contains the result of the minus prefix operator or an error if the
 *          operator cannot be applied to a particular object type
 */
void evalMinusPrefixOperatorExpression() {
  object * Obj = objectStackPop();
  switch(Obj->Type) {
  case FLUFF_OBJECT_NUMBER: {
    object_number * Number = (object_number *) Obj;
    object_number * EvalNumber = NewNumber();
    EvalNumber->Type = Number->Type;
    switch (Number->Type) {
    case NUM_INTEGER: {
      EvalNumber->Int = -Number->Int;
    } break;
    case NUM_DOUBLE: {
      EvalNumber->Dbl = -Number->Dbl;
    } break;
    }
    objectStackPush(EvalNumber);
  } break;
  default: {
    objectStackPush(NewError("unknown operator: !%s", FluffObjectType[Obj->Type]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* evaluates an infix expression ast node
 *
 * @return: evaluated object or error on the stack */
void evalInfix(ast_infix_expression * Infix, environment *Env) {
  /* NOTE: we are evaluating right side first, THEN left. This means that the left evaluated object
   * will be on the top of the stack. */
  internalEvalReturnIfError(Infix->Right, Env);
  internalEvalReturnIfError(Infix->Left, Env);
  object * LeftEval = objectStackPeek();
  /* NOTE: don't need to worry about type mismatch here, just call the 
   * infix evaluator for the left hand's type and let them handle it. */
  switch (LeftEval->Type) {
  case FLUFF_OBJECT_NUMBER: {
    evalNumberInfixExpression(Infix->Operation);
  } break;
  case FLUFF_OBJECT_BOOLEAN: {
    evalBooleanInfixExpression(Infix->Operation);
  } break;
  case FLUFF_OBJECT_STRING: {
    evalStringInfixExpression(Infix->Operation);
  } break;
  default: {
    objectStackPush(NewError("unsupported type %s in infix expression", FluffTokenType[Infix->Operation]));
  }
  }
  /* leave result/error on stack as the return value */
}


/* Helper functon fo infix expressions for numbers.
 * This particular version of the helper function should be called when
 * both the left and right side of the expression are integers
 *
 * @return: number containing the evaluated integer or error on the stack */
evalNumberInfixExpressionii(fluff_token_type Op, int Left, int Right) {
  switch (Op) {
  case TOKEN_PLUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_INTEGER;
    Num->Int = Left + Right;
    objectStackPush(Num);
  } break;
  case TOKEN_MINUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_INTEGER;
    Num->Int = Left - Right;
    objectStackPush(Num);
  } break;
  case TOKEN_SLASH: {
    object_number * Num = NewNumber();
    Num->Type = NUM_INTEGER;
    Num->Int = Left / Right;
    objectStackPush(Num);
  } break;
  case TOKEN_ASTERISK: {
    object_number * Num = NewNumber();
    Num->Type = NUM_INTEGER;
    Num->Int = Left * Right;
    objectStackPush(Num);
  } break;
  case TOKEN_LT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left < Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_GT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left > Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left == Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_NOT_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left != Right;
    objectStackPush(Bool);
  } break;
  default: {
    objectStackPush(NewError("Uknown operator in integer infix expression: %s",
                             FluffTokenType [ Op ]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* Helper functon fo infix expressions for numbers.
 * This particular version of the helper function should be called when
 * both the left and right side of the expression are int and double
 * respectively.
 *
 * @return: number containing the evaluated double or error on the stack */
evalNumberInfixExpressionid(fluff_token_type Op, int Left, double Right) {
  switch (Op) {
  case TOKEN_PLUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left + Right;
    objectStackPush(Num);
  } break;
  case TOKEN_MINUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left - Right;
    objectStackPush(Num);
  } break;
  case TOKEN_SLASH: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left / Right;
    objectStackPush(Num);
  } break;
  case TOKEN_ASTERISK: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left * Right;
    objectStackPush(Num);
  } break;
  case TOKEN_LT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left < Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_GT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left > Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left == Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_NOT_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left != Right;
    objectStackPush(Bool);
  } break;
  default: {
    objectStackPush(NewError("Uknown operator in integer infix expression: %s",
                             FluffTokenType [ Op ]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* Helper functon fo infix expressions for numbers.
 * This particular version of the helper function should be called when
 * both the left and right side of the expression are double and int
 * respectively.
 *
 * @return: number containing the evaluated double or error on the stack */
evalNumberInfixExpressiondi(fluff_token_type Op, double Left, int Right) {
  switch (Op) {
  case TOKEN_PLUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left + Right;
    objectStackPush(Num);
  } break;
  case TOKEN_MINUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left - Right;
    objectStackPush(Num);
  } break;
  case TOKEN_SLASH: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left / Right;
    objectStackPush(Num);
  } break;
  case TOKEN_ASTERISK: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left * Right;
    objectStackPush(Num);
  } break;
  case TOKEN_LT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left < Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_GT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left > Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left == Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_NOT_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left != Right;
    objectStackPush(Bool);
  } break;
  default: {
    objectStackPush(NewError("Uknown operator in integer infix expression: %s",
                             FluffTokenType [ Op ]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* Helper functon fo infix expressions for numbers.
 * This particular version of the helper function should be called when
 * both the left and right side of the expression are doubles.
 *
 * @return: number containing the evaluated double or error on the stack */
evalNumberInfixExpressiondd(fluff_token_type Op, double Left, double Right) {
  switch (Op) {
  case TOKEN_PLUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left + Right;
    objectStackPush(Num);
  } break;
  case TOKEN_MINUS: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left - Right;
    objectStackPush(Num);
  } break;
  case TOKEN_SLASH: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left / Right;
    objectStackPush(Num);
  } break;
  case TOKEN_ASTERISK: {
    object_number * Num = NewNumber();
    Num->Type = NUM_DOUBLE;
    Num->Dbl = Left * Right;
    objectStackPush(Num);
  } break;
  case TOKEN_LT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left < Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_GT: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left > Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left == Right;
    objectStackPush(Bool);
  } break;
  case TOKEN_NOT_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = Left != Right;
    objectStackPush(Bool);
  } break;
  default: {
    objectStackPush(NewError("Uknown operator in integer infix expression: %s",
                             FluffTokenType [ Op ]));
  }
  }
  /* leave result/error on stack as the return value */
}

/* Evaluates an infix expression between two numbers.
 * The Left object of the infix expression (top of the stack) is expected to be a number.
 *
 * @return: an object containing the evaluated number on the stack */
void evalNumberInfixExpression(fluff_token_type Op) {
  object * LeftObj = objectStackPop();
  object * RightObj = objectStackPop();

  if (LeftObj->Type != RightObj->Type) {
    objectStackPush(NewError("unknown operator: %s %s %s",
                             FluffObjectType[LeftObj->Type],
                             FluffTokenType[Op],
                             FluffObjectType[RightObj->Type]));
    return;
  }
  object_number * LeftNum = (object_number *) LeftObj;
  object_number * RightNum = (object_number *) RightObj;

  if (LeftNum->Type == NUM_INTEGER && RightNum->Type == NUM_INTEGER) {
    evalNumberInfixExpressionii(Op, LeftNum->Int, RightNum->Int);
  } else if (LeftNum->Type == NUM_INTEGER && RightNum->Type == NUM_DOUBLE) {
    evalNumberInfixExpressionid(Op, LeftNum->Int, RightNum->Dbl);
  } else if (LeftNum->Type == NUM_DOUBLE && RightNum->Type == NUM_INTEGER) {
    evalNumberInfixExpressiondi(Op, LeftNum->Dbl, RightNum->Int);
  } else {
    evalNumberInfixExpressiondd(Op, LeftNum->Dbl, RightNum->Dbl);
  } 
  /* leave result/error on stack as the return value */
}

/* Evaluates an infix expression between two booleans.
 * The Left object of the infix expression (top of the stack) is expected to be a boolean.
 *
 * @return: an object containing the evaluated number on the stack */
void evalBooleanInfixExpression(fluff_token_type Op) {
  object * LeftObj = objectStackPop();
  object * RightObj = objectStackPop();

  if (LeftObj->Type != RightObj->Type) {
    objectStackPush(NewError("unknown operator: %s %s %s",
                             FluffObjectType[LeftObj->Type],
                             FluffTokenType[Op],
                             FluffObjectType[RightObj->Type]));
    return;
  }

  object_boolean * LeftBool = (object_boolean *) LeftObj;
  object_boolean * RightBool = (object_boolean *) RightObj;

  switch (Op) {
  case TOKEN_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = LeftBool->Value == RightBool->Value;
    objectStackPush(Bool);
  } break;
  case TOKEN_NOT_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = LeftBool->Value != RightBool->Value;
    objectStackPush(Bool);
  } break;
  default: {
    objectStackPush(NewError("Uknown operator in boolean infix expression: %s",
                             FluffTokenType [ Op ]));
  } break;
  }
  /* leave result/error on stack as the return value */
}

/* Evaluates an infix expression between two strings.
 * The Left object of the infix expression (top of the stack) is expected to be a string.
 *
 * @return: an object containing the evaluated string or bool on the stack depending on the
            operation */
void evalStringInfixExpression(fluff_token_type Op) {
  object * LeftObj = objectStackPop();
  object * RightObj = objectStackPop();

  if (LeftObj->Type != RightObj->Type) {
    objectStackPush(NewError("unknown operator: %s %s %s",
                             FluffObjectType[LeftObj->Type],
                             FluffTokenType[Op],
                             FluffObjectType[RightObj->Type]));
    return;
  }

  object_string * LeftStr = (object_string *) LeftObj;
  object_string * RightStr = (object_string *) RightObj;

  switch (Op) {
  case TOKEN_PLUS: {
    size_t LeftSize = strlen(LeftStr->Value);
    size_t RightSize = strlen(RightStr->Value);
    size_t Size = LeftSize + RightSize + 1;

    object_string * Str = NewString(Size);
    memcpy(Str->Value, LeftStr->Value, LeftSize);
    memcpy(Str->Value + LeftSize, RightStr->Value, RightSize);
    Str->Value[Size - 1] = '\0';

    objectStackPush(Str);
  } break;
  case TOKEN_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = (0 == strcmp(LeftStr->Value, RightStr->Value));

    objectStackPush(Bool);
  } break;
  case TOKEN_NOT_EQ: {
    object_boolean * Bool = NewBoolean();
    Bool->Value = (0 != strcmp(LeftStr->Value, RightStr->Value));

    objectStackPush(Bool);
  } break;
  default: {
    objectStackPush(NewError("Uknown operator in string infix expression: %s",
                             FluffTokenType [ Op ]));
  }
  }
  /* leave result/error on stack as the return value */
}

void evalBlock(ast_block_statement *Block, environment *Env) {
  ast_base ** Statements = Block->Statements;
  size_t StatementsSize = ArraySize(Statements);

  for (size_t i = 0; i < StatementsSize; i++) {
    internalEvalReturnIfErrorOrReturn(Statements [ i ], Env);
  }
  /* leave result/error on stack as the return value */
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

/* Evaluates an if expression.
 * 
 * @return: null object or error on the stack */
void evalIfExpression(ast_if_expression * IfExpr, environment *Env) {
  internalEvalReturnIfError(IfExpr->Condition, Env);
  object * Condition = objectStackPop();
  if (isTruthy(Condition)) {
    evalBlock(IfExpr->Consequence, Env);
  } else if (IfExpr->Alternative) {
    evalBlock(IfExpr->Alternative, Env);
  } else {
    objectStackPush(&NullObject);
  }
  /* leave result/error on stack as the return value */
}

/* Evaluates a return statement.
 *
 * @return: a return object containing the evaluated expression or and error */
void evalReturnStatement(ast_return_statement * ReturnStatement, environment *Env) {
  internalEvalReturnIfError(ReturnStatement->Expr, Env);
  object_return * Return = NewReturn();
  Return->Retval = objectStackPop();
  objectStackPush(Return);
  /* leave result/error on stack as the return value */
}

/* Evaluates a var statement by adding an entry into the environment that is specified.
 * 
 * @return: an error if the variable exists OR null object on the stack */
void evalVarStatement(ast_var_statement * VarStatement, environment *Env) {
  internalEvalReturnIfError(VarStatement->Value, Env);
  object * EvaluatedObj = objectStackPop();
  if (!FindInEnv(Env, VarStatement->Name->Value)) {
    AddToEnv(Env, VarStatement->Name->Value, EvaluatedObj);
    objectStackPush(&NullObject);
  } else {
    objectStackPush(NewError("variable %s already exists!", VarStatement->Name->Value));
  }
  /* leave result/error on stack as the return value */
}

void evalIdentifier(ast_identifier * Ident, environment *Env) {
  /* Check if the identifier exists in the current Env */
  object * EnvObject = FindInEnv(Env, Ident->Value);
  if (EnvObject) {
    objectStackPush (EnvObject);
    return;
  }

  /* Check if the identifier exists in the builtin env */
  EnvObject = FindInEnv(&BuiltinEnv, Ident->Value);
  if (EnvObject) {
    objectStackPush(EnvObject);
  } else {
    objectStackPush(NewError("could not find identifier: %s", Ident->Value));
  }
  /* leave result/error of the builtin env lookup on stack as the return value */
}

/* Evaluates a function literal ast node by storing the parameter, body, and
 * environment information that is given. This information is then used whenever
 * the function literal is invoked.
 *
 * @return: a function literal object on the stack */
void evalFunctionLiteral(ast_function_literal * FunctionLiteral, environment *Env) {
  object_function * Function = NewFunction();

  Function->Parameters = FunctionLiteral->Parameters;
  Function->Body = FunctionLiteral->Body;
  Function->Env = Env;

  objectStackPush(Function);
  /* leave object on stack as the return value */
}

void evalFunction(ast_function_call * FunctionCall, environment *Env) {
  object_function_instance * FunctionInstance = NewFunctionInstance();
  objectStackPush(FunctionInstance); /* Save the instance on the stack */

  /* look up the previously defined function */
  evalIdentifier(FunctionCall->FunctionName, Env) ;
  object * Function = objectStackPeek();
  if (Function->Type != FLUFF_OBJECT_FUNCTION) {
    if (Function->Type == FLUFF_OBJECT_ERROR) {
      return; /* pass along the error */
    } else {
      objectStackPush(NewError("expected identifier %s to reference a function",
                               FunctionCall->FunctionName));
      return;
    }
  }
  FunctionInstance->Function = (object_function *) objectStackPop();
  
  /* create a new enclosed environment for the function instance */
  FunctionInstance->Env = CreateEnclosedEnvironment(FunctionInstance->Function->Env);

  /* evaluate the arguments passed to the function */
  ast_base ** FunctionArgs = FunctionCall->Arguments;
  size_t FunctionArgsLength = ArraySize(FunctionArgs);
  FunctionInstance->EvaluatedArguments = NULL;
  for (size_t i = 0; i < FunctionArgsLength; i++) {
    internalEvalReturnIfError(FunctionArgs [ i ], Env);
    object * Param = objectStackPop();
    ArrayPush(FunctionInstance->EvaluatedArguments, Param); 
    AddToEnv(FunctionInstance->Env,
             FunctionInstance->Function->Parameters [ i ]->Value,
             Param);
  }

  
  applyFunction();
}

void applyFunction(void) {
  object * Arg = objectStackPeek();

  /* Make sure our Arg is a function instance */
  if (Arg->Type != FLUFF_OBJECT_FUNCTION_INSTANCE) {
    objectStackPop();
    objectStackPush(NewError("not a function: %s", FluffObjectType [ Arg->Type ]));
    return;
  }
  object_function_instance * FunctionInstance = (object_function_instance *) Arg;

  /* Check that we were passed the correct number of arguments */
  size_t NumParameters = ArraySize(FunctionInstance->EvaluatedArguments);
  size_t NumExpectedParameters = ArraySize(FunctionInstance->Function->Parameters);
  if (NumParameters != NumExpectedParameters) {
    objectStackPush(NewError(
        "invalid number of arguments to fn: expected %d, recieved %d",
        NumExpectedParameters, NumParameters));
    return;
  }

  evalBlock(FunctionInstance->Function->Body, FunctionInstance->Env);
}
