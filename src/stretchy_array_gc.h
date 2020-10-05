#define ARRAY_WITH_MEMCPY
bool gcArrayFull(void *Array) {
  if (Array) {
    stretchy_array_header Header;
    memcpy(&Header, arrayHeader(Array), sizeof(stretchy_array_header));
    return (Header.Size == Header.Capacity);
  }
  return true;
}

void *gcArrayGrow(void *Array, unsigned int ItemSize) {
  stretchy_array_header Header;
  char *ArrayHeader;
  if (Array) {
    ArrayHeader = arrayHeader(Array);
    memcpy(&Header, ArrayHeader, sizeof(stretchy_array_header));
    Header.Capacity *= 2;
    ArrayHeader = GCRealloc(ArrayHeader, sizeof(stretchy_array_header) +
                                             Header.Capacity * ItemSize);

    memcpy(ArrayHeader, &Header, sizeof(stretchy_array_header));
    return ArrayHeader + sizeof(stretchy_array_header);
  } else {
    ArrayHeader = GCMalloc(sizeof(stretchy_array_header) + 2 * ItemSize);
    memcpy(&Header, ArrayHeader, sizeof(stretchy_array_header));
    Header.Size = 0;
    Header.Capacity = 2;
    memcpy(ArrayHeader, &Header, sizeof(stretchy_array_header));
    return ArrayHeader + sizeof(stretchy_array_header);
  }
}

size_t GCArraySize(void *Array) {
  if (Array) {
    stretchy_array_header Header;
    memcpy(&Header, arrayHeader(Array), sizeof(stretchy_array_header));
    return Header.Size;
  }
  return 0;
}

void GCArrayMarkAllocation(void *Array) {
  if (Array) {
    GCMarkAllocation(arrayHeader(Array));
  }
}

#define GCArrayPush(a, item)                                                   \
  {                                                                            \
    stretchy_array_header ___Header;                                           \
    gcArrayFull(a) ? a = gcArrayGrow(a, sizeof(*a)) : 0;                       \
    memcpy(&___Header, arrayHeader(a), sizeof(stretchy_array_header));         \
    a[___Header.Size++] = item;                                                \
    memcpy(arrayHeader(a), &___Header, sizeof(stretchy_array_header));         \
  }
