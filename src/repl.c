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
  unsigned int StringStoreSize = 0x10000;
  char *StringStore = malloc(StringStoreSize);

  size_t ReadBufferSize = 0x1000;
  size_t GetlineSize;
  char *ReadBuffer = malloc(ReadBufferSize);

  while (1) {
    token_type Token;
    printf(">> ");
    ReadLine(ReadBuffer, &GetlineSize, stdin);

    LexerInit(&Lexer, ReadBuffer, ReadBuffer + GetlineSize, StringStore, StringStoreSize);
    Token = NextToken(&Lexer);
    while (Token != TOKEN_END) {
      printf("Token type: %s\n", TokenType[Token]);
      Token = NextToken(&Lexer);
    }
  }
}
