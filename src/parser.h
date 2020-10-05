struct lexer;
typedef struct lexer lexer;

struct ast_program;
typedef struct ast_program ast_program;
typedef struct {
  /* Token information */
  lexer *Lexer;

  fluff_token_type CurToken;
  char *CurString;
  long CurInteger;
  double CurDouble;

  fluff_token_type PeekToken;
} parser;

void ParserInit(parser *Parser, lexer *Lexer);
ast_program *ParseProgram(parser *Parser);
void ProgramDelete(
    ast_program *Program); // TODO: this function was never implemented
