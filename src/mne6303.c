/*
    the DASM macro assembler (aka small systems cross assembler)

    Copyright (c) 1988-2002 by Matthew Dillon.
    Copyright (c) 1995 by Olaf "Rhialto" Seibert.
    Copyright (c) 2003-2008 by Andrew Davie.
    Copyright (c) 2008 by Peter H. Froehlich.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
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
 *  MNE6303.C
 */

#include "asm.h"

/*
 *  IMP IMM8 IMM16 BYTE BYTEX BYTEY WORD WORDX WORDY REL (,x) (),y (WORD)
 *   0	 1    2     3	 4     5     6    7     8     9   10   11    12
 *
 *  0,x 0,y BIT BITBRA
 *   13  14  15   16
 */

MNEMONIC Mne6803[] = {
    { NULL, v_mnemonic, "aba", 0,   AF_IMP,	{ 0x1B }},
    { NULL, v_mnemonic, "abx", 0,   AF_IMP,	{ 0x3A }},
    { NULL, v_mnemonic, "adca", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x89, 0x99, 0xA9, 0xB9 }},
    { NULL, v_mnemonic, "adcb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC9, 0xD9, 0xE9, 0xF9 }},
    { NULL, v_mnemonic, "adda", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x8B, 0x9B, 0xAB, 0xBB }},
    { NULL, v_mnemonic, "addb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xCB, 0xDB, 0xEB, 0xFB }},
    { NULL, v_mnemonic, "addd", 0,  AF_IMM16|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC3, 0xD3, 0xE3, 0xF3 }},
    { NULL, v_mnemonic, "anda", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x84, 0x94, 0xA4, 0xB4 }},
    { NULL, v_mnemonic, "andb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC4, 0xD4, 0xE4, 0xF4 }},
    { NULL, v_mnemonic, "bita", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x85, 0x95, 0xA5, 0xB5 }},
    { NULL, v_mnemonic, "bitb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC5, 0xD5, 0xE5, 0xF5 }},
    { NULL, v_mnemonic, "bra", 0,   AF_REL, { 0x20 }},
    { NULL, v_mnemonic, "brn", 0,   AF_REL, { 0x21 }},
    { NULL, v_mnemonic, "bcc", 0,   AF_REL, { 0x24 }},
    { NULL, v_mnemonic, "bhs", 0,   AF_REL, { 0x24 }},
    { NULL, v_mnemonic, "bcs", 0,   AF_REL, { 0x25 }},
    { NULL, v_mnemonic, "blo", 0,   AF_REL, { 0x25 }},
    { NULL, v_mnemonic, "beq", 0,   AF_REL, { 0x27 }},
    { NULL, v_mnemonic, "bge", 0,   AF_REL, { 0x2C }},
    { NULL, v_mnemonic, "bgt", 0,   AF_REL, { 0x2E }},
    { NULL, v_mnemonic, "bhi", 0,   AF_REL, { 0x22 }},
    { NULL, v_mnemonic, "ble", 0,   AF_REL, { 0x2F }},
    { NULL, v_mnemonic, "bls", 0,   AF_REL, { 0x23 }},
    { NULL, v_mnemonic, "blt", 0,   AF_REL, { 0x2D }},
    { NULL, v_mnemonic, "bmi", 0,   AF_REL, { 0x2B }},
    { NULL, v_mnemonic, "bne", 0,   AF_REL, { 0x26 }},
    { NULL, v_mnemonic, "bvc", 0,   AF_REL, { 0x28 }},
    { NULL, v_mnemonic, "bvs", 0,   AF_REL, { 0x29 }},
    { NULL, v_mnemonic, "bpl", 0,   AF_REL, { 0x2A }},
    { NULL, v_mnemonic, "bsr", 0,   AF_REL, { 0x8D }},
    { NULL, v_mnemonic, "clc", 0,   AF_IMP, { 0x0C }},
    { NULL, v_mnemonic, "cli", 0,   AF_IMP, { 0x0E }},
    { NULL, v_mnemonic, "clv", 0,   AF_IMP, { 0x0A }},
    { NULL, v_mnemonic, "sec", 0,   AF_IMP, { 0x0D }},
    { NULL, v_mnemonic, "sei", 0,   AF_IMP, { 0x0F }},
    { NULL, v_mnemonic, "sev", 0,   AF_IMP, { 0x0B }},
    { NULL, v_mnemonic, "tap", 0,   AF_IMP, { 0x06 }},
    { NULL, v_mnemonic, "tpa", 0,   AF_IMP, { 0x07 }},
    { NULL, v_mnemonic, "clr", 0,   AF_BYTEADRX|AF_WORDADR, { 0x6F, 0x7F }},
    { NULL, v_mnemonic, "clra", 0,  AF_IMP, { 0x4F }},
    { NULL, v_mnemonic, "clrb", 0,  AF_IMP, { 0x5F }},
    { NULL, v_mnemonic, "cmpa", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x81, 0x91, 0xA1, 0xB1 }},
    { NULL, v_mnemonic, "cmpb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC1, 0xD1, 0xE1, 0xF1 }},
    { NULL, v_mnemonic, "cba", 0,   AF_IMP, { 0x11 }},
    { NULL, v_mnemonic, "com", 0,   AF_BYTEADRX|AF_WORDADR, { 0x63, 0x73 }},
    { NULL, v_mnemonic, "coma", 0,  AF_IMP, { 0x43 }},
    { NULL, v_mnemonic, "comb", 0,  AF_IMP, { 0x53 }},
    { NULL, v_mnemonic, "neg", 0,   AF_BYTEADRX|AF_WORDADR, { 0x60, 0x70 }},
    { NULL, v_mnemonic, "nega", 0,  AF_IMP, { 0x40 }},
    { NULL, v_mnemonic, "negb", 0,  AF_IMP, { 0x50 }},
    { NULL, v_mnemonic, "daa", 0,   AF_IMP, { 0x19 }},
    { NULL, v_mnemonic, "dec", 0,   AF_BYTEADRX|AF_WORDADR, { 0x6A, 0x7A }},
    { NULL, v_mnemonic, "deca", 0,  AF_IMP, { 0x4A }},
    { NULL, v_mnemonic, "decb", 0,  AF_IMP, { 0x5A }},
    { NULL, v_mnemonic, "eora", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x88, 0x98, 0xA8, 0xB8 }},
    { NULL, v_mnemonic, "eorb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC8, 0xD8, 0xE8, 0xF8 }},
    { NULL, v_mnemonic, "inc", 0,   AF_BYTEADRX|AF_WORDADR, { 0x6C, 0x7C }},
    { NULL, v_mnemonic, "inca", 0,  AF_IMP, { 0x4C }},
    { NULL, v_mnemonic, "incb", 0,  AF_IMP, { 0x5C }},
    { NULL, v_mnemonic, "jmp",  0,  AF_BYTEADRX|AF_WORDADR, { 0x6E, 0x7E }},
    { NULL, v_mnemonic, "jsr",  0,  AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x9D, 0xAD, 0xBD }},
    { NULL, v_mnemonic, "ldaa", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x86, 0x96, 0xA6, 0xB6 }},
    { NULL, v_mnemonic, "ldab", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC6, 0xD6, 0xE6, 0xF6 }},
    { NULL, v_mnemonic, "ldd", 0,   AF_IMM16|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xCC, 0xDC, 0xEC, 0xFC }},
    { NULL, v_mnemonic, "mul", 0,   AF_IMP, { 0x3D }},
    { NULL, v_mnemonic, "nop", 0,   AF_IMP, { 0x01 }},
    { NULL, v_mnemonic, "oraa",0,   AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x8A, 0x9A, 0xAA, 0xBA }},
    { NULL, v_mnemonic, "orab", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xCA, 0xDA, 0xEA, 0xFA }},
    { NULL, v_mnemonic, "psha", 0,  AF_IMP, { 0x36 }},
    { NULL, v_mnemonic, "pshb", 0,  AF_IMP, { 0x37 }},
    { NULL, v_mnemonic, "pshx", 0,  AF_IMP, { 0x3C }},
    { NULL, v_mnemonic, "pulx", 0,  AF_IMP, { 0x38 }},
    { NULL, v_mnemonic, "pula", 0,  AF_IMP, { 0x32 }},
    { NULL, v_mnemonic, "pulb", 0,  AF_IMP, { 0x33 }},
    { NULL, v_mnemonic, "rol", 0,   AF_BYTEADRX|AF_WORDADR, { 0x69, 0x79 }},
    { NULL, v_mnemonic, "rola", 0,  AF_IMP, { 0x49 }},
    { NULL, v_mnemonic, "rolb", 0,  AF_IMP, { 0x59 }},
    { NULL, v_mnemonic, "ror", 0,   AF_BYTEADRX|AF_WORDADR, { 0x66, 0x76 }},
    { NULL, v_mnemonic, "rora", 0,  AF_IMP, { 0x46 }},
    { NULL, v_mnemonic, "rorb", 0,  AF_IMP, { 0x56 }},
    { NULL, v_mnemonic, "rti", 0,   AF_IMP, { 0x3B }},
    { NULL, v_mnemonic, "rts", 0,   AF_IMP, { 0x39 }},
    { NULL, v_mnemonic, "swi", 0,   AF_IMP, { 0x3F }},
    { NULL, v_mnemonic, "wai", 0,   AF_IMP, { 0x3E }},
    { NULL, v_mnemonic, "asl", 0,   AF_BYTEADRX|AF_WORDADR, { 0x68, 0x78 }},
    { NULL, v_mnemonic, "lsl", 0,   AF_BYTEADRX|AF_WORDADR, { 0x68, 0x78 }},
    { NULL, v_mnemonic, "asla", 0,  AF_IMP, { 0x48 }},
    { NULL, v_mnemonic, "aslb", 0,  AF_IMP, { 0x58 }},
    { NULL, v_mnemonic, "asld", 0,  AF_IMP, { 0x05 }},
    { NULL, v_mnemonic, "lsla", 0,  AF_IMP, { 0x48 }},  /* same thing */
    { NULL, v_mnemonic, "lslb", 0,  AF_IMP, { 0x58 }},
    { NULL, v_mnemonic, "lsld", 0,  AF_IMP, { 0x05 }},
    { NULL, v_mnemonic, "asr", 0,   AF_BYTEADRX|AF_WORDADR, { 0x67, 0x77 }},
    { NULL, v_mnemonic, "asra", 0,  AF_IMP, { 0x47 }},
    { NULL, v_mnemonic, "asrb", 0,  AF_IMP, { 0x57 }},
    { NULL, v_mnemonic, "cpx",  0,  AF_IMM16|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x8C, 0x9C, 0xAC, 0xBC }},
    { NULL, v_mnemonic, "dex", 0,   AF_IMP, { 0x09 }},
    { NULL, v_mnemonic, "des", 0,   AF_IMP, { 0x34 }},
    { NULL, v_mnemonic, "inx", 0,   AF_IMP, { 0x08 }},
    { NULL, v_mnemonic, "ins", 0,   AF_IMP, { 0x31 }},
    { NULL, v_mnemonic, "ldx", 0,   AF_IMM16|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xCE, 0xDE, 0xEE, 0xFE }},
    { NULL, v_mnemonic, "lds", 0,   AF_IMM16|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x8E, 0x9E, 0xAE, 0xBE }},
    { NULL, v_mnemonic, "lsr", 0,   AF_BYTEADRX|AF_WORDADR, { 0x64, 0x74 }},
    { NULL, v_mnemonic, "lsra", 0,  AF_IMP, { 0x44 }},
    { NULL, v_mnemonic, "lsrb", 0,  AF_IMP, { 0x54 }},
    { NULL, v_mnemonic, "lsrd", 0,  AF_IMP, { 0x04 }},
    { NULL, v_mnemonic, "staa", 0,  AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x97, 0xA7, 0xB7 }},
    { NULL, v_mnemonic, "stab", 0,  AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xD7, 0xE7, 0xF7 }},
    { NULL, v_mnemonic, "std", 0,   AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xDD, 0xED, 0xFD }},
    { NULL, v_mnemonic, "sts", 0,   AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x9F, 0xAF, 0xBF }},
    { NULL, v_mnemonic, "stx", 0,   AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xDF, 0xEF, 0xFF }},
    { NULL, v_mnemonic, "suba", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x80, 0x90, 0xA0, 0xB0 }},
    { NULL, v_mnemonic, "subb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC0, 0xD0, 0xE0, 0xF0 }},
    { NULL, v_mnemonic, "subd", 0,  AF_IMM16|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x83, 0x93, 0xA3, 0xB3 }},
    { NULL, v_mnemonic, "sba", 0,   AF_IMP, { 0x10 }},
    { NULL, v_mnemonic, "sbca", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x82, 0x92, 0xA2, 0xB2 }},
    { NULL, v_mnemonic, "sbcb", 0,  AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0xC2, 0xD2, 0xE2, 0xF2 }},
    { NULL, v_mnemonic, "tab", 0,   AF_IMP, { 0x16 }},
    { NULL, v_mnemonic, "tba", 0,   AF_IMP, { 0x17 }},
    { NULL, v_mnemonic, "tst", 0,   AF_BYTEADRX|AF_WORDADR, { 0x6D, 0x7D }},
    { NULL, v_mnemonic, "tsta", 0,  AF_IMP, { 0x4D }},
    { NULL, v_mnemonic, "tstb", 0,  AF_IMP, { 0x5D }},
    { NULL, v_mnemonic, "tsx", 0,   AF_IMP, { 0x30 }},
    { NULL, v_mnemonic, "txs", 0,   AF_IMP, { 0x35 }},
    MNEMONIC_NULL
};

MNEMONIC MneHD6303[] = {
    { NULL, v_mnemonic, "slp", 0,   AF_IMP, { 0x1A }},
    { NULL, v_mnemonic, "xgdx", 0,  AF_IMP, { 0x18 }},
    /* [phf] these are broken and fixing them would require major surgery, see BUGS file */
    { NULL, v_mnemonic, "aim", 0,   AF_BYTEADR|AF_BYTEADRX, { 0x71, 0x61 }},
    { NULL, v_mnemonic, "oim", 0,   AF_BYTEADR|AF_BYTEADRX, { 0x72, 0x62 }},
    { NULL, v_mnemonic, "eim", 0,   AF_BYTEADR|AF_BYTEADRX, { 0x75, 0x65 }},
    { NULL, v_mnemonic, "tim", 0,   AF_BYTEADR|AF_BYTEADRX, { 0x7B, 0x6B }},
    MNEMONIC_NULL
};

