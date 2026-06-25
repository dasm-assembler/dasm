#ifndef _DASM_ERRORS_H
#define _DASM_ERRORS_H

/*
    the DASM macro assembler (aka small systems cross assembler)

    Copyright (c) 1988-2002 by Matthew Dillon.
    Copyright (c) 1995 by Olaf "Rhialto" Seibert.
    Copyright (c) 2003-2008 by Andrew Davie.
    Copyright (c) 2008 by Peter H. Froehlich.
    Copyright (c) 2019-2026 by the DASM team.

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
 *
 * @brief Error handling for DASM.
 *
 * Messages are emitted via the notify_fmt() family. Each message has a
 * severity level and is formatted printf-style. Levels control both
 * filtering (set_error_level) and whether assembly continues.
 *
 * Multi-pass deferred mode: because DASM assembles in multiple passes,
 * errors raised in early passes are often spurious (forward references
 * that resolve later). Call set_deferred_errors(true) at pass start to
 * buffer non-fatal messages; call clear_deferred_errors() on the next
 * pass if assembly is retried, or flush_deferred_errors(fp) on the
 * final pass if assembly failed.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * Portability: strip GNU C attributes on non-GCC compilers.
 * Placed here since errors.h is included by almost everything.
 */
#ifndef __GNUC__
#define __attribute__(x) /* GNU C __attribute__ not available */
#endif


/* ------------------------------------------------------------------ */
/* Error format (-E option)                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Output format for error messages, selected via -E.
 *
 * ERRORFORMAT_MAX is one past the last valid value (exclusive),
 * consistent with the SORTMODE_MAX sentinel in asm.h.
 * Use: valid = (fmt >= ERRORFORMAT_MIN && fmt < ERRORFORMAT_MAX)
 */
typedef enum {
    ERRORFORMAT_WOE = 0,    /* "file (line): error: msg"  -- MS Visual Studio */
    ERRORFORMAT_DILLON = 1, /* "line N  file msg"         -- Matthew Dillon original */
    ERRORFORMAT_GNU = 2,    /* "file:line: error: msg"    -- GNU coding standards */
    /* sentinels */
    ERRORFORMAT_MIN = ERRORFORMAT_WOE,
    ERRORFORMAT_DEFAULT = ERRORFORMAT_WOE,
    ERRORFORMAT_MAX = 3 /* exclusive upper bound — keep in sync with last entry */
} error_format_t;

bool valid_error_format(int format);
void set_error_format(error_format_t format);


/* ------------------------------------------------------------------ */
/* Error levels                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Severity levels, modelled after syslog(3).
 *
 * Only messages at or above the current level (set_error_level) are
 * displayed. The -v command-line option should map to these levels via
 * verbosity_to_error_level():
 *
 *   -v0 (default) -> ERRORLEVEL_WARNING  (warnings and above)
 *   -v1           -> ERRORLEVEL_NOTICE
 *   -v2           -> ERRORLEVEL_INFO
 *   -v3 / -v4     -> ERRORLEVEL_DEBUG
 *
 * ERRORLEVEL_MAX is one past the last valid value (exclusive).
 */
typedef enum {
    ERRORLEVEL_DEBUG = 0,   /**< internal tracing, very verbose */
    ERRORLEVEL_INFO = 1,    /**< informational, enabled by -v2  */
    ERRORLEVEL_NOTICE = 2,  /**< notable events, enabled by -v1 */
    ERRORLEVEL_WARNING = 3, /**< potential problem (default minimum) */
    ERRORLEVEL_ERROR = 4,   /**< assembly error; pass continues */
    ERRORLEVEL_FATAL = 5,   /**< assembly stops after this pass */
    ERRORLEVEL_PANIC = 6,   /**< internal error; process exits immediately */
    /* sentinels */
    ERRORLEVEL_MIN = ERRORLEVEL_DEBUG,
    ERRORLEVEL_DEFAULT = ERRORLEVEL_WARNING,
    ERRORLEVEL_MAX = 7 /**< exclusive upper bound */
} error_level_t;

bool valid_error_level(int level);
void set_error_level(error_level_t level);

/**
 * @brief Convert -v verbosity integer to the corresponding error level.
 * Maps: 0->WARNING, 1->NOTICE, 2->INFO, >=3->DEBUG.
 */
error_level_t verbosity_to_error_level(int verbosity);


/* ------------------------------------------------------------------ */
/* Error statistics                                                     */
/* ------------------------------------------------------------------ */

/** Number of ERRORLEVEL_FATAL messages since last reset_error_counts(). */
size_t number_of_fatals(void);

/** Number of ERRORLEVEL_ERROR messages since last reset_error_counts(). */
size_t number_of_errors(void);

/** Number of ERRORLEVEL_WARNING messages since last reset_error_counts(). */
size_t number_of_warnings(void);

