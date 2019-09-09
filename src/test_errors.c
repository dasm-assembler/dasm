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
 * @todo how do we keep panic* from aborting while unit test
 * runs? or maybe it doesn't matter if we switch to Check?
 */

#include "errors.h"

#include "asm.h"
#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* fakes for unit test */
FILE *FI_listfile = NULL;
char *F_listfile = NULL;
INCFILE *pIncfile = NULL;

int main(int argc, char *argv[])
{
    assert(argc == 1);
    setprogname(argv[0]);
    /* fake a current file */
    pIncfile = malloc(sizeof(INCFILE));
    pIncfile->next = NULL;
    pIncfile->name = checked_strdup("someFileName");
    pIncfile->lineno = 47;
    /* enable all messages */
    set_error_level(ERRORLEVEL_DEBUG);
    /* test SUPER new API :-) */
    debug_fmt(WARNING_RANGE, "CRAZY", "DEBUG", 2, 14);
    info_fmt(WARNING_RANGE, "CRAZY", "DEBUG", 2, 14);
    notice_fmt(WARNING_RANGE, "CRAZY", "DEBUG", 2, 14);
    warning_fmt(WARNING_RANGE, "CRAZY", "DEBUG", 2, 14);
    error_fmt(WARNING_RANGE, "CRAZY", "DEBUG", 2, 14);
    fatal_fmt(WARNING_RANGE, "CRAZY", "FATAL", 2, 14);
    panic_fmt(WARNING_RANGE, "CRAZY", "DEBUG", 2, 14);
    return EXIT_SUCCESS;
}

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
