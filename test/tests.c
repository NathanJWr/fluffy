
void testIdentifier() {
    lexer Lexer;
    unsigned int StringStoreSize = 0x10000;
    char *StringStore = malloc(StringStoreSize);
    token_type type;

    const char str[] = "test";
    int length = sizeof str;

    LexerInit(&Lexer, &str[0], &str[length], StringStore, StringStoreSize);
    type = NextToken(&Lexer);
    assert(type == TOKEN_PLUS);
    assert(0 == strcmp(Lexer.String, str));
}