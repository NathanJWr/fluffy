#include <stdarg.h>
#include <assert.h>

/**
 * If we expect an identifier, then also assert the Lexer's output is equal to the string input.
 */
void testLexer(const char *str, int numTypes, ...) {
    lexer Lexer;
    unsigned int StringStoreSize = 0x10000;
    char *StringStore = malloc(StringStoreSize);
    token_type type, expectedType;
    int i;
    va_list expectedTypes;
    va_start(expectedTypes, numTypes);

    LexerInit(&Lexer, &str[0], &str[strlen(str)], StringStore, StringStoreSize);

    printf("%s\n", str);
    for(i = 0; i < numTypes; i++) {
        expectedType = va_arg(expectedTypes, token_type);
        type = NextToken(&Lexer);
        printf("  [%d]: %s ? %s\n", i+1, TokenType[expectedType], TokenType[type]);
        assert(type == expectedType);
        if(expectedType == TOKEN_IDENT) {
            assert(0 == strcmp(Lexer.String, va_arg(expectedTypes, char*)));
            printf("   | %s\n", Lexer.String);
        }
    }
    va_end(expectedTypes);
}