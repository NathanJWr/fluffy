
int main() {
    printf("Executing tests.\n");
    testLexer("<hello> ", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("<hello >", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer("< hello>", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    testLexer(" <hello>", 4, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT, TOKEN_END);
    return 0;
}