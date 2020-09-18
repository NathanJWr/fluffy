#define AST_TYPE_LIST                                                          \
  X(AST_BASE)                                                                  \
  X(AST_PROGRAM)                                                               \
  X(AST_IDENTIFIER)                                                            \
  X(AST_INTEGER_LITERAL)                                                       \
  XX(AST_INFIX_EXPRESSION)

#define X(name) name,
#define XX(name) name
typedef enum { AST_TYPE_LIST } ast_type;
#undef X
#undef XX

#define X(name) #name,
#define XX(name) #name
const char *AstType[] = {AST_TYPE_LIST};
#undef X
#undef XX

typedef struct {
  unsigned char Type; /* Type of the "actual" ast node */
  unsigned char Size; /* Size of the "actual" ast node */
} ast_base;

typedef struct {
  ast_base Base;

  ast_base **Statements;
} ast_program;

typedef struct {
  ast_base Base;

  const char *Value;
} ast_identifier;

typedef struct {
  ast_base Base;

  long Integer;
} ast_integer_literal;

typedef struct {
  ast_base Base;

  ast_base *Left;
  ast_base *Right;
} ast_infix_expression;
