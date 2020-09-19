void AstProgramDelete(ast_program *Program);
void AstBlockDelete(ast_block_statement *Block);

void AstNodeDelete(ast_base *Node) {
  switch (Node->Type) {
  case AST_PROGRAM:
    AstProgramDelete((ast_program *)Node);
    break;
  case AST_BLOCK_STATEMENT:
    AstBlockDelete((ast_block_statement *)Node);
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
