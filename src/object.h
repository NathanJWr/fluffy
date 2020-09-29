#define OBJECT_TYPE_LIST                                                       \
  X(OBJECT_NUMBER)                                                             \
  X(OBJECT_BOOLEAN)                                                            \
  X(OBJECT_STRING)                                                             \
  X(OBJECT_ARRAY)                                                              \
  X(OBJECT_NULL)                                                               \
  X(OBJECT_RETURN)                                                             \
  X(OBJECT_ERROR)                                                              \
  X(OBJECT_FUNCTION)                                                           \
  X(OBJECT_BUILTIN)                                                            \
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
#ifdef DEBUG_TYPES
  object_type Type;
#else
  unsigned char Type;
#endif
  unsigned char Size;
} object;

typedef struct {
  object Base;

  num_type Type;
  union {
    long Int;
    double Dbl;
  };
} object_number;

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

  /* Array of pointers to object* stored elsewhere */
  /* This is done so we don't invalidate pointers by resizing the array */
  object ***Items; /* gc stretchy array */
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
typedef struct environment_linked {
  struct environment *Env;

  struct environment_linked *Next;
  struct environment_linked *Prev;
} environment_linked;

typedef struct {
  object Base;

  ast_identifier **Parameters;
  ast_block_statement *Body;
  struct environment *Env;

  environment_linked *RecurEnvs;
} object_function;

typedef object *(*BuiltinFunction)(object **Args);
typedef struct {
  object Base;

  BuiltinFunction Fn;
} object_builtin;

object *NewObject(object_type Type, unsigned int Size);
object *NewError(const char *Message, ...);
void PrintObject(object *Obj);

/* Constructors for other objects */

#define NewNumber()                                                            \
  ((object_number *)NewObject(OBJECT_NUMBER, sizeof(object_number)))
#define NewBoolean()                                                           \
  ((object_boolean *)NewObject(OBJECT_BOOLEAN, sizeof(object_boolean)))
#define NewReturn()                                                            \
  ((object_return *)NewObject(OBJECT_RETURN, sizeof(object_return)))
#define NewFunction()                                                          \
  ((object_function *)NewObject(OBJECT_FUNCTION, sizeof(object_function)))

#define NewString(StrSize)                                                     \
  ((object_string *)NewObject(OBJECT_STRING, sizeof(object_string) + StrSize))
object *NewStringCopy(const char *Str);
