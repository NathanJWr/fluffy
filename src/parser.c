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
ast_base *parsePrefixExpression(parser *Parser);
ast_base *parseIdentifier(parser *Parser);
ast_base *parseIntegerLiteral(parser *Parser);
ast_base *parseBoolean(parser *Parser);

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
  case TOKEN_INT:
    return parseIntegerLiteral;
  case TOKEN_BANG:
  case TOKEN_MINUS:
    return parsePrefixExpression;
  case TOKEN_TRUE:
  case TOKEN_FALSE:
    return parseBoolean;
  default:
    printf("no prefix parse function for (%s) found\n", TokenType[Token]);
    return NULL;
  }
}

InfixParseFunction findInfixParseFunction(token_type Token) {
  switch (Token) {
  case TOKEN_PLUS:
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
  unsigned int Precedence = getPrecedence(Parser->CurToken);

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
