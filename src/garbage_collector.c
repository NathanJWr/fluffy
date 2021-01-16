typedef void (*gc_on_free_func)(void **Args);
typedef struct gc_alloc_info {
  struct gc_alloc_info *Next;
  struct gc_alloc_info *Prev;

  unsigned int Size;
  unsigned char Mark;

  gc_on_free_func OnFree;
  void **OnFreeArgs; /* stretchy array */
} gc_alloc_info;

static gc_alloc_info *GCHead = NULL;
static size_t AllocationsSinceSweep = 0;
void GCRemoveNode(gc_alloc_info *Node);

bool GCNeedsCleanup() { return (AllocationsSinceSweep > 1); }

void *GCMalloc(size_t Size) {
  gc_alloc_info *Info = malloc(sizeof(gc_alloc_info) + Size);
  Info->Next = NULL;
  Info->Prev = NULL;
  Info->Mark = 0;
  Info->Size = Size;
  Info->OnFree = NULL;
  Info->OnFreeArgs = NULL;

  if (GCHead) {
    Info->Next = GCHead;
    GCHead->Prev = Info;
  }
  GCHead = Info;
  AllocationsSinceSweep++;

  return Info + 1;
}

void GCSetOnFreeFunc(void *Allocation, gc_on_free_func Fn, void **Args) {
  gc_alloc_info *Info =
      (gc_alloc_info *)((char *)Allocation - sizeof(gc_alloc_info));
  Info->OnFree = Fn;
  Info->OnFreeArgs = Args;
}

void *GCRealloc(void *Allocation, unsigned int Size) {
  gc_alloc_info *Info =
      (gc_alloc_info *)((char *)Allocation - sizeof(gc_alloc_info));
  char *NewAllocation = GCMalloc(Size);
  gc_alloc_info *NewInfo =
      (gc_alloc_info *)(NewAllocation - sizeof(gc_alloc_info));
  if (NewInfo) {
    memcpy(NewInfo + 1, Info + 1, Info->Size);
    GCRemoveNode(Info);
    return NewInfo + 1;
  }
  return NULL;
}

void GCMarkAllocation(void *Allocation) {
  gc_alloc_info *Info =
      (gc_alloc_info *)((char *)Allocation - sizeof(gc_alloc_info));
  Info->Mark = 1;
}

bool GCMarked(void *Allocation) {
  gc_alloc_info *Info =
      (gc_alloc_info *)((char *)Allocation - sizeof(gc_alloc_info));
  return Info->Mark;
}

void GCRemoveNode(gc_alloc_info *Node) {
  if (Node == GCHead) {
    GCHead = Node->Next;
  }

  if (Node->Next != NULL) {
    Node->Next->Prev = Node->Prev;
  }

  if (Node->Prev != NULL) {
    Node->Prev->Next = Node->Next;
  }

  if (Node->OnFree) {
    Node->OnFree(Node->OnFreeArgs);
    free(Node->OnFreeArgs);
  }

  free(Node);
}

void GCSweep(void) {
  gc_alloc_info *Head = GCHead;
  while (Head) {
    if (Head->Mark == 0) {
      gc_alloc_info *ToRemove = Head;
      Head = Head->Next;
      GCRemoveNode(ToRemove);
      continue;
    } else {
      Head->Mark = 0;
    }

    Head = Head->Next;
  }
}

size_t debugGetGCAllocationSize() {
  gc_alloc_info *Head = GCHead;
  size_t Size = 0;
  while (Head) {
    Size += Head->Size;
    Head = Head->Next;
  }
  return Size;
}

void GCMarkAndSweep(environment *RootEnv, object ** Stack, int StackSize) {
  AllocationsSinceSweep = 0;
  printf("GC Allocated: %zu bytes\n", debugGetGCAllocationSize());
  GCMarkEnvironment(RootEnv);
  for (int i = 0; i < StackSize; i++) {
    GCMarkObject(Stack [ i ]);
  }
  GCSweep();
  printf("GC Allocated After Sweep: %zu bytes\n", debugGetGCAllocationSize());
}

