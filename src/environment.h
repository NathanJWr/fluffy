typedef struct {
  char ProbeSequenceLength;
  char *Var;
  object *Obj;
} object_bucket;

typedef struct {
  object_bucket *Objects;
  unsigned int ObjectsLength;
  unsigned int ObjectsExist;
} environment;

void InitEnv(environment *Env, unsigned int Size);
void AddToEnv(environment *Env, char *Var, object *Obj);
