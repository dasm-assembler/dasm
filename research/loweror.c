/*
  the findext() function in main.c was full of the "| 0x20" stuff
  below; I used this to verify that the intention was to convert
  a character to lower case
*/

#include <ctype.h>
#include <stdio.h>

int main(void)
{
  for (int i = 0; i < 256; i++) {
    int raw = i;
    int low = i | 0x20;
    printf(" %c \t %c \n", isprint(raw) ? raw : 'X', isprint(low) ? low : 'Y');
  }
}
