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
void initializeNodeArray(parser *P, size_t Size);

/* will insert the node into a flat array
 * NOTE(Nathan): This operation can resize the array and invalidate pointers
 * to any node contained within it */
size_t insertIntoArrayOfNodes(parser *Parser, ast_base *Node);

/* Returns a pointer to a particular node based in the index.
 * This is most likely pretty slow because of the variable sized nodes
 * and having to increment into the array based on those variable sizes */
ast_base *getAstNodeFromArray(uint8_t *Array, size_t Index);

/* basic parse functions */
void nextToken(parser *Parser);
void parseStatement(parser *Parser);
void parseExpressionStatement(parser *Parser);
void parseExpression(parser *Parser, unsigned int Precedence);

/* Prefix parsing function */
typedef size_t (*PrefixParseFunction)(parser *);
PrefixParseFunction findPrefixParseFunction(token_type Token);
size_t parseIdentifier(parser *Parser);

void debugPrintAstNodes(parser *Parser);
void debugPrintAstNode(ast_base *Node);

void ParserInit(parser *Parser, lexer *Lexer) {
  initializeNodeArray(Parser, 8);
  Parser->AstNodesIndex = 0;
  Parser->Lexer = Lexer;

  /* Get the first two tokens so CurToken and PeekToken can be populated */
  nextToken(Parser);
  nextToken(Parser);
}

void ParserDelete(parser *Parser) { free(Parser->AstNodes); }

ast_program ParseProgram(parser *Parser) {
  /* initialize the AstNodes to store nodes */
  ast_program Program;
  size_t ProgramIndex;
  size_t ProgramStatementCount = 0;

  Program.Base.Size = sizeof(ast_program);
  Program.Base.Type = AST_PROGRAM;
  ProgramIndex = insertIntoArrayOfNodes(Parser, (ast_base *)&Program);

  while (Parser->CurToken != TOKEN_END) {
    parseStatement(Parser);
    nextToken(Parser);
    ProgramStatementCount++;
  }

  /* make sure to set the program statement count in memory */
  ((ast_program *)getAstNodeFromArray(Parser->AstNodes, ProgramIndex))
      ->StatementsLength = ProgramStatementCount;

  debugPrintAstNodes(Parser);
}

void initializeNodeArray(parser *P, size_t Size) {
  P->AstNodes = malloc(Size);
  P->AstNodesPos = P->AstNodes;
  P->AstNodesEnd = P->AstNodes + Size;
  P->AstNodesSize = Size;
}

size_t insertIntoArrayOfNodes(parser *Parser, ast_base *Node) {
  /* resize the array if needed */
  if (Parser->AstNodesEnd < Parser->AstNodesPos + Node->Size) {
    size_t NewSize = Parser->AstNodesSize * 2;
    uint8_t *A = realloc(Parser->AstNodes, NewSize);
    if (A) {
      /* change out all the pointers that are tracking positions */
      Parser->AstNodes = A;
      Parser->AstNodesPos = A + Parser->AstNodesSize;
      Parser->AstNodesEnd = A + NewSize;

      /* actually change the ast nodes size */
      Parser->AstNodesSize = NewSize;
    }
  }

  /* insert Node into the array */
  memcpy(Parser->AstNodesPos, Node, Node->Size);
  Parser->AstNodesPos += Node->Size;
  return Parser->AstNodesIndex++;
}

ast_base *getAstNodeFromArray(uint8_t *Array, size_t Index) {
  unsigned int I = 0;
  while (I != Index) {
    size_t Size = ((ast_base *)Array)->Size;
    Array += Size;
    I++;
  }
  return (ast_base *)Array;
}

void parseStatement(parser *Parser) {
  switch (Parser->CurToken) {
  default:
    parseExpressionStatement(Parser);
  }
}

void parseExpressionStatement(parser *Parser) {
  parseExpression(Parser, PRECEDENCE_LOWEST);
  if (Parser->PeekToken == TOKEN_SEMICOLON) {
    nextToken(Parser);
  }
}

void parseExpression(parser *Parser, unsigned int Precedence) {
  PrefixParseFunction PrefixFn = findPrefixParseFunction(Parser->CurToken);
  PrefixFn(Parser);
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

size_t parseIdentifier(parser *Parser) {
  ast_identifier Ident;
  Ident.Base.Type = AST_IDENTIFIER;
  Ident.Base.Size = sizeof(ast_identifier);
  Ident.Value = Parser->CurString;
  return insertIntoArrayOfNodes(Parser, (ast_base *)&Ident);
}

void debugPrintAstNodes(parser *Parser) {
  size_t Index = 0;
  uint8_t *Node = Parser->AstNodes;
  while (Index < Parser->AstNodesIndex) {
    debugPrintAstNode((ast_base *)Node);
    Node += ((ast_base *)Node)->Size;
    Index++;
  }
}

void debugPrintAstNode(ast_base *Node) {
  switch (Node->Type) {
  case AST_IDENTIFIER: {
    printf("AST_IDENTIFIER: %s\n", ((ast_identifier *)Node)->Value);
  } break;
  case AST_PROGRAM: {
    printf("AST_FUNCTION: %d statements\n",
           ((ast_program *)Node)->StatementsLength);
  } break;
  }
}
