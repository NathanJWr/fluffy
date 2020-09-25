/**
 * IN PROGRESS - NIC - 24 sep 2020
 */

void testParser(const char *str, ...) {
  lexer Lexer;
  parser Parser;
  ast_program *Program;

  unsigned int StringStoreSize = 0x10000;
  char *StringStore = calloc(1, StringStoreSize);

  char *debugLine = calloc(1, 0x1000);
  int i = 0;
  ast_type expectedType;
  va_list expectedTypes;
  va_start(expectedTypes, str);

  LexerInit(&Lexer, &str[0], &str[strlen(str)], StringStore, StringStoreSize);
  ParserInit(&Parser, &Lexer);
  Program = ParseProgram(&Parser);

  sprintf(debugLine, "%s", str);
  printLog(debugLine, 1);
  printLog("================================", 0);
  while (1) {
    // expectedType = va_arg(expectedTypes, ast_type);
    // type = NextToken(&Lexer);
    // sprintf(debugLine, "  [%2d] : %s ? %s", i + 1, AstType[expectedType],
    //         AstType[type]);
    // printLog(debugLine, 1);
    // assert(type == expectedType);
    // switch (expectedType) {
    //     case TOKEN_IDENT:
    //     case TOKEN_STRING: {
    //         expectedString = va_arg(expectedTypes, char *);
    //         sprintf(debugLine, "   | %s ? %s", expectedString, Lexer.String);
    //         printLog(debugLine, 1);
    //         assert(0 == strcmp(Lexer.String, expectedString));
    //         break;
    //     }
    //     case TOKEN_INT: {
    //         expectedInt = va_arg(expectedTypes, long);
    //         sprintf(debugLine, "   | %ld ? %ld", expectedInt, Lexer.Integer);
    //         printLog(debugLine, 1);
    //         assert(Lexer.Integer == expectedInt);
    //         break;
    //     }
    //     case TOKEN_END: {
    //         atEnd = 1;
    //         break;
    //     }
    //     default: {
    //         break;
    //     }
    // }
    // if (atEnd) {
    //     break;
    // } else {
    //     i++;
    // }
    break;
  }
  printLog("================================", 0);
  va_end(expectedTypes);

  /* only now should we free any buffers we've allocated */
  free(debugLine);
  free(StringStore);
}

void readBlock(ast_base *base) {
  switch (base->Type) {
  case AST_PROGRAM: {
    /* use stretchy arrays */
    ast_program *base_program = (ast_program *)base;
    int len = ArraySize(base_program->Statements);
    break;
  }
  default:
    break;
  }
}