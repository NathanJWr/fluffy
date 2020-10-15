environment FileMethodEnv;
static void fluffMethodFileDestructor(void **Args);
static object *fluffMethodFileReadAll(object **Args);
static object *fluffMethodFileWrite(object **Args);

STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodFileReadAll,
                                 fluffMethodFileReadAll);
STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodFileWrite, fluffMethodFileWrite);

void InitObjectFileEnv() {
  InitEnv(&FileMethodEnv, 16, malloc, free);
  AddToEnv(&FileMethodEnv, "readAll", (object *)&FluffMethodFileReadAll);
  AddToEnv(&FileMethodEnv, "write", (object *)&FluffMethodFileWrite);
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

static object *fluffMethodFileWrite(object **Args) {
  if (ArraySize(Args) != 2) {
    return NewError("write expected 2 arguments. got %d", ArraySize(Args));
  }
  object_file *File = (object_file *)Args[0];
  object *ToWrite = Args[1];
  if (ToWrite->Type != FLUFF_OBJECT_STRING) {
    return NewError("write expected object of type %s. got %s",
                    FluffObjectType[FLUFF_OBJECT_STRING],
                    FluffObjectType[ToWrite->Type]);
  }

  size_t DataLen = strlen(((object_string *)ToWrite)->Value);
  bool Success = PlatformWriteFile(File->File, ((object_string *)ToWrite)->Value, DataLen);

  object_boolean *RetVal = NewBoolean();
  RetVal->Value = Success;
  return (object *)RetVal;
}

static void fluffMethodFileDestructor(void **Args) {
  platform_file_handle **HandlesPtr = (platform_file_handle **)Args;
  platform_file_handle *Handle = HandlesPtr[0];
  if (!PlatformCloseFileHandle(Handle)) {
    printf("Failed to close file handle\n");
    exit(1);
  }
}
