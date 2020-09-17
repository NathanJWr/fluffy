typedef enum {
  AST_BASE,
  AST_PROGRAM,
  AST_IDENTIFIER,
  AST_INFIX_EXPRESSION
} ast_type;

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

  ast_base *Left;
  ast_base *Right;
} ast_infix_expression;
