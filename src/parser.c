/* precedence table where the larger the number
 * the more "precedence" it will have
 * e.g. a function call has more precedence than
 * a prefix operator */
enum parser_precedence {
  PRECEDENCE_LOWEST,
  PRECEDENCE_EQUALS,
  PRECEDENCE_LESSGREATER,
  PRECEDENCE_SUM,
  PRECEDENCE_PRODUCT,
  PRECEDENCE_PREFIX,
  PRECEDENCE_CALL
};
/* allocating functions */
void initializeNodeArray(parser *P, size_t Size);
ast_base *createNode(parser *P, unsigned int Size, token_type Type);

/* basic parse functions */
void nextToken(parser *Parser);
ast_base *parseStatement(parser *Parser);
ast_base *parseExpressionStatement(parser *Parser);
ast_base *parseExpression(parser *Parser, unsigned int Precedence);

/* Prefix parsing function */
typedef ast_base *(*PrefixParseFunction)(parser *);
PrefixParseFunction findPrefixParseFunction(token_type Token);

ast_base *parseIdentifier(parser *Parser);

void debugPrintAstNode(ast_base *restrict Node);

void ParserInit(parser *Parser, lexer *Lexer) {
  /* Get the first two tokens so CurToken and PeekToken can be populated */
  Parser->Lexer = Lexer;
  nextToken(Parser);
  nextToken(Parser);
}

void ParserDelete(parser *Parser) {}

ast_program ParseProgram(parser *Parser) {
  /* initialize the AstNodes to store nodes */
  ast_base *ProgramNode = createNode(Parser, sizeof(ast_program), AST_PROGRAM);
  ast_program Program;
  memcpy(&Program, ProgramNode, sizeof(ast_program));

  while (Parser->CurToken != TOKEN_END) {
    ast_base *Stmt = parseStatement(Parser);
    debugPrintAstNode(Stmt);
    nextToken(Parser);
  }

  memcpy(ProgramNode, &Program, sizeof(ast_program));
}

ast_base *parseStatement(parser *Parser) {
  switch (Parser->CurToken) {
  default:
    return parseExpressionStatement(Parser);
  }
}

ast_base *parseExpressionStatement(parser *Parser) {
  ast_base *Expr = parseExpression(Parser, PRECEDENCE_LOWEST);
  if (Parser->PeekToken == TOKEN_SEMICOLON) {
    nextToken(Parser);
  }
  return Expr;
}

ast_base *parseExpression(parser *Parser, unsigned int Precedence) {
  PrefixParseFunction PrefixFn = findPrefixParseFunction(Parser->CurToken);
  return PrefixFn(Parser);
}

void nextToken(parser *Parser) {
  Parser->CurToken = Parser->PeekToken;
  Parser->CurString = Parser->Lexer->String;
  Parser->CurInteger = Parser->Lexer->Integer;

  Parser->PeekToken = NextToken(Parser->Lexer);
}

PrefixParseFunction findPrefixParseFunction(token_type Token) {
  switch (Token) {
  case TOKEN_IDENT:
    return parseIdentifier;
  default:
    return NULL;
  }
}

ast_base *parseIdentifier(parser *restrict Parser) {
  ast_base *Node = createNode(Parser, sizeof(ast_identifier), AST_IDENTIFIER);
  ast_identifier Ident;

  /* Set the identifier string value */
  memcpy(&Ident, Node, sizeof(ast_identifier));
  Ident.Value = Parser->CurString;
  memcpy(Node, &Ident, sizeof(ast_identifier));

  return Node;
}

void debugPrintAstNode(ast_base *restrict Node) {
  switch (Node->Type) {
  case AST_IDENTIFIER: {
    ast_identifier Ident;
    memcpy(&Ident, Node, sizeof(ast_identifier));
    printf("AST_IDENTIFIER: %s\n", ((ast_identifier *)Node)->Value);
  } break;
  case AST_PROGRAM: {
    ast_program Program;
    memcpy(&Program, Node, sizeof(ast_program));
    printf("AST_FUNCTION\n");
  } break;
  }
}

ast_base *createNode(parser *P, unsigned int Size, token_type Type) {
  ast_base *Ret = malloc(Size);
  Ret->Size = Size;
  Ret->Type = Type;
  return Ret;
}
