/*
    Test potentially unportable get/setprogname(3) functions.

    WORKS:
        OS X
    BREAKS:
        Linux, Solaris
    TODO:
        Windows, NetBSD, ...
*/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  printf("argv[0] == %s\n", argv[0]);
  printf("before setprogname(): getprogname() == %s\n", getprogname());
  setprogname(argv[0]);
  printf("after setprogname(): getprogname() == %s\n", getprogname());
  return EXIT_SUCCESS;
}
