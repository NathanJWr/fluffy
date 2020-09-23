/* Note: This is pretty hacky! Don't include it if you don't want to check for
 * leaks.
 * 
 * This file will redefine malloc, calloc, realloc, and free to track your
 * memory usage. Beause of this, you should include it BEFORE any calls to
 * these functions  */

typedef struct leak_check_info {
  char *Filename;
  unsigned int LineNum;
  unsigned int Size;
  char *AllocType;

  struct leak_check_info *Prev;
  struct leak_check_info *Next;
} leak_check_info;

static leak_check_info *LeakInfoHead;
static int LeakDebugPrint = 0;

void DebugPrintLeakInfo(void) {
  leak_check_info *Info = LeakInfoHead;
  while (Info) {
    printf("MEMLEAK: %d byte(s) %s at %s:%d\n", Info->Size, Info->AllocType,
           Info->Filename, Info->LineNum);
    Info = Info->Next;
  }
}

void *LeakCheckMalloc(const char *Filename, unsigned int LineNum,
                      unsigned int Size) {
  leak_check_info *Info = malloc(sizeof(leak_check_info) + Size);
  if (!LeakDebugPrint) {
    atexit(DebugPrintLeakInfo);
    LeakDebugPrint = 1;
  }
  memset(Info, 0, sizeof(leak_check_info));

  Info->Filename = (char *)Filename;
  Info->LineNum = LineNum;
  Info->Size = Size;
  Info->AllocType = "malloc";

  if (LeakInfoHead) {
    Info->Next = LeakInfoHead;
    LeakInfoHead->Prev = Info;
  }
  LeakInfoHead = Info;

  return Info + 1;
}

void *LeakCheckCalloc(const char *Filename, unsigned int LineNum,
                      unsigned int Size) {
  void *Mem = LeakCheckMalloc(Filename, LineNum, Size);
  memset(Mem, 0, Size);
  return Mem;
}

void LeakCheckFree(void *Mem) {
  leak_check_info *Info =
      ((leak_check_info *)((char *)Mem - sizeof(leak_check_info)));

  if (Info == LeakInfoHead) {
    LeakInfoHead = Info->Next;
  }

  if (Info->Next != NULL) {
    Info->Next->Prev = Info->Prev;
  }

  if (Info->Prev != NULL) {
    Info->Prev->Next = Info->Next;
  }

  free(Info);
}

void *LeakCheckRealloc(void *Mem, unsigned int NewSize, const char *Filename,
                       unsigned int LineNum) {
  leak_check_info *Info =
      ((leak_check_info *)((char *)Mem - sizeof(leak_check_info)));
  leak_check_info *NewInfo = realloc(Info, sizeof(leak_check_info) + NewSize);
  if (!NewInfo)
    return NULL;

  Info = NewInfo;
  Info->LineNum = LineNum;
  Info->Size = NewSize;
  Info->Filename = (char *)Filename;
  Info->AllocType = "realloc";

  Info->Prev->Next = Info;
  Info->Next->Prev = Info;

  return Info + 1;
}


#define malloc(Size) LeakCheckMalloc(__FILE__, __LINE__, Size)
#define calloc(Size) LeakCheckCalloc(__FILE__, __LINE__, Size);
#define realloc(Ptr, Size) LeakCheckRealloc(Ptr, Size, __FILE__, __LINE__);
#define free(Ptr) LeakCheckFree(Ptr);
