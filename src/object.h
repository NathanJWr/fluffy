#define FLUFF_OBJECT_TYPE_LIST                                                 \
  X(FLUFF_OBJECT_NUMBER)                                                       \
  X(FLUFF_OBJECT_BOOLEAN)                                                      \
  X(FLUFF_OBJECT_STRING)                                                       \
  X(FLUFF_OBJECT_ARRAY)                                                        \
  X(FLUFF_OBJECT_NULL)                                                         \
  X(FLUFF_OBJECT_RETURN)                                                       \
  X(FLUFF_OBJECT_ERROR)                                                        \
  X(FLUFF_OBJECT_FUNCTION)                                                     \
  X(FLUFF_OBJECT_BUILTIN)                                                      \
  X(FLUFF_OBJECT_METHOD)                                                       \
  X(FLUFF_OBJECT_CLASS)                                                        \
  X(FLUFF_OBJECT_BUILTIN_CLASS)                                                \
  X(FLUFF_OBJECT_CLASS_INSTANTIATION)                                          \
  XX(OJECT_TYPE_LIST_COUNT)

#define X(name) name,
#define XX(name) name
typedef enum { FLUFF_OBJECT_TYPE_LIST } object_type;
#undef X
#undef XX

#define X(name) #name,
#define XX(name) #name
const char *FluffObjectType[] = {FLUFF_OBJECT_TYPE_LIST};
#undef X
#undef XX

/* Typedefs and pre-declarations */
struct object;
typedef struct object object;
typedef object *(*object_method_function)(object *This, object **Args);


struct environment;
typedef struct {
  object_method_function MethodLength;
} fluff_object_function_table;

struct object {
#ifdef DEBUG_TYPES
  object_type Type;
#else
  unsigned char Type;
#endif

  unsigned char Size;
  struct environment *MethodEnv;
};

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

typedef struct {
  object Base;

  ast_var_statement **Variables;
} object_class;

typedef struct {
  object Base;

  environment *Locals;
} object_class_instantiation;

typedef object *(*builtin_function)(object **Args);
typedef struct {
  object Base;

  builtin_function Fn;
} object_builtin;

object *NewObject(object_type Type, unsigned int Size);
object *NewError(const char *Message, ...);
void PrintObject(object *Obj);

#define STATIC_BUILTIN_FUNCTION_VARIABLE(Name, Function)                       \
  static object_builtin Name = {                                               \
      .Base.Type = FLUFF_OBJECT_BUILTIN,                                       \
      .Base.Size = sizeof(object_builtin),                                     \
      .Fn = Function,                                                          \
  }
/* Constructors for other objects */

#define NewNumber()                                                            \
  ((object_number *)NewObject(FLUFF_OBJECT_NUMBER, sizeof(object_number)))
#define NewBoolean()                                                           \
  ((object_boolean *)NewObject(FLUFF_OBJECT_BOOLEAN, sizeof(object_boolean)))
#define NewReturn()                                                            \
  ((object_return *)NewObject(FLUFF_OBJECT_RETURN, sizeof(object_return)))
#define NewFunction()                                                          \
  ((object_function *)NewObject(FLUFF_OBJECT_FUNCTION, sizeof(object_function)))
#define NewArray()                                                             \
  ((object_array *)NewObject(FLUFF_OBJECT_ARRAY, sizeof(object_array)))

#define NewString(StrSize)                                                     \
  ((object_string *)NewObject(FLUFF_OBJECT_STRING,                             \
                              sizeof(object_string) + StrSize))
object *NewStringCopy(const char *Str);

static object_null NullObject = {
    .Base.Type = FLUFF_OBJECT_NULL,
    .Base.Size = 0,
};

