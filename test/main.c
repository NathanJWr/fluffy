
int main() {
    printf("Executing tests.\n");
    testLexer("<hello> ", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("<hello >", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("< hello>", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer(" <hello>", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("{1.99}", 6, TOKEN_LBRACE, TOKEN_INT, 1, TOKEN_ILLEGAL, TOKEN_INT, 99, TOKEN_RBRACE, TOKEN_END);
    return 0;
}