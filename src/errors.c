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
 * @brief Error reporting implementation for DASM.
 */

#include "errors.h"

#include "asm.h"
#include "util.h"
#include "version.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ------------------------------------------------------------------ */
/* Module state                                                         */
/* ------------------------------------------------------------------ */

static error_format_t F_error_format = ERRORFORMAT_DEFAULT;
static error_level_t  F_error_level  = ERRORLEVEL_DEFAULT;

static size_t nof_fatals   = 0;
static size_t nof_errors   = 0;
static size_t nof_warnings = 0;
static size_t nof_notices  = 0;

char source_location_buffer[SOURCE_LOCATION_LENGTH];

/*
 * Human-readable level names, indexed by error_level_t.
 * Used in the formatted message prefix: "file:line: warning: ..."
 */
static const char *level_names[] =
{
    [ERRORLEVEL_DEBUG]   = "debug",
    [ERRORLEVEL_INFO]    = "info",
    [ERRORLEVEL_NOTICE]  = "notice",
    [ERRORLEVEL_WARNING] = "warning",
    [ERRORLEVEL_ERROR]   = "error",
    [ERRORLEVEL_FATAL]   = "fatal",
    [ERRORLEVEL_PANIC]   = "***panic***",
};


/* ------------------------------------------------------------------ */
/* Deferred (pass-buffered) output state                                */
/* ------------------------------------------------------------------ */

static bool   use_deferred    = false;
static char  *deferred_buf    = NULL;
static size_t deferred_cap    = 0;
static size_t deferred_len    = 0;

/*
 * Internal buffer size for formatting individual message components.
 * Sized to hold a full source path + long operand string comfortably.
 */
#define MSG_BUF_SIZE 4096


/* ------------------------------------------------------------------ */
/* Validation and configuration                                         */
/* ------------------------------------------------------------------ */

bool valid_error_format(int format)
{
    return (format >= ERRORFORMAT_MIN && format < ERRORFORMAT_MAX);
}

void set_error_format(error_format_t format)
{
    assert(valid_error_format(format));
    F_error_format = format;
}

bool valid_error_level(int level)
{
    return (level >= ERRORLEVEL_MIN && level < ERRORLEVEL_MAX);
}

void set_error_level(error_level_t level)
{
    assert(valid_error_level(level));
    F_error_level = level;
}

/**
 * @brief Map -v verbosity integer to error_level_t.
 *
 * Verbosity 0 (default): show warnings and above.
 * Verbosity 1          : show notices and above.
 * Verbosity 2          : show info and above.
 * Verbosity 3+         : show everything including debug.
 */
error_level_t verbosity_to_error_level(int verbosity)
{
    switch (verbosity) {
        case 0:  return ERRORLEVEL_WARNING;
        case 1:  return ERRORLEVEL_NOTICE;
        case 2:  return ERRORLEVEL_INFO;
        default: return ERRORLEVEL_DEBUG;
    }
}

static bool visible_error_level(error_level_t level)
{
    return (level >= F_error_level);
}


/* ------------------------------------------------------------------ */
/* Statistics                                                           */
/* ------------------------------------------------------------------ */

size_t number_of_fatals(void)   { return nof_fatals;   }
size_t number_of_errors(void)   { return nof_errors;   }
size_t number_of_warnings(void) { return nof_warnings; }
size_t number_of_notices(void)  { return nof_notices;  }

void reset_error_counts(void)
{
    nof_fatals   = 0;
    nof_errors   = 0;
    nof_warnings = 0;
    nof_notices  = 0;
}


/* ------------------------------------------------------------------ */
/* Deferred output                                                      */
/* ------------------------------------------------------------------ */

void set_deferred_errors(bool deferred)
{
    use_deferred = deferred;
}

void flush_deferred_errors(FILE *fp)
{
    if (deferred_buf != NULL && deferred_len > 0) {
        fputs(deferred_buf, fp);
    }
    clear_deferred_errors();
}

void clear_deferred_errors(void)
{
    deferred_len = 0;
    if (deferred_buf != NULL)
        deferred_buf[0] = '\0';
}


/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * Super low-level panic for disasters *inside* the errors module itself.
 * Cannot use notify_fmt() here — that would recurse.
 */