/** Number of ERRORLEVEL_NOTICE messages since last reset_error_counts(). */
size_t number_of_notices(void);

/**
 * @brief Reset all message counters to zero.
 * Call at the start of each assembly pass so per-pass statistics are clean.
 */
void reset_error_counts(void);


/* ------------------------------------------------------------------ */
/* Deferred (pass-buffered) output                                      */
/* ------------------------------------------------------------------ */

/**
 * @brief Enable or disable deferred error output.
 *
 * When deferred mode is on, messages at ERRORLEVEL_ERROR and below are
 * accumulated in an internal buffer rather than printed to stderr.
 * PANIC and FATAL messages are always printed and acted upon immediately.
 *
 * This mirrors the old passbuffer[ERRORBUF] behaviour: call
 *   set_deferred_errors(true)   at the start of each pass
 *   clear_deferred_errors()     when starting another pass (discard)
 *   flush_deferred_errors(fp)   on the final pass if assembly failed
 */
void set_deferred_errors(bool deferred);

/**
 * @brief Print the deferred error buffer to fp, then clear it.
 */
void flush_deferred_errors(FILE *fp);

/**
 * @brief Discard the deferred error buffer without printing.
 */
void clear_deferred_errors(void);


/* ------------------------------------------------------------------ */
/* Source location helper                                               */
/* ------------------------------------------------------------------ */

/** Buffer size for SOURCE_LOCATION. Sized to hold a long file path. */
#define SOURCE_LOCATION_LENGTH 512

/** Global buffer written by the SOURCE_LOCATION macro. */
extern char source_location_buffer[SOURCE_LOCATION_LENGTH];

/**
 * @brief Evaluates to a string describing the current C source location.
 *
 * Example: SOURCE_LOCATION -> "src/main.c/parse()/1264"
 *
 * @warning Uses a single global buffer — do not use more than once
 * per expression (e.g. in a single printf argument list).
 */
#define SOURCE_LOCATION                                                                                                \
    (snprintf(source_location_buffer, SOURCE_LOCATION_LENGTH, "%s/%s()/%d", __FILE__, __func__, __LINE__),             \
     source_location_buffer)


/* ------------------------------------------------------------------ */
/* printf-style notification interface                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Emit a message at an explicit level.
 */
void notify_fmt(error_level_t level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

/** Emit at ERRORLEVEL_DEBUG. */
void debug_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/** Emit at ERRORLEVEL_INFO. */
void info_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/** Emit at ERRORLEVEL_NOTICE. */
void notice_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/** Emit at ERRORLEVEL_WARNING. */
void warning_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/** Emit at ERRORLEVEL_ERROR. */
void error_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/** Emit at ERRORLEVEL_FATAL. Assembly stops after current pass. */
void fatal_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/**
 * @brief Emit at ERRORLEVEL_PANIC and exit immediately.
 * Never returns.
 */
void panic_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2))) __attribute__((noreturn));

/**
 * @brief Print message to stderr and exit. Never returns.
 * Declared here so all translation units can call it without implicit declaration.
 */
void panic(const char *str) __attribute__((noreturn));


/* ------------------------------------------------------------------ */
/* Standard message format strings                                      */
/* ------------------------------------------------------------------ */

/* Unclassified */
#define LOG_NOTHING "Nothing to tell you — this is a bug, please report it!"
#define LOG_GENERIC "%s"

/* Debug */
#define DEBUG_ENTER "<<< Entered %s."
#define DEBUG_LEAVE ">>> Left %s."
#define DEBUG_HASH_COLLISIONS "%d mnemonic hash collisions, %d symbol hash collisions."

/* Warnings */
#define WARNING_RANGE "The %s value in '%s' should be between %d and %d!"

/* Errors */
#define ERROR_SYNTAX_NONE "Syntax error!"
#define ERROR_SYNTAX_ONE "Syntax error in '%s'!"
#define ERROR_SYNTAX_TWO "Syntax error in '%s %s'!"
#define ERROR_VALUE_RANGE "The %s value in '%s' should be between %d and %d!"
#define ERROR_VALUE_ONEOF "The %s value in '%s' should be one of %s!"
#define ERROR_BRANCH_RANGE "Branch out of range (%ld bytes)!"
#define ERROR_ADDRESS_RANGE_DETAIL "The %s address in '%s' should be between %d and %d!"
#define ERROR_ADDRESS_RANGE "Address should be between %d and %d!"
#define ERROR_INVALID_BIT "Invalid bit specification in '%s', must be < 8!"
#define ERROR_INVALID_ARGS "Not enough arguments!"

/* Panics */
#define PANIC_MEMORY "Failed to allocate %zu bytes in %s!"
#define PANIC_SMALL_MEMORY "Failed to allocate %zu bytes (small alloc) in %s!"


#endif /* _DASM_ERRORS_H */

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
