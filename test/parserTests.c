void readBlock(ast_base *, va_list, int);

void testParser(const char *str, ...) {
  lexer Lexer;
  parser Parser;
  ast_program *Program;

  unsigned int StringStoreSize = 0x10000;
  char *StringStore = calloc(1, StringStoreSize);

  char *debugLine = calloc(1, 0x1000);
  va_list expectedTypes;
  va_start(expectedTypes, str);

  LexerInit(&Lexer, &str[0], &str[strlen(str)], StringStore, StringStoreSize);
  ParserInit(&Parser, &Lexer);
  Program = ParseProgram(&Parser);

  sprintf(debugLine, "%s", str);
  printLog(debugLine, 1);
  printLog("================================", 0);
  readBlock((ast_base *)Program, expectedTypes, 0);
  printLog("================================", 0);
  va_end(expectedTypes);

  free(debugLine);
  free(StringStore);
}

void readBlock(ast_base *base, va_list expectedTypes, int tabs) {
  char *debugLine = calloc(1, 0x1000);
  char *expectedString;
  long expectedInt;
  double expectedDbl, delta, epsilon = __DBL_EPSILON__;
  bool expectedBool;
  fluff_token_type expectedToken;
  ast_type expectedType;
  expectedType = va_arg(expectedTypes, ast_type);
  sprintf(debugLine, "  [%d] : %s ? %s", tabs, AstType[expectedType],
          AstType[base->Type]);
  printLog(debugLine, 1);
  assert(expectedType == base->Type);
  switch (base->Type) {
  case AST_PROGRAM: {
    /* use stretchy arrays */
    ast_program *base_program = (ast_program *)base;
    int i, len = ArraySize(base_program->Statements);
    for (i = 0; i < len; i++) {
      readBlock(base_program->Statements[i], expectedTypes, tabs + 1);
    }
  } break;
  case AST_IDENTIFIER: {
    ast_identifier *identifier = (ast_identifier *)base;
    expectedString = va_arg(expectedTypes, char *);
    sprintf(debugLine, "   | %s ? %s", expectedString, identifier->Value);
    printLog(debugLine, 1);
    assert(0 == strcmp(expectedString, identifier->Value));
  } break;
  case AST_STRING: {
    ast_string *stringy = (ast_string *)base;
    expectedString = va_arg(expectedTypes, char *);
    sprintf(debugLine, "   | %s ? %s", expectedString, stringy->Value);
    printLog(debugLine, 1);
    assert(0 == strcmp(expectedString, stringy->Value));
  } break;
  case AST_NUMBER: {
    ast_number *number = (ast_number *)base;
    switch (number->Type) {
    case NUM_INTEGER: {
      expectedInt = va_arg(expectedTypes, long);
      sprintf(debugLine, "   | %ld ? %ld", expectedInt, number->Int);
      printLog(debugLine, 1);
      assert(expectedInt == number->Int);
    } break;
    case NUM_DOUBLE: {
      expectedDbl = va_arg(expectedTypes, double);
      delta = abs(expectedDbl - number->Dbl);
      sprintf(debugLine, "   | %lf ? %lf", expectedDbl, number->Dbl);
      printLog(debugLine, 1);
      assert(delta <= epsilon);
    } break;
    default: {
      assert(0);
    } break;
    }
    break;
  }
  case AST_BOOLEAN: {
    ast_boolean *boolean = (ast_boolean *)base;
    expectedBool = va_arg(expectedTypes, bool);
    sprintf(debugLine, "   | %d ? %d", expectedBool, boolean->Value);
    printLog(debugLine, 1);
    assert(expectedBool == boolean->Value);
  } break;
  case AST_PREFIX_EXPRESSION: {
    ast_prefix_expression *prefix = (ast_prefix_expression *)base;
    expectedToken = va_arg(expectedTypes, fluff_token_type);
    sprintf(debugLine, "   | %s ? %s", FluffTokenType[expectedToken],
            FluffTokenType[prefix->Operation]);
    printLog(debugLine, 1);
    assert(expectedToken == prefix->Operation);
    readBlock(prefix->Right, expectedTypes, tabs + 1);
  } break;
  case AST_INFIX_EXPRESSION: {
    ast_infix_expression *infix = (ast_infix_expression *)base;
    expectedToken = va_arg(expectedTypes, fluff_token_type);
    sprintf(debugLine, "   | %s ? %s", FluffTokenType[expectedToken],
            FluffTokenType[infix->Operation]);
    printLog(debugLine, 1);
    assert(expectedToken == infix->Operation);
    readBlock(infix->Left, expectedTypes, tabs + 1);
    readBlock(infix->Right, expectedTypes, tabs + 1);
  } break;
  case AST_VAR_STATEMENT: {
    ast_var_statement *var = (ast_var_statement *)base;
    readBlock((ast_base *)var->Name, expectedTypes, tabs + 1);
    readBlock(var->Value, expectedTypes, tabs + 1);
  } break;
  case AST_RETURN_STATEMENT: {
    ast_return_statement *ret = (ast_return_statement *)base;
    readBlock(ret->Expr, expectedTypes, tabs + 1);
  } break;
  case AST_IF_EXPRESSION: {
    ast_if_expression *ifst = (ast_if_expression *)base;
    readBlock(ifst->Condition, expectedTypes, tabs + 1);
    readBlock(ifst->Consequence, expectedTypes, tabs + 1);
    readBlock(ifst->Alternative, expectedTypes, tabs + 1);
  } break;
  case AST_BLOCK_STATEMENT: {
    ast_block_statement *block = (ast_block_statement *)base;
    int i, len = ArraySize(block->Statements);
    for (i = 0; i < len; i++) {
      readBlock(block->Statements[i], expectedTypes, tabs + 1);
    }
  } break;
  case AST_FUNCTION_LITERAL: {
    ast_function_literal *fnlit = (ast_function_literal *)base;
    int i, len = ArraySize(fnlit->Parameters);
    for (i = 0; i < len; i++) {
      readBlock(fnlit->Parameters[i], expectedTypes, tabs + 1);
    }
    readBlock(fnlit->Body, expectedTypes, tabs + 1);
  } break;
  case AST_FUNCTION_CALL: {
    ast_function_call *fncall = (ast_function_call *)base;
    readBlock(fncall->FunctionName, expectedTypes, tabs + 1);
    int i, len = ArraySize(fncall->Arguments);
    for (i = 0; i < len; i++) {
      readBlock(fncall->Arguments[i], expectedTypes, tabs + 1);
    }
  } break;
  default: {
    assert(0);
  } break;
  }
  free(debugLine);
}
