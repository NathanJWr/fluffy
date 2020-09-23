object *NewObject(object_type Type, unsigned int Size) {
  object *Obj = GCMalloc(Size);
  Obj->Type = Type;
  Obj->Size = Size;

  return Obj;
}

object *NewError(const char *Message, ...) {
  va_list args1, args2;
  unsigned int StringSize;
  object_error *Err;
  va_start(args1, Message);
  va_start(args2, Message);

  StringSize = vsnprintf(NULL, 0, Message, args1);
  Err = (object_error *)NewObject(
      OBJECT_ERROR, sizeof(object_error) + StringSize + 1);

  vsprintf(Err->Message, Message, args2);
  va_end(args1);
  va_end(args2);
  return (object *)Err;
}

void PrintObject(object *Obj) {
  switch (Obj->Type) {

  case OBJECT_INTEGER: {
    printf("%ld", ((object_integer *)Obj)->Value);
  } break;

  case OBJECT_BOOLEAN: {
    (((object_boolean *)Obj)->Value) ? printf("true") : printf("false");
  } break;

  case OBJECT_NULL: {
    printf("NULL");
  } break;

  case OBJECT_ERROR: {
    printf("Error: %s", ((object_error *)Obj)->Message);
  } break;

  default: {
    printf("Error: unknown object");
  }
  }

  printf("\n");
}
