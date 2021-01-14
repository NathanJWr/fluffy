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
  char *StringStore;
  char *StringStoreBegin;

  size_t ReadBufferSize = 0x1000;
  char *ReadBuffer;

  size_t GetlineSize;
  ast_base **Programs = NULL;
  environment *Env = CreateEnvironment();

  StringStore = calloc(1, StringStoreSize);
  ReadBuffer = calloc(1, ReadBufferSize);
  StringStoreBegin = StringStore;
  EvalInit(Env);

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
    EvalInit(Env);

    Program = ParseProgram(&Parser);
    Obj = Eval((ast_base *)Program, Env);
    //if (Obj->Type == FLUFF_OBJECT_ERROR) {
      PrintObject(Obj);
      printf("\n");
      //}
    ArrayPush(Programs, (ast_base *)Program);

    /* Hacky things to keep pointers from being invalidated */
    Lexer.StringStorage += strlen(Lexer.StringStorage) + 1;
    StringStoreSize -= Lexer.StringStorage - StringStore;
    StringStore = Lexer.StringStorage;

    GCMarkAndSweep(Env, NULL, 0);
  }

  for (i = 0; i < ArraySize(Programs); i++) {
    AstNodeDelete(Programs[i]);
  }
  ArrayFree(Programs);

  free(ReadBuffer);
  free(StringStoreBegin);

  GCSweep();
  return 0;
}
