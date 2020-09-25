typedef struct {
  /* Token information */
  lexer *Lexer;

  token_type CurToken;
  char *CurString;
  long CurInteger;

  token_type PeekToken;
} parser;

void ParserInit(parser *Parser, lexer *Lexer);
ast_program *ParseProgram(parser *Parser);
void ProgramDelete(ast_program *Program); // TODO: this function was never implemented
