environment FileMethodEnv;
static void fluffMethodFileDestructor(void **Args);
static object *fluffMethodFileReadAll(object **Args);

STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodFileReadAll,
                                 fluffMethodFileReadAll);

void InitObjectFileEnv() {
  InitEnv(&FileMethodEnv, 16, malloc, free);
  AddToEnv(&FileMethodEnv, "readAll", (object *)&FluffMethodFileReadAll);
}

environment *GetObjectFileEnv() { return &FileMethodEnv; }

object_file *NewFileObject() {
  object_file *FileObj =
      (object_file *)NewObject(FLUFF_OBJECT_FILE, sizeof(object_file));
  FileObj->Base.MethodEnv = &FileMethodEnv;
  FileObj->File = GCMalloc(sizeof(platform_file_handle));

  void **Args = malloc(sizeof(*Args) * sizeof(platform_file_handle));
  *Args = FileObj->File;
  GCSetOnFreeFunc(FileObj, fluffMethodFileDestructor, Args);

  return FileObj;
}

static object *fluffMethodFileReadAll(object **Args) {
  if (ArraySize(Args) != 1) {
    return NewError("readAll expected 1 argument. got %d", ArraySize(Args));
  }
  object_file *File = (object_file *)Args[0];

  size_t FSize = PlatformGetFileSize(File->File);
  object_string *Str = NewString(FSize);
  PlatformReadWholeFile(File->File, Str->Value, FSize);
  Str->Value[FSize] = 0;

  return (object *)Str;
}

static void fluffMethodFileDestructor(void **Args) {
  platform_file_handle **HandlesPtr = (platform_file_handle **)Args;
  platform_file_handle *Handle = HandlesPtr[0];
  if (!PlatformCloseFileHandle(Handle)) {
    printf("Failed to close file handle\n");
    exit(1);
  }
}
