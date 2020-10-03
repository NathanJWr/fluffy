struct object;
typedef struct object object;

struct ast_base;
typedef struct ast_base ast_base;

struct environment;
typedef struct environment environment;

void EvalInit(environment *Root);
object *Eval(ast_base *Node, environment *Env);
