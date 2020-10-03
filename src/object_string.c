environment StringMethodEnv;

static object *fluffMethodStringLength(object **Args) {
  if (ArraySize(Args) != 1) {
    return NewError("expected 1 arguments, got %d", ArraySize(Args));
  }
  object_string *Str = (object_string *)Args[0];
  object_number *Length = NewNumber();
  Length->Type = NUM_INTEGER;
  Length->Int = strlen(Str->Value);
  return (object *)Length;
}

static object_builtin FluffMethodStringLength = {
    .Base.Type = FLUFF_OBJECT_BUILTIN,
    .Base.Size = sizeof(object_builtin),
    .Fn = fluffMethodStringLength,
};

void InitObjectStringEnv() {
  InitEnv(&StringMethodEnv, 16, malloc);
  AddToEnv(&StringMethodEnv, "length", (object *)&FluffMethodStringLength);
}

environment *GetObjectStringEnv() { return &StringMethodEnv; }
