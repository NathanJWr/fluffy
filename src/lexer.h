typedef struct {
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
  double Double;
} lexer;

void LexerInit(lexer *Lexer, const char *Input, const char *InputEndLocation,
               char *StringStore, unsigned int StringStoreSize);
token_type NextToken(lexer *l);
