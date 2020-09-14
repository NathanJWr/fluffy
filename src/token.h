#define TOKEN_TYPE_LIST                                                        \
  X(ILLEGAL)                                                                   \
  X(END)                                                                       \
                                                                               \
  /* Identifiers + Literals */                                                 \
  X(IDENT)                                                                     \
  X(INT)                                                                       \
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
  X(RETURN)

#define X(name) TOKEN_##name,
typedef enum { TOKEN_TYPE_LIST } token_type;
#undef X

#define X(name) #name,
const char *TokenType[] = { TOKEN_TYPE_LIST };
