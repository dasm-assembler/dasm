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
 *  MNE68705.C
 */

#include "asm.h"

#define AFSTD	AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX|AF_0X

MNEMONIC Mne68705[] = {
    { NULL, v_mnemonic, "adc", 0, AF_IMM8|AFSTD, { 0xA9, 0xB9, 0xE9, 0xC9, 0xD9, 0xF9 } },
    { NULL, v_mnemonic, "add", 0, AF_IMM8|AFSTD, { 0xAB, 0xBB, 0xEB, 0xCB, 0xDB, 0xFB } },
    { NULL, v_mnemonic, "and", 0, AF_IMM8|AFSTD, { 0xA4, 0xB4, 0xE4, 0xC4, 0xD4, 0xF4 } },
    { NULL, v_mnemonic, "asl", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x48, 0x38, 0x68, 0x78 } },
    { NULL, v_mnemonic, "asla", 0,AF_IMP, { 0x48 } },
    { NULL, v_mnemonic, "aslx", 0,AF_IMP, { 0x58 } },
    { NULL, v_mnemonic, "asr", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x47, 0x37, 0x67, 0x77 } },
    { NULL, v_mnemonic, "asra", 0,AF_IMP, { 0x47 } },
    { NULL, v_mnemonic, "asrx", 0,AF_IMP, { 0x57 } },
    { NULL, v_mnemonic, "bcc", 0, AF_REL, { 0x24 } },
    { NULL, v_mnemonic, "bclr", MF_IMOD,AF_BITMOD, { 0x11 } },
    { NULL, v_mnemonic, "bcs", 0, AF_REL, { 0x25 } },
    { NULL, v_mnemonic, "beq", 0, AF_REL, { 0x27 } },
    { NULL, v_mnemonic, "bhcc", 0,AF_REL, { 0x28 } },
    { NULL, v_mnemonic, "bhcs", 0,AF_REL, { 0x29 } },
    { NULL, v_mnemonic, "bhi", 0, AF_REL, { 0x22 } },
    { NULL, v_mnemonic, "bhs", 0, AF_REL, { 0x24 } },
    { NULL, v_mnemonic, "bih", 0, AF_REL, { 0x2F } },
    { NULL, v_mnemonic, "bil", 0, AF_REL, { 0x2E } },
    { NULL, v_mnemonic, "bit", 0, AF_IMM8|AFSTD, { 0xA5, 0xB5, 0xE5, 0xC5, 0xD5, 0xF5 } },
    { NULL, v_mnemonic, "blo", 0, AF_REL, { 0x25 } },
    { NULL, v_mnemonic, "bls", 0, AF_REL, { 0x23 } },
    { NULL, v_mnemonic, "bmc", 0, AF_REL, { 0x2C } },
    { NULL, v_mnemonic, "bmi", 0, AF_REL, { 0x2B } },
    { NULL, v_mnemonic, "bms", 0, AF_REL, { 0x2D } },
    { NULL, v_mnemonic, "bne", 0, AF_REL, { 0x26 } },
    { NULL, v_mnemonic, "bpl", 0, AF_REL, { 0x2A } },
    { NULL, v_mnemonic, "bra", 0, AF_REL, { 0x20 } },
    { NULL, v_mnemonic, "brn", 0, AF_REL, { 0x21 } },
    { NULL, v_mnemonic, "brclr", MF_IMOD|MF_REL,   AF_BITBRAMOD, { 0x01 } },
    { NULL, v_mnemonic, "brset", MF_IMOD|MF_REL,   AF_BITBRAMOD, { 0x00 } },
    { NULL, v_mnemonic, "bset", MF_IMOD,AF_BITMOD, { 0x10 } },
    { NULL, v_mnemonic, "bsr", 0, AF_REL, { 0xAD } },
    { NULL, v_mnemonic, "clc", 0, AF_IMP, { 0x98 } },
    { NULL, v_mnemonic, "cli", 0, AF_IMP, { 0x9A } },
    { NULL, v_mnemonic, "clr", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x4F, 0x3F, 0x6F, 0x7F } },
    { NULL, v_mnemonic, "clra", 0,AF_IMP, { 0x4F } },
    { NULL, v_mnemonic, "clrx", 0,AF_IMP, { 0x5F } },
    { NULL, v_mnemonic, "cmp", 0, AF_IMM8|AFSTD, { 0xA1, 0xB1, 0xE1, 0xC1, 0xD1, 0xF1 } },
    { NULL, v_mnemonic, "com", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x43, 0x33, 0x63, 0x73 } },
    { NULL, v_mnemonic, "coma", 0,AF_IMP, { 0x43 } },
    { NULL, v_mnemonic, "comx", 0,AF_IMP, { 0x53 } },
    { NULL, v_mnemonic, "cpx", 0, AF_IMM8|AFSTD, { 0xA3, 0xB3, 0xE3, 0xC3, 0xD3, 0xF3 } },
    { NULL, v_mnemonic, "dec", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x4A, 0x3A, 0x6A, 0x7A } },
    { NULL, v_mnemonic, "deca", 0,AF_IMP, { 0x4A } },
    { NULL, v_mnemonic, "decx", 0,AF_IMP, { 0x5A } },
    { NULL, v_mnemonic, "dex", 0, AF_IMP, { 0x5A } },
    { NULL, v_mnemonic, "eor", 0, AF_IMM8|AFSTD, { 0xA8, 0xB8, 0xE8, 0xC8, 0xD8, 0xF8 } },
    { NULL, v_mnemonic, "inc", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x4C, 0x3C, 0x6C, 0x7C } },
    { NULL, v_mnemonic, "inca", 0,AF_IMP, { 0x4C } },
    { NULL, v_mnemonic, "incx", 0,AF_IMP, { 0x5C } },
    { NULL, v_mnemonic, "inx", 0, AF_IMP, { 0x5C } },
    { NULL, v_mnemonic, "jmp", 0, AFSTD, { 0xBC, 0xEC, 0xCC, 0xDC, 0xFC } },
    { NULL, v_mnemonic, "jsr", 0, AFSTD, { 0xBD, 0xED, 0xCD, 0xDD, 0xFD } },
    { NULL, v_mnemonic, "lda", 0, AF_IMM8|AFSTD, { 0xA6, 0xB6, 0xE6, 0xC6, 0xD6, 0xF6 } },
    { NULL, v_mnemonic, "ldx", 0, AF_IMM8|AFSTD, { 0xAE, 0xBE, 0xEE, 0xCE, 0xDE, 0xFE } },
    { NULL, v_mnemonic, "lsl", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x48, 0x38, 0x68, 0x78 } },
    { NULL, v_mnemonic, "lsla", 0,AF_IMP, { 0x48 } },
    { NULL, v_mnemonic, "lslx", 0,AF_IMP, { 0x58 } },
    { NULL, v_mnemonic, "lsr", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x44, 0x34, 0x64, 0x74 } },
    { NULL, v_mnemonic, "lsra", 0,AF_IMP, { 0x44 } },
    { NULL, v_mnemonic, "lsrx", 0,AF_IMP, { 0x54 } },
    { NULL, v_mnemonic, "neg", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x40, 0x30, 0x60, 0x70 } },
    { NULL, v_mnemonic, "nega", 0,AF_IMP, { 0x40 } },
    { NULL, v_mnemonic, "negx", 0,AF_IMP, { 0x50 } },
    { NULL, v_mnemonic, "nop", 0, AF_IMP, { 0x9D } },
    { NULL, v_mnemonic, "ora", 0, AF_IMM8|AFSTD, { 0xAA, 0xBA, 0xEA, 0xCA, 0xDA, 0xFA } },
    { NULL, v_mnemonic, "rol", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x49, 0x39, 0x69, 0x79 } },
    { NULL, v_mnemonic, "rola", 0,AF_IMP, { 0x49 } },
    { NULL, v_mnemonic, "rolx", 0,AF_IMP, { 0x59 } },
    { NULL, v_mnemonic, "ror", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x46, 0x36, 0x66, 0x76 } },
    { NULL, v_mnemonic, "rora", 0,AF_IMP, { 0x46 } },
    { NULL, v_mnemonic, "rorx", 0,AF_IMP, { 0x56 } },
    { NULL, v_mnemonic, "rsp", 0, AF_IMP, { 0x9C } },
    { NULL, v_mnemonic, "rti", 0, AF_IMP, { 0x80 } },
    { NULL, v_mnemonic, "rts", 0, AF_IMP, { 0x81 } },
    { NULL, v_mnemonic, "sbc", 0, AF_IMM8|AFSTD, { 0xA2, 0xB2, 0xE2, 0xC2, 0xD2, 0xF2 } },
    { NULL, v_mnemonic, "sec", 0, AF_IMP, { 0x99 } },
    { NULL, v_mnemonic, "sei", 0, AF_IMP, { 0x9B } },
    { NULL, v_mnemonic, "sta", 0, AFSTD, { 0xB7, 0xE7, 0xC7, 0xD7, 0xF7 } },
    { NULL, v_mnemonic, "stx", 0, AFSTD, { 0xBF, 0xEF, 0xCF, 0xDF, 0xFF } },
    { NULL, v_mnemonic, "sub", 0, AF_IMM8|AFSTD, { 0xA0, 0xB0, 0xE0, 0xC0, 0xD0, 0xF0 } },
    { NULL, v_mnemonic, "swi", 0, AF_IMP, { 0x83 } },
    { NULL, v_mnemonic, "tax", 0, AF_IMP, { 0x97 } },
    { NULL, v_mnemonic, "tst", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_0X, { 0x4D, 0x3D, 0x6D, 0x7D } },
    { NULL, v_mnemonic, "tsta", 0,AF_IMP, { 0x4D } },
    { NULL, v_mnemonic, "tstx", 0,AF_IMP, { 0x5D } },
    { NULL, v_mnemonic, "txa", 0, AF_IMP, { 0x9F } },
    MNEMONIC_NULL
};

