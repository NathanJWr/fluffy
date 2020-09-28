#define AST_TYPE_LIST                                                          \
  X(AST_BASE)                                                                  \
  X(AST_PROGRAM)                                                               \
  X(AST_IDENTIFIER)                                                            \
  X(AST_NUMBER)                                                                \
  X(AST_PREFIX_EXPRESSION)                                                     \
  X(AST_BOOLEAN)                                                               \
  X(AST_IF_EXPRESSION)                                                         \
  X(AST_BLOCK_STATEMENT)                                                       \
  X(AST_FUNCTION_LITERAL)                                                      \
  X(AST_FUNCTION_CALL)                                                         \
  X(AST_INFIX_EXPRESSION)                                                      \
  X(AST_VAR_STATEMENT)                                                         \
  X(AST_RETURN_STATEMENT)                                                      \
  X(AST_STRING)                                                                \
  X(AST_ARRAY_LITERAL)                                                         \
  XX(AST_TYPE_LIST_COUNT)

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

typedef enum {
  NUM_INTEGER,
  NUM_DOUBLE,
} num_type;

typedef struct {
  ast_base Base;

  num_type Type;
  union {
    long Int;
    double Dbl;
  };
} ast_number;

typedef struct {
  ast_base Base;

  bool Value;
} ast_boolean;

typedef struct {
  ast_base Base;

  token_type Operation;
  ast_base *Right;
} ast_prefix_expression;

typedef struct {
  ast_base Base;

  ast_base *Condition;
  ast_base *Consequence;
  ast_base *Alternative;
} ast_if_expression;

typedef struct {
  ast_base Base;

  ast_base **Statements;
} ast_block_statement;

typedef struct {
  ast_base Base;

  ast_base **Parameters;
  ast_base *Body;
} ast_function_literal;

typedef struct {
  ast_base Base;

  token_type Operation;
  ast_base *Left;
  ast_base *Right;
} ast_infix_expression;

typedef struct {
  ast_base Base;

  ast_base *FunctionName;
  ast_base **Arguments;
} ast_function_call;

typedef struct {
  ast_base Base;

  ast_identifier *Name;
  ast_base *Value;
} ast_var_statement;

typedef struct {
  ast_base Base;

  ast_base *Expr;
} ast_return_statement;

typedef struct {
  ast_base Base;

  char *Value;
} ast_string;

typedef struct {
  ast_base Base;

  ast_base **Items; /* stretchy array */
} ast_array_literal;
