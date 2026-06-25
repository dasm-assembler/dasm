#ifndef _DASM_SYMBOLS_H
#define _DASM_SYMBOLS_H

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
 * @brief Symbol table management: creation, lookup, and memory.
 *
 * The sortmode_t enum and F_sortmode global are declared in asm.h.
 * ShowSymbols() and DumpSymbolTable() are in main.c (they depend on
 * display/output state that lives there).
 */

#include "asm.h"

#include <stdbool.h>

/**
 * @brief Set the ".." special symbol value used by the DV pseudo-op.
 * @note Only the value and flags fields of the symbol are set.
 */
void setspecial(int value, int flags);

/**
 * @brief Find a symbol by name in the hash table.
 * @warning Truncates names to MAX_SYM_LEN.
 * @note Handles '.' (current PC), '..' (DV special), '...' (checksum),
 *       and local names starting with '.' or ending with '$'.
 * @pre str != NULL && len > 0
 */
SYMBOL *findsymbol(const char *str, int len);

/**
 * @brief Create a new symbol and add it to the hash table.
 * @warning Truncates names to MAX_SYM_LEN.
 * @note Generates mangled names for local symbols ('.' prefix or '$' suffix).
 *       The created symbol has SYM_UNKNOWN set.
 * @pre str != NULL && len > 0
 */
SYMBOL *CreateSymbol(const char *str, int len, bool addToOrder);

/**
 * @brief Allocate a fresh, zeroed symbol from the free list (or permalloc).
 */
SYMBOL *allocsymbol(void);

/**
 * @brief Return a chain of symbols to the free list.
 * @note Frees any SYM_STRING string storage before recycling.
 */
void FreeSymbolList(SYMBOL *sym);

/**
 * @brief Define the label at the current program counter, handling
 *        forward references and phase errors.
 */
void programlabel(void);

#endif /* _DASM_SYMBOLS_H */

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
