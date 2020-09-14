void skipWhitespace(struct lexer *l);
void readChar(struct lexer *l);
void readIdentifier(struct lexer *Lexer);
char peekChar(struct lexer *l);
enum token_type lookupTokenType(char *ident);

enum token_type NextToken(struct lexer *Lexer) {
  enum token_type Token;
  skipWhitespace(Lexer);

  char c = *Lexer->ParseLocation;
  switch (c) {
  default: {
    if (isalpha(c)) {
      readIdentifier(Lexer);
      Token = lookupTokenType(Lexer->String);
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
    Token = TOKEN_ASSIGN;
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


void LexerInit(struct lexer *Lexer, const char *Input,
               const char *InputEndLocation, char *StringStore,
               unsigned int StringStoreSize) {
  Lexer->Input = Input;
  Lexer->EndLocation = InputEndLocation;
  Lexer->ParseLocation = (char *)Input;
  Lexer->StringStorage = StringStore;
  Lexer->StringStorageLength = StringStoreSize;
}

void skipWhitespace(struct lexer *l) {
  while (isspace(*l->ParseLocation)) {
    readChar(l);
  }
}

void readChar(struct lexer *l) {
  if (l->ParseLocation != l->EndLocation) {
    l->ParseLocation++;
  }
}

char peekChar(struct lexer *l) {
  if (l->ParseLocation + 1!= l->EndLocation) {
    return *(l->ParseLocation + 1);
  } else {
    return 0;
  }
}

void readIdentifier(struct lexer *Lexer) {
  char *String = Lexer->StringStorage;
  while (isalpha(*Lexer->ParseLocation)) {
    *String++ = *Lexer->ParseLocation++;
  }
  *String++ = '\0';
  Lexer->String = Lexer->StringStorage;
  Lexer->StringStorage = String;
}

enum token_type lookupTokenType(char *ident) {
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
