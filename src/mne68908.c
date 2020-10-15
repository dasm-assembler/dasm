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
 *  MNE68908.C 
 *     based on MNE68705.C	 Thomas Pantzer, 2003-06-18
 */

#include "asm.h"


#define AFSTD		(AF_BYTEADR | AF_BYTEADRX | AF_WORDADR | AF_WORDADRX | AF_0X)

#define AF_BCLR  	(AF_BYTEADR|AF_BYTEADRX|AF_BYTEADRY)

#define AF_HMASK	(AF_IMM8 | AF_BYTEADR | AF_BYTEADRX | AF_WORDADR | AF_WORDADRX | AF_0X | AF_BYTEADR_SP | AF_WORDADR_SP)

#define AF_IMASK    	(AF_BYTEADR| AF_BYTEADRX| AF_0X| AF_BYTEADR_SP)


MNEMONIC Mne68908[] = {

    { NULL, v_mnemonic, "adc", 0, AF_HMASK, { 0xA9, 0xB9, 0xE9, 0xC9, 0xD9, 0xF9, 0x9EE9, 0x9ED9 } },
    { NULL, v_mnemonic, "add", 0, AF_HMASK, { 0xAB, 0xBB, 0xEB, 0xCB, 0xDB, 0xFB, 0x9EEB, 0x9EDB } },
    { NULL, v_mnemonic, "ais", 0, AF_IMM8, { 0xa7 } },
    { NULL, v_mnemonic, "aix", 0, AF_IMM8, { 0xaf } },
    { NULL, v_mnemonic, "and", 0, AF_HMASK, { 0xA4, 0xB4, 0xE4, 0xC4, 0xD4, 0xF4, 0x9EE4, 0x9ED4 } },
    { NULL, v_mnemonic, "asl", 0, AF_IMASK, { 0x38, 0x68, 0x78, 0x9E68 } },
    { NULL, v_mnemonic, "asla", 0,AF_IMP, { 0x48 } },
    { NULL, v_mnemonic, "aslx", 0,AF_IMP, { 0x58 } },
    { NULL, v_mnemonic, "asr", 0, AF_IMASK, { 0x37, 0x67, 0x77, 0x9E67 } },
    { NULL, v_mnemonic, "asra", 0,AF_IMP, { 0x47 } },
    { NULL, v_mnemonic, "asrx", 0,AF_IMP, { 0x57 } },
    { NULL, v_mnemonic, "bcc", 0, AF_REL, { 0x24 } },
    { NULL, v_mnemonic, "bclr", MF_IMOD,AF_BITMOD, { 0x11 } },
    { NULL, v_mnemonic, "bcs", 0, AF_REL, { 0x25 } },
    { NULL, v_mnemonic, "beq", 0, AF_REL, { 0x27 } },
    { NULL, v_mnemonic, "bge", 0, AF_REL, { 0x90 } },
    { NULL, v_mnemonic, "bgt", 0, AF_REL, { 0x92 } },
    { NULL, v_mnemonic, "bhcc", 0,AF_REL, { 0x28 } },
    { NULL, v_mnemonic, "bhcs", 0,AF_REL, { 0x29 } },
    { NULL, v_mnemonic, "bhi", 0, AF_REL, { 0x22 } },
    { NULL, v_mnemonic, "bhs", 0, AF_REL, { 0x24 } },
    { NULL, v_mnemonic, "bih", 0, AF_REL, { 0x2F } },
    { NULL, v_mnemonic, "bil", 0, AF_REL, { 0x2E } },
    { NULL, v_mnemonic, "bit", 0, AF_HMASK, { 0xA5, 0xB5, 0xE5, 0xC5, 0xD5, 0xF5, 0x9EE5, 0x9ED5 } },
    { NULL, v_mnemonic, "ble", 0, AF_REL, { 0x93 } },
    { NULL, v_mnemonic, "blo", 0, AF_REL, { 0x25 } },
    { NULL, v_mnemonic, "bls", 0, AF_REL, { 0x23 } },
    { NULL, v_mnemonic, "blt", 0, AF_REL, { 0x91 } },
    { NULL, v_mnemonic, "bmc", 0, AF_REL, { 0x2C } },
    { NULL, v_mnemonic, "bmi", 0, AF_REL, { 0x2B } },
    { NULL, v_mnemonic, "bms", 0, AF_REL, { 0x2D } },
    { NULL, v_mnemonic, "bne", 0, AF_REL, { 0x26 } },
    { NULL, v_mnemonic, "bpl", 0, AF_REL, { 0x2A } },
    { NULL, v_mnemonic, "bra", 0, AF_REL, { 0x20 } },
    { NULL, v_mnemonic, "brclr", MF_IMOD|MF_REL,   AF_BITBRAMOD, { 0x01 } },
    { NULL, v_mnemonic, "brn", 0, AF_REL, { 0x21 } },
    { NULL, v_mnemonic, "brset", MF_IMOD|MF_REL,   AF_BITBRAMOD, { 0x00 } },
    { NULL, v_mnemonic, "bset", MF_IMOD,AF_BITMOD, { 0x10 } },
    { NULL, v_mnemonic, "bsr", 0, AF_REL, { 0xAD } },

    { NULL, v_mnemonic, "cbeqa", MF_MASK, AF_REL, { 0x41 } }, 
    { NULL, v_mnemonic, "cbeq",  MF_REL, (AF_BYTEADR | AF_INDBYTEX | AF_0X | AF_BYTEADR_SP), { 0x31, 0x61,  0x71, 0x9E61 } }, 
    { NULL, v_mnemonic, "cbeqx", MF_MASK, AF_REL, { 0x51 } }, 

    { NULL, v_mnemonic, "clc", 0, AF_IMP, { 0x98 } },
    { NULL, v_mnemonic, "cli", 0, AF_IMP, { 0x9A } },
    { NULL, v_mnemonic, "clr", 0, AF_IMASK, { 0x3F, 0x6F, 0x7F, 0x9E6F } },
    { NULL, v_mnemonic, "clra", 0,AF_IMP, { 0x4F } },
    { NULL, v_mnemonic, "clrh", 0,AF_IMP, { 0x8C } },
    { NULL, v_mnemonic, "clrx", 0,AF_IMP, { 0x5F } },
    { NULL, v_mnemonic, "cmp", 0, AF_HMASK, { 0xA1, 0xB1, 0xE1, 0xC1, 0xD1, 0xF1, 0x9EE1, 0x9ED1 } },
    { NULL, v_mnemonic, "com", 0, AF_IMASK, { 0x33, 0x63, 0x73, 0x9e63 } },
    { NULL, v_mnemonic, "coma", 0,AF_IMP, { 0x43 } },
    { NULL, v_mnemonic, "comx", 0,AF_IMP, { 0x53 } },
    { NULL, v_mnemonic, "cphx", 0, AF_IMM16|AF_BYTEADR, { 0x65 , 0x75} },
    { NULL, v_mnemonic, "cpx", 0, AF_HMASK, { 0xA3, 0xB3, 0xE3, 0xC3, 0xD3, 0xF3, 0x9EE3, 0x9ED3 } },

    { NULL, v_mnemonic, "daa", 0, AF_IMP, { 0x72 } },

    { NULL, v_mnemonic, "dbnz",  MF_REL, (AF_BYTEADR | AF_BYTEADRX | AF_0X | AF_BYTEADR_SP), { 0x3B, 0x6B, 0x7B, 0x9E6B } }, 
    { NULL, v_mnemonic, "dbnza",      0, AF_REL, { 0x4b } },
    { NULL, v_mnemonic, "dbnzx",      0, AF_REL, { 0x5b } },

    { NULL, v_mnemonic, "dec", 0, AF_IMASK, { 0x3A, 0x6A, 0x7A, 0x9E6A } },
    { NULL, v_mnemonic, "deca", 0,AF_IMP, { 0x4A } },
    { NULL, v_mnemonic, "decx", 0,AF_IMP, { 0x5A } },
    { NULL, v_mnemonic, "div", 0, AF_IMP, { 0x52 } },
    { NULL, v_mnemonic, "eor", 0, AF_HMASK, { 0xA8, 0xB8, 0xE8, 0xC8, 0xD8, 0xF8, 0x9EE8, 0x9ED8 } },
    { NULL, v_mnemonic, "inc", 0, AF_IMASK, { 0x3C, 0x6C, 0x7C, 0x9E6C } },
    { NULL, v_mnemonic, "inca", 0,AF_IMP, { 0x4C } },
    { NULL, v_mnemonic, "incx", 0,AF_IMP, { 0x5C } },

    { NULL, v_mnemonic, "jmp", 0, AFSTD, { 0xBC, 0xEC, 0xCC, 0xDC, 0xFC } },

    { NULL, v_mnemonic, "jsr", 0, AFSTD, { 0xBD, 0xED, 0xCD, 0xDD, 0xFD } },

    { NULL, v_mnemonic, "lda", 0, AF_HMASK, { 0xA6, 0xB6, 0xE6, 0xC6, 0xD6, 0xF6, 0x9EE6, 0x9ED6 } },

    { NULL, v_mnemonic, "ldhx", 0, AF_IMM16|AF_BYTEADR, { 0x45, 0x55 } },
    { NULL, v_mnemonic, "ldx", 0, AF_HMASK, { 0xAE, 0xBE, 0xEE, 0xCE, 0xDE, 0xFE, 0x9EEE, 0x9EDE } },
    { NULL, v_mnemonic, "lsl", 0, AF_IMASK, { 0x38, 0x68, 0x78, 0x9E68 } },
    { NULL, v_mnemonic, "lsla", 0,AF_IMP, { 0x48 } },
    { NULL, v_mnemonic, "lslx", 0,AF_IMP, { 0x58 } },
    { NULL, v_mnemonic, "lsr", 0, AF_IMASK, { 0x34, 0x64, 0x74, 0x9E64 } },
    { NULL, v_mnemonic, "lsra", 0,AF_IMP, { 0x44 } },
    { NULL, v_mnemonic, "lsrx", 0,AF_IMP, { 0x54 } },

    { NULL, v_mnemonic, "mov", MF_MASK, AF_IMM8|AF_BYTEADR, {0x6e, 0x4e} },
    { NULL, v_mnemonic, "movp", 0, AF_BYTEADRX, {0x5e} },
    { NULL, v_mnemonic, "xmov", 0, AF_BYTEADR, {0x7e} },

    { NULL, v_mnemonic, "mul", 0, AF_IMP, { 0x42 } },
    { NULL, v_mnemonic, "neg",  0, AF_IMASK, { 0x30, 0x60, 0x70, 0x9E60 } },
    { NULL, v_mnemonic, "nega", 0,AF_IMP, { 0x40 } },
    { NULL, v_mnemonic, "negx", 0,AF_IMP, { 0x50 } },
    { NULL, v_mnemonic, "nsa", 0, AF_IMP, { 0x62 } },
    { NULL, v_mnemonic, "nop", 0, AF_IMP, { 0x9D } },
    { NULL, v_mnemonic, "ora", 0, AF_HMASK, { 0xAA, 0xBA, 0xEA, 0xCA, 0xDA, 0xFA, 0x9EEA, 0x9EDA } },
    { NULL, v_mnemonic, "psha", 0, AF_IMP, { 0x87 } },
    { NULL, v_mnemonic, "pshh", 0, AF_IMP, { 0x8b } },
    { NULL, v_mnemonic, "pshx", 0, AF_IMP, { 0x89 } },
    { NULL, v_mnemonic, "pula", 0, AF_IMP, { 0x86 } },
    { NULL, v_mnemonic, "pulh", 0, AF_IMP, { 0x8a } },
    { NULL, v_mnemonic, "pulx", 0, AF_IMP, { 0x88 } },
    { NULL, v_mnemonic, "rol", 0, AF_IMASK, { 0x39, 0x69, 0x79, 0x9E69 } },
    { NULL, v_mnemonic, "rola", 0,AF_IMP, { 0x49 } },
    { NULL, v_mnemonic, "rolx", 0,AF_IMP, { 0x59 } },
    { NULL, v_mnemonic, "ror", 0, AF_IMASK, { 0x36, 0x66, 0x76, 0x9E66 } },
    { NULL, v_mnemonic, "rora", 0,AF_IMP, { 0x46 } },
    { NULL, v_mnemonic, "rorx", 0,AF_IMP, { 0x56 } },
    { NULL, v_mnemonic, "rsp", 0, AF_IMP, { 0x9C } },
    { NULL, v_mnemonic, "rti", 0, AF_IMP, { 0x80 } },
    { NULL, v_mnemonic, "rts", 0, AF_IMP, { 0x81 } },
    { NULL, v_mnemonic, "sbc", 0, AF_HMASK     , { 0xA2, 0xB2, 0xE2, 0xC2, 0xD2, 0xF2, 0x9EE2, 0x9ED2 } },
    { NULL, v_mnemonic, "sec", 0, AF_IMP, { 0x99 } },
    { NULL, v_mnemonic, "sei", 0, AF_IMP, { 0x9B } },
    { NULL, v_mnemonic, "sta", 0, AFSTD|AF_BYTEADR_SP|AF_WORDADR_SP, { 0xB7, 0xE7, 0xC7, 0xD7, 0xF7, 0x9EE7, 0x9ED7 } },
    { NULL, v_mnemonic, "sthx", 0, AF_BYTEADR, { 0x35 } }, 
    { NULL, v_mnemonic, "stop", 0, AF_IMP, { 0x8E } },
    { NULL, v_mnemonic, "stx", 0, AFSTD|AF_BYTEADR_SP|AF_WORDADR_SP, { 0xBF, 0xEF, 0xCF, 0xDF, 0xFF, 0x9EEF, 0x9EDF } },
    { NULL, v_mnemonic, "sub", 0, AF_HMASK, { 0xA0, 0xB0, 0xE0, 0xC0, 0xD0, 0xF0, 0x9EE0, 0x9ED0 } },
    { NULL, v_mnemonic, "swi", 0, AF_IMP, { 0x83 } },
    { NULL, v_mnemonic, "tap", 0, AF_IMP, { 0x84 } },
    { NULL, v_mnemonic, "tax", 0, AF_IMP, { 0x97 } },
    { NULL, v_mnemonic, "thxs", 0, AF_IMP, { 0x94 } },
    { NULL, v_mnemonic, "tpa", 0, AF_IMP, { 0x85 } },
    { NULL, v_mnemonic, "tshx", 0, AF_IMP, { 0x95 } },
    { NULL, v_mnemonic, "tst", 0, AF_IMASK, {0x3D, 0x6D, 0x7D, 0x9e6D } },
    { NULL, v_mnemonic, "tsta", 0,AF_IMP, { 0x4D } },
    { NULL, v_mnemonic, "tstx", 0,AF_IMP, { 0x5D } },
    { NULL, v_mnemonic, "txa", 0, AF_IMP, { 0x9F } },
    { NULL, v_mnemonic, "wait", 0, AF_IMP, { 0x8F } },

    MNEMONIC_NULL
};

