object *NewObject(object_type Type, unsigned int Size) {
  object *Obj = GCMalloc(Size);
  Obj->Type = Type;
  Obj->Size = Size;

  return Obj;
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

  default: {
    printf("Error: unknown object");
  }
  }

  printf("\n");
}