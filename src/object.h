#define OBJECT_TYPE_LIST                                                       \
  X(OBJECT_INTEGER)                                                            \
  X(OBJECT_BOOLEAN)                                                            \
  X(OBJECT_NULL)                                                               \
  X(OBJECT_RETURN)                                                             \
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

  bool Value;
} object_boolean;

typedef struct {
  object Base;
} object_null;

typedef struct {
  object Base;

  object *Retval;
} object_return;

object *NewObject(object_type Type, unsigned int Size);
void PrintObject(object *Obj);
