static environment ObjectArrayEnv;
object *fluffMethodArrayLength(object *This, object **Args);

static object_method FluffMethodArrayLength = {
    .Base.Type = FLUFF_OBJECT_METHOD,
    .Base.Size = sizeof(object_method),
    .Method = fluffMethodArrayLength,
};

void InitObjectArrayEnv() {
  InitEnv(&ObjectArrayEnv, 16, malloc);
  AddToEnv(&ObjectArrayEnv, "length", (object *)&FluffMethodArrayLength);
}

environment *GetObjectArrayEnv() { return &ObjectArrayEnv; }

object *fluffMethodArrayLength(object *This, object **Args) {
  if (ArraySize(Args) > 0) {
    return NewError("expected 0 arguments, got %d", ArraySize(Args));
  }
  object_array *Arr = (object_array *)This;
  object_number *Length = NewNumber();
  Length->Type = NUM_INTEGER;
  Length->Int = ArraySize(Arr->Items);
  return (object *)Length;
}
