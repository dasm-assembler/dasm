#ifndef _DASM_SYMBOLS_H
#define _DASM_SYMBOLS_H

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
 * @brief Dealing with symbols.
 */

#include "asm.h"

#include <stdbool.h>

void programlabel();
void ShowSymbols(FILE *file);
size_t ShowUnresolvedSymbols(void);
void DumpSymbolTable(void);

/**
 * @brief Remove the SYM_REF flag from all symbols in the
 * hash table.
 */
void clear_all_symbol_refs(void);

/**
 * @brief Set the special symbol for ".." used as part of
 * a DV pseudo-op.
 * @note Only the value and flags fields of the symbol are
 * set.
 */
void set_special_dv_symbol(int value, dasm_flag_t flags);

/**
 * @brief Create symbol with given name and add it to the hash table.
 * @warning Truncates names to MAX_SYM_LEN!
 * @note Generates custom names for local symbols (those starting
 * with '.' or ending with '$'). Uses alloc_symbol() internally.
 * Uses small_alloc() internally for the name of the symbol. The
 * created symbol is SYM_UNKNOWN.
 * @pre str != NULL && len > 0
 */
SYMBOL *create_symbol(const char *str, size_t len, bool addToOrder);

/**
 * @brief Find symbol with given name in hash table.
 * @warning Truncates names to MAX_SYM_LEN!
 * @note Handles special names such as '.' for current PC,
 * ".." for the special argument to EQM as part of DV (see
 * set_special_dv_symbol()), and "..." for the current checksum; also
 * handles local names (those starting with '.' or ending
 * with '$').
 * @pre str != NULL && len > 0
 */
SYMBOL *find_symbol(const char *str, size_t len);

/**
 * @brief Allocate a fresh symbol.
 * @note Uses small_alloc() internally and manages a custom
 * free list of symbols to reuse memory.
 */
SYMBOL *alloc_symbol(void);

/**
 * @brief Free zero or more symbols.
 * @note Manages a custom free list of symbols to reuse memory.
 */
void free_symbol_list(SYMBOL *sym);

/**
 * @brief A symbol file of the given name will be produced.
 * @warning Can only be called once!
 * @pre name != NULL
 */
void set_symbol_file_name(const char *name);

/**
 * @brief Sort mode for symbol table for -T option.
 */
typedef enum
{
    /* actual sort modes */
    SORTMODE_ALPHA,
    SORTMODE_ADDRESS,
    SORTMODE_ORDER,

    /* meta data */
    SORTMODE_MIN = SORTMODE_ALPHA,
    SORTMODE_DEFAULT = SORTMODE_ALPHA,
    SORTMODE_MAX = SORTMODE_ADDRESS
}
sortmode_t;

/**
 * @brief Valid sort mode for -T option?
 */
bool valid_sort_mode(int mode);

/**
 * @brief Set sort mode, -T option.
 */
void set_sort_mode(sortmode_t mode);

/**
 * @brief Print statistics about symbol hash table.
 * @warning For debugging only.
 */
void debug_symbol_hash_collisions(void);

#endif /* _DASM_SYMBOLS_H */

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
