/**
 * If we expect an identifier, then also assert the Lexer's output is equal to
 * the string input. The last expected type must be TOKEN_END to break out of
 * the function.
 */
void testLexer(const char *str, ...) {
  lexer Lexer;
  unsigned int StringStoreSize = 0x10000;
  char *StringStore = malloc(StringStoreSize);
  fluff_token_type type, expectedType;
  int i = 0;
  char *expectedString;
  long expectedInt;
  double expectedDbl;
  char *debugLine = calloc(
      1, 0x1000); /* only malloc once, continously overwrite, free at end */
  int atEnd = 0;
  va_list expectedTypes;
  va_start(expectedTypes, str);

  LexerInit(&Lexer, &str[0], &str[strlen(str)], StringStore, StringStoreSize);

  sprintf(debugLine, "%s", str);
  printLog(debugLine, 1);
  printLog("================================", 0);
  while (1) {
    expectedType = va_arg(expectedTypes, fluff_token_type);
    type = NextToken(&Lexer);
    sprintf(debugLine, "  [%2d] : %s ? %s", i + 1, FluffTokenType[expectedType],
            FluffTokenType[type]);
    printLog(debugLine, 1);
    assert(type == expectedType);
    switch (expectedType) {
    case TOKEN_IDENT:
    case TOKEN_STRING: {
      expectedString = va_arg(expectedTypes, char *);
      sprintf(debugLine, "   | %s ? %s", expectedString, Lexer.String);
      printLog(debugLine, 1);
      assert(0 == strcmp(Lexer.String, expectedString));
      break;
    }
    case TOKEN_INT: {
      expectedInt = va_arg(expectedTypes, long);
      sprintf(debugLine, "   | %ld ? %ld", expectedInt, Lexer.Integer);
      printLog(debugLine, 1);
      assert(Lexer.Integer == expectedInt);
      break;
    }
    case TOKEN_DOUBLE: {
      expectedDbl = va_arg(expectedTypes, double);
      sprintf(debugLine, "   | %lf ? %lf", expectedDbl, Lexer.Double);
      printLog(debugLine, 1);
      assert(Lexer.Double == expectedDbl);
      break;
    }
    case TOKEN_END: {
      atEnd = 1;
      break;
    }
    default: {
      break;
    }
    }
    if (atEnd) {
      break;
    } else {
      i++;
    }
  }
  printLog("================================", 0);
  va_end(expectedTypes);

  free(debugLine);
  free(StringStore);
}
