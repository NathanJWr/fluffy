environment StringMethodEnv;

static object *fluffMethodStringLength(object *This, object **Args) {
  if (ArraySize(Args) > 0) {
    return NewError("expected 0 arguments, got %d", ArraySize(Args));
  }
  object_string *Str = (object_string *)This;
  object_number *Length = NewNumber();
  Length->Type = NUM_INTEGER;
  Length->Int = strlen(Str->Value);
  return (object *)Length;
}

static object_method FluffMethodStringLength = {
    .Base.Type = FLUFF_OBJECT_METHOD,
    .Base.Size = sizeof(object_method),
    .Method = fluffMethodStringLength,
};

void InitObjectStringEnv() {
  InitEnv(&StringMethodEnv, 16, malloc);
  AddToEnv(&StringMethodEnv, "length", (object *)&FluffMethodStringLength);
}

environment *GetObjectStringEnv() { return &StringMethodEnv; }
