typedef struct {
  /* Token information */
  lexer *Lexer;

  token_type CurToken;
  char *CurString;
  long CurInteger;

  token_type PeekToken;
} parser;

void ParserInit(parser *Parser, lexer *Lexer);
ast_program ParseProgram(parser *Parser);
