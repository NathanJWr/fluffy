bool PlatformReadWholeFile(object_file_handle *Handle, void *Buffer,
                           size_t BufferSize) {
  DWORD BytesRead;
  ReadFile(Handle->Handle, Buffer, BufferSize, &BytesRead, NULL);
  if (BytesRead != BufferSize) {
    return false;
  }

  return true;
}

size_t PlatformGetFileSize(object_file_handle *Handle) {
  ULARGE_INTEGER FSize;
  FSize.LowPart = GetFileSize(Handle->Handle, &FSize.HighPart);

  return FSize.QuadPart;
}

bool PlatformCreateReadFileHandle(const char *Filename,
                                  object_file_handle *Handle) {
  HANDLE FileHandle = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (FileHandle == INVALID_HANDLE_VALUE) {
    return false;
  }

  Handle->Handle = FileHandle;

  return true;
}

bool PlatformCloseFileHandle(object_file_handle *Handle) {
  return CloseHandle(Handle->Handle);
}
