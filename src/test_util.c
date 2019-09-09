/*
    the DASM macro assembler (aka small systems cross assembler)

    Copyright (c) 1988-2002 by Matthew Dillon.
    Copyright (c) 1995 by Olaf "Rhialto" Seibert.
    Copyright (c) 2003-2008 by Andrew Davie.
    Copyright (c) 2008 by Peter H. Froehlich.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/**
 * @file
 * @brief Unit tests for util module.
 * @todo how do we keep panic* from aborting while unit test
 * runs? or maybe it doesn't matter if we switch to Check?
 */

#include "util.h"

#include "asm.h"
#include "errors.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if 0
#if defined(TEST)
/* hack for unit tests where we don't want to exit! */
void new_panic(error_t _error, const char *s)
{
    (void) printf("Simulated Panic: %s\n", s);
}
void debug(error_t _error, const char *s)
{
    (void) printf("Simulated Debug: %s\n", s);
}
#endif /* defined(TEST) */
#endif

/* fakes for unit test */
FILE *FI_listfile = NULL;
char *F_listfile = NULL;
INCFILE *pIncfile = NULL;

int main(int argc, char *argv[])
{
    char *one;
    char *two;
    union align { long l; void *p; void (*fp)(void); };
    const size_t BIG = ((size_t) 1) << 31;
    char buffer[256];
    char fuffer[256];
    char poop[5] = {'a', 'b', 'c', 'd', 'X'};
    size_t res;
    char* excellent = NULL;
    assert(argc == 1);
    setprogname(argv[0]);

    /* test some string functions */
    excellent = checked_strdup("Peter's own thing!");
    assert(excellent != NULL);
    assert(strcmp(excellent, "Peter's own thing!") == 0);

    res = strlcpy(poop, "Hello 47!", sizeof(poop)-1);
    assert(res == 9);
    assert(poop[0] == 'H');
    assert(poop[1] == 'e');
    assert(poop[2] == 'l');
    assert(poop[3] == '\0');
    assert(poop[4] == 'X');

    res = strlcpy(buffer, "Hello 47!", sizeof(buffer));
    assert(res == 9);
    assert(strcmp(buffer, "Hello 47!") == 0);

    assert(!match_either_case(buffer, "hello 47!"));

    res = strlower(fuffer, buffer, sizeof(fuffer));
    assert(res == 9);
    assert(strcmp(fuffer, "hello 47!") == 0);

    assert(match_either_case(fuffer, "hello 47!"));

    res = strupper(buffer, fuffer, sizeof(buffer));
    assert(res == 9);
    assert(strcmp(buffer, "HELLO 47!") == 0);

    assert(match_either_case(buffer, "hello 47!"));

    res = strlcpy(buffer, "\r\t\n    ", sizeof(buffer));
    assert(res == 7);

    res = strip_whitespace(poop, buffer, sizeof(poop)-1);
    assert(res == 0);
    assert(poop[0] == '\0');
    assert(poop[1] == 'e');
    assert(poop[2] == 'l');
    assert(poop[3] == '\0');
    assert(poop[4] == 'X');

    res = strlcpy(buffer, "\rThere are\tso\nmany good things!    ", sizeof(buffer));
    assert(res == 35);

    res = strip_whitespace(poop, buffer, sizeof(poop)-1);
    assert(res == 25);
    assert(poop[0] == 'T');
    assert(poop[1] == 'h');
    assert(poop[2] == 'e');
    assert(poop[3] == '\0');
    assert(poop[4] == 'X');

    res = strip_whitespace(fuffer, buffer, sizeof(fuffer));
    assert(res == 25);
    assert(strcmp(fuffer, "Therearesomanygoodthings!") == 0);

    /* fake a current file */
    pIncfile = malloc(sizeof(INCFILE));
    pIncfile->next = NULL;
    pIncfile->name = checked_strdup("someFileName");
    pIncfile->lineno = 47;
    /* enable all messages */
    set_error_level(ERRORLEVEL_DEBUG);
    /* the tests */
    puts(getprogname());
    setprogname(argv[0]);
    puts(getprogname());
    printf("sizeof(align)==%zu\n", sizeof(union align));
    one = checked_malloc(BIG);
    puts("First malloc()ed!");
    two = checked_malloc(BIG);
    puts("Second malloc()ed! Should have caused a panic?");
    if (two) free(two);
    puts("Second free()d!");
    if (one) free(one);
    puts("First free()d!");
    one = small_alloc(4096);
    printf("First small_alloc()ed returned %p!\n", one);
    two = small_alloc(4096);
    assert(one < two);
    printf("Second small_alloc()ed returned %p!\n", two);
    one = small_alloc(10240);
    small_free_all();
    return EXIT_SUCCESS;
}

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
