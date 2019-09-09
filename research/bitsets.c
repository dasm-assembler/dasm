/*
  Experiments with bit sets.
*/

#include <stdio.h>
#include <assert.h>

/*
  Defining bit sets.

  Short of writing out own bit set abstraction, it seems
  obvious that we should use enum to generate the values.
  Also, splint can do a little checking if there's a new
  type involved.

  Some naming convention for bits and masks is necessary
  and below we append _B to bits and _M to masks. A mask
  can only be generated if a bit of that name has already
  been defined, another neat little check.
*/

#define BS_NEW_BIT(x) x ## _B

typedef enum
{
  BS_NEW_BIT(OK),
  BS_NEW_BIT(IFFY),
  BS_NEW_BIT(SOSO),
  BS_NEW_BIT(WRONG)
} elements;

#define BS_NEW_MASK(x) x ## _M = 1 << x ## _B

typedef enum
{
  BS_NEW_MASK(OK),
  BS_NEW_MASK(IFFY),
  BS_NEW_MASK(SOSO),
  BS_NEW_MASK(WRONG)
} masks;

/*
  Operations on bitsets.

  This is much more unstable territory. We obviously need
  support for testing whether bits and groups of bits are
  set or not. In what form to express that is unclear for
  now. The operations drafted below are not as simple to
  understand as I would like them yet. :-/
*/

#define BS_INCLUDE(set, mask) (set |= mask)
#define BS_EXCLUDE(set, mask) (set &= ~mask)
#define BS_INCLUDES_ONE(set, mask) ((set & mask) != 0)
#define BS_INCLUDES_ALL(set, mask) ((set & mask) == mask)
#define BS_EXCLUDES_ALL(set, mask) ((set & mask) == 0)
/* this is probably not correct */
#define BS_EXCLUDES_ONE(set, mask) ((set & mask) == ~mask)

int main(void)
{
  /* An empty bit set. */
  masks set = 0;

  /* Basic sanity checks. */
  assert(sizeof(elements) == sizeof(int));
  assert(sizeof(masks) == sizeof(int));
  assert(sizeof(set) == sizeof(int));

  assert(OK_B == 0);
  assert(IFFY_B == 1);
  assert(SOSO_B == 2);
  assert(WRONG_B == 3);

  assert(OK_M == 1);
  assert(IFFY_M == 2);
  assert(SOSO_M == 4);
  assert(WRONG_M == 8);

  /* Bit set operations on empty set. */
  assert(!BS_INCLUDES_ALL(set, OK_M));
  assert(!BS_INCLUDES_ALL(set, IFFY_M));
  assert(!BS_INCLUDES_ALL(set, SOSO_M));
  assert(!BS_INCLUDES_ALL(set, WRONG_M));

  assert(!BS_INCLUDES_ONE(set, OK_M));
  assert(!BS_INCLUDES_ONE(set, IFFY_M));
  assert(!BS_INCLUDES_ONE(set, SOSO_M));
  assert(!BS_INCLUDES_ONE(set, WRONG_M));

  /* Bit set operations on singleton set. */
  BS_INCLUDE(set, OK_M);

  assert(BS_INCLUDES_ALL(set, OK_M));
  assert(!BS_INCLUDES_ALL(set, IFFY_M));
  assert(!BS_INCLUDES_ALL(set, SOSO_M));
  assert(!BS_INCLUDES_ALL(set, WRONG_M));

  assert(BS_INCLUDES_ONE(set, OK_M));
  assert(!BS_INCLUDES_ONE(set, IFFY_M));
  assert(!BS_INCLUDES_ONE(set, SOSO_M));
  assert(!BS_INCLUDES_ONE(set, WRONG_M));

  /* Bit set operations on bigger set. */
  BS_INCLUDE(set, WRONG_M);

  assert(BS_INCLUDES_ALL(set, OK_M));
  assert(!BS_INCLUDES_ALL(set, IFFY_M));
  assert(!BS_INCLUDES_ALL(set, SOSO_M));
  assert(BS_INCLUDES_ALL(set, WRONG_M));

  assert(BS_INCLUDES_ONE(set, OK_M));
  assert(!BS_INCLUDES_ONE(set, IFFY_M));
  assert(!BS_INCLUDES_ONE(set, SOSO_M));
  assert(BS_INCLUDES_ONE(set, WRONG_M));

  assert(BS_INCLUDES_ALL(set, (OK_M|WRONG_M)));
  assert(!BS_INCLUDES_ALL(set, (SOSO_M|WRONG_M)));
  assert(!BS_INCLUDES_ALL(set, (OK_M|SOSO_M|WRONG_M)));
  assert(!BS_INCLUDES_ALL(set, (SOSO_M|IFFY_M)));

  assert(BS_INCLUDES_ONE(set, (OK_M|WRONG_M)));
  assert(BS_INCLUDES_ONE(set, (SOSO_M|WRONG_M)));
  assert(BS_INCLUDES_ONE(set, (OK_M|SOSO_M|WRONG_M)));
  assert(!BS_INCLUDES_ONE(set, (SOSO_M|IFFY_M)));
}
