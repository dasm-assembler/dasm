#ifndef _DASM_ERRORS_H
#define _DASM_ERRORS_H

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
 *
 * @brief Error handling for DASM.
 *
 * The idea for separate error levels came from syslog(3)
 * and vague memories of Ralf Michl's Java version of same.
 * The idea for printf-style interfaces came from Thomas Mathys
 * as well as from Brian W. Kernighan's and Rob Pike's great
 * little book The Practice of Programming. None of them are
 * to blame for this implementation of course...
 */

#include <stdbool.h>
#include <stdlib.h>

/* remove cool GNU stuff for other compilers */
/* TODO: find more permanent place for this, more .h might need it! */
#ifndef __GNUC__
#define __attribute__(x)  /* GNU C __attribute__ removed */
#endif

/**
 * @brief Format of error messages for -E option.
 */
typedef enum
{
    /* actual error formats */
    ERRORFORMAT_WOE,
    ERRORFORMAT_DILLON,
    ERRORFORMAT_GNU,
    /* meta data */
    ERRORFORMAT_MIN = ERRORFORMAT_WOE,
    ERRORFORMAT_DEFAULT = ERRORFORMAT_WOE,
    ERRORFORMAT_MAX = ERRORFORMAT_GNU
}
error_format_t;

/**
 * @brief Valid error format for -E option?
 */
bool valid_error_format(int format);

/**
 * @brief Set error format, -E option.
 */
void set_error_format(error_format_t format);

/**
 * @brief How many fatals so far, tells us whether to stop after the
 * current pass.
 */
size_t number_of_fatals(void);

/**
 * @brief How many errors so far?
 */
size_t number_of_errors(void);

/**
 * @brief How many warnings so far?
 */
size_t number_of_warnings(void);

/**
 * @brief Severity of error messages.
 */
typedef enum
{
    /* actual error levels */

    /** displayed in debug mode, -d option */
    ERRORLEVEL_DEBUG,
    /** displayed in high verbose mode, -v option */
    ERRORLEVEL_INFO,
    /** displayed in low verbose mode, -v option */
    ERRORLEVEL_NOTICE,
    /** displayed if warnings enabled, -w option */
    ERRORLEVEL_WARNING,
    /** regular error, always displayed, assembly continues */
    ERRORLEVEL_ERROR,
    /** fatal error, always displayed, assembly stops after current pass */
    ERRORLEVEL_FATAL,
    /** panic insanity, always displayed, breaks out right away */
    ERRORLEVEL_PANIC,

    /* meta data */
    ERRORLEVEL_MIN = ERRORLEVEL_DEBUG,
    ERRORLEVEL_DEFAULT = ERRORLEVEL_WARNING,
    ERRORLEVEL_MAX = ERRORLEVEL_PANIC
}
error_level_t;

/**
 * @brief Valid error level?
 */
bool valid_error_level(int level);

/**
 * @brief Set error level, messages down to and including
 * this level are printed.
 * @todo Currently command line options don't affect this
 * thing yet.
 */
void set_error_level(error_level_t level);

/**
 * @brief Length of buffer for source locations.
 */
#define SOURCE_LOCATION_LENGTH 256

/**
 * @brief Global buffer for source locations.
 */
extern char source_location_buffer[SOURCE_LOCATION_LENGTH];

/**
 * @brief Macro to capture current location in the C source
 * into a string buffer, sometimes useful for certain errors.
 * @example SOURCE_LOCATION printf("You're at %s\n", SOURCE_LOCATION);
 * @warning Since there is only one global buffer, our
 * SOURCE_LOCATION macro is not "reentrant"; never use
 * SOURCE_LOCATION more than once in a single evaluation
 * context!
 * @note We have to use snprintf() and a global buffer
 * because __func__ is not expanded into a string literal
 * by the preprocessor! :-/
 * @todo the example tag above doesn't work as I thought it would...
 * also, should this use sane_snprintf()?
 */
#define SOURCE_LOCATION \
    (snprintf(source_location_buffer, SOURCE_LOCATION_LENGTH, \
              "%s/%s()/%d", __FILE__, __func__, __LINE__), \
              source_location_buffer)

/**
 * @brief Generic interface for printf(3)-style error handling
 * framework.
 * @see http://www.unixwiz.net/techtips/gnu-c-attributes.html
 */

void notify_fmt(error_level_t level, const char *fmt, ...)
     __attribute__((format(printf, 2, 3)));

/**
 * @brief Helpers to make common levels easier to read.
 */

void debug_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void info_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void notice_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void warning_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void error_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void fatal_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief An insane problem occurred, print message and terminate
 * DASM with EXIT_FAILURE immediately.
 *
 * @todo the old panic, need to refactor to new use where an error
 * code is required
 *
 * @warning You really don't want to call this. Not ever. We should
 * have REAL error handling instead.
 */

void panic_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)))
     /* TODO: __attribute__((noreturn)) leads to bus error? :-/ */;

/**
 * @brief Standard messages DASM spits out under certain conditions.
 *
 * Note that there's nothing special about these, you can add
 * messages here or write your own literal messages. However,
 * it is highly recommended that you use standard messages if
 * at all possible.
 *
 * The messages are named according to the severity they are
 * most often associated with, however there's nothing wrong
 * with passing PANIC_MEMORY to warning_fmt() when you want
 * to make the user aware of the condition but have a backup
 * plan of how to handle it. (The symbol table sorting done
 * in main.c provides an example of this.)
 *
 * @todo As of right now, the above is not entirely true. It
 * will be when I am done integrating the new log system with
 * the rest of DASM. :-) [phf]
 */

/* unclassified messages (so far) */
#define LOG_NOTHING "Nothing to tell you, which is a bug you should report!"
#define LOG_GENERIC "%s (generic)."

/* messages that support debugging, *very* low-level stuff. */
#define DEBUG_ENTER "<<< Entered function %s."
#define DEBUG_LEAVE ">>> Left function %s."
#define DEBUG_HASH_COLLISIONS "%d mnemonic collisions, %d symbol collisions."

/* warnings related to assembly source code */ 
#define WARNING_RANGE "The %s value in '%s' should be between %d and %d!"

/* errors related to assembly source code */
#define ERROR_SYNTAX_NONE "Syntax error!"
#define ERROR_SYNTAX_ONE "Syntax error in '%s'!"
#define ERROR_SYNTAX_TWO "Syntax error in '%s %s'!"
#define ERROR_VALUE_RANGE "The %s value in '%s' should be between %d and %d!"
#define ERROR_VALUE_ONEOF "The %s value in '%s' should one of %s!"
#define ERROR_BRANCH_RANGE "Branch out of range (%ld bytes)!"
#define ERROR_ADDRESS_RANGE_DETAIL "The %s address in '%s' should be between %d and %d!"
#define ERROR_ADDRESS_RANGE "Address should be between %d and %d!"
#define ERROR_INVALID_BIT "Invalid bit specification in '%s' must be <8!"
#define ERROR_INVALID_ARGS "Not enough arguments!"

/* messages that usually indicate terminal conditions :-) */
#define PANIC_MEMORY "Failed to allocate %zu bytes of memory in %s!"
#define PANIC_SMALL_MEMORY "Failed to allocate %zu bytes of small memory in %s!"

#endif /* _DASM_ERRORS_H */

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
