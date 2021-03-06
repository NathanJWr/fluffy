/**
 * Use the "-d" flag to run in debug mode. (Print more details.)
 */
int main(int argc, char **argv) {
  isDebug = (argc > 1 && 0 == strcmp("-d", argv[1]));
  printf("Executing tests.\n");
  /* LEXER TESTING */
  testLexer("<hello> ", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
  testLexer("<hello >", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
  testLexer("< hello>", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
  testLexer(" <hello>", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
  testLexer("{1$99}", TOKEN_LBRACE, TOKEN_INT, 1, TOKEN_ILLEGAL, TOKEN_INT, 99,
            TOKEN_RBRACE, TOKEN_END);
  testLexer("{1.99}", TOKEN_LBRACE, TOKEN_DOUBLE, 1.99, TOKEN_RBRACE,
            TOKEN_END);
  testLexer("fn hello(n = 10) { return n; }", TOKEN_FUNCTION, TOKEN_IDENT,
            "hello", TOKEN_LPAREN, TOKEN_IDENT, "n", TOKEN_ASSIGN, TOKEN_INT,
            10, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RETURN, TOKEN_IDENT, "n",
            TOKEN_SEMICOLON, TOKEN_RBRACE, TOKEN_END);
  testLexer("if(!!n != 10) { var thing = true; } else { var thing = false; }",
            TOKEN_IF, TOKEN_LPAREN, TOKEN_BANG, TOKEN_BANG, TOKEN_IDENT, "n",
            TOKEN_NOT_EQ, TOKEN_INT, 10, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_VAR,
            TOKEN_IDENT, "thing", TOKEN_ASSIGN, TOKEN_TRUE, TOKEN_SEMICOLON,
            TOKEN_RBRACE, TOKEN_ELSE, TOKEN_LBRACE, TOKEN_VAR, TOKEN_IDENT,
            "thing", TOKEN_ASSIGN, TOKEN_FALSE, TOKEN_SEMICOLON, TOKEN_RBRACE,
            TOKEN_END);
  testLexer("str == \"string\"", TOKEN_IDENT, "str", TOKEN_EQ, TOKEN_STRING,
            "string", TOKEN_END);
  testLexer("testLexer(const char *str, ...)", TOKEN_IDENT, "testLexer",
            TOKEN_LPAREN, TOKEN_IDENT, "const", TOKEN_IDENT, "char",
            TOKEN_ASTERISK, TOKEN_IDENT, "str", TOKEN_COMMA, TOKEN_DOT,
            TOKEN_DOT, TOKEN_DOT, TOKEN_RPAREN, TOKEN_END);
  /* PARSER TESTING */
  testParser("number is 43", AST_PROGRAM, AST_IDENTIFIER, "number",
             AST_IDENTIFIER, "is", AST_NUMBER, 43);
  testParser("-20", AST_PROGRAM, AST_PREFIX_EXPRESSION, TOKEN_MINUS, AST_NUMBER,
             20);
  testParser("var a = (!10 + 2.2) / 0;", AST_PROGRAM, AST_VAR_STATEMENT,
             AST_IDENTIFIER, "a", AST_INFIX_EXPRESSION, TOKEN_SLASH,
             AST_INFIX_EXPRESSION, TOKEN_PLUS, AST_PREFIX_EXPRESSION,
             TOKEN_BANG, AST_NUMBER, 10, AST_NUMBER, 2.2, AST_NUMBER, 0);
  testParser("if(true) { 1; } else { 0; }", AST_PROGRAM, AST_IF_EXPRESSION,
             AST_BOOLEAN, true, AST_BLOCK_STATEMENT, AST_NUMBER, 1,
             AST_BLOCK_STATEMENT, AST_NUMBER, 0);
  testParser("var a = fn(b) { return b; }", AST_PROGRAM, AST_VAR_STATEMENT,
             AST_IDENTIFIER, "a", AST_FUNCTION_LITERAL, AST_IDENTIFIER, "b",
             AST_BLOCK_STATEMENT, AST_RETURN_STATEMENT, AST_IDENTIFIER, "b");
  testParser("var b = a(\"hello\");", AST_PROGRAM, AST_VAR_STATEMENT,
             AST_IDENTIFIER, "b", AST_FUNCTION_CALL, AST_IDENTIFIER, "a",
             AST_STRING, "hello");
  printf("All tests passed.\n");
  return 0;
}