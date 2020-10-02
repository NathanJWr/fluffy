void AstProgramDelete(ast_program *Program);
void AstBlockDelete(ast_block_statement *Block);
void AstIfExpressionDelete(ast_if_expression *Expr);
void AstFunctionLiteralDelete(ast_function_literal *Func);
void AstFunctionCallDelete(ast_function_call *Call);
void AstVarStatementDelete(ast_var_statement *Stmt);
void AstInfixExpressionDelete(ast_infix_expression *Expr);
void AstReturnStatementDelete(ast_return_statement *Ret);
void AstPrefixExpressionDelete(ast_prefix_expression *Prefix);
void AstArrayLiteralDelete(ast_array_literal *Arr);
void AstIndexExpressionDelete(ast_index *Index);

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
  case AST_FUNCTION_LITERAL:
    AstFunctionLiteralDelete((ast_function_literal *)Node);
    break;
  case AST_FUNCTION_CALL:
    AstFunctionCallDelete((ast_function_call *)Node);
    break;
  case AST_VAR_STATEMENT:
    AstVarStatementDelete((ast_var_statement *)Node);
    break;
  case AST_INFIX_EXPRESSION:
    AstInfixExpressionDelete((ast_infix_expression *)Node);
    break;
  case AST_RETURN_STATEMENT:
    AstReturnStatementDelete((ast_return_statement *)Node);
    break;
  case AST_PREFIX_EXPRESSION:
    AstPrefixExpressionDelete((ast_prefix_expression *)Node);
    break;
  case AST_ARRAY_LITERAL:
    AstArrayLiteralDelete((ast_array_literal *)Node);
    break;
  case AST_INDEX_EXPRESSION:
    AstIndexExpressionDelete((ast_index *)Node);
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
  ArrayFree(Func->Parameters);
  AstNodeDelete(Func->Body);
  free(Func);
}

void AstFunctionCallDelete(ast_function_call *Call) {
  unsigned int ArgumentsSize = ArraySize(Call->Arguments);
  unsigned int i;
  AstNodeDelete(Call->FunctionName);
  for (i = 0; i < ArgumentsSize; i++) {
    AstNodeDelete(Call->Arguments[i]);
  }
  ArrayFree(Call->Arguments);
  free(Call);
}

void AstVarStatementDelete(ast_var_statement *Stmt) {
  AstNodeDelete((ast_base *)Stmt->Name);
  AstNodeDelete(Stmt->Value);
  free(Stmt);
}

void AstInfixExpressionDelete(ast_infix_expression *Expr) {
  AstNodeDelete(Expr->Left);
  AstNodeDelete(Expr->Right);
  free(Expr);
}

void AstReturnStatementDelete(ast_return_statement *Ret) {
  AstNodeDelete(Ret->Expr);
  free(Ret);
}

void AstPrefixExpressionDelete(ast_prefix_expression *Prefix) {
  AstNodeDelete(Prefix->Right);
  free(Prefix);
}

void AstArrayLiteralDelete(ast_array_literal *Arr) {
  unsigned int i;
  unsigned int ArrLength = ArraySize(Arr->Items);
  for (i = 0; i < ArrLength; i++) {
    AstNodeDelete(Arr->Items[i]);
  }
  ArrayFree(Arr->Items);
  free(Arr);
}

void AstIndexExpressionDelete(ast_index *Index) {
  AstNodeDelete((ast_base *)Index->Var);
  AstNodeDelete(Index->Index);
  free(Index);
}

