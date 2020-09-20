void AstProgramDelete(ast_program *Program);
void AstBlockDelete(ast_block_statement *Block);
void AstIfExpressionDelete(ast_if_expression *Expr);
void AstFunctionLiteralDelete(ast_function_literal *Func);

void AstNodeDelete(ast_base *Node) {
  switch (Node->Type) {
  case AST_PROGRAM:
    AstProgramDelete((ast_program *)Node);
    break;
  case AST_BLOCK_STATEMENT:
    AstBlockDelete((ast_block_statement *)Node);
    break;
  case AST_IF_EXPRESSION:
    AstIfExpressionDelete((ast_if_expression *)Node);
    break;
  default:
    free(Node);
    break;
  }
}

void AstProgramDelete(ast_program *Program) {
  unsigned int StatementsSize = ArraySize(Program->Statements);
  unsigned int i;
  for (i = 0; i < StatementsSize; i++) {
    AstNodeDelete(Program->Statements[i]);
  }
  ArrayFree(Program->Statements);
  free(Program);
}

void AstBlockDelete(ast_block_statement *Block) {
  unsigned int StatementsSize = ArraySize(Block->Statements);
  unsigned int i;
  for (i = 0; i < StatementsSize; i++) {
    AstNodeDelete(Block->Statements[i]);
  }
  ArrayFree(Block->Statements);
  free(Block);
}

void AstIfExpressionDelete(ast_if_expression *Expr) {
  AstNodeDelete(Expr->Condition);
  AstNodeDelete(Expr->Consequence);
  if (Expr->Alternative)
    AstNodeDelete(Expr->Alternative);
  free(Expr);
}

void AstFunctionLiteralDelete(ast_function_literal *Func) {
  unsigned int ParametersSize = ArraySize(Func->Parameters);
  unsigned int i;
  for (i = 0; i < ParametersSize; i++) {
    AstNodeDelete(Func->Parameters[i]);
  }
  AstNodeDelete(Func->Body);
  free(Func);
}
