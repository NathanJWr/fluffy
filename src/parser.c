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

static unsigned int PrecedenceTable[TOKEN_ENUM_COUNT];
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
ast_base *parsePrefixExpression(parser *Parser);
ast_base *parseIdentifier(parser *Parser);
ast_base *parseIntegerLiteral(parser *Parser);
ast_base *parseBoolean(parser *Parser);
ast_base *parseGroupedExpression(parser *Parser);
ast_base *parseIfExpression(parser *Parser);

/* Infix parsing function */
typedef ast_base *(*InfixParseFunction)(parser *, ast_base *left);
InfixParseFunction findInfixParseFunction(token_type Token);
ast_base *parseInfixExpression(parser *Parser, ast_base *left);

/* Helper parsing function */
ast_base *parseBlockStatement(parser *Parser);

void debugPrintAstNode(ast_base *Node);

void ParserInit(parser *Parser, lexer *Lexer) {
  /* Get the first two tokens so CurToken and PeekToken can be populated */
  Parser->Lexer = Lexer;
  nextToken(Parser);
  nextToken(Parser);

  /* Initialize the precedence table */
  memset(PrecedenceTable, 0, sizeof(PrecedenceTable));
  PrecedenceTable[TOKEN_EQ] = PRECEDENCE_EQUALS;
  PrecedenceTable[TOKEN_NOT_EQ] = PRECEDENCE_EQUALS;
  PrecedenceTable[TOKEN_PLUS] = PRECEDENCE_SUM;
  PrecedenceTable[TOKEN_MINUS] = PRECEDENCE_SUM;
  PrecedenceTable[TOKEN_LT] = PRECEDENCE_LESSGREATER;
  PrecedenceTable[TOKEN_GT] = PRECEDENCE_LESSGREATER;
  PrecedenceTable[TOKEN_SLASH] = PRECEDENCE_PRODUCT;
  PrecedenceTable[TOKEN_ASTERISK] = PRECEDENCE_PRODUCT;
  PrecedenceTable[TOKEN_LPAREN] = PRECEDENCE_CALL;
}

ast_program *ParseProgram(parser *Parser) {
  /* initialize the AstNodes to store nodes */
  ast_program *ProgramNode =
      (ast_program *)createNode(Parser, sizeof(ast_program), AST_PROGRAM);

  ProgramNode->Statements = NULL;
  while (Parser->CurToken != TOKEN_END) {
    ast_base *Stmt = parseStatement(Parser);
    ArrayPush(ProgramNode->Statements, Stmt);
    nextToken(Parser);
  }
  debugPrintAstNode((ast_base *)ProgramNode);
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
    return NULL;
  }
  LeftExpression = PrefixFn(Parser);

  while (Parser->PeekToken != TOKEN_SEMICOLON &&
         Precedence < PrecedenceTable[Parser->PeekToken]) {
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
  case TOKEN_INT:
    return parseIntegerLiteral;
  case TOKEN_BANG:
  case TOKEN_MINUS:
    return parsePrefixExpression;
  case TOKEN_TRUE:
  case TOKEN_FALSE:
    return parseBoolean;
  case TOKEN_LPAREN:
    return parseGroupedExpression;
  case TOKEN_IF:
    return parseIfExpression;
  default:
    printf("no prefix parse function for (%s) found\n", TokenType[Token]);
    return NULL;
  }
}

InfixParseFunction findInfixParseFunction(token_type Token) {
  switch (Token) {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_SLASH:
  case TOKEN_ASTERISK:
  case TOKEN_EQ:
  case TOKEN_NOT_EQ:
  case TOKEN_LT:
  case TOKEN_GT:
    return parseInfixExpression;

  default:
    printf("no prefix parse function for (%s) found\n", TokenType[Token]);
    return NULL;
  }
}

ast_base *parseInfixExpression(parser *Parser, ast_base *Left) {
  ast_base *Node =
      createNode(Parser, sizeof(ast_infix_expression), AST_INFIX_EXPRESSION);
  ast_infix_expression Infix;
  unsigned int Precedence = PrecedenceTable[Parser->CurToken];

  memcpy(&Infix, Node, sizeof(ast_infix_expression));

  Infix.Operation = Parser->CurToken;
  Infix.Left = Left;
  nextToken(Parser);
  Infix.Right = parseExpression(Parser, Precedence);

  memcpy(Node, &Infix, sizeof(ast_infix_expression));
  return Node;
}

