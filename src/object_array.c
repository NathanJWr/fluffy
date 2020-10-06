static environment ObjectArrayEnv;
object *fluffMethodArrayLength(object **Args);
object *fluffMethodArrayReverse(object **Args);

STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodArrayLength,
                                 fluffMethodArrayLength);
STATIC_BUILTIN_FUNCTION_VARIABLE(FluffMethodArrayReverse,
                                 fluffMethodArrayReverse);

void InitObjectArrayEnv() {
  InitEnv(&ObjectArrayEnv, 16, malloc, free);
  AddToEnv(&ObjectArrayEnv, "length", (object *)&FluffMethodArrayLength);
  AddToEnv(&ObjectArrayEnv, "reverse", (object *)&FluffMethodArrayReverse);
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

object *fluffMethodArrayReverse(object **Args) {
  if (ArraySize(Args) != 1) {
    return NewError("expected 1 argument, got %d", ArraySize(Args));
  }

  object_array *Arr = (object_array *)Args[0];
  object_array *ReversedArr = NewArray();
  ReversedArr->Items = NULL;
  size_t ArrayLen = ArraySize(Arr->Items);
  for (int i = ArrayLen - 1; i >= 0; i--) {
    GCArrayPush(ReversedArr->Items, Arr->Items[i]);
  }
  return (object *)ReversedArr;
}
