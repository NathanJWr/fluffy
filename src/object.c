object *NewObject(object_type Type, unsigned int Size) {
  object *Obj = GCMalloc(Size);
  Obj->Type = Type;
  Obj->Size = Size;

  switch (Type) {
  case FLUFF_OBJECT_STRING: {
    Obj->MethodEnv = GetObjectStringEnv();
  } break;
  case FLUFF_OBJECT_ARRAY: {
  } break;
  default: {
    Obj->MethodEnv = NULL;
  }
  }
  return Obj;
}

object *NewStringCopy(const char *Str) {
  object_string *NewStr = NewString(strlen(Str) + 1);
  strcpy(NewStr->Value, Str);
  return (object *)NewStr;
}

object *NewError(const char *Message, ...) {
  va_list args1, args2;
  unsigned int StringSize;
  object_error *Err;
  va_start(args1, Message);
  va_start(args2, Message);

  StringSize = vsnprintf(NULL, 0, Message, args1);
  Err = (object_error *)NewObject(FLUFF_OBJECT_ERROR,
                                  sizeof(object_error) + StringSize + 1);

  vsprintf(Err->Message, Message, args2);
  va_end(args1);
  va_end(args2);
  return (object *)Err;
}

void PrintObject(object *Obj) {
  switch (Obj->Type) {

  case FLUFF_OBJECT_NUMBER: {
    switch (((object_number *)Obj)->Type) {
    case NUM_INTEGER: {
      printf("%ld", ((object_number *)Obj)->Int);
    } break;
    case NUM_DOUBLE: {
      printf("%lf", ((object_number *)Obj)->Dbl);
    } break;
    default: {
      assert(0);
    } break;
    }
  } break;

  case FLUFF_OBJECT_BOOLEAN: {
    (((object_boolean *)Obj)->Value) ? printf("true") : printf("false");
  } break;

  case FLUFF_OBJECT_STRING: {
    printf("%s", ((object_string *)Obj)->Value);
  } break;

  case FLUFF_OBJECT_ARRAY: {
    unsigned int i;
    object_array *Arr = (object_array *)Obj;
    unsigned int ArrLength = ArraySize(Arr->Items);
    printf("[");
    for (i = 0; i < ArrLength - 1; i++) {
      PrintObject(*Arr->Items[i]);
      printf(", ");
    }
    PrintObject(*Arr->Items[i]);
    printf("]");
  } break;

  case FLUFF_OBJECT_NULL: {
    printf("NULL");
  } break;

  case FLUFF_OBJECT_ERROR: {
    printf("Error: %s", ((object_error *)Obj)->Message);
  } break;

  case FLUFF_OBJECT_FUNCTION: {
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
}
