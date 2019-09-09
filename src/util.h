#ifndef _DASM_UTIL_H
#define _DASM_UTIL_H

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
 * @brief Utility functions for string manipulation and memory allocation.
 */

#include <stdbool.h>
#include <string.h>

/**
 * @brief
 *   Wrapper for malloc(3) that terminates DASM with panic() if no
 *   memory is available.
 *
 * @pre
 *   bytes > 0
 */

/*@out@*/
void *checked_malloc(size_t bytes);

/**
 * @brief
 *   Wrapper for checked_malloc() that zero's the allocated memory
 *   using memset(3).
 *
 * @pre
 *   bytes > 0
 *
 * @warning
 *   memset(3) may not initialize pointers or floats/doubles
 *   correctly to NULL or 0.0 on some (very strange) machines.
 */

void *zero_malloc(size_t bytes);

/**
 * @brief
 *   Efficiently allocate small amounts of memory.
 *
 * @pre
 *   bytes > 0
 *
 * @warning
 *   You can only request *small* amounts of memory from this
 *   function, less than 1024 bytes at a time is a good rule of
 *   thumb.
 *   You *cannot* free(3) the pointer returned by small_alloc(),
 *   truly bad things will happen if you try.
 *   You can *only* free *all* the memory ever allocated through
 *   small_alloc() using small_free_all() below.
 */

void *small_alloc(size_t bytes);

/**
 * @brief
 *   Free *all* memory allocated by small_alloc() so far.
 *
 * @warning
 *   This function should only be called if your process is
 *   about to exit(3) or if you *really* know what you're doing;
 *   otherwise you'll end up with lots of dangling pointers.
 */

void small_free_all(void);

/**
 * @brief
 *   An excellent hash function for strings.
 *
 * @pre
 *   string != NULL && length > 0 && length <= strlen(string)
 */

unsigned int hash_string(const char *string, size_t length);

/**
 * @brief
 *   Return a fresh copy of the given string. Memory is
 *   allocated with checked_malloc() so it won't return
 *   NULL on failure like the regular strdup() does.
 *
 * @pre
 *   s != NULL && strlen(s) > 0
 */

char *checked_strdup(const char *s);

/**
 * @brief
 *   Convert string to lower case.
 *
 * @pre
 *   dst != NULL && src != NULL && size > 0
 */

size_t strlower(/*@out@*/ char *dst, const char *src, size_t size);

/**
 * @brief
 *   Convert string to upper case.
 *
 * @pre
 *   dst != NULL && src != NULL && size > 0
 */

size_t strupper(/*@out@*/ char *dst, const char *src, size_t size);

/**
 * @brief
 *   Remove all whitespace from a string.
 *
 * @pre
 *   dst != NULL && src != NULL && size > 0
 */

size_t strip_whitespace(/*@out@*/ char *dst, const char *src, size_t size);

/**
 * @brief
 *   True if strcmp(string, strlower(either)) == 0 or
 *   strcmp(string, strupper(either)) == 0, so mixed
 *   capitalization is not allowed.
 *
 * @pre
 *   string != NULL && either != NULL
 */
bool match_either_case(const char *string, const char *either);

#if !defined(__APPLE__) && !defined(__BSD__)

/**
 * @brief
 *   Append src to string dst of size siz.
 *
 * Unlike in strncat(3), siz is the full size of dst, not space left.
 * At most siz-1 characters will be copied.
 * Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 *
 * @pre
 *   dst != NULL && src != NULL && siz > 0
 *
 * @note
 *   On BSD (including OS X) this function is defined in the C library!
 */

size_t strlcat(/*@in@*/ /*@out@*/ char *dst, const char *src, size_t siz);

/**
 * @brief
 *   Copy src to string dst of size siz.
 *
 * At most siz-1 characters will be copied.
 * Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 *
 * @pre
 *   dst != NULL && src != NULL && siz > 0
 *
 * @note
 *   On BSD (including OS X) this function is defined in the C library!
 */

size_t strlcpy(/*@out@*/ char *dst, const char *src, size_t siz);

/**
 * @brief
 *   Name of current program.
 * @warning
 *   If name not set, a string to that effect is returned.
 * @note
 *   On BSD (including OS X) this function is defined in the C library!
 */

/*@temp@*/
const char *getprogname(void);

/**
 * @brief
 *   Sets name of current program sets to last component of given path.
 * @pre
 *   name != NULL
 * @warning
 *   Just pass argv[0] if you want, however the parsing is simplistic so
 *   you should be careful if your environment is the least bit exotic.
 * @note
 *   On BSD (including OS X) this function is defined in the C library!
 */

void setprogname(const char *name);

#endif /* !defined(__APPLE__) && !defined(__BSD__) */

#endif /* _DASM_UTIL_H */

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
