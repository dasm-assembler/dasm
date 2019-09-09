/*
  The issue here was to avoid fseek() and friends to get away
  from potential issue with how long "long" really is. So we
  "abuse" freopen() in append mode to get to the end of the
  file once we establish it exists at all with fopen() in read
  mode. Pretty convoluted, but probably the best way. Comments
  welcome! :-)
*/

#include <stdio.h>
#include <stdlib.h>

int file_size(const char *name, fpos_t *size)
{
  int status = -1;
  FILE *f = fopen(name, "r+");
  if (f != NULL) {
    FILE *g = freopen(NULL, "a+", f);
    if (g != NULL) {
      if (fgetpos(g, size) == 0) {
        status = 0;
      }
      fclose(g);
    }
  }
  return status;
}

int main(int argc, char *argv[])
{
  printf("Filename %s\n", argv[1]);
  fpos_t size;
  if (file_size(argv[1], &size) == 0) {
    printf("Length of file %s is %ld!\n", argv[1], (long)size);
    return EXIT_SUCCESS;
  }
  else {
    printf("File %s not found!\n", argv[1]);
    return EXIT_FAILURE;
  }
}
