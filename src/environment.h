typedef struct {
  char ProbeSequenceLength;
  char *Var;
  object *Obj;
} object_bucket;

typedef struct environment {
  object_bucket *Objects;
  unsigned int ObjectsLength;
  unsigned int ObjectsExist;

  struct environment *Outer;
} environment;

void InitEnv(environment *Env, unsigned int Size);
void AddToEnv(environment *Env, const char *Var, object *Obj);

environment *CreateEnvironment(void);
environment *CreateEnclosedEnvironment(environment *Outer);
void EnvironmentMark(environment *Env);
