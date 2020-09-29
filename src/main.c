void *ReadWholeFile(const char *Filename, size_t *Length) {
  int Fd = open(Filename, O_RDONLY);
  *Length = lseek(Fd, 0, SEEK_END);
  void *Data = mmap(0, *Length, PROT_READ, MAP_PRIVATE, Fd, 0);
  return Data;
}

int main(int argc, char *argv[]) {
  unsigned int i;
  for (i = 1; i < argc; i++) {
    size_t FileLength = 0;
    char *FileBuffer = ReadWholeFile(argv[i], &FileLength);
    char *StringStorage = calloc(1, FileLength);
    lexer Lexer;
    parser Parser;
    environment *Env = CreateEnvironment();
    ast_program *ProgramTree;
    object *ReturnValue;

    LexerInit(&Lexer, FileBuffer, FileBuffer + FileLength, StringStorage, FileLength);
    ParserInit(&Parser, &Lexer);
    EvalInit(Env);

    ProgramTree = ParseProgram(&Parser);
    ReturnValue = Eval((ast_base *) ProgramTree, Env);
    if (ReturnValue->Type != OBJECT_NULL) {
      PrintObject(ReturnValue);
    }


    /* Cleanup */
    GCSweep();
    free(StringStorage);
    AstNodeDelete((ast_base *) ProgramTree);
  }
}