ast_base *parsePrefixExpression(parser *Parser) {
  ast_prefix_expression *Prefix = (ast_prefix_expression *)createNode(
      Parser, sizeof(ast_prefix_expression), AST_PREFIX_EXPRESSION);
  Prefix->Operation = Parser->CurToken;
  nextToken(Parser);
  Prefix->Right = parseExpression(Parser, PRECEDENCE_PREFIX);
  return (ast_base *)Prefix;
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

ast_base *parseIntegerLiteral(parser *Parser) {
  ast_integer_literal *Integer = (ast_integer_literal *)createNode(
      Parser, sizeof(ast_integer_literal), AST_INTEGER_LITERAL);

  Integer->Integer = Parser->CurInteger;
  return (ast_base *)Integer;
}

ast_base *parseBoolean(parser *Parser) {
  ast_boolean *Boolean =
      (ast_boolean *)createNode(Parser, sizeof(ast_boolean), AST_BOOLEAN);

  Boolean->Value = Parser->CurToken == TOKEN_TRUE;

  return (ast_base *)Boolean;
}

ast_base *parseGroupedExpression(parser *Parser) {
  ast_base *Expr;
  nextToken(Parser);

  Expr = parseExpression(Parser, PRECEDENCE_LOWEST);
  if (Parser->PeekToken != TOKEN_RPAREN) {
    return NULL;
  }
  nextToken(Parser);

  return Expr;
}

ast_base *parseIfExpression(parser *Parser) {
  ast_if_expression *IfExpr = (ast_if_expression *)createNode(
      Parser, sizeof(ast_if_expression), AST_IF_EXPRESSION);

  IfExpr->Alternative = NULL;
  if (Parser->PeekToken != TOKEN_LPAREN) {
    return NULL;
  }
  nextToken(Parser);
  nextToken(Parser);

  IfExpr->Condition = parseExpression(Parser, PRECEDENCE_LOWEST);

  if (Parser->PeekToken != TOKEN_RPAREN) {
    return NULL;
  }
  nextToken(Parser);

  if (Parser->PeekToken != TOKEN_LBRACE) {
    return NULL;
  }
  nextToken(Parser);

  IfExpr->Consequence = parseBlockStatement(Parser);

  if (Parser->PeekToken == TOKEN_ELSE) {
    nextToken(Parser);

    if (Parser->PeekToken != TOKEN_LBRACE) {
      return NULL;
    }
    nextToken(Parser);

    IfExpr->Alternative = parseBlockStatement(Parser);
  }

  return (ast_base *)IfExpr;
}

ast_base *parseBlockStatement(parser *Parser) {
  ast_block_statement *Block = (ast_block_statement *)createNode(
      Parser, sizeof(ast_block_statement), AST_BLOCK_STATEMENT);
  Block->Statements = NULL;
  nextToken(Parser);

  while (Parser->CurToken != TOKEN_RBRACE && Parser->CurToken != TOKEN_END) {
    ast_base *Stmt = parseStatement(Parser);
    if (Stmt) {
      ArrayPush(Block->Statements, Stmt);
    }
    nextToken(Parser);
  }

  return (ast_base *)Block;
}

void debugPrintAstNode(ast_base *Node) {
  switch (Node->Type) {
  case AST_IDENTIFIER: {
    ast_identifier Ident;
    memcpy(&Ident, Node, sizeof(ast_identifier));
    printf("%s", Ident.Value);
  } break;
  case AST_PROGRAM: {
    ast_program Program;
    unsigned int StatementsSize;
    unsigned int i;
    memcpy(&Program, Node, sizeof(ast_program));

    StatementsSize = ArraySize(Program.Statements);
    for (i = 0; i < StatementsSize; i++) {
      debugPrintAstNode(Program.Statements[i]);
    }
    printf("\n");
  } break;
  case AST_INFIX_EXPRESSION: {
    ast_infix_expression *Infix = (ast_infix_expression *)Node;
    printf("(");
    debugPrintAstNode(Infix->Left);

    switch (Infix->Operation) {
    case TOKEN_PLUS:
      printf("+");
      break;
    case TOKEN_MINUS:
      printf("-");
      break;
    case TOKEN_SLASH:
      printf("/");
      break;
    case TOKEN_ASTERISK:
      printf("*");
      break;
    case TOKEN_GT:
      printf(">");
      break;
    case TOKEN_LT:
      printf("<");
      break;
    case TOKEN_EQ:
      printf("==");
      break;
    case TOKEN_NOT_EQ:
      printf("!=");
      break;
    default:
      break;
    }

    debugPrintAstNode(Infix->Right);
    printf(")");
  } break;
  case AST_INTEGER_LITERAL: {
    printf("%ld", ((ast_integer_literal *)Node)->Integer);
  } break;
  case AST_PREFIX_EXPRESSION: {
    ast_prefix_expression *Prefix = (ast_prefix_expression *)Node;
    switch (Prefix->Operation) {
    case TOKEN_BANG:
      printf("!");
      break;
    case TOKEN_MINUS:
      printf("-");
      break;
    default:
      break;
    }
    debugPrintAstNode(Prefix->Right);
  } break;
  case AST_BOOLEAN: {
    ast_boolean *Boolean = (ast_boolean *)Node;
    (Boolean->Value) ? printf("true") : printf("false");
  } break;
  case AST_IF_EXPRESSION: {
    ast_if_expression *Expr = (ast_if_expression *) Node;
    printf("if(");
    debugPrintAstNode(Expr->Condition);
    printf(") {");
    debugPrintAstNode(Expr->Consequence);
    printf("}");

    if(Expr->Alternative) {
      printf("else {");
      debugPrintAstNode(Expr->Consequence);
      printf("}");
    }
  } break;
  case AST_BLOCK_STATEMENT: {
    ast_block_statement *Block = (ast_block_statement *) Node;
    unsigned int i;
    for (i = 0; i < ArraySize(Block->Statements); i++) {
      debugPrintAstNode(Block->Statements[i]);
    }
  } break;
  default:
    printf("\n");
  }
}

ast_base *createNode(parser *P, unsigned int Size, ast_type Type) {
  ast_base *Ret = malloc(Size);
  Ret->Size = Size;
  Ret->Type = Type;
  return Ret;
}
