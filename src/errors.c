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
 */

#include "errors.h"

#include "asm.h"
#include "util.h"
#include "version.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

/*
    TODO: I simply replaced "error" with the current level in
    all messages, not sure that works on Windows? GNU is fine,
    it doesn't require "error" in the message...
*/

/*
    TODO: the original asmerr() would set bStopAtEnd (what is
    now nof_fatals>0) if the error had the "fatal" flag set in
    the struct that described it; if would *abort* on a true
    argument; in my previous reading of the code, I had assumed
    that a true argument meant simply "fatal" and not abort. To
    be verified again and addressed, maybe there are a few fatals
    that should really be panics...
*/

static error_format_t F_error_format = ERRORFORMAT_DEFAULT;
static error_level_t F_error_level = ERRORLEVEL_DEFAULT;
static size_t nof_fatals = 0;
static size_t nof_errors = 0;
static size_t nof_warnings = 0;
char source_location_buffer[SOURCE_LOCATION_LENGTH];

static const char *level_names[] =
{
    [ERRORLEVEL_DEBUG] = "debug",
    [ERRORLEVEL_INFO] = "info",
    [ERRORLEVEL_NOTICE] = "notice",
    [ERRORLEVEL_WARNING] = "warning",
    [ERRORLEVEL_ERROR] = "error",
    [ERRORLEVEL_FATAL] = "fatal",
    [ERRORLEVEL_PANIC] = "***panic***",
};

bool valid_error_format(int format)
{
    return (ERRORFORMAT_MIN <= format && format <= ERRORFORMAT_MAX);
}

void set_error_format(error_format_t format)
{
    assert(valid_error_format(format));
    F_error_format = format;
}

bool valid_error_level(int level)
{
    return (ERRORLEVEL_MIN <= level && level <= ERRORLEVEL_MAX);
}

void set_error_level(error_level_t level)
{
    assert(valid_error_level(level));
    F_error_level = level;
}

/**
 * @brief Display this error level?
 */
static bool visible_error_level(error_level_t level)
{
    return (level >= F_error_level);
}

size_t number_of_fatals(void)
{
  return nof_fatals;
}

size_t number_of_errors(void)
{
  return nof_errors;
}

size_t number_of_warnings(void)
{
  return nof_warnings;
}

/* Super low-level panic for disasters *inside* the errors module! */
static void internal_panic(const char *message)
{
  fprintf(
    stderr,
    "\n%s: FATAL INTERNAL PANIC (errors.c): %s\n\n",
    getprogname(),
    message
  );
  exit(EXIT_FAILURE);
}

/**
 * @brief Print final error message to all relevant streams.
 * @note We always print to stderr; we print to the listing
 * file if we have one. Messages to the listing file get a
 * leading "*" just like Matt's version did years ago; at
 * one point I thought that the "*" starts a comment, but I
 * can't confirm that in the code (only ";" seems to be a
 * comment), so the motivation must have been different.
 * We only get here after all the other filters checked that
 * we should really print, so we don't check anything else
 * about the error message.
 */
static void print_error_message(const char *message)
{
    assert(message != NULL);
    assert(strlen(message) > 0);

    fprintf(stderr, "%s\n", message);

    if (FI_listfile != NULL) {
        fprintf(FI_listfile, "*%s\n", message);
    }
    else {
        /* sanity check: if there was no FILE* there should be no name */
        assert(F_listfile == NULL);
    }
}

/**
 * @brief Sane wrapper for vsnprintf().
 * @note See sane_snprintf() for details.
 */
static size_t
sane_vsnprintf(/*@out@*/ char *restrict str, size_t size, const char *restrict fmt, va_list ap)
{
    int res;
    assert(str != NULL);
    assert(size > 0);
    assert(fmt != NULL);

    res = vsnprintf(str, size, fmt, ap);
    if (res < 0) {
        internal_panic("sane_vsnprintf() failed!");
    }
    /* res >= 0 here so cast to size_t is okay */
    return (size_t) res;
}

static size_t
sane_snprintf(/*@out@*/ char *restrict str, size_t size, const char *restrict fmt, ...)
__attribute__((format(printf, 3, 4)));

/**
 * @brief Sane wrapper for snprintf().
 * @note The interface for snprintf() is a little retarded since
 * the return type is int instead of size_t. Due to it's stdio.h
 * heritage, returning something negative on error is expected.
 * But we're using it to format strings, so we don't care about
 * those errors in detail (if there ever are any, not even sure).
 * We handle potential errors here and return a size_t suitable
 * for overflow checking.
 */
static size_t
sane_snprintf(/*@out@*/ char *restrict str, size_t size, const char *restrict fmt, ...)
{
    size_t res;
    va_list ap;

    va_start(ap, fmt);

    res = sane_vsnprintf(str, size, fmt, ap);

    va_end(ap);
    return res;
}

/**
 * @brief Format the level part of the error message.
 * @note Follows strlcat() conventions.
 */
