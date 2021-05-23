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

/*
 *  GLOBALS.C
 */

#include "asm.h"

SYMBOL *SHash[SHASHSIZE];   /*	symbol hash table   */


MNEMONIC    *MHash[MHASHSIZE];   /*	mnemonic hash table */
INCFILE *pIncfile;	    /*	include file stack  */
REPLOOP *Reploop;	    /*	repeat loop stack   */
SEGMENT *Seglist;	    /*	segment list	    */
SEGMENT *Csegment;	    /*	current segment     */
IFSTACK *Ifstack;	    /*	IF/ELSE/ENDIF stack */
char	*Av[256];	    /*	up to 256 arguments */
char	Avbuf[512];
unsigned char	MsbOrder = 1;
int	Mnext;
char	Inclevel;
unsigned int	Mlevel;
unsigned long	Localindex;	   /*  to generate local variables */
unsigned long	Lastlocalindex;

unsigned long	Localdollarindex;
unsigned long	Lastlocaldollarindex;

unsigned long Processor = 0;
bool bTrace = false;
bool Xdebug;
bool bStrictMode = false;

unsigned char	Outputformat;

unsigned long   Redo_why = 0;
int	Redo_eval = 0;	   /*  infinite loop detection only    */
int Redo = 0;

int nMacroDeclarations = 0;
int nMacroClosings = 0;

unsigned long maxFileSize = 640 * 1024;		// avoid recursive growing via set,eqm
	// 640k are enough for everybody ... said Bill G. How many 64k pages have you ?

unsigned long	Redo_if = 0;

char	ListMode = 1;
unsigned long	CheckSum;	    /*	output data checksum		*/

int F_format = FORMAT_DEFAULT;

/* -T option [phf] */
sortmode_t F_sortmode = SORTMODE_DEFAULT;
/* -E option [phf] */
errorformat_t F_errorformat = ERRORFORMAT_DEFAULT;

unsigned char	 F_verbose;
const char	*F_outfile = "a.out";
char	*F_listfile;
char	*F_symfile;
FILE	*FI_listfile;
FILE	*FI_temp;
unsigned char	 Fisclear;
unsigned long	 Plab, Pflags;

/*unsigned int	Adrbytes[]  = { 1, 2, 3, 2, 2, 2, 3, 3, 3, 2, 2, 2, 3, 1, 1, 2, 3 };*/
/*                              1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22*/
unsigned int    Cvt[]       = { 0, 2, 0, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 5, 0, 0, 0, 0 };
unsigned int    Opsize[]    = { 0, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 2, 2, 1, 0, 0, 1, 1, 1, 2 };

MNEMONIC Ops[] = {
    { NULL, v_list    , "list",           0,      0, {0,} },
    { NULL, v_include , "include",        0,      0, {0,} },
    { NULL, v_seg     , "seg",            0,      0, {0,} },
    { NULL, v_hex     , "hex",            0,      0, {0,} },
    { NULL, v_err     , "err",            0,      0, {0,} },
    { NULL, v_dc      , "dc",             0,      0, {0,} },
    { NULL, v_dc      , "byte",           0,      0, {0,} },
    { NULL, v_dc      , "word",           0,      0, {0,} },
    { NULL, v_dc      , "long",           0,      0, {0,} },
    { NULL, v_ds      , "ds",             0,      0, {0,} },
    { NULL, v_dc      , "dv",             0,      0, {0,} },
    { NULL, v_end     , "end",            0,      0, {0,} },
    { NULL, v_trace   , "trace",          0,      0, {0,} },
    { NULL, v_org     , "org",            0,      0, {0,} },
    { NULL, v_rorg    , "rorg",           0,      0, {0,} },
    { NULL, v_rend    , "rend",           0,      0, {0,} },
    { NULL, v_align   , "align",          0,      0, {0,} },
    { NULL, v_subroutine, "subroutine",   0,      0, {0,} },
    { NULL, v_equ     , "equ",            0,      0, {0,} },
    { NULL, v_equ     , "=",              0,      0, {0,} },
    { NULL, v_eqm     , "eqm",            0,      0, {0,} },
    { NULL, v_set     , "set",            0,      0, {0,} },
    { NULL, v_setstr  , "setstr",         0,      0, {0,} },
    { NULL, v_macro   , "mac",            MF_IF|MF_BEGM,  0, {0,} },
    { NULL, v_macro   , "macro",          MF_IF|MF_BEGM,  0, {0,} },
    { NULL, v_endm    , "endm",           MF_ENDM,0, {0,} },
    { NULL, v_mexit   , "mexit",          0,      0, {0,} },
    { NULL, v_ifconst , "ifconst",        MF_IF,  0, {0,} },
    { NULL, v_ifnconst, "ifnconst",       MF_IF,  0, {0,} },
    { NULL, v_ifnconst, "ifndef",         MF_IF,  0, {0,} },
    { NULL, v_if      , "if",             MF_IF,  0, {0,} },
    { NULL, v_else    , "else",           MF_IF,  0, {0,} },
    { NULL, v_endif   , "endif",          MF_IF,  0, {0,} },
    { NULL, v_endif   , "eif",            MF_IF,  0, {0,} },
    { NULL, v_repeat  , "repeat",         MF_IF,  0, {0,} },
    { NULL, v_repend  , "repend",         MF_IF,  0, {0,} },
    { NULL, v_echo    , "echo",           0,      0, {0,} },
    { NULL, v_processor,"processor",      0,      0, {0,} },
    { NULL, v_incbin  , "incbin",         0,      0, {0,} },
    { NULL, v_incdir  , "incdir",         0,      0, {0,} },
    MNEMONIC_NULL
};

