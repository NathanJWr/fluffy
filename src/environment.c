hashed_string hashString(const char *Str) {
  hashed_string Return = {0};
  size_t Hash = 5381;
  int c;

  Return.Str = Str;
  while ((c = *Str++))
    Hash = ((Hash << 5) + Hash) + c; /* hash * 33 + c */

  Return.Hash = Hash;
  return Return;
}

static inline bool HashEqual(hashed_string A, hashed_string B) {
  return (A.Hash == B.Hash && (0 == strcmp(A.Str, B.Str)));
}

size_t fastModulus(size_t Hash, size_t BucketLength) {
  return (Hash & (BucketLength - 1));
}

bool isBucketEmpty(object_bucket *Objects, unsigned int Index) {
  return (Objects[Index].ProbeSequenceLength == 0);
}

void findSpotForKey(object_bucket *Objects, object_bucket Item,
                    unsigned int Index, unsigned int BucketLength) {
  /* Start at the index and move forward till we've met the requirements of
   * Robin Hood hashing */
  unsigned int InsertionIndex = Index;
  while (Item.ProbeSequenceLength <=
         Objects[InsertionIndex].ProbeSequenceLength) {
    InsertionIndex++;
    Item.ProbeSequenceLength++;

    if (InsertionIndex > BucketLength - 1) {
      InsertionIndex = 0;
    }
  }

  /* If the ProbeSeqenceLength of our item is higher than the one at Insertion
   * Index, we need to swap and find a new spot for the item that was previously
   * at the index */
  if (isBucketEmpty(Objects, InsertionIndex)) {
    Objects[InsertionIndex] = Item;
  } else if (Item.ProbeSequenceLength >
             Objects[InsertionIndex].ProbeSequenceLength) {
    object_bucket Temp = Objects[InsertionIndex];
    Objects[InsertionIndex] = Item;

    findSpotForKey(Objects, Temp, InsertionIndex, BucketLength);
  } else {
    Objects[InsertionIndex] = Item;
  }
}

void rehashEnv(environment *OldEnv, environment *NewEnv) {
  unsigned int i;
  for (i = 0; i < OldEnv->ObjectsLength; i++) {
    if (OldEnv->Objects[i].ProbeSequenceLength != 0) {
      hashed_string Var = OldEnv->Objects[i].Var;
      object *Obj = OldEnv->Objects[i].Obj;

      AddToEnv(NewEnv, Var.Str, Obj);
    }
  }
}

void rehashAndResize(environment *Env) {
  environment NewEnv;
  InitEnv(&NewEnv, Env->ObjectsLength * 2, Env->Malloc, Env->Free);
  rehashEnv(Env, &NewEnv);

  if (Env->Free) {
    Env->Free(Env->Objects);
  }
  Env->Objects = NewEnv.Objects;
  Env->ObjectsExist = NewEnv.ObjectsExist;
  Env->ObjectsLength = NewEnv.ObjectsLength;
}

void possiblRehashAndResize(environment *Env) {
  /* We might need to resize the array */
  if (Env->ObjectsLength <= Env->ObjectsExist * 2) {
    rehashAndResize(Env);
  }
}

void InitEnv(environment *Env, unsigned int Size, mallocFunc MallocFunc,
             freeFunc FreeFunc) {
  /* Note: Using power of two array sizes for faster modulus */
  unsigned int AllocSize = sizeof(object_bucket) * Size;
  Env->ObjectsLength = Size;
  Env->ObjectsExist = 0;

  Env->Objects = MallocFunc(AllocSize);
  Env->Malloc = MallocFunc;
  Env->Free = FreeFunc;
  memset(Env->Objects, 0, AllocSize);

  Env->Outer = NULL;
}

void AddToEnv(environment *Env, const char *Var, object *Obj) {
  hashed_string HashedStr;
  unsigned int Index;
  object_bucket Item;

  possiblRehashAndResize(Env);

  /* Hash the string to get the index we're going to try to put
   * our Object in */
  HashedStr = hashString(Var);
  Index = fastModulus(HashedStr.Hash, Env->ObjectsLength);

  Item =
      (object_bucket){.ProbeSequenceLength = 1, .Var = HashedStr, .Obj = Obj};

  /* Check if the index is empty. If it is we can put our Obj right there */
  if (isBucketEmpty(Env->Objects, Index)) {
    Env->Objects[Index] = Item;
    Env->ObjectsExist++;
  } else if (HashEqual(HashedStr, Env->Objects[Index].Var)) {
    /* The index is not empty, but it's the same string */
    Env->Objects[Index] = Item;
  } else {
    printf("Collision detected! \n");
    findSpotForKey(Env->Objects, Item, Index, Env->ObjectsLength);
    Env->ObjectsExist++;
  }
}

