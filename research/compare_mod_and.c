/*
    Simple hack to compare performance of % and & on modern
    machines. Seems that & still wins on a wide variety of
    machines, so we'll stick with it for DASM's compression
    step in hashing.

    1.5 GHz PowerPC G4:

        -O3, MAX 1<<31

            Takes 5 seconds to do 2147483648*4 modulos.
        ?   Takes 5 seconds to do 2147483648*4 ands.
        ?   Takes 3 seconds to do 2147483648*4 ands and minuses.

        -O0, MAX 1<<27

            Takes 11 seconds to do 134217728*4 modulos.
            Takes 3 seconds to do 134217728*4 ands.
        ?   Takes 2 seconds to do 134217728*4 ands and minuses.

    ugradx i686:

        -O3, MAX 1<<31

            Takes 2 seconds to do 2147483648*4 modulos.
            Takes 1 seconds to do 2147483648*4 ands.
            Takes 1 seconds to do 2147483648*4 ands and minuses.

        -O0, MAX 1<<27

            Takes 8 seconds to do 134217728*4 modulos.
            Takes 1 seconds to do 134217728*4 ands.
            Takes 1 seconds to do 134217728*4 ands and minuses.
*/

#include <stdio.h>
#include <time.h>

#define MAX 1<<27

double time_mod(void)
{
  unsigned a, b, c, i, d = 13;
  time_t before, after;

  before = time(NULL);

  for (i = 0; i < MAX; i++) {
    a = i % d;
    b = i % d;
    c = i % d;
    a = i % d;
  }

  after = time(NULL);
  return difftime(after, before);
}

double time_and(void)
{
  unsigned a, b, c, i, d = 13;
  time_t before, after;

  before = time(NULL);

  for (i = 0; i < MAX; i++) {
    a = i & d;
    b = i & d;
    c = i & d;
    a = i & d;
  }

  after = time(NULL);
  return difftime(after, before);
}

double time_and_minus(void)
{
  unsigned a, b, c, i, d = 13;
  time_t before, after;

  before = time(NULL);

  for (i = 0; i < MAX; i++) {
    a = i & (d-1);
    b = i & (d-1);
    c = i & (d-1);
    a = i & (d-1);
  }

  after = time(NULL);
  return difftime(after, before);
}

/*
    Added approximations of the actual hash functions that
    would be used. Seems that Dan's is a *tiny* bit slower
    (figures since there's an extra + in there) but in the
    actual program we have to weigh better distribution of
    hash values against that. Still to actually measure...
*/

double time_dillon(void)
{
  unsigned a = 0, b, d = 13, *c = &d, i;
  time_t before, after;

  before = time(NULL);

  for (i = 0; i < MAX; i++) {
    a = (a << 2) + *c; /* ++ is missing :-/ */
    a = (a << 2) + *c;
    a = (a << 2) + *c;
    a = (a << 2) + *c;
  }

  after = time(NULL);
  return difftime(after, before);
}

double time_bernstein(void)
{
  unsigned a = 0, b, d = 13, *c = &d, i;
  time_t before, after;

  before = time(NULL);

  for (i = 0; i < MAX; i++) {
    a = ((a << 5) + a) + *c; /* ++ is missing :-/ */
    a = ((a << 5) + a) + *c; /* ++ is missing :-/ */
    a = ((a << 5) + a) + *c; /* ++ is missing :-/ */
    a = ((a << 5) + a) + *c; /* ++ is missing :-/ */
  }

  after = time(NULL);
  return difftime(after, before);
}

int main(void)
{
  printf("Takes %g seconds to do %u*4 modulos.\n",
         time_mod(), MAX);
  printf("Takes %g seconds to do %u*4 ands.\n",
         time_and(), MAX);
  printf("Takes %g seconds to do %u*4 ands and minuses.\n",
         time_and_minus(), MAX);
  printf("Takes %g seconds to do %u*4 dillons.\n",
         time_dillon(), MAX);
  printf("Takes %g seconds to do %u*4 bernsteins.\n",
         time_bernstein(), MAX);
}
