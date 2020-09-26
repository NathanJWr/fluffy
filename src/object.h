#define OBJECT_TYPE_LIST                                                       \
  X(OBJECT_INTEGER)                                                            \
  X(OBJECT_DOUBLE)                                                             \
  X(OBJECT_BOOLEAN)                                                            \
  X(OBJECT_STRING)                                                             \
  X(OBJECT_ARRAY)                                                              \
  X(OBJECT_NULL)                                                               \
  X(OBJECT_RETURN)                                                             \
  X(OBJECT_ERROR)                                                              \
  X(OBJECT_FUNCTION)                                                           \
  XX(OJECT_TYPE_LIST_COUNT)

#define X(name) name,
#define XX(name) name
typedef enum { OBJECT_TYPE_LIST } object_type;
#undef X
#undef XX

#define X(name) #name,
#define XX(name) #name
const char *ObjectType[] = {OBJECT_TYPE_LIST};
#undef X
#undef XX

typedef struct {
  unsigned char Type;
  unsigned char Size;
} object;

typedef struct {
  object Base;

  long Value;
} object_integer;

typedef struct {
  object Base;

  double Value;
} object_double;

typedef struct {
  object Base;

  bool Value;
} object_boolean;

typedef struct {
  object Base;

  char Value[];
} object_string;

typedef struct {
  object Base;

  object **Items; /* stretchy array */
} object_array;

typedef struct {
  object Base;
} object_null;

typedef struct {
  object Base;

  object *Retval;
} object_return;

typedef struct {
  object Base;

  char Message[];
} object_error;

struct environment;
typedef struct {
  object Base;

  ast_identifier **Parameters;
  ast_block_statement *Body;
  struct environment *Env;
} object_function;

object *NewObject(object_type Type, unsigned int Size);
object *NewError(const char *Message, ...);
void PrintObject(object *Obj);
