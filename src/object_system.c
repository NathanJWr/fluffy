environment SystemMethodEnv;
static object *fluffMethodSystemType(object **Args);
static object *fluffMethodSystemPrint(object **Args);

STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodSystemType, fluffMethodSystemType);
STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodSystemPrint,
                                 fluffMethodSystemPrint);

static object_class SystemClass = {
    .Base.Type = FLUFF_OBJECT_CLASS,
    .Base.Size = sizeof(object_class),
    .Base.MethodEnv = &SystemMethodEnv,
};

void InitObjectSystemEnv() {
  InitEnv(&SystemMethodEnv, 16, malloc, free);
  AddToEnv(&SystemMethodEnv, "print", (object *)&FluffMethodSystemPrint);
  AddToEnv(&SystemMethodEnv, "type", (object *)&FluffMethodSystemType);
}

environment *GetObjectSystemEnv() { return &SystemMethodEnv; }

object_class *GetObjectSystemClass() { return &SystemClass; }

static object *fluffMethodSystemType(object **Args) {
  if (ArraySize(Args) != 1) {
    return NewError("method, type, expected 1 argument, got %d",
                    ArraySize(Args));
  }
  object_string *Str =
      (object_string *)NewStringCopy(FluffObjectType[Args[0]->Type]);
  return (object *)Str;
}

static object *fluffMethodSystemPrint(object **Args) {
  if (ArraySize(Args) != 1) {
    return NewError("method, print, expected 1 argument, got %d",
                    ArraySize(Args));
  }
  PrintObject(Args[0]);
  printf("\n");
  return (object *)&NullObject;
}
