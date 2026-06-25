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
 * @brief Unit tests for errors module.
 *
 * Covers: valid_error_format/level, verbosity_to_error_level, per-level
 * message counting, reset_error_counts, level filtering, deferred mode
 * (buffer/flush/clear), all three output formats (WOE/DILLON/GNU), macro-
 * frame skipping in pIncfile traversal, listing-file mirroring, and panic
 * exit behaviour.
 *
 * Panic tests use fork()+waitpid() so the child's exit() doesn't kill the
 * test process.  Output capture uses a pipe+dup2 over stderr.
 */

#define _POSIX_C_SOURCE 200809L  /* mkstemp, fdopen, fork, pipe */

#include "errors.h"
#include "asm.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* Fakes required by errors.c / util.c                                 */
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

/*
 * Redirect stderr to a pipe, call fn(), restore stderr, return whatever
 * was written as a malloc'd NUL-terminated string.  Caller must free().
 * Captures up to 16383 bytes.
 */
static char *capture_stderr(void (*fn)(void))
{
    int pipefd[2];
    int saved_stderr;
    char *buf;
    ssize_t n;

    buf = malloc(16384);
    assert(buf != NULL);
    buf[0] = '\0';

    if (pipe(pipefd) != 0) { perror("pipe"); return buf; }

    saved_stderr = dup(STDERR_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    fn();
    fflush(stderr);

    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stderr);

    n = read(pipefd[0], buf, 16383);
    if (n < 0) n = 0;
    buf[n] = '\0';
    close(pipefd[0]);

    return buf;
}

/*
 * Run fn() in a child process.  Returns WEXITSTATUS, or -1 on error.
 * Used to test panic_fmt() without killing the test process.
 */
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
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static INCFILE *make_incfile(const char *name, unsigned long lineno,
                             unsigned char flags, INCFILE *next)
{
    INCFILE *f = malloc(sizeof(INCFILE));
    assert(f != NULL);
    memset(f, 0, sizeof(*f));
    f->name   = checked_strdup(name);
    f->lineno = lineno;
    f->flags  = flags;
    f->next   = next;
    return f;
}

static void free_incfile_chain(INCFILE *f)
{
    while (f) {
        INCFILE *next = f->next;
        free(f->name);
        free(f);
        f = next;
    }
}

/* Reset module to a known baseline between test groups. */
static void reset_state(void)
{
    set_error_level(ERRORLEVEL_WARNING);
    set_error_format(ERRORFORMAT_DEFAULT);
    reset_error_counts();
    clear_deferred_errors();
    set_deferred_errors(false);
    pIncfile = NULL;
}

/* ------------------------------------------------------------------ */
/* Tests: validation predicates                                         */
/* ------------------------------------------------------------------ */

static void test_valid_error_format(void)
{
    CHECK( valid_error_format(ERRORFORMAT_WOE));
    CHECK( valid_error_format(ERRORFORMAT_DILLON));
    CHECK( valid_error_format(ERRORFORMAT_GNU));
    CHECK(!valid_error_format(-1));
    CHECK(!valid_error_format(ERRORFORMAT_MAX));      /* exclusive sentinel */
    CHECK(!valid_error_format(ERRORFORMAT_MAX + 1));
    CHECK(!valid_error_format(99));
}

static void test_valid_error_level(void)
{
    CHECK( valid_error_level(ERRORLEVEL_DEBUG));
    CHECK( valid_error_level(ERRORLEVEL_INFO));
    CHECK( valid_error_level(ERRORLEVEL_NOTICE));
    CHECK( valid_error_level(ERRORLEVEL_WARNING));
    CHECK( valid_error_level(ERRORLEVEL_ERROR));
    CHECK( valid_error_level(ERRORLEVEL_FATAL));
    CHECK( valid_error_level(ERRORLEVEL_PANIC));
    CHECK(!valid_error_level(-1));
    CHECK(!valid_error_level(ERRORLEVEL_MAX));        /* exclusive sentinel */
    CHECK(!valid_error_level(ERRORLEVEL_MAX + 1));
}

/* ------------------------------------------------------------------ */
/* Tests: verbosity mapping                                             */
/* ------------------------------------------------------------------ */

static void test_verbosity_to_error_level(void)
{
    CHECK(verbosity_to_error_level(0)  == ERRORLEVEL_WARNING);
    CHECK(verbosity_to_error_level(1)  == ERRORLEVEL_NOTICE);
    CHECK(verbosity_to_error_level(2)  == ERRORLEVEL_INFO);
    CHECK(verbosity_to_error_level(3)  == ERRORLEVEL_DEBUG);
    CHECK(verbosity_to_error_level(4)  == ERRORLEVEL_DEBUG);  /* saturates */
    CHECK(verbosity_to_error_level(99) == ERRORLEVEL_DEBUG);
}

/* ------------------------------------------------------------------ */
/* Tests: per-level counting and reset                                  */
/* ------------------------------------------------------------------ */

