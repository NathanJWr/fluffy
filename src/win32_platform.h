#ifdef _WIN32

typedef struct object_file_handle {
  HANDLE Handle;
} object_file_handle;

bool PlatformReadWholeFile(object_file_handle *Handle, void *Buffer,
                           size_t BufferSize);
bool PlatformCreateFileHandle(const char *Filename, object_file_handle *Handle);
bool PlatformCloseFileHandle(object_file_handle *Handle);

#endif
