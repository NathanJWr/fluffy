typedef struct gc_alloc_info {
  struct gc_alloc_info *Next;
  struct gc_alloc_info *Prev;

  unsigned int Size;
  unsigned char Mark;
} gc_alloc_info;

static gc_alloc_info *GCHead = NULL;
void GCRemoveNode(gc_alloc_info *Node);

void *GCMalloc(size_t Size) {
  gc_alloc_info *Info = malloc(sizeof(gc_alloc_info) + Size);
  Info->Next = NULL;
  Info->Prev = NULL;
  Info->Mark = 0;
  Info->Size = Size;

  if (GCHead) {
    Info->Next = GCHead;
    GCHead->Prev = Info;
  }
  GCHead = Info;

  return Info + 1;
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

void GCMarkAndSweep(environment *RootEnv) {
  EnvironmentMark(RootEnv);
  GCSweep();
}
