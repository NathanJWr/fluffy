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

void ParserInit(parser *Parser) {
  initializeNodeArray(Parser, 8);
  Parser->AstNodesIndex = 0;
}

ast_program ParseProgram(parser *Parser, lexer *Lexer) {
  /* initialize the AstNodes to store nodes */
  ast_program Program;
  size_t ProgramIndex;

  Program.Base.Size = sizeof(ast_program);
  Program.Base.Type = AST_PROGRAM;
  ProgramIndex = insertIntoArrayOfNodes(Parser, (ast_base *) &Program);
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
    size_t Size = ((ast_base *) Array)->Size;
    Array += Size;
    I++;
  }
  return (ast_base *) Array;
}
