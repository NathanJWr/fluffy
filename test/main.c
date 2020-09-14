
int main() {
    printf("Executing tests.\n");
    testLexer("<hello >", 3, TOKEN_LT, TOKEN_IDENT, "hello", TOKEN_GT);
    return 0;
}