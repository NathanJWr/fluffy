void AstProgramDelete(ast_program *Program);
void AstIdentifierDelete(ast_identifier *Ident);

void AstNodeDelete(ast_base *Node) {
  switch (Node->Type) {
  case AST_PROGRAM:
    AstProgramDelete((ast_program *)Node);
    break;
  case AST_IDENTIFIER:
    AstIdentifierDelete((ast_identifier *)Node);
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

void AstIdentifierDelete(ast_identifier *Ident) { free(Ident); }
