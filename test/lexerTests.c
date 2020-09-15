int isDebug = 0;

void printLog(char *);

/**
 * If we expect an identifier, then also assert the Lexer's output is equal to
 * the string input. The last expected type must be TOKEN_END to break out of
 * the function.
 */
void testLexer(const char *str, ...) {
  lexer Lexer;
  unsigned int StringStoreSize = 0x10000;
  char *StringStore = malloc(StringStoreSize);
  token_type type, expectedType;
  int i = 0;
  char *expectedString;
  long expectedInt;
  char *debugLine;
  int atEnd = 0;
  va_list expectedTypes;
  va_start(expectedTypes, str);

  LexerInit(&Lexer, &str[0], &str[strlen(str)], StringStore, StringStoreSize);

  debugLine = calloc(1, 0x1000);
  sprintf(debugLine, "%s", str);
  printLog(debugLine);
  printLog("================================");
  while (1) {
    expectedType = va_arg(expectedTypes, token_type);
    type = NextToken(&Lexer);
    sprintf(debugLine, "  [%d]: %s ? %s", i + 1, TokenType[expectedType],
            TokenType[type]);
    printLog(debugLine);
    assert(type == expectedType);
    switch (expectedType) {
    case TOKEN_IDENT: {
      expectedString = va_arg(expectedTypes, char *);
      sprintf(debugLine, "   | %s ? %s", expectedString, Lexer.String);
      printLog(debugLine);
      assert(0 == strcmp(Lexer.String, expectedString));
      break;
    }
    case TOKEN_INT: {
      expectedInt = va_arg(expectedTypes, long);
      sprintf(debugLine, "   | %ld ? %ld", expectedInt, Lexer.Integer);
      printLog(debugLine);
      assert(Lexer.Integer == expectedInt);
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
  printLog("================================");
  va_end(expectedTypes);

  /* only now should we free any buffers we've allocated */
  free(debugLine);
  free(StringStore);
}

void printLog(char *line) {
  if (isDebug) {
    printf("%s\n", line);
  }
  line[0] = '\0'; /* reset the buffer so new message can be put in it */
}
