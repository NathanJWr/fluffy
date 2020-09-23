typedef struct environment_ll {
  environment Env;

  struct environment_ll *Prev;
  struct environment_ll *Next;
} environment_ll;

environment_ll *EnvironmentsHead;

size_t hashString(const char *Str) {
  size_t Hash = 5381;
  int c;

  while ((c = *Str++))
    Hash = ((Hash << 5) + Hash) + c; /* hash * 33 + c */

  return Hash;
}

size_t fastModulus(size_t Hash, size_t BucketLength) {
  return (Hash & (BucketLength - 1));
}

bool isBucketEmpty(object_bucket *Objects, unsigned int Index) {
  return (Objects[Index].ProbeSequenceLength == -1);
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
    if (OldEnv->Objects[i].ProbeSequenceLength != -1) {
      char *Var = OldEnv->Objects[i].Var;
      object *Obj = OldEnv->Objects[i].Obj;

      AddToEnv(NewEnv, Var, Obj);
    }
  }
}

void rehashAndResize(environment *Env) {
  environment NewEnv;
  InitEnv(&NewEnv, Env->ObjectsLength * 2);
  rehashEnv(Env, &NewEnv);

  free(Env->Objects);
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

void InitEnv(environment *Env, unsigned int Size) {
  /* Note: Using power of two array sizes for faster modulus */
  unsigned int i;
  Env->ObjectsLength = Size;
  Env->Objects = malloc(sizeof(object_bucket) * Env->ObjectsLength);

  /* Set all the ProbeSequenceLengths to -1 so we know what slots are empty */
  for (i = 0; i < Env->ObjectsLength; i++) {
    Env->Objects[i].ProbeSequenceLength = -1;
  }
}

void AddToEnv(environment *Env, const char *Var, object *Obj) {
  size_t Hash;
  unsigned int Index;

  possiblRehashAndResize(Env);

  /* Hash the string to get the index we're going to try to put
   * our Object in */
  Hash = hashString(Var);
  Index = fastModulus(Hash, Env->ObjectsLength);

  object_bucket Item = {
      .ProbeSequenceLength = 0, .Var = (char *)Var, .Obj = Obj};

  /* Check if the index is empty. If it is we can put our Obj right there */
  if (isBucketEmpty(Env->Objects, Index)) {
    Env->Objects[Index] = Item;
  } else if (0 == strcmp(Env->Objects[Index].Var, Var)) {
    /* The index is not empty, but it's the same string */
    free(Env->Objects[Index].Var);
    Env->Objects[Index] = Item;
  } else {
    printf("Collision detected! \n");
    findSpotForKey(Env->Objects, Item, Index, Env->ObjectsLength);
  }
  Env->ObjectsExist++;
}

object *FindInEnv(environment *Env, const char *Var) {
  size_t Hash = hashString(Var);
  unsigned int Index = fastModulus(Hash, Env->ObjectsLength);
  unsigned int IndexesTried = 0;

  /* Ideally the first Index we try is where our Var is located.
   * However, worst case is that this turns into a linear search */
  while (IndexesTried < Env->ObjectsLength) {
    if (!isBucketEmpty(Env->Objects, Index) &&
        (0 == strcmp(Env->Objects[Index].Var, Var))) {
      return Env->Objects[Index].Obj;
    }
    IndexesTried++;
    Index++;

    /* Wrap around to the start of the buffer */
    if (Index > Env->ObjectsLength - 1) {
      Index = 0;
    }
  }

  return NULL;
}

environment *CreateEnclosedEnvironment(environment *Outer) {
  environment *NewEnv = CreateEnvironment();
  NewEnv->Outer = Outer;
  return NewEnv;
}

environment *CreateEnvironment(void) {
  environment_ll *Env = malloc(sizeof(environment_ll));
  InitEnv(&Env->Env, 4);

  if (EnvironmentsHead) {
    Env->Next = EnvironmentsHead;
    EnvironmentsHead->Prev = Env;
  }
  EnvironmentsHead = Env;

  return (environment *)Env;
}

void FreeEnvironemnt(environment *Env) {
  environment_ll *EnvLL = (environment_ll *)Env;

  if (EnvLL == EnvironmentsHead) {
    EnvironmentsHead = EnvLL->Next;
  }

  if (EnvLL->Next != NULL) {
    EnvLL->Next->Prev = EnvLL->Prev;
  }

  if (EnvLL->Prev != NULL) {
    EnvLL->Prev->Next = EnvLL->Next;
  }

  free(EnvLL->Env.Objects);
  free(EnvLL);
}