static void internal_panic(const char *message)
{
    fprintf(stderr,
            "\n%s: FATAL INTERNAL PANIC (errors.c): %s\n\n",
            getprogname(),
            message);
    exit(EXIT_FAILURE);
}

/*
 * Append message to the deferred buffer, growing it as needed.
 */
static void deferred_append(const char *message)
{
    size_t msglen = strlen(message);
    /* +2: newline + null */
    if (deferred_len + msglen + 2 > deferred_cap) {
        size_t newcap = (deferred_cap == 0) ? 16384 : deferred_cap * 2;
        if (newcap < deferred_len + msglen + 2)
            newcap = deferred_len + msglen + 2;
        char *newbuf = realloc(deferred_buf, newcap);
        if (newbuf == NULL)
            internal_panic("deferred_append: out of memory");
        deferred_buf = newbuf;
        deferred_cap = newcap;
        if (deferred_len == 0)
            deferred_buf[0] = '\0';
    }
    memcpy(deferred_buf + deferred_len, message, msglen);
    deferred_len += msglen;
    deferred_buf[deferred_len++] = '\n';
    deferred_buf[deferred_len]   = '\0';
}

/*
 * Output the fully-formatted message to the appropriate destination.
 *
 * PANIC and FATAL always go to stderr immediately.
 * ERROR and below go to the deferred buffer if deferred mode is on,
 * otherwise also to stderr.
 * All visible messages also go to the listing file (with a '*' prefix
 * matching Matt's original convention).
 */
static void print_error_message(error_level_t level, const char *message)
{
    assert(message != NULL);
    assert(strlen(message) > 0);

    bool immediate = (level >= ERRORLEVEL_FATAL) || !use_deferred;

    if (immediate) {
        fprintf(stderr, "%s\n", message);
    } else {
        deferred_append(message);
    }

    /* Always mirror to listing file regardless of deferral */
    if (FI_listfile != NULL) {
        fprintf(FI_listfile, "*%s\n", message);
    }
}

/*
 * Sane wrapper for vsnprintf() — panics on encoding error.
 */
static size_t
sane_vsnprintf(char *restrict str, size_t size, const char *restrict fmt, va_list ap)
{
    int res;
    assert(str != NULL);
    assert(size > 0);
    assert(fmt != NULL);

    res = vsnprintf(str, size, fmt, ap);
    if (res < 0)
        internal_panic("sane_vsnprintf: encoding error");

    return (size_t) res;
}

static size_t
sane_snprintf(char *restrict str, size_t size, const char *restrict fmt, ...)
__attribute__((format(printf, 3, 4)));

static size_t
sane_snprintf(char *restrict str, size_t size, const char *restrict fmt, ...)
{
    size_t res;
    va_list ap;
    va_start(ap, fmt);
    res = sane_vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return res;
}

/*
 * Format the file:line location prefix according to F_error_format.
 * Returns strlcat-style result (total would-be length).
 *
 * When file is NULL (no active source file), WOE and DILLON formats
 * emit nothing; GNU format emits the program name instead, following
 * the GNU coding standard for tool-level messages.
 *
 * The file pointer passed in is the result of walking pIncfile upwards
 * past any INF_MACRO frames. Macro invocations don't have real file
 * positions — pIncfile->fi is a garbage pointer when INF_MACRO is set,
 * and lineno tracks the macro body line, not the source file line.
 * We skip macro frames to report the innermost real file location.
 */
static size_t append_location(char *buffer, const INCFILE *file, size_t size)
{
    char location[MSG_BUF_SIZE];
    size_t res = 0;
    location[0] = '\0';

    switch (F_error_format) {
        case ERRORFORMAT_WOE:
            if (file != NULL) {
                res = sane_snprintf(location, sizeof(location),
                                    "%s (%lu): ", file->name, file->lineno);
            }
            break;

        case ERRORFORMAT_DILLON:
            if (file != NULL) {
                res = sane_snprintf(location, sizeof(location),
                                    "line %7lu %-10s ", file->lineno, file->name);
            }
            break;

        case ERRORFORMAT_GNU:
            if (file != NULL) {
                res = sane_snprintf(location, sizeof(location),
                                    "%s:%lu: ", file->name, file->lineno);
            } else {
                res = sane_snprintf(location, sizeof(location),
                                    "%s: ", getprogname());
            }
            break;

        default:
            internal_panic("Invalid error format in append_location!");
            break;
    }

    if (res >= sizeof(location))
        internal_panic("Buffer overflow in append_location!");

    return strlcat(buffer, location, size);
}

