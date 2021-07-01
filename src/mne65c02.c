/*
    $Id: mne6502.c 327 2014-02-09 13:06:55Z adavie $

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
 *  MNEWDC65c02.C
 */

#include "asm.h"

#define ASTD	AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX|\
	AF_WORDADRY|AF_INDBYTEX|AF_INDBYTEY|AF_INDBYTE

MNEMONIC Mne65c02[] = {
    { NULL, v_mnemonic, "adc", 0, AF_IMM8|ASTD, { 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71, 0x72 } },
    { NULL, v_mnemonic, "and", 0, AF_IMM8|ASTD, { 0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31, 0x32 } },
    { NULL, v_mnemonic, "asl", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0x0A, 0x06, 0x16, 0x0E, 0x1E } },
    { NULL, v_mnemonic, "bbr0", 0, AF_BYTEREL, { 0x0f } },  /* Branch on Bit 0 Reset */
    { NULL, v_mnemonic, "bbr1", 0, AF_BYTEREL, { 0x1f } },  /* Branch on Bit 1 Reset */
    { NULL, v_mnemonic, "bbr2", 0, AF_BYTEREL, { 0x2f } },  /* Branch on Bit 2 Reset */
    { NULL, v_mnemonic, "bbr3", 0, AF_BYTEREL, { 0x3f } },  /* Branch on Bit 3 Reset */
    { NULL, v_mnemonic, "bbr4", 0, AF_BYTEREL, { 0x4f } },  /* Branch on Bit 4 Reset */
    { NULL, v_mnemonic, "bbr5", 0, AF_BYTEREL, { 0x5f } },  /* Branch on Bit 5 Reset */
    { NULL, v_mnemonic, "bbr6", 0, AF_BYTEREL, { 0x6f } },  /* Branch on Bit 6 Reset */
    { NULL, v_mnemonic, "bbr7", 0, AF_BYTEREL, { 0x7f } },  /* Branch on Bit 7 Reset */
    { NULL, v_mnemonic, "bbs0", 0, AF_BYTEREL, { 0x8f } },  /* Branch on Bit 0 Set */
    { NULL, v_mnemonic, "bbs1", 0, AF_BYTEREL, { 0x9f } },  /* Branch on Bit 1 Set */
    { NULL, v_mnemonic, "bbs2", 0, AF_BYTEREL, { 0xaf } },  /* Branch on Bit 2 Set */
    { NULL, v_mnemonic, "bbs3", 0, AF_BYTEREL, { 0xbf } },  /* Branch on Bit 3 Set */
    { NULL, v_mnemonic, "bbs4", 0, AF_BYTEREL, { 0xcf } },  /* Branch on Bit 4 Set */
    { NULL, v_mnemonic, "bbs5", 0, AF_BYTEREL, { 0xdf } },  /* Branch on Bit 5 Set */
    { NULL, v_mnemonic, "bbs6", 0, AF_BYTEREL, { 0xef } },  /* Branch on Bit 6 Set */
    { NULL, v_mnemonic, "bbs7", 0, AF_BYTEREL, { 0xff } },  /* Branch on Bit 7 Set */
    { NULL, v_mnemonic, "bcc", 0, AF_REL, { 0x90 } },
    { NULL, v_mnemonic, "bcs", 0, AF_REL, { 0xB0 } },
    { NULL, v_mnemonic, "beq", 0, AF_REL, { 0xF0 } },
    { NULL, v_mnemonic, "bit", 0, AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0x89, 0x24, 0x34, 0x2C, 0x3c } },
    { NULL, v_mnemonic, "bmi", 0, AF_REL, { 0x30 } },
    { NULL, v_mnemonic, "bne", 0, AF_REL, { 0xD0 } },
    { NULL, v_mnemonic, "bpl", 0, AF_REL, { 0x10 } },
    { NULL, v_mnemonic, "bra", 0, AF_REL, { 0x80 } },   /* BRanch Always */
    { NULL, v_mnemonic, "brk", 0, AF_IMP, { 0x00 } },
    { NULL, v_mnemonic, "bvc", 0, AF_REL, { 0x50 } },
    { NULL, v_mnemonic, "bvs", 0, AF_REL, { 0x70 } },
    { NULL, v_mnemonic, "clc", 0, AF_IMP, { 0x18 } },
    { NULL, v_mnemonic, "cld", 0, AF_IMP, { 0xD8 } },
    { NULL, v_mnemonic, "cli", 0, AF_IMP, { 0x58 } },
    { NULL, v_mnemonic, "clv", 0, AF_IMP, { 0xB8 } },
    { NULL, v_mnemonic, "cmp", 0, AF_IMM8|ASTD, { 0xC9, 0xC5, 0xD5, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1, 0xd2 } },
    { NULL, v_mnemonic, "cpx", 0, AF_IMM8|AF_BYTEADR|AF_WORDADR, { 0xE0, 0xE4, 0xEC } },
    { NULL, v_mnemonic, "cpy", 0, AF_IMM8|AF_BYTEADR|AF_WORDADR, { 0xC0, 0xC4, 0xCC } },
    { NULL, v_mnemonic, "dec", 0, AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0xC6, 0xD6, 0xCE, 0xDE } },
    { NULL, v_mnemonic, "dea", 0, AF_IMP, { 0x3A } },   /* DEcrement A */
    { NULL, v_mnemonic, "dex", 0, AF_IMP, { 0xCA } },
    { NULL, v_mnemonic, "dey", 0, AF_IMP, { 0x88 } },
    { NULL, v_mnemonic, "eor", 0, AF_IMM8|ASTD, { 0x49, 0x45, 0x55, 0x4D, 0x5D, 0x59, 0x41,0x51, 0x52 } },
    { NULL, v_mnemonic, "ina", 0, AF_IMP, { 0x1A } },   /* INcrement A */
    { NULL, v_mnemonic, "inc", 0, AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0xE6, 0xF6, 0xEE, 0xFE } },
    { NULL, v_mnemonic, "inx", 0, AF_IMP, { 0xE8 } },
    { NULL, v_mnemonic, "iny", 0, AF_IMP, { 0xC8 } },
    { NULL, v_mnemonic, "jmp", 0, AF_WORDADR|AF_INDBYTEX|AF_INDWORD|AF_INDWORDX|AF_INDBYTE, { 0x4C, 0x42, 0x6C, 0x7c, 0x42 } },   
    { NULL, v_mnemonic, "jsr", 0, AF_WORDADR, { 0x20 } },
    { NULL, v_mnemonic, "lda", 0, AF_IMM8|ASTD, { 0xA9, 0xA5, 0xB5, 0xAD, 0xBD, 0xB9, 0xA1, 0xB1, 0xb2 } },
    { NULL, v_mnemonic, "ldx", 0, AF_IMM8|AF_BYTEADR|AF_BYTEADRY|AF_WORDADR|AF_WORDADRY, { 0xA2, 0xA6, 0xB6, 0xAE, 0xBE } },
    { NULL, v_mnemonic, "ldy", 0, AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0xA0, 0xA4, 0xB4, 0xAC, 0xBC } },
    { NULL, v_mnemonic, "lsr", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0x4A, 0x46, 0x56, 0x4E, 0x5E } },
    { NULL, v_mnemonic, "nop", 0, AF_IMP|AF_IMM8|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0xEA, 0x80, 0x04, 0x14, 0x0c, 0x1c } },
    { NULL, v_mnemonic, "ora", 0, AF_IMM8|ASTD, { 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11, 0x12 } },
    { NULL, v_mnemonic, "pha", 0, AF_IMP, { 0x48 } },
    { NULL, v_mnemonic, "phx", 0, AF_IMP, { 0xda } },   /* PusH X */
    { NULL, v_mnemonic, "phy", 0, AF_IMP, { 0x5a } },   /* PusH Y */
    { NULL, v_mnemonic, "php", 0, AF_IMP, { 0x08 } },
    { NULL, v_mnemonic, "pla", 0, AF_IMP, { 0x68 } },
    { NULL, v_mnemonic, "plp", 0, AF_IMP, { 0x28 } },
    { NULL, v_mnemonic, "plx", 0, AF_IMP, { 0xfa } },   /* Pull X */
    { NULL, v_mnemonic, "ply", 0, AF_IMP, { 0x7a } },   /* Pull Y */
    { NULL, v_mnemonic, "rmb0", 0, AF_BYTEADR, { 0x07 } },  /* Reset Memory Bit 0 */
    { NULL, v_mnemonic, "rmb1", 0, AF_BYTEADR, { 0x17 } },  /* Reset Memory Bit 1 */
    { NULL, v_mnemonic, "rmb2", 0, AF_BYTEADR, { 0x27 } },  /* Reset Memory Bit 2 */
    { NULL, v_mnemonic, "rmb3", 0, AF_BYTEADR, { 0x37 } },  /* Reset Memory Bit 3 */
    { NULL, v_mnemonic, "rmb4", 0, AF_BYTEADR, { 0x47 } },  /* Reset Memory Bit 4 */
    { NULL, v_mnemonic, "rmb5", 0, AF_BYTEADR, { 0x57 } },  /* Reset Memory Bit 5 */
    { NULL, v_mnemonic, "rmb6", 0, AF_BYTEADR, { 0x67 } },  /* Reset Memory Bit 6 */
    { NULL, v_mnemonic, "rmb7", 0, AF_BYTEADR, { 0x77 } },  /* Reset Memory Bit 7 */
    { NULL, v_mnemonic, "rol", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0x2A, 0x26, 0x36, 0x2E, 0x3E } },
    { NULL, v_mnemonic, "ror", 0, AF_IMP|AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0x6A, 0x66, 0x76, 0x6E, 0x7E } },
    { NULL, v_mnemonic, "rti", 0, AF_IMP, { 0x40 } },
    { NULL, v_mnemonic, "rts", 0, AF_IMP, { 0x60 } },
    { NULL, v_mnemonic, "sbc", 0, AF_IMM8|ASTD, { 0xE9, 0xE5, 0xF5, 0xED, 0xFD, 0xF9, 0xE1, 0xF1, 0xf2 } },
    { NULL, v_mnemonic, "sec", 0, AF_IMP, { 0x38 } },
    { NULL, v_mnemonic, "sed", 0, AF_IMP, { 0xF8 } },
    { NULL, v_mnemonic, "sei", 0, AF_IMP, { 0x78 } },
    { NULL, v_mnemonic, "smb0", 0, AF_BYTEADR, { 0x87 } },    /* Set Memory Bit 0 */
    { NULL, v_mnemonic, "smb1", 0, AF_BYTEADR, { 0x97 } },    /* Set Memory Bit 1 */
    { NULL, v_mnemonic, "smb2", 0, AF_BYTEADR, { 0xa7 } },    /* Set Memory Bit 2 */
    { NULL, v_mnemonic, "smb3", 0, AF_BYTEADR, { 0xb7 } },    /* Set Memory Bit 3 */
    { NULL, v_mnemonic, "smb4", 0, AF_BYTEADR, { 0xc7 } },    /* Set Memory Bit 4 */
    { NULL, v_mnemonic, "smb5", 0, AF_BYTEADR, { 0xd7 } },    /* Set Memory Bit 5 */
    { NULL, v_mnemonic, "smb6", 0, AF_BYTEADR, { 0xe7 } },    /* Set Memory Bit 6 */
    { NULL, v_mnemonic, "smb7", 0, AF_BYTEADR, { 0xf7 } },    /* Set Memory Bit 7 */
    { NULL, v_mnemonic, "sta", 0, ASTD, { 0x85, 0x95, 0x8D, 0x9D, 0x99, 0x81, 0x91, 0x92 } },
    { NULL, v_mnemonic, "stx", 0, AF_BYTEADR|AF_BYTEADRY|AF_WORDADR, { 0x86, 0x96, 0x8E } },
    { NULL, v_mnemonic, "stp", 0, AF_IMP, { 0xdb } },   /* SToP the processor until reset */
    { NULL, v_mnemonic, "sty", 0, AF_BYTEADR|AF_BYTEADRX|AF_WORDADR, { 0x84, 0x94, 0x8C } },
    { NULL, v_mnemonic, "stz", 0, AF_BYTEADR|AF_BYTEADRX|AF_WORDADR|AF_WORDADRX, { 0x64, 0x74, 0x9c, 0x9e } },  /* Store zero */
    { NULL, v_mnemonic, "tax", 0, AF_IMP, { 0xAA } },
    { NULL, v_mnemonic, "tay", 0, AF_IMP, { 0xA8 } },
    { NULL, v_mnemonic, "trb", 0, AF_BYTEADR|AF_WORDADR, { 0x14, 0x1c } },  /* Test and Reset Bits */
    { NULL, v_mnemonic, "tsb", 0, AF_BYTEADR|AF_WORDADR, { 0x04, 0x0c } },  /* Test and Set Bits */
    { NULL, v_mnemonic, "tsx", 0, AF_IMP, { 0xBA } },
    { NULL, v_mnemonic, "txa", 0, AF_IMP, { 0x8A } },
    { NULL, v_mnemonic, "txs", 0, AF_IMP, { 0x9A } },
    { NULL, v_mnemonic, "tya", 0, AF_IMP, { 0x98 } },
    { NULL, v_mnemonic, "wai", 0, AF_IMP, { 0xcb } },   /* WAit for Interrupt */
    MNEMONIC_NULL
};

