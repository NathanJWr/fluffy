void *ReadWholeFile(const char *Filename, size_t *Length) {
  void *Data = NULL;

#ifdef __linux__
  int Fd = open(Filename, O_RDONLY);
  *Length = lseek(Fd, 0, SEEK_END);
  Data = mmap(0, *Length, PROT_READ, MAP_PRIVATE, Fd, 0);
#endif

#ifdef _WIN32
  ULARGE_INTEGER FileSize;
  HANDLE FileHandle, FileMappingHandle;

  /* Open the file handle */
  FileHandle = CreateFileA(Filename, GENERIC_READ, 0, 0, OPEN_EXISTING,
                           FILE_ATTRIBUTE_READONLY, NULL);
  /* Get the file size */
  FileSize.LowPart = GetFileSize(FileHandle, &FileSize.HighPart);

  /* Map file into memory
   * NOTE: Not naming the file mapping right now */
  FileMappingHandle =
      CreateFileMappingA(FileHandle, NULL, PAGE_READONLY, FileSize.HighPart,
                         FileSize.LowPart, NULL);

  /* Get the pointer to the file mapping, from start to end */
  Data = MapViewOfFile(FileMappingHandle, FILE_MAP_READ, 0, 0, 0);
  *Length = FileSize.QuadPart;
#endif

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

    LexerInit(&Lexer, FileBuffer, FileBuffer + FileLength, StringStorage,
              FileLength);
    ParserInit(&Parser, &Lexer);
    EvalInit(Env);

    ProgramTree = ParseProgram(&Parser);
    ReturnValue = Eval((ast_base *)ProgramTree, Env);
    if (ReturnValue->Type != FLUFF_OBJECT_NULL) {
      PrintObject(ReturnValue);
    }

    /* Cleanup */
    GCSweep();
    free(StringStorage);
    AstNodeDelete((ast_base *)ProgramTree);
  }
}
