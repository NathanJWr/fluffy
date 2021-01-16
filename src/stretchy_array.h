typedef struct {
  uint32_t Size;
  uint32_t Capacity;
} stretchy_array_header;

#define ARRAY_WITH_MEMCPY
#define arrayHeader(a) ((char *)a) - sizeof(stretchy_array_header)

bool arrayFull(void *Array) {
  if (Array) {
#ifdef ARRAY_WITH_MEMCPY
    stretchy_array_header Header;
    memcpy(&Header, arrayHeader(Array), sizeof(stretchy_array_header));
    return (Header.Size == Header.Capacity);
#endif
#ifdef ARRAY_WITH_CASTS
    stretchy_array_header *Header = (stretchy_array_header *)arrayHeader(Array);
    return Header->Capacity == Header->Size;
#endif
  }
  return true;
}

void *arrayGrow(void *Array, unsigned int ItemSize) {
#ifdef ARRAY_WITH_MEMCPY
  stretchy_array_header Header;
  char *ArrayHeader;
  if (Array) {
    ArrayHeader = arrayHeader(Array);
    memcpy(&Header, ArrayHeader, sizeof(stretchy_array_header));
    Header.Capacity *= 2;
    ArrayHeader = realloc(ArrayHeader, sizeof(stretchy_array_header) +
                                           Header.Capacity * ItemSize);

    memcpy(ArrayHeader, &Header, sizeof(stretchy_array_header));
    return ArrayHeader + sizeof(stretchy_array_header);
  } else {
    ArrayHeader = malloc(sizeof(stretchy_array_header) + 2 * ItemSize);
    memcpy(&Header, ArrayHeader, sizeof(stretchy_array_header));
    Header.Size = 0;
    Header.Capacity = 2;
    memcpy(ArrayHeader, &Header, sizeof(stretchy_array_header));
    return ArrayHeader + sizeof(stretchy_array_header);
  }
#endif

#ifdef ARRAY_WITH_CASTS
  stretchy_array_header *Header;
  if (Array) {
    Header = (stretchy_array_header *)arrayHeader(Array);
    Header->Capacity *= 2;
    Header = realloc(Header, sizeof(stretchy_array_header) +
                                 Header->Capacity * ItemSize);
  } else {
    Header = malloc(sizeof(stretchy_array_header) + 2 * ItemSize);
    Header->Size = 0;
    Header->Capacity = 2;
  }

  return (char *)Header + sizeof(stretchy_array_header);
#endif
}

size_t ArraySize(void *Array) {
  if (Array) {
#ifdef ARRAY_WITH_MEMCPY
    stretchy_array_header Header;
    memcpy(&Header, arrayHeader(Array), sizeof(stretchy_array_header));
    return Header.Size;
#endif
#ifdef ARRAY_WITH_CASTS
    return ((stretchy_array_header *)arrayHeader(Array))->Size;
#endif
  }
  return 0;
}

void ArrayFree(void *Array) {
  if (Array) {
    free(arrayHeader(Array));
  }
}

#ifdef ARRAY_WITH_MEMCPY
#define ArrayPush(a, item)                                              \
  {                                                                     \
    stretchy_array_header ___Header;                                    \
    arrayFull((a)) ? (a) = arrayGrow((a), sizeof(*(a))) : 0;            \
    memcpy(&___Header, arrayHeader((a)), sizeof(stretchy_array_header)); \
    (a)[___Header.Size++] = item;                                       \
    memcpy(arrayHeader(a), &___Header, sizeof(stretchy_array_header));  \
  }
#endif

#ifdef ARRAY_WITH_CASTS
#define ArrayPush(a, item)                                                     \
  arrayFull(a) ? a = arrayGrow(a, sizeof(*a)) : 0;                             \
  a[((stretchy_array_header *)arrayHeader(a))->Size++] = item
#endif
