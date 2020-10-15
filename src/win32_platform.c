bool PlatformReadWholeFile(platform_file_handle *Handle, void *Buffer,
                           size_t BufferSize) {
  DWORD BytesRead;
  ReadFile(Handle->Handle, Buffer, BufferSize, &BytesRead, NULL);
  if (BytesRead != BufferSize) {
    return false;
  }

  return true;
}

size_t PlatformGetFileSize(platform_file_handle *Handle) {
  ULARGE_INTEGER FSize;
  FSize.LowPart = GetFileSize(Handle->Handle, &FSize.HighPart);

  return FSize.QuadPart;
}

bool PlatformWriteFile(platform_file_handle *Handle, const char *Buff,
                       size_t BuffSize) {
  DWORD SizeWritten = 0;
  bool Success = WriteFile(Handle->Handle, Buff, BuffSize, &SizeWritten, NULL);
  /* Note(Nathan): WriteFile can return false, but not be in an error state if
   * an asynchronous handle is used */

  return (Success && (SizeWritten == BuffSize));
}

bool PlatformCreateFileHandle(const char *Filename,
                              platform_file_handle *Handle) {

  /* Note: Open file handle with read and write access for simplicity */
  HANDLE FileHandle = CreateFile(Filename, GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (FileHandle == INVALID_HANDLE_VALUE) {
    /* If we can't open an existing file, create a new one */
    FileHandle = CreateFile(Filename, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (FileHandle == INVALID_HANDLE_VALUE) {
      return false;
    }
  }

  Handle->Handle = FileHandle;

  return true;
}

bool PlatformCloseFileHandle(platform_file_handle *Handle) {
  return CloseHandle(Handle->Handle);
}