void ReplaceInEnv(environment *Env, const char *Var, object *Item) {
  hashed_string HashedStr = hashString(Var);
  unsigned int Index = fastModulus(HashedStr.Hash, Env->ObjectsLength);
  unsigned int IndexesTried = 0;

  while (IndexesTried < Env->ObjectsLength &&
         !isBucketEmpty(Env->Objects, Index)) {

    if (!isBucketEmpty(Env->Objects, Index) &&
        (HashEqual(Env->Objects[Index].Var, HashedStr))) {
      /* Replace the old object with the new one */
      Env->Objects[Index].Obj = Item;
      return;
    }
    IndexesTried++;
    Index++;

    /* Wrap around to the start of the buffer */
    if (Index > Env->ObjectsLength - 1) {
      Index = 0;
    }
  }

  /* We could not find the object to replace in the current
   * environment, but it could be in the outer env */
  if (Env->Outer) {
    ReplaceInEnv(Env->Outer, Var, Item);
  }
}

object *FindInEnv(environment *Env, const char *Var) {
  if (Env == NULL || Var == NULL) {
    return NULL;
  }
  hashed_string HashedStr = hashString(Var);
  unsigned int Index = fastModulus(HashedStr.Hash, Env->ObjectsLength);
  unsigned int IndexesTried = 0;

  /* Ideally the first Index we try is where our Var is located.
   * However, worst case is that this turns into a linear search */

  /* We either look until we've tried everything in the array, or until
   * we hit an empty slot */
  while (IndexesTried < Env->ObjectsLength &&
         !isBucketEmpty(Env->Objects, Index)) {

    if (!isBucketEmpty(Env->Objects, Index) &&
        (HashEqual(Env->Objects[Index].Var, HashedStr))) {
      return Env->Objects[Index].Obj;
    }
    IndexesTried++;
    Index++;

    /* Wrap around to the start of the buffer */
    if (Index > Env->ObjectsLength - 1) {
      Index = 0;
    }
  }

  if (Env->Outer) {
    return FindInEnv(Env->Outer, Var);
  }
  return NULL;
}

environment *CreateEnclosedEnvironment(environment *Outer) {
  environment *NewEnv = CreateEnvironment();
  NewEnv->Outer = Outer;
  return NewEnv;
}

environment *CreateEnvironment(void) {
  environment *Env = GCMalloc(sizeof(environment));
  InitEnv(Env, 4, GCMalloc, NULL);
  return Env;
}

void markEnvironment(environment *Env) {
  GCMarkAllocation(Env);
  GCMarkAllocation(Env->Objects);
}

void markObject(object *Obj);
void markAllObjectsInEnv(environment *Env) {
  unsigned int i;
  for (i = 0; i < Env->ObjectsLength; i++) {
    if (!isBucketEmpty(Env->Objects, i)) {
      object_bucket *Bucket = &Env->Objects[i];
      markObject(Bucket->Obj);
    }
  }
}

void markArrayObject(object_array *Arr) {
  unsigned int i;
  unsigned int ArrLength = ArraySize(Arr->Items);
  GCArrayMarkAllocation(Arr->Items);
  for (i = 0; i < ArrLength; i++) {
    markObject(*Arr->Items[i]);
    GCMarkAllocation(Arr->Items[i]);
  }
}

void markFunctionObject(object_function *Func) {
  if (!GCMarked(Func->Env)) {
    EnvironmentMark(Func->Env);
  }

  environment_linked *Env = Func->RecurEnvs;
  while (Env) {
    GCMarkAllocation(Env);
    EnvironmentMark(Env->Env);

    Env = Env->Next;
  }
}

void markObject(object *Obj) {
  GCMarkAllocation(Obj);
  switch (Obj->Type) {
  case FLUFF_OBJECT_RETURN: {
    object_return *Ret = (object_return *)Obj;
    if (Ret->Retval) {
      markObject(Ret->Retval);
    }
  } break;

  case FLUFF_OBJECT_FUNCTION: {
    object_function *Func = (object_function *)Obj;
    markFunctionObject(Func);
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

    EnvironmentMark(Class->Base.MethodEnv);
  } break;

  case FLUFF_OBJECT_CLASS_INSTANTIATION: {
    object_class_instantiation *Instance = (object_class_instantiation *)Obj;
    EnvironmentMark(Instance->Locals);
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

void EnvironmentMark(environment *Env) {
  markEnvironment(Env);
  markAllObjectsInEnv(Env);
}
