/* Set up the function tables for each object type */

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

environment ArrayMethodEnv;
void InitObjectMethodEnvs() {
  InitObjectStringEnv();
  InitEnv(&ArrayMethodEnv, 16, malloc);
}