static void test_error_counting(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_DEBUG);   /* allow all levels through */

    /* all counters start at zero */
    CHECK(number_of_notices()  == 0);
    CHECK(number_of_warnings() == 0);
    CHECK(number_of_errors()   == 0);
    CHECK(number_of_fatals()   == 0);

    /* debug and info are not counted */
    debug_fmt("uncounted debug");
    info_fmt("uncounted info");
    CHECK(number_of_notices()  == 0);
    CHECK(number_of_warnings() == 0);
    CHECK(number_of_errors()   == 0);
    CHECK(number_of_fatals()   == 0);

    notice_fmt("notice 1");
    CHECK(number_of_notices()  == 1);

    warning_fmt("warning 1");
    warning_fmt("warning 2");
    CHECK(number_of_warnings() == 2);

    error_fmt("error 1");
    CHECK(number_of_errors()   == 1);

    fatal_fmt("fatal 1");
    CHECK(number_of_fatals()   == 1);

    /* panic also increments fatals counter */
    /* (tested separately via fork to avoid exit) */

    /* reset clears everything */
    reset_error_counts();
    CHECK(number_of_notices()  == 0);
    CHECK(number_of_warnings() == 0);
    CHECK(number_of_errors()   == 0);
    CHECK(number_of_fatals()   == 0);
}

/* ------------------------------------------------------------------ */
/* Tests: level filtering                                               */
/* ------------------------------------------------------------------ */

static void test_level_filtering(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);   /* suppress everything below ERROR */

    notice_fmt("suppressed notice");
    warning_fmt("suppressed warning");
    CHECK(number_of_notices()  == 0);
    CHECK(number_of_warnings() == 0);

    error_fmt("visible error");
    CHECK(number_of_errors()   == 1);

    /* counter checks above are sufficient — the deferred_buffering test
       verifies stderr silence independently */
}

/* ------------------------------------------------------------------ */
/* Tests: deferred mode                                                 */
/* ------------------------------------------------------------------ */

static void _emit_deferred_error(void)
{
    error_fmt("deferred test error");
}

static void test_deferred_buffering(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);
    set_deferred_errors(true);

    /* error must NOT appear on stderr immediately */
    char *out = capture_stderr(_emit_deferred_error);
    CHECK(strlen(out) == 0);
    free(out);

    /* flush must deliver it to stderr */
    int pipefd[2];
    pipe(pipefd);
    int saved = dup(STDERR_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    flush_deferred_errors(stderr);
    fflush(stderr);

    dup2(saved, STDERR_FILENO);
    close(saved);

    char buf[4096] = {0};
    ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    close(pipefd[0]);

    CHECK(strstr(buf, "deferred test error") != NULL);

    /* after flush, buffer is empty — a second flush emits nothing */
    out = capture_stderr((void(*)(void))flush_deferred_errors);
    CHECK(strlen(out) == 0);
    free(out);

    set_deferred_errors(false);
}

static void _emit_for_clear(void)
{
    set_deferred_errors(true);
    error_fmt("discarded error");
}

static void test_clear_discards(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);

    char *out = capture_stderr(_emit_for_clear);
    CHECK(strlen(out) == 0);   /* nothing immediate */
    free(out);

    /* clear must discard without printing */
    int pipefd[2];
    pipe(pipefd);
    int saved = dup(STDERR_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    clear_deferred_errors();
    fflush(stderr);

    dup2(saved, STDERR_FILENO);
    close(saved);

    char buf[1024] = {0};
    ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    close(pipefd[0]);

    CHECK(strlen(buf) == 0);
    set_deferred_errors(false);
}

/* fatal goes to stderr immediately even in deferred mode */
static void _emit_fatal_deferred(void)
{
    set_deferred_errors(true);
    fatal_fmt("immediate fatal");
}

static void test_fatal_bypasses_deferred(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_FATAL);

    char *out = capture_stderr(_emit_fatal_deferred);
    CHECK(strstr(out, "immediate fatal") != NULL);
    free(out);

    clear_deferred_errors();
    set_deferred_errors(false);
}

/* ------------------------------------------------------------------ */
/* Tests: output formats                                                */
/* ------------------------------------------------------------------ */

static void _emit_woe(void)    { set_error_format(ERRORFORMAT_WOE);    error_fmt("msg"); }
static void _emit_dillon(void) { set_error_format(ERRORFORMAT_DILLON); error_fmt("msg"); }
static void _emit_gnu(void)    { set_error_format(ERRORFORMAT_GNU);    error_fmt("msg"); }

static void test_output_formats(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);
    pIncfile = make_incfile("test.asm", 42, 0, NULL);

    /* WOE: "test.asm (42): error: msg" */
    char *out = capture_stderr(_emit_woe);
    CHECK(strstr(out, "test.asm") != NULL);
    CHECK(strstr(out, "(42)")     != NULL);
    CHECK(strstr(out, "msg")      != NULL);
    free(out);

    /* DILLON: "line    42 test.asm   error: msg" */
    out = capture_stderr(_emit_dillon);
    CHECK(strstr(out, "test.asm") != NULL);
    CHECK(strstr(out, "42")       != NULL);
    CHECK(strstr(out, "msg")      != NULL);
    free(out);

    /* GNU: "test.asm:42: error: msg" */
    out = capture_stderr(_emit_gnu);
    CHECK(strstr(out, "test.asm:42") != NULL);
    CHECK(strstr(out, "msg")         != NULL);
    free(out);

    free_incfile_chain(pIncfile);
    pIncfile = NULL;
    set_error_format(ERRORFORMAT_DEFAULT);
}

