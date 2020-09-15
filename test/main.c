/**
 * Use the "-d" flag to run in debug mode. (Print more details.)
 */
int main(int argc, char** argv) {
    isDebug = (argc > 1 && 0 == strcmp("-d", argv[1]));
    printf("Executing tests.\n");
    testLexer("<hello> ", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("<hello >", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("< hello>", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer(" <hello>", TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("{1.99}", TOKEN_LBRACE, TOKEN_INT, 1, TOKEN_ILLEGAL, TOKEN_INT, 99, TOKEN_RBRACE, TOKEN_END);
    testLexer("fn hello(n = 10) { return n; }", TOKEN_FUNCTION, TOKEN_IDENT, "hello", TOKEN_LPAREN,
        TOKEN_IDENT, "n", TOKEN_ASSIGN, TOKEN_INT, 10, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RETURN, TOKEN_IDENT, "n",
        TOKEN_SEMICOLON, TOKEN_RBRACE, TOKEN_END);
    testLexer("if(!!n != 10) { var thing = true; } else { var thing = false; }",
        TOKEN_IF, TOKEN_LPAREN, TOKEN_BANG, TOKEN_BANG, TOKEN_IDENT, "n", TOKEN_NOT_EQ, TOKEN_INT, 10, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_VAR, TOKEN_IDENT, "thing", TOKEN_ASSIGN, TOKEN_TRUE, TOKEN_SEMICOLON, TOKEN_RBRACE, TOKEN_ELSE,
        TOKEN_LBRACE, TOKEN_VAR, TOKEN_IDENT, "thing", TOKEN_ASSIGN, TOKEN_FALSE, TOKEN_SEMICOLON, TOKEN_RBRACE, TOKEN_END);
    testLexer("str == \"string\"", TOKEN_IDENT, "str", TOKEN_EQ, TOKEN_STRING, "string", TOKEN_END);
    printf("All tests passed.\n");
    return 0;
}