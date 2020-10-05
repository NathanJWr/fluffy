#define FLUFF_fluff_token_type_LIST                                            \
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
  X(LSQUARE)                                                                   \
  X(RSQUARE)                                                                   \
                                                                               \
  /* Keywords */                                                               \
  X(FUNCTION)                                                                  \
  X(CLASS)                                                                     \
  X(NEW)                                                                       \
  X(VAR)                                                                       \
  X(TRUE)                                                                      \
  X(FALSE)                                                                     \
  X(IF)                                                                        \
  X(ELSE)                                                                      \
  X(RETURN)                                                                    \
  XX(ENUM_COUNT)

#define X(name) TOKEN_##name,
#define XX(name) TOKEN_##name
typedef enum { FLUFF_fluff_token_type_LIST } fluff_token_type;
#undef X
#undef XX

#define X(name) #name,
#define XX(name) #name
const char *FluffTokenType[] = {FLUFF_fluff_token_type_LIST};
#undef X
#undef XX
