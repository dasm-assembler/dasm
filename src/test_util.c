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
 *
 * Covers: checked_strdup, strlcpy, strlcat, strlower, strupper,
 * match_either_case, strip_whitespace, getprogname/setprogname,
 * small_alloc/small_free_all, and checked_malloc OOM panic (fork-isolated).
 */

#define _POSIX_C_SOURCE 200809L  /* fork, waitpid */

#include "util.h"
#include "errors.h"
#include "asm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* Fakes required by errors.c                                          */
/* ------------------------------------------------------------------ */

FILE    *FI_listfile = NULL;
char    *F_listfile  = NULL;
INCFILE *pIncfile    = NULL;

/* ------------------------------------------------------------------ */
/* Test infrastructure                                                  */
/* ------------------------------------------------------------------ */

static int tests_run    = 0;
static int tests_failed = 0;

#define CHECK(expr) do { \
    tests_run++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #expr); \
        tests_failed++; \
    } \
} while (0)

static int run_in_child(void (*fn)(void))
{
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) { fn(); exit(0); }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* ------------------------------------------------------------------ */
/* Tests: checked_strdup                                                */
/* ------------------------------------------------------------------ */

static void test_checked_strdup(void)
{
    char *s = checked_strdup("hello world");
    CHECK(s != NULL);
    CHECK(strcmp(s, "hello world") == 0);
    free(s);

    /* empty string */
    s = checked_strdup("");
    CHECK(s != NULL);
    CHECK(strcmp(s, "") == 0);
    free(s);
}

/* ------------------------------------------------------------------ */
/* Tests: strlcpy                                                       */
/* ------------------------------------------------------------------ */

static void test_strlcpy(void)
{
    char dst[16];
    size_t res;

    /* normal copy */
    res = strlcpy(dst, "Hello!", sizeof(dst));
    CHECK(res == 6);
    CHECK(strcmp(dst, "Hello!") == 0);

    /* truncation: src longer than dst */
    char small[5] = {'X', 'X', 'X', 'X', 'X'};
    res = strlcpy(small, "Hello 47!", sizeof(small) - 1);  /* size=4 */
    CHECK(res == 9);            /* returns full src length */
    CHECK(small[0] == 'H');
    CHECK(small[1] == 'e');
    CHECK(small[2] == 'l');
    CHECK(small[3] == '\0');    /* NUL-terminated within size */
    CHECK(small[4] == 'X');     /* byte beyond size untouched */

    /* exact fit */
    res = strlcpy(dst, "12345678901234", sizeof(dst));  /* 14 chars + NUL = 15 */
    CHECK(res == 14);
    CHECK(strlen(dst) == 14);
}

/* ------------------------------------------------------------------ */
/* Tests: strlcat                                                       */
/* ------------------------------------------------------------------ */

static void test_strlcat(void)
{
    char dst[16];
    size_t res;

    strlcpy(dst, "Hello", sizeof(dst));
    res = strlcat(dst, " World", sizeof(dst));
    CHECK(res == 11);                          /* total would-be length */
    CHECK(strcmp(dst, "Hello World") == 0);

    /* truncation */
    strlcpy(dst, "Hello", sizeof(dst));
    res = strlcat(dst, " World!!!!", 8);       /* room for 2 more chars */
    CHECK(res == 15);                          /* would-be total */
    CHECK(strcmp(dst, "Hello W") == 0);        /* truncated, NUL-terminated */

    /* appending to empty */
    dst[0] = '\0';
    res = strlcat(dst, "abc", sizeof(dst));
    CHECK(res == 3);
    CHECK(strcmp(dst, "abc") == 0);
}

/* ------------------------------------------------------------------ */
/* Tests: strlower / strupper                                           */
/* ------------------------------------------------------------------ */

