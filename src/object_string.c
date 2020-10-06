environment StringMethodEnv;
static object *fluffMethodStringLength(object **Args);
static object *fluffMethodStringReverse(object **Args);

STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodStringLength,
                                 fluffMethodStringLength);
STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodStringReverse,
                                 fluffMethodStringReverse);

void InitObjectStringEnv() {
  InitEnv(&StringMethodEnv, 16, malloc, free);
  AddToEnv(&StringMethodEnv, "length", (object *)&FluffMethodStringLength);
  AddToEnv(&StringMethodEnv, "reverse", (object *)&FluffMethodStringReverse);
}

environment *GetObjectStringEnv() { return &StringMethodEnv; }

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

static object *fluffMethodStringReverse(object **Args) {
  if (ArraySize(Args) != 1) {
    return NewError("expected 1 arguments, got %d", ArraySize(Args));
  }

  object_string *Str = (object_string *)Args[0];
  size_t StringLength = strlen(Str->Value);
  object_string *ReversedStr = NewString(StringLength + 1);

  int j = 0;
  for (int i = StringLength - 1; i >= 0; i--) {
    ReversedStr->Value[j] = Str->Value[i];
    j++;
  }

  ReversedStr->Value[StringLength] = '\0';
  return (object *)ReversedStr;
}
