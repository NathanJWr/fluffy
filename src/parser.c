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
unsigned int getPrecedence(token_type Token);

/* allocating functions */
void initializeNodeArray(parser *P, size_t Size);
ast_base *createNode(parser *P, unsigned int Size, ast_type Type);

/* basic parse functions */
void nextToken(parser *Parser);
ast_base *parseStatement(parser *Parser);
ast_base *parseExpressionStatement(parser *Parser);
ast_base *parseExpression(parser *Parser, unsigned int Precedence);

/* Prefix parsing function */
typedef ast_base *(*PrefixParseFunction)(parser *);
PrefixParseFunction findPrefixParseFunction(token_type Token);
ast_base *parseIdentifier(parser *Parser);

/* Infix parsing function */
typedef ast_base *(*InfixParseFunction)(parser *, ast_base *left);
InfixParseFunction findInfixParseFunction(token_type Token);
ast_base *parseInfixExpression(parser *Parser, ast_base *left);

void debugPrintAstNode(ast_base *Node);

void ParserInit(parser *Parser, lexer *Lexer) {
  /* Get the first two tokens so CurToken and PeekToken can be populated */
  Parser->Lexer = Lexer;
  nextToken(Parser);
  nextToken(Parser);
}

ast_program *ParseProgram(parser *Parser) {
  /* initialize the AstNodes to store nodes */
  ast_program *ProgramNode =
      (ast_program *)createNode(Parser, sizeof(ast_program), AST_PROGRAM);

  ProgramNode->Statements = NULL;
  while (Parser->CurToken != TOKEN_END) {
    ast_base *Stmt = parseStatement(Parser);
    ArrayPush(ProgramNode->Statements, Stmt);
    debugPrintAstNode(Stmt);
    nextToken(Parser);
  }
  return ProgramNode;
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
  ast_base *LeftExpression;
  PrefixParseFunction PrefixFn = findPrefixParseFunction(Parser->CurToken);
  if (PrefixFn == NULL) {
    printf("no prefix parse function for (%s) found\n",
           TokenType[Parser->CurToken]);
    return NULL;
  }
  LeftExpression = PrefixFn(Parser);

  while (Parser->PeekToken != TOKEN_SEMICOLON &&
         Precedence < getPrecedence(Parser->PeekToken)) {
    InfixParseFunction InfixFn = findInfixParseFunction(Parser->PeekToken);
    if (InfixFn == NULL) {
      return LeftExpression;
    }

    nextToken(Parser);
    LeftExpression = InfixFn(Parser, LeftExpression);
  }
  return LeftExpression;
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

InfixParseFunction findInfixParseFunction(token_type Token) {
  switch (Token) {
  case TOKEN_PLUS:
    return parseInfixExpression;
  default:
    return NULL;
  }
}

ast_base *parseInfixExpression(parser *Parser, ast_base *Left) {
  ast_base *Node =
      createNode(Parser, sizeof(ast_infix_expression), AST_INFIX_EXPRESSION);
  ast_infix_expression Infix;
  unsigned int Precedence = getPrecedence(Parser->CurToken);

  memcpy(&Infix, Node, sizeof(ast_infix_expression));

  Infix.Left = Left;
  nextToken(Parser);
  Infix.Right = parseExpression(Parser, Precedence);

  memcpy(Node, &Infix, sizeof(ast_infix_expression));
  return Node;
}

ast_base *parseIdentifier(parser *Parser) {
  ast_base *Node = createNode(Parser, sizeof(ast_identifier), AST_IDENTIFIER);
  ast_identifier Ident;

  /* Set the identifier string value */
  memcpy(&Ident, Node, sizeof(ast_identifier));
  Ident.Value = Parser->CurString;
  memcpy(Node, &Ident, sizeof(ast_identifier));

  return Node;
}

void debugPrintAstNode(ast_base *Node) {
  switch (Node->Type) {
  case AST_IDENTIFIER: {
    printf("AST_IDENTIFIER: %s\n", ((ast_identifier *)Node)->Value);
  } break;
  case AST_PROGRAM: {
    printf("AST_FUNCTION\n");
  } break;
  case AST_INFIX_EXPRESSION: {
    printf("AST_INFIX_EXPRESSION\n");
  }
  }
}

ast_base *createNode(parser *P, unsigned int Size, ast_type Type) {
  ast_base *Ret = malloc(Size);
  Ret->Size = Size;
  Ret->Type = Type;
  return Ret;
}

unsigned int getPrecedence(token_type Token) {
  switch (Token) {
  case TOKEN_EQ:
  case TOKEN_NOT_EQ:
    return PRECEDENCE_EQUALS;
  case TOKEN_LT:
  case TOKEN_GT:
    return PRECEDENCE_LESSGREATER;
  case TOKEN_PLUS:
  case TOKEN_MINUS:
    return PRECEDENCE_SUM;
  case TOKEN_SLASH:
  case TOKEN_ASTERISK:
    return PRECEDENCE_PRODUCT;
  case TOKEN_LPAREN:
    return PRECEDENCE_CALL;
  default:
    return 0;
  }
}