/*
 * Append "levelname: " to buffer.
 */
static size_t append_level(char *buffer, error_level_t level, size_t size)
{
    char name[64];
    size_t res;
    assert(valid_error_level(level));
    res = sane_snprintf(name, sizeof(name), "%s: ", level_names[level]);
    if (res >= sizeof(name))
        internal_panic("Buffer overflow in append_level!");
    return strlcat(buffer, name, size);
}

/*
 * Format the message body from fmt+ap and append to buffer.
 */
static size_t
append_information(char *buffer, const char *fmt, va_list ap, size_t size)
{
    char information[MSG_BUF_SIZE];
    size_t res = sane_vsnprintf(information, sizeof(information), fmt, ap);
    if (res >= sizeof(information))
        internal_panic("Buffer overflow in append_information!");
    return strlcat(buffer, information, size);
}


/* ------------------------------------------------------------------ */
/* Core notification function                                           */
/* ------------------------------------------------------------------ */

static void vanotify(error_level_t level, const char *fmt, va_list ap)
{
    char buffer[MSG_BUF_SIZE];
    size_t res;

    /*
     * Walk pIncfile upward past any INF_MACRO frames to find the
     * innermost real source file. See append_location() for why.
     */
    const INCFILE *file = pIncfile;
    while (file != NULL && (file->flags & INF_MACRO) != 0) {
        file = file->next;
    }

    assert(valid_error_level(level));

    if (!visible_error_level(level))
        return;

    buffer[0] = '\0';

    res = append_location(buffer, file, sizeof(buffer));
    if (res >= sizeof(buffer))
        internal_panic("Buffer overflow assembling error message (location)!");

    res = append_level(buffer, level, sizeof(buffer));
    if (res >= sizeof(buffer))
        internal_panic("Buffer overflow assembling error message (level)!");

    res = append_information(buffer, fmt, ap, sizeof(buffer));
    if (res >= sizeof(buffer))
        internal_panic("Buffer overflow assembling error message (body)!");

    print_error_message(level, buffer);

    /* Maintain per-level statistics */
    switch (level) {
        case ERRORLEVEL_NOTICE:  nof_notices++;  break;
        case ERRORLEVEL_WARNING: nof_warnings++; break;
        case ERRORLEVEL_ERROR:   nof_errors++;   break;
        case ERRORLEVEL_FATAL:   nof_fatals++;   break;
        case ERRORLEVEL_PANIC:   nof_fatals++;   break;
        default: break; /* debug/info not counted */
    }

    /* PANIC exits immediately, unconditionally */
    if (level == ERRORLEVEL_PANIC) {
        exit(EXIT_FAILURE);
    }
}


/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

#define IMPLEMENT_FMT(level) \
    va_list ap; \
    va_start(ap, fmt); \
    vanotify(level, fmt, ap); \
    va_end(ap)

#define DEFINE_FMT(name, level) \
void name(const char *fmt, ...) \
{ \
    IMPLEMENT_FMT(level); \
}

void notify_fmt(error_level_t level, const char *fmt, ...)
{
    IMPLEMENT_FMT(level);
}

DEFINE_FMT(debug_fmt,   ERRORLEVEL_DEBUG)
DEFINE_FMT(info_fmt,    ERRORLEVEL_INFO)
DEFINE_FMT(notice_fmt,  ERRORLEVEL_NOTICE)
DEFINE_FMT(warning_fmt, ERRORLEVEL_WARNING)
DEFINE_FMT(error_fmt,   ERRORLEVEL_ERROR)
DEFINE_FMT(fatal_fmt,   ERRORLEVEL_FATAL)

/*
 * panic_fmt is separate because it is __attribute__((noreturn)).
 * The macro expansion hides the va_end / return structure from the
 * compiler, so we spell it out explicitly so noreturn is satisfied.
 */
void panic_fmt(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vanotify(ERRORLEVEL_PANIC, fmt, ap);
    va_end(ap);
    /* vanotify() calls exit() for PANIC — this is unreachable,
     * but keeps compilers that don't understand __attribute__((noreturn))
     * from complaining about a missing return in a void function. */
    exit(EXIT_FAILURE);
}

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
