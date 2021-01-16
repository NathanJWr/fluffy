struct environment;
typedef struct environment environment;

struct object;
typedef struct object object;

void GCMarkObject(object * Obj);
void GCMarkEnvironment(environment * Env);
