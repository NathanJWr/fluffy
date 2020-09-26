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
  Err = (object_error *)NewObject(OBJECT_ERROR,
                                  sizeof(object_error) + StringSize + 1);

  vsprintf(Err->Message, Message, args2);
  va_end(args1);
  va_end(args2);
  return (object *)Err;
}

void PrintObject(object *Obj) {
  switch (Obj->Type) {

  case OBJECT_NUMBER: {
    switch (((object_number *)Obj)->Type) {
    case num_integer: {
      printf("%ld", ((object_number *)Obj)->Int);
    } break;
    case num_double: {
      printf("%lf", ((object_number *)Obj)->Dbl);
    } break;
    default: {
      assert(0);
    } break;
    }
  } break;

  case OBJECT_BOOLEAN: {
    (((object_boolean *)Obj)->Value) ? printf("true") : printf("false");
  } break;

  case OBJECT_STRING: {
    printf("%s", ((object_string *)Obj)->Value);
  } break;

  case OBJECT_NULL: {
    printf("NULL");
  } break;

  case OBJECT_ERROR: {
    printf("Error: %s", ((object_error *)Obj)->Message);
  } break;

  case OBJECT_FUNCTION: {
    unsigned int i;
    object_function *Fn = (object_function *)Obj;
    printf("fn(");

    /* Print the parameters */
    for (i = 0; i < ArraySize(Fn->Parameters) - 1; i++) {
      printf("%s, ", Fn->Parameters[i]->Value);
    }
    printf("%s", Fn->Parameters[i]->Value);

    printf(") { ");
    debugPrintAstNode((ast_base *)Fn->Body);
    printf(" }");
  } break;

  default: {
    printf("Error: unknown object");
  }
  }

  printf("\n");
}
