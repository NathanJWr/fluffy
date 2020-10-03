static environment ObjectArrayEnv;
object *fluffMethodArrayLength(object **Args);

static object_builtin FluffMethodArrayLength = {
    .Base.Type = FLUFF_OBJECT_BUILTIN,
    .Base.Size = sizeof(object_builtin),
    .Fn = fluffMethodArrayLength,
};

void InitObjectArrayEnv() {
  InitEnv(&ObjectArrayEnv, 16, malloc);
  AddToEnv(&ObjectArrayEnv, "length", (object *)&FluffMethodArrayLength);
}

environment *GetObjectArrayEnv() { return &ObjectArrayEnv; }

object *fluffMethodArrayLength(object **Args) {
  if (ArraySize(Args) > 1) {
    return NewError("expected 1 arguments, got %d", ArraySize(Args));
  }
  object_array *Arr = (object_array *)Args[0];
  object_number *Length = NewNumber();
  Length->Type = NUM_INTEGER;
  Length->Int = ArraySize(Arr->Items);
  return (object *)Length;
}
