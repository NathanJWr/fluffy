enum ast_type {
  AST_BASE,
  AST_PROGRAM,
  AST_IDENTIFIER
};

typedef struct {
  unsigned char Type; /* Type of the "actual" ast node */
  unsigned char Size; /* Size of the "actual" ast node */
} ast_base;

typedef struct {
  ast_base Base;

  unsigned int StatementsLength;
  /* Statements are contiguosly stored in memory after the program ast data */
} ast_program;

typedef struct {
  ast_base Base;

  const char *Value;
} ast_identifier;