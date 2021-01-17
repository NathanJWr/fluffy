struct object;
typedef struct object object;

typedef struct {
  const char *Str;
  size_t Hash;
} hashed_string;

typedef struct {
  char ProbeSequenceLength;
  hashed_string Var;
  object *Obj;
} object_bucket;

typedef void *(*mallocFunc)(size_t);
typedef void (*freeFunc)(void *);
typedef struct environment {
  object_bucket *Objects;
  unsigned int ObjectsLength;
  unsigned int ObjectsExist;

  mallocFunc Malloc;
  freeFunc Free;
  struct environment *Outer;
} environment;

void InitEnv(environment *Env, unsigned int Size, mallocFunc MallocFunc,
             freeFunc FreeFunc);
void AddToEnv(environment *Env, const char *Var, object *Obj);
void ReplaceInEnv(environment *Env, const char *Var, object *Item);

environment *CreateEnvironment(void);
environment *CreateEnclosedEnvironment(environment *Outer);
void EnvironmentMark(environment *Env);
object *FindInEnv(environment *Env, const char *Var);
void markObject(object *Obj);
bool isBucketEmpty(object_bucket *Objects, size_t Index);