static void test_strlower(void)
{
    char src[64];
    char dst[64];
    size_t res;

    strlcpy(src, "Hello 47!", sizeof(src));
    res = strlower(dst, src, sizeof(dst));
    CHECK(res == 9);
    CHECK(strcmp(dst, "hello 47!") == 0);
    CHECK(strcmp(src, "Hello 47!") == 0);  /* src unchanged */

    /* already lowercase */
    res = strlower(dst, "already", sizeof(dst));
    CHECK(strcmp(dst, "already") == 0);

    /* all uppercase */
    res = strlower(dst, "DASM", sizeof(dst));
    CHECK(strcmp(dst, "dasm") == 0);
    CHECK(res == 4);

    /* truncation */
    char small[4];
    res = strlower(small, "HELLO", sizeof(small));
    CHECK(res == 5);               /* full would-be length */
    CHECK(strcmp(small, "hel") == 0);
}

static void test_strupper(void)
{
    char dst[64];
    size_t res;

    res = strupper(dst, "hello 47!", sizeof(dst));
    CHECK(res == 9);
    CHECK(strcmp(dst, "HELLO 47!") == 0);

    res = strupper(dst, "ALREADY", sizeof(dst));
    CHECK(strcmp(dst, "ALREADY") == 0);

    res = strupper(dst, "dasm", sizeof(dst));
    CHECK(strcmp(dst, "DASM") == 0);
}

/* ------------------------------------------------------------------ */
/* Tests: match_either_case                                             */
/* ------------------------------------------------------------------ */

static void test_match_either_case(void)
{
    char lower[64];
    char upper[64];
    strlower(lower, "hello 47!", sizeof(lower));
    strupper(upper, "hello 47!", sizeof(upper));

    CHECK( match_either_case(lower, "hello 47!"));
    CHECK( match_either_case(upper, "hello 47!"));

    /* must not match different strings */
    CHECK(!match_either_case(lower, "WORLD"));
    CHECK(!match_either_case(upper, "world"));

    /* empty strings */
    char empty[4] = {0};
    strlower(empty, "", sizeof(empty));
    CHECK( match_either_case(empty, ""));
    CHECK(!match_either_case(empty, "x"));
}

/* ------------------------------------------------------------------ */
/* Tests: strip_whitespace                                              */
/* ------------------------------------------------------------------ */

static void test_strip_whitespace(void)
{
    char src[64];
    char dst[64];
    size_t res;

    /* pure whitespace strips to empty */
    strlcpy(src, "\r\t\n    ", sizeof(src));
    res = strip_whitespace(dst, src, sizeof(dst));
    CHECK(res == 0);
    CHECK(dst[0] == '\0');

    /* leading/trailing whitespace stripped, internal removed */
    strlcpy(src, "\rThere are\tso\nmany good things!    ", sizeof(src));
    res = strip_whitespace(dst, src, sizeof(dst));
    CHECK(res == 25);
    CHECK(strcmp(dst, "Therearesomanygoodthings!") == 0);

    /* no whitespace at all */
    res = strip_whitespace(dst, "hello", sizeof(dst));
    CHECK(res == 5);
    CHECK(strcmp(dst, "hello") == 0);

    /* truncation: dst too small */
    char small[5] = {'X', 'X', 'X', 'X', 'X'};
    strlcpy(src, "\rThere are\tso\nmany good things!    ", sizeof(src));
    res = strip_whitespace(small, src, sizeof(small) - 1);  /* size=4 */
    CHECK(res == 25);       /* full would-be length */
    CHECK(small[0] == 'T');
    CHECK(small[1] == 'h');
    CHECK(small[2] == 'e');
    CHECK(small[3] == '\0');
    CHECK(small[4] == 'X');  /* untouched */
}

/* ------------------------------------------------------------------ */
/* Tests: getprogname / setprogname                                     */
/* ------------------------------------------------------------------ */

