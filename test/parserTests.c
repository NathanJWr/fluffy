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
  bool expectedBool;
  token_type expectedToken;
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
    break;
  }
  case AST_IDENTIFIER: {
    ast_identifier *identifier = (ast_identifier *)base;
    expectedString = va_arg(expectedTypes, char *);
    sprintf(debugLine, "   | %s ? %s", expectedString, identifier->Value);
    printLog(debugLine, 1);
    assert(0 == strcmp(expectedString, identifier->Value));
    break;
  }
  case AST_INTEGER_LITERAL: {
    ast_integer_literal *integer_literal = (ast_integer_literal *)base;
    expectedInt = va_arg(expectedTypes, long);
    sprintf(debugLine, "   | %ld ? %ld", expectedInt, integer_literal->Integer);
    printLog(debugLine, 1);
    assert(expectedInt == integer_literal->Integer);
    break;
  }
  case AST_BOOLEAN: {
    ast_boolean *boolean = (ast_boolean *)base;
    expectedBool = va_arg(expectedTypes, bool);
    sprintf(debugLine, "   | %d ? %d", expectedBool, boolean->Value);
    printLog(debugLine, 1);
    assert(expectedBool == boolean->Value);
    break;
  }
  case AST_PREFIX_EXPRESSION: {
    ast_prefix_expression *prefix = (ast_prefix_expression *)base;
    expectedToken = va_arg(expectedTypes, token_type);
    sprintf(debugLine, "   | %s ? %s", TokenType[expectedToken],
            TokenType[prefix->Operation]);
    printLog(debugLine, 1);
    assert(expectedToken == prefix->Operation);
    break;
  }
  default:
    break;
  }
  free(debugLine);
}