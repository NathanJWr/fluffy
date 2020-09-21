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
  unsigned int StringStoreSize = 0x10000;
  char *StringStore = malloc(StringStoreSize);

  size_t ReadBufferSize = 0x1000;
  size_t GetlineSize;
  char *ReadBuffer = malloc(ReadBufferSize);

  while (1) {
    ast_program *Program;

    printf(">> ");
    ReadLine(ReadBuffer, &GetlineSize, stdin);

    if (0 == strcmp(ReadBuffer, "exit")) {
      break;
    }

    LexerInit(&Lexer, ReadBuffer, ReadBuffer + GetlineSize, StringStore,
              StringStoreSize);
    ParserInit(&Parser, &Lexer);
    Program = ParseProgram(&Parser);
    AstNodeDelete((ast_base *)Program);
  }

  free(ReadBuffer);
  free(StringStore);
  return 0;
}
