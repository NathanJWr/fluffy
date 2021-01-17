void markFunctionInstanceObject(object_function_instance *Func);
void markFunctionObject(object_function *Func);
void markArrayObject(object_array *Arr);
void markAllObjectsInEnv(environment *Env);
void markEnvironment(environment *Env);
void markReturnObject(object_return * Return);
void markClassObject(object_class * Class);
void markClassInstanceObject(object_class_instantiation * Instance);
void markFileObject(object_file * File);

void GCMarkEnvironment(environment *Env) {
  markEnvironment(Env);
  markAllObjectsInEnv(Env);
}

void GCMarkObject(object * Obj) {
  GCMarkAllocation(Obj);

  switch (Obj->Type) {
  case FLUFF_OBJECT_RETURN:
    markReturnObject((object_return *) Obj);
    break;
  case FLUFF_OBJECT_FUNCTION:
    markFunctionObject((object_function *) Obj);
    break;
  case FLUFF_OBJECT_FUNCTION_INSTANCE:
    markFunctionInstanceObject((object_function_instance *) Obj);
    break;
  case FLUFF_OBJECT_ARRAY:
    markArrayObject((object_array *) Obj);
    break;
  case FLUFF_OBJECT_CLASS:
    markClassObject((object_class *) Obj);
    break;
  case FLUFF_OBJECT_CLASS_INSTANTIATION:
    markClassInstanceObject((object_class_instantiation *) Obj);
    break;
  case FLUFF_OBJECT_FILE:
    markFileObject((object_file *) Obj);
    break;
  default:
    return;
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
  size_t ArrLength = ArraySize(Arr->Items);
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

void markReturnObject(object_return * Return) {
  if (Return->Retval) {
    GCMarkObject(Return->Retval);
  }
}

void markClassObject(object_class * Class) {
  /* Don't need to iterate through array and mark
   * each item because they are allocated in the ast */
  GCArrayMarkAllocation(Class->Variables);

  GCMarkEnvironment(Class->Base.MethodEnv);
}

void markClassInstanceObject(object_class_instantiation * Instance) {
  GCMarkEnvironment(Instance->Locals);
}

void markFileObject(object_file * File) {
  GCMarkAllocation(File->File);
}