static size_t append_level(char *buffer, error_level_t level, size_t size)
{
    char name[1024];
    size_t res = 0;
    assert(valid_error_level(level));
    res = sane_snprintf(name, sizeof(name), "%s: ", level_names[level]);
    if (res >= sizeof(name)) {
        internal_panic("Buffer overflow in append_level()!");
    }
    return strlcat(buffer, name, size);
}

/**
 * @brief Format the location part of the error message.
 * @note Follows strlcat() conventions.
 */
static size_t append_location(char *buffer, /*@null@*/ const INCFILE *file, size_t size)
{
    char location[1024];
    size_t res = 0;

    /* clear buffer */
    location[0] = '\0';

    switch (F_error_format) {
        case ERRORFORMAT_WOE:
            /*
                Error format for MS VisualStudio and relatives:
                "file (line): error: string"
            */
            if (file != NULL) {
                res = sane_snprintf(
                    location, sizeof(location), "%s (%lu): ",
                    file->name, file->lineno
                );
            }
            break;
        case ERRORFORMAT_DILLON:
            /*
                Matthew Dillon's original format. Note that
                Matt's 2.16 uses this instead:
                  "line %4ld %-10s %s\n" (terminal)
            */
            if (file != NULL) {
                res = sane_snprintf(
                    location, sizeof(location), "line %7lu %-10s ",
                    file->lineno, file->name
                );
            }
            break;
        case ERRORFORMAT_GNU:
            /*
                GNU format error messages, from their coding
                standards: "source-file-name:lineno: message"
            */
            if (file != NULL) {
                res = sane_snprintf(
                    location, sizeof(location), "%s:%lu: ",
                    file->name, file->lineno
                );
            }
            else {
                res = sane_snprintf(
                    location, sizeof(location), "%s: ",
                    getprogname()
                );
            }
            break;
        default:
            internal_panic("Invalid error format in append_location()!"); 
            break;
    }
    if (res >= sizeof(location)) {
        internal_panic("Buffer overflow in append_location()!");
    }
    return strlcat(buffer, location, size);
}

/**
 * @brief Format the information part of the error message.
 * @note Follows strlcat() conventions.
 */
static size_t
append_information(char *buffer, const char *fmt, va_list ap, size_t size)
{
    char information[1024];
    size_t res = 0;
    res = sane_vsnprintf(information, sizeof(information), fmt, ap);
    if (res >= sizeof(information)) {
        internal_panic("Buffer overflow in append_information()!");
    }
    return strlcat(buffer, information, size);
}

static void vanotify(error_level_t level, const char *fmt, va_list ap)
{
    /* buffer for formatting error message into  */
    char buffer[1024];
    /* include file we're in right now (if any) */
    INCFILE *file = pIncfile;
    /* holds the return value from strlcat */
    size_t res;

    assert(valid_error_level(level));

    if (!visible_error_level(level)) {
        /* condition not severe enough */
        return;
    }

    /* find the file we're in (if any) */
    /* TODO: how does this work exactly? */
    while (file != NULL && (file->flags & INF_MACRO) != 0) {
        file = file->next;
    }

    /* clear buffer */
    buffer[0] = '\0';

    /* append the various pieces of the message */
    res = append_location(buffer, file, sizeof(buffer));
    if (res > sizeof(buffer)) {
        internal_panic("Buffer overflow in vanotify()!");
    }
    res = append_level(buffer, level, sizeof(buffer));
    if (res > sizeof(buffer)) {
        internal_panic("Buffer overflow in vanotify()!");
    }
    res = append_information(buffer, fmt, ap, sizeof(buffer));
    if (res > sizeof(buffer)) {
        internal_panic("Buffer overflow in vanotify()!");
    }

    /* print the message */
    print_error_message(buffer);

    /* maintain statistics about warnings and errors */
    /* TODO: count everything < PANIC? */
    if (level == ERRORLEVEL_WARNING)
    {
         nof_warnings += 1;
    }
    if (level == ERRORLEVEL_ERROR)
    {
         nof_errors += 1;
    }

    /* fatal and higher errors lead to (eventual) termination */
    if (level >= ERRORLEVEL_FATAL)
    {
        nof_fatals += 1; /* stop after current pass */
    }
    if (level == ERRORLEVEL_PANIC)
    {
        exit(EXIT_FAILURE); /* stop right now! */
    }
}

/* avoid code replication through macros, sweet [phf] */
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

DEFINE_FMT(debug_fmt, ERRORLEVEL_DEBUG)
DEFINE_FMT(info_fmt, ERRORLEVEL_INFO)
DEFINE_FMT(notice_fmt, ERRORLEVEL_NOTICE)
DEFINE_FMT(warning_fmt, ERRORLEVEL_WARNING)
DEFINE_FMT(error_fmt, ERRORLEVEL_ERROR)
DEFINE_FMT(fatal_fmt, ERRORLEVEL_FATAL)
DEFINE_FMT(panic_fmt, ERRORLEVEL_PANIC)

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
