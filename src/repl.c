void ReadLine(char *Buffer, size_t *SizeRead, FILE *ReadLocation) {
  char *WriteBuffer = Buffer;
  char c = fgetc(ReadLocation);

  while (c != '\n') {
    *WriteBuffer++ = c;
    c = fgetc(ReadLocation);
  }
  *WriteBuffer = '\0';
  *SizeRead = WriteBuffer - Buffer;
}

int main() {
  lexer Lexer;
  parser Parser;
  unsigned int i;
  unsigned int StringStoreSize = 0x10000;
  size_t ReadBufferSize = 0x1000;
  size_t GetlineSize;
  environment *Env = CreateEnvironment();
  ast_base **Programs = NULL;

  char *StringStore = malloc(StringStoreSize);
  char *StringStoreBegin = StringStore;
  char *ReadBuffer = malloc(ReadBufferSize);

  EvalInit();

  while (1) {
    ast_program *Program;
    object *Obj;

    printf(">> ");
    ReadLine(ReadBuffer, &GetlineSize, stdin);

    if (0 == strcmp(ReadBuffer, "exit")) {
      break;
    }

    LexerInit(&Lexer, ReadBuffer, ReadBuffer + GetlineSize, StringStore,
              StringStoreSize);
    ParserInit(&Parser, &Lexer);

    Program = ParseProgram(&Parser);
    Obj = Eval((ast_base *)Program, Env);
    PrintObject(Obj);
    ArrayPush(Programs, (ast_base *)Program);

    /* Hacky things to keep pointers from being invalidated */
    Lexer.StringStorage += strlen(Lexer.StringStorage) + 1;
    StringStoreSize -= Lexer.StringStorage - StringStore;
    StringStore = Lexer.StringStorage;

    GCMarkAndSweep();
  }

  for (i = 0; i < ArraySize(Programs); i++) {
    AstNodeDelete(Programs[i]);
  }
  ArrayFree(Programs);

  free(ReadBuffer);
  free(StringStoreBegin);
  FreeEnvironemnt(Env);
  GCMarkAndSweep();
  return 0;
}
