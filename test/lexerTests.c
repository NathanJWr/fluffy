#ifdef DEBUG
#define DEBUGLOG printf
#else
#define DEBUGLOG doNothing
#endif

void doNothing(const char* format, ...) {}

/**
 * If we expect an identifier, then also assert the Lexer's output is equal to the string input.
 */
void testLexer(const char *str, int numTypes, ...) {
    lexer Lexer;
    unsigned int StringStoreSize = 0x10000;
    char *StringStore = malloc(StringStoreSize);
    token_type type, expectedType;
    int i;
    char* expectedString;
    long expectedInt;
    va_list expectedTypes;
    va_start(expectedTypes, numTypes);

    LexerInit(&Lexer, &str[0], &str[strlen(str)], StringStore, StringStoreSize);

    DEBUGLOG("\n%s\n\n", str);
    for(i = 0; i < numTypes; i++) {
        expectedType = va_arg(expectedTypes, token_type);
        type = NextToken(&Lexer);
        DEBUGLOG("  [%d]: %s ? %s\n", i+1, TokenType[expectedType], TokenType[type]);
        assert(type == expectedType);
        switch (expectedType) {
        case TOKEN_IDENT:
            expectedString = va_arg(expectedTypes, char*);
            DEBUGLOG("   | %s ? %s\n", expectedString, Lexer.String);
            assert(0 == strcmp(Lexer.String, expectedString));
            break;
        case TOKEN_INT:
            expectedInt = va_arg(expectedTypes, long);
            DEBUGLOG("   | %ld ? %ld\n", expectedInt, Lexer.Integer);
            assert(Lexer.Integer == expectedInt);
            break;
        default:
            break;
        }
    }
    va_end(expectedTypes);
}