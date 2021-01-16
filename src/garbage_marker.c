void markFunctionInstanceObject(object_function_instance *Func);
void markFunctionObject(object_function *Func);
void markArrayObject(object_array *Arr);
void markAllObjectsInEnv(environment *Env);
void markEnvironment(environment *Env);

void GCMarkEnvironment(environment *Env) {
  markEnvironment(Env);
  markAllObjectsInEnv(Env);
}

void GCMarkObject(object * Obj) {
  GCMarkAllocation(Obj);
  switch (Obj->Type) {
  case FLUFF_OBJECT_RETURN: {
    object_return *Ret = (object_return *)Obj;
    if (Ret->Retval) {
      GCMarkObject(Ret->Retval);
    }
  } break;

  case FLUFF_OBJECT_FUNCTION: {
    object_function *Func = (object_function *)Obj;
    markFunctionObject(Func);
  } break;

  case FLUFF_OBJECT_FUNCTION_INSTANCE: {
    markFunctionInstanceObject((object_function_instance *) Obj);
  } break;

  case FLUFF_OBJECT_ARRAY: {
    object_array *Arr = (object_array *)Obj;
    markArrayObject(Arr);
  } break;

  case FLUFF_OBJECT_CLASS: {
    object_class *Class = (object_class *)Obj;
    size_t VarLength = ArraySize(Class->Variables);

    /* Don't need to iterate through array and mark
     * each item because they are allocated in the ast */
    GCArrayMarkAllocation(Class->Variables);

    GCMarkEnvironment(Class->Base.MethodEnv);
  } break;

  case FLUFF_OBJECT_CLASS_INSTANTIATION: {
    object_class_instantiation *Instance = (object_class_instantiation *)Obj;
    GCMarkEnvironment(Instance->Locals);
  } break;

  case FLUFF_OBJECT_FILE: {
    object_file *File = (object_file *)Obj;
    GCMarkAllocation(File->File);
  }

  default: {
    return;
  }
  }
}

void markEnvironment(environment *Env) {
  GCMarkAllocation(Env);
  GCMarkAllocation(Env->Objects);
}

void markAllObjectsInEnv(environment *Env) {
  unsigned int i;
  for (i = 0; i < Env->ObjectsLength; i++) {
    if (!isBucketEmpty(Env->Objects, i)) {
      object_bucket *Bucket = &Env->Objects[i];
      GCMarkObject(Bucket->Obj);
    }
  }
}

void markArrayObject(object_array *Arr) {
  unsigned int i;
  unsigned int ArrLength = ArraySize(Arr->Items);
  GCArrayMarkAllocation(Arr->Items);
  for (i = 0; i < ArrLength; i++) {
    GCMarkObject(*Arr->Items[i]);
    GCMarkAllocation(Arr->Items[i]);
  }
}

void markFunctionObject(object_function *Func) {
  if (!GCMarked(Func->Env)) {
    GCMarkEnvironment(Func->Env);
  }
}

void markFunctionInstanceObject(object_function_instance *Func) {
  if (!GCMarked(Func->Env)) {
    GCMarkEnvironment(Func->Env);
  }
  markFunctionObject(Func->Function);
}
