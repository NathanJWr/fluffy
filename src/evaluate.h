struct object;
typedef struct object object;

struct ast_base;
typedef struct ast_base ast_base;

struct environment;
typedef struct environment environment;

void EvalInit(environment *Root);

typedef struct executing_block {
  object **InFlightObjects;

  struct executing_block *Next;
  struct executing_block *Prev;
} executing_block;
object *Eval(ast_base *Node, environment *Env);