static void test_progname(void)
{
    setprogname("/usr/local/bin/dasm");
    CHECK(strcmp(getprogname(), "dasm") == 0);  /* directory stripped */

    setprogname("dasm");
    CHECK(strcmp(getprogname(), "dasm") == 0);  /* no directory */

    setprogname("/a/b/c/foo");
    CHECK(strcmp(getprogname(), "foo") == 0);

    /* Windows-style separator doesn't apply on Unix, but the path
       separator logic should not crash on unusual inputs */
    setprogname("nopath");
    CHECK(strcmp(getprogname(), "nopath") == 0);
}

/* ------------------------------------------------------------------ */
/* Tests: small_alloc / small_free_all                                  */
/* ------------------------------------------------------------------ */

static void test_small_alloc(void)
{
    /* multiple small allocations return non-overlapping pointers */
    void *a = small_alloc(16);
    void *b = small_alloc(16);
    void *c = small_alloc(32);
    CHECK(a != NULL);
    CHECK(b != NULL);
    CHECK(c != NULL);
    CHECK(a != b);
    CHECK(b != c);
    CHECK(a != c);
    /* later allocations must be at higher addresses */
    CHECK((char *)b >= (char *)a + 16);
    CHECK((char *)c >= (char *)b + 16);

    /* write and read back without corruption */
    memset(a, 0xAA, 16);
    memset(b, 0xBB, 16);
    memset(c, 0xCC, 32);
    CHECK(((unsigned char *)a)[0]  == 0xAA);
    CHECK(((unsigned char *)b)[0]  == 0xBB);
    CHECK(((unsigned char *)c)[0]  == 0xCC);
    CHECK(((unsigned char *)a)[15] == 0xAA);
    CHECK(((unsigned char *)b)[15] == 0xBB);
    CHECK(((unsigned char *)c)[31] == 0xCC);

    /* allocation across a block boundary (request > remaining space) */
    void *big = small_alloc(8192);  /* forces a new internal block */
    CHECK(big != NULL);

    small_free_all();

    /* after free_all, can allocate again */
    void *fresh = small_alloc(64);
    CHECK(fresh != NULL);
    small_free_all();
}

/* ------------------------------------------------------------------ */
/* Tests: checked_malloc OOM (fork-isolated)                            */
/* ------------------------------------------------------------------ */

static void _oom_child(void)
{
    /* Request more memory than any realistic system can provide.
       checked_malloc must call panic_fmt and exit non-zero. */
    const size_t HUGE = ((size_t)1) << 40;   /* 1 TiB */
    (void) checked_malloc(HUGE);
    /* should not reach here */
}

static void test_checked_malloc_oom(void)
{
    int status = run_in_child(_oom_child);
    CHECK(status != 0);  /* panic exits non-zero */
}

/* ------------------------------------------------------------------ */
/* Tests: checked_malloc normal path                                    */
/* ------------------------------------------------------------------ */

static void test_checked_malloc_normal(void)
{
    void *p = checked_malloc(1);
    CHECK(p != NULL);
    free(p);

    p = checked_malloc(4096);
    CHECK(p != NULL);
    memset(p, 0x5A, 4096);
    CHECK(((unsigned char *)p)[0]    == 0x5A);
    CHECK(((unsigned char *)p)[4095] == 0x5A);
    free(p);
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    (void)argc;
    setprogname(argv[0]);
    set_error_level(ERRORLEVEL_WARNING);   /* suppress debug spam from small_alloc */

    test_checked_strdup();
    test_strlcpy();
    test_strlcat();
    test_strlower();
    test_strupper();
    test_match_either_case();
    test_strip_whitespace();
    test_progname();
    test_small_alloc();
    test_checked_malloc_normal();
    test_checked_malloc_oom();

    if (tests_failed == 0) {
        printf("test_util: all %d tests passed.\n", tests_run);
        return EXIT_SUCCESS;
    } else {
        printf("test_util: %d/%d tests FAILED.\n", tests_failed, tests_run);
        return EXIT_FAILURE;
    }
}

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
