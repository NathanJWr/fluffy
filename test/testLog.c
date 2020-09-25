int isDebug = 0;

/**
 * Only print a debug log if debug mode is enabled and only reset the buffer if
 * char *line is allocated.
 */
void printLog(char *line, int isAllocated) {
  if (isDebug) {
    printf("%s\n", line);
  }
  if (isAllocated) {
    line[0] = '\0';
  }
}