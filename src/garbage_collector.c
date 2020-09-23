typedef struct gc_alloc_info { 
  struct gc_alloc_info *Next;
  struct gc_alloc_info *Prev;

  unsigned char Mark;
} gc_alloc_info;

gc_alloc_info *GCHead;

void *GCMalloc(unsigned int Size) {
  gc_alloc_info *Info = malloc(sizeof(gc_alloc_info) + Size);
  Info->Mark = 0;

  if (GCHead) {
    Info->Next = GCHead;
    GCHead->Prev = Info;
  }
  GCHead = Info;

  return Info + 1;
}