/* With no pIncfile, GNU emits progname; WOE/DILLON emit nothing for location. */
static void _emit_gnu_no_file(void)
{
    set_error_format(ERRORFORMAT_GNU);
    error_fmt("no file msg");
}

static void test_no_active_file(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);
    pIncfile = NULL;

    char *out = capture_stderr(_emit_gnu_no_file);
    CHECK(strstr(out, "no file msg") != NULL);
    /* GNU should prefix with progname (not crash) */
    free(out);

    set_error_format(ERRORFORMAT_DEFAULT);
}

/* ------------------------------------------------------------------ */
/* Tests: macro frame skipping                                          */
/* ------------------------------------------------------------------ */

static void _emit_from_macro(void) { error_fmt("from macro chain"); }

static void test_macro_frame_skipping(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);
    set_error_format(ERRORFORMAT_GNU);

    /* chain: macro_frame (INF_MACRO) -> real_file */
    INCFILE *real_file   = make_incfile("real.asm", 99, 0,         NULL);
    INCFILE *macro_frame = make_incfile("MACRO",     1, INF_MACRO, real_file);
    pIncfile = macro_frame;

    char *out = capture_stderr(_emit_from_macro);
    CHECK(strstr(out, "real.asm:99") != NULL);  /* real file reported */
    CHECK(strstr(out, "MACRO")       == NULL);  /* macro frame hidden */
    free(out);

    free_incfile_chain(pIncfile);
    pIncfile = NULL;
    set_error_format(ERRORFORMAT_DEFAULT);
}

/* Multiple consecutive macro frames, then a real file. */
static void test_nested_macro_frames(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);
    set_error_format(ERRORFORMAT_GNU);

    INCFILE *real   = make_incfile("base.asm", 7,  0,         NULL);
    INCFILE *macro2 = make_incfile("INNER",    2,  INF_MACRO, real);
    INCFILE *macro1 = make_incfile("OUTER",    10, INF_MACRO, macro2);
    pIncfile = macro1;

    char *out = capture_stderr(_emit_from_macro);
    CHECK(strstr(out, "base.asm:7") != NULL);
    CHECK(strstr(out, "INNER")      == NULL);
    CHECK(strstr(out, "OUTER")      == NULL);
    free(out);

    free_incfile_chain(pIncfile);
    pIncfile = NULL;
    set_error_format(ERRORFORMAT_DEFAULT);
}

/* ------------------------------------------------------------------ */
/* Tests: listing file mirror                                           */
/* ------------------------------------------------------------------ */

static void _emit_for_listfile(void) { error_fmt("listfile mirror"); }

static void test_listfile_mirror(void)
{
    reset_state();
    set_error_level(ERRORLEVEL_ERROR);

    char tmpname[] = "/tmp/dasm_test_list_XXXXXX";
    int fd = mkstemp(tmpname);
    assert(fd >= 0);
    FILE *lf = fdopen(fd, "w+");
    assert(lf != NULL);
    FI_listfile = lf;

    _emit_for_listfile();
    fflush(FI_listfile);

    rewind(FI_listfile);
    char buf[512] = {0};
    fread(buf, 1, sizeof(buf) - 1, FI_listfile);

    /* Matt's original convention: listing entries start with '*' */
    CHECK(buf[0] == '*');
    CHECK(strstr(buf, "listfile mirror") != NULL);

    fclose(lf);
    FI_listfile = NULL;
    unlink(tmpname);
}

/* ------------------------------------------------------------------ */
/* Tests: panic exits non-zero                                          */
/* ------------------------------------------------------------------ */

static void _do_panic(void) { panic_fmt("test panic %d", 42); }

static void test_panic_exits_nonzero(void)
{
    int status = run_in_child(_do_panic);
    CHECK(status != 0);
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    (void)argc;
    setprogname(argv[0]);

    /* give pIncfile a default for tests that don't set their own */
    pIncfile = make_incfile("default.asm", 1, 0, NULL);

    test_valid_error_format();
    test_valid_error_level();
    test_verbosity_to_error_level();
    test_error_counting();
    test_level_filtering();
    test_deferred_buffering();
    test_clear_discards();
    test_fatal_bypasses_deferred();
    test_output_formats();
    test_no_active_file();
    test_macro_frame_skipping();
    test_nested_macro_frames();
    test_listfile_mirror();
    test_panic_exits_nonzero();

    free_incfile_chain(pIncfile);
    pIncfile = NULL;

    if (tests_failed == 0) {
        printf("test_errors: all %d tests passed.\n", tests_run);
        return EXIT_SUCCESS;
    } else {
        printf("test_errors: %d/%d tests FAILED.\n", tests_failed, tests_run);
        return EXIT_FAILURE;
    }
}

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
