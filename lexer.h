struct lexer {
  /* lexer internal variables */
  const char *Input;
  char *ParseLocation;
  const char *EndLocation;

  /* Information for storing strings or identifiers */
  char *StringStorage;
  unsigned int StringStorageLength;

  /* locations for error messages */
  char *WhereFirstChar;
  char *WhereLastChar;

  /* possible variables */
  char *String;
  long Integer;
};

void LexerInit(struct lexer *Lexer, const char *Input,
               const char *InputEndLocation, char *StringStore,
               unsigned int StringStoreSize);
enum token_type NextToken(struct lexer *l);
