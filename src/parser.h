typedef struct {
  /* Flat array that stores ast nodes of variable type.
  * To determine the size of the actual node you're looking at,
  * examine the Size variable in ast_base to know how much you should
  * advance into the array */
  uint8_t *AstNodes;
  uint8_t *AstNodesPos;
  uint8_t *AstNodesEnd;
  size_t AstNodesSize;
  size_t AstNodesIndex;

  /* Token information */
  lexer *Lexer;

  token_type CurToken;
  char *CurString;
  long CurInteger;

  token_type PeekToken;
} parser;

void ParserInit(parser *Parser, lexer *Lexer);
ast_program ParseProgram(parser *Parser);
