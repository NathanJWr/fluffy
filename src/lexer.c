void skipWhitespace(lexer *l);
void readChar(lexer *l);
token_type readIdentifier(lexer *Lexer);
char peekChar(lexer *l);
token_type lookupTokenType(char *ident);

token_type NextToken(lexer *Lexer) {
  token_type Token;
  char c = *Lexer->ParseLocation;

  skipWhitespace(Lexer);

  switch (c) {
  default: {
    if (isalpha(c)) {
      Token = readIdentifier(Lexer);
    } else {
      Token = TOKEN_ILLEGAL;
    }
  } break;
  case '>': {
    Token = TOKEN_GT;
  } break;
  case '<': {
    Token = TOKEN_LT;
  } break;
  case '+': {
    Token = TOKEN_PLUS;
  } break;
  case '-': {
    Token = TOKEN_MINUS;
  } break;
  case '!': {
    if (peekChar(Lexer) == '=') {
      readChar(Lexer);
      Token = TOKEN_NOT_EQ;
    } else {
      Token = TOKEN_BANG;
    }
  } break;
  case '*': {
    Token = TOKEN_ASTERISK;
  } break;
  case '/': {
    Token = TOKEN_SLASH;
  } break;
  case ',': {
    Token = TOKEN_COMMA;
  } break;
  case ';': {
    Token = TOKEN_SEMICOLON;
  } break;
  case '(': {
    Token = TOKEN_LPAREN;
  } break;
  case ')': {
    Token = TOKEN_RPAREN;
  } break;
  case '{': {
    Token = TOKEN_LBRACE;
  } break;
  case '}': {
    Token = TOKEN_RBRACE;
  } break;
  case '=': {
    if (peekChar(Lexer) == '=') {
      readChar(Lexer);
      Token = TOKEN_EQ;
    } else {
      Token = TOKEN_ASSIGN;
    }
  } break;
  } /* end switch */

  readChar(Lexer);
  return Token;
}

void LexerInit(lexer *Lexer, const char *Input, const char *InputEndLocation,
               char *StringStore, unsigned int StringStoreSize) {
  Lexer->Input = Input;
  Lexer->EndLocation = InputEndLocation;
  Lexer->ParseLocation = (char *)Input;
  Lexer->StringStorage = StringStore;
  Lexer->StringStorageLength = StringStoreSize;
}

void skipWhitespace(lexer *l) {
  while (isspace(*l->ParseLocation)) {
    readChar(l);
  }
}

void readChar(lexer *l) {
  if (l->ParseLocation != l->EndLocation) {
    l->ParseLocation++;
  }
}

char peekChar(lexer *l) {
  if (l->ParseLocation + 1 != l->EndLocation) {
    return *(l->ParseLocation + 1);
  } else {
    return 0;
  }
}

token_type readIdentifier(lexer *Lexer) {
  token_type Token;

  /* construct a string in the string storage memory */
  char *String = Lexer->StringStorage;
  char *StringEnd = Lexer->StringStorage;
  while (isalpha(*Lexer->ParseLocation)) {
    *StringEnd++ = *Lexer->ParseLocation++;
  }
  *StringEnd++ = '\0';

  Token = lookupTokenType(String);
  if (Token == TOKEN_IDENT) {
    /* actually keep the string around in memory */
    Lexer->String = String;
    Lexer->StringStorage = StringEnd;
  }
  /* Note(Nathan): We don't care about keeping strings around that
   * are not identifiers (e.g. return, var, etc.). Therefore they will be
   * overwritten on subsequent calls to readIdentifier() */

  return Token;
}

token_type lookupTokenType(char *ident) {
  if (0 == strcmp(ident, "var")) {
    return TOKEN_VAR;
  } else if (0 == strcmp(ident, "fn")) {
    return TOKEN_FUNCTION;
  } else if (0 == strcmp(ident, "true")) {
    return TOKEN_TRUE;
  } else if (0 == strcmp(ident, "false")) {
    return TOKEN_FALSE;
  } else if (0 == strcmp(ident, "if")) {
    return TOKEN_IF;
  } else if (0 == strcmp(ident, "else")) {
    return TOKEN_ELSE;
  } else if (0 == strcmp(ident, "return")) {
    return TOKEN_RETURN;
  } else {
    return TOKEN_IDENT;
  }
}
