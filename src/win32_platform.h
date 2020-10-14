#ifdef _WIN32

typedef struct platform_file_handle {
  HANDLE Handle;
} platform_file_handle;

bool PlatformReadWholeFile(platform_file_handle *Handle, void *Buffer,
                           size_t BufferSize);
bool PlatformCreateFileHandle(const char *Filename, platform_file_handle *Handle);
bool PlatformWriteFile(platform_file_handle *Handle, const char *Buff, size_t BuffSize);
bool PlatformCloseFileHandle(platform_file_handle *Handle);

#endif
