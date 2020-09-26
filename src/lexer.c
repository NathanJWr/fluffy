void skipWhitespace(lexer *l);
void readChar(lexer *l);
token_type readIdentifier(lexer *Lexer);
token_type readNumber(lexer *Lexer);
char peekChar(lexer *l);
token_type lookupTokenType(char *ident);
void readString(lexer *Lexer);

token_type NextToken(lexer *Lexer) {
  token_type Token;
  char c;

  if (Lexer->ParseLocation == Lexer->EndLocation) {
    return TOKEN_END;
  }

  skipWhitespace(Lexer);
  c = *Lexer->ParseLocation;

  switch (c) {
  default: {
    if (isalpha(c)) {
      Token = readIdentifier(Lexer);
    } else if (isdigit(c)) {
      Token = readNumber(Lexer);
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
  case '"': {
    readString(Lexer);
    Token = TOKEN_STRING;
  } break;
  case '.': {
    Token = TOKEN_DOT;
  } break;
  case '[': {
    Token = TOKEN_LSQUARE;
  } break;
  case ']': {
    Token = TOKEN_RSQUARE;
  } break;
  case '\0': {
    Token = TOKEN_END;
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

void readString(lexer *Lexer) {
  char *String = Lexer->StringStorage;
  char *StringEnd = Lexer->StringStorage;

  Lexer->ParseLocation++; /* skip the first '"' */
  /* Read until we reach the ending \" */
  while (*Lexer->ParseLocation != '"') {
    *StringEnd++ = *Lexer->ParseLocation++;
  }
  *StringEnd++ = '\0';

  /* Move string storage ahead by the size of the string we just read */
  Lexer->StringStorage = StringEnd;

  /* Set the public string variable to what we just read */
  Lexer->String = String;
}

token_type readIdentifier(lexer *Lexer) {
  token_type Token;

  /* construct a string in the string storage memory */
  char *String = Lexer->StringStorage;
  char *StringEnd = Lexer->StringStorage;
  while (isalpha(*Lexer->ParseLocation) &&
         Lexer->ParseLocation != Lexer->EndLocation) {
    *StringEnd++ = *Lexer->ParseLocation++;
  }
  Lexer->ParseLocation--;
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

token_type readNumber(lexer *Lexer) {
  /* construct an integer in the string storage memory */
  char *Num = Lexer->StringStorage;
  char *NumEnd = Lexer->StringStorage;
  char *End;
  int dotAmt = 0;
  while ((isdigit(*Lexer->ParseLocation) || *Lexer->ParseLocation == '.') &&
         Lexer->ParseLocation != Lexer->EndLocation) {
    if (*Lexer->ParseLocation == '.') {
      dotAmt++;
    }
    *NumEnd++ = *Lexer->ParseLocation++;
  }
  Lexer->ParseLocation--;
  *NumEnd++ = '\0';

  switch (dotAmt) {
  case 0: { /* int */
    /* convert number string into an integer */
    errno = 0;
    Lexer->Integer = strtol(Num, &End, 10);

    /* Check for various possible errors */
    if ((errno == ERANGE &&
         (Lexer->Integer == LONG_MAX || Lexer->Integer == LONG_MIN)) ||
        (errno != 0 && Lexer->Integer == 0)) {
      perror("failed integer conversion");
      exit(EXIT_FAILURE);
    }
    return TOKEN_INT;
  }
  case 1: { /* double */
    /* convert number string into a double */
    errno = 0;
    Lexer->Double = strtod(Num, &End);

    /* Check for various possible errors */
    if (errno == ERANGE || (errno != 0 && Lexer->Double == 0)) {
      perror("failed double conversion");
      exit(EXIT_FAILURE);
    }
    return TOKEN_DOUBLE;
  }
  default: {
    /* error */
    perror("incorrect number format");
    exit(EXIT_FAILURE);
    break;
  }
  }
  if (End == Num) {
    fprintf(stderr, "no digits were found\n");
    exit(EXIT_FAILURE);
  }
}
