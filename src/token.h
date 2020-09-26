#define TOKEN_TYPE_LIST                                                        \
  X(ILLEGAL)                                                                   \
  X(END)                                                                       \
                                                                               \
  /* Identifiers + Literals */                                                 \
  X(IDENT)                                                                     \
  X(INT)                                                                       \
  X(DOUBLE)                                                                    \
  X(STRING)                                                                    \
                                                                               \
  /* Operators */                                                              \
  X(ASSIGN)                                                                    \
  X(PLUS)                                                                      \
  X(MINUS)                                                                     \
  X(BANG)                                                                      \
  X(ASTERISK)                                                                  \
  X(SLASH)                                                                     \
  X(EQ)                                                                        \
  X(NOT_EQ)                                                                    \
                                                                               \
  X(LT)                                                                        \
  X(GT)                                                                        \
                                                                               \
  /* Delimiters */                                                             \
  X(DOT)                                                                       \
  X(COMMA)                                                                     \
  X(SEMICOLON)                                                                 \
                                                                               \
  X(LPAREN)                                                                    \
  X(RPAREN)                                                                    \
  X(LBRACE)                                                                    \
  X(RBRACE)                                                                    \
                                                                               \
  /* Keywords */                                                               \
  X(FUNCTION)                                                                  \
  X(VAR)                                                                       \
  X(TRUE)                                                                      \
  X(FALSE)                                                                     \
  X(IF)                                                                        \
  X(ELSE)                                                                      \
  X(RETURN)                                                                    \
  XX(ENUM_COUNT)

#define X(name) TOKEN_##name,
#define XX(name) TOKEN_##name
typedef enum { TOKEN_TYPE_LIST } token_type;
#undef X
#undef XX

#define X(name) #name,
#define XX(name) #name
const char *TokenType[] = {TOKEN_TYPE_LIST};
#undef X
#undef XX
