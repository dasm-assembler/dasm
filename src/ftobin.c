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

/*
 *  FTOBIN.C
 *
 *  FTOBIN format infile [outfile]
 *
 *  format: format used when assembling (asm705/asm65)
 *	    1,2,3	    -generate sequenced (raw) binary file
 *
 *  Used with formats 1 or 2 (format 3 is already raw binary).	This
 *  program converts formats 1 and 2 to format 3.  This is required
 *  because some assembly programs might want to reverse-index the
 *  origin (go back), usually to lay down a checksum, and this can only be
 *  done with format 2.
 *
 *  Restrictions:   Lowest address must be referenced first.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned char ubyte;
typedef unsigned short uword;

#define PERLINE 16

static uword getwlh(FILE *in);
static void exiterr(const char *str);
static void convert(short format, FILE *in, FILE *out);

int
main(int ac, char **av)
{
    short format;
    FILE *infile;
    FILE *outfile;

    if (ac < 3) {
	puts("FTOBIN format infile [outfile]");
	puts("format 1,2, or 3.  3=raw");
	puts("(C)Copyright 1988 by Matthew Dillon, All Rights Reserved");
	exit(EXIT_FAILURE);
    }
    format = strtol(av[1], NULL, 0);
    if (format < 1 || format > 3)
	exiterr("specify infile format 1, 2, or 3");
    if (format == 3) {
	puts("Note: Format 3 is already a raw binary file, output will");
	puts("be equivalent to input");
    }
    infile = fopen(av[2], "r");
    if (infile == NULL)
	exiterr("unable to open input file");
    outfile = (av[3]) ? fopen(av[3], "w") : stdout;
    if (outfile == NULL)
	exiterr("unable to open output file");
    convert(format, infile, outfile);
    fclose(infile);
    fclose(outfile);
    return(0);
}

static
void
exiterr(const char *str)
{
    fputs(str, stderr);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

/*
 *  Formats:
 *
 *  1:	origin (word:lsb,msb) + data
 *  2:	origin (word:lsb,msb) + length (word:lsb,msb) + data  (repeat)
 *  3:	data
 *
 *  Hex output:
 *
 *  :lloooo00(ll bytes hex code)cc	  ll=# of bytes
 *					oooo=origin
 *					  cc=invert of checksum all codes
 */

static
void
convert(short format, FILE *in, FILE *out)
{
    uword org = 0;
    uword base = 0;
    long len;
    long maxseek = 0;
    ubyte buf[256];

    if (format < 3)
	base = org = getwlh(in);
    if (format == 2) {
	len = getwlh(in);
    } else {
	long begin = ftell(in);
	fseek(in, 0L, 2);
	len = ftell(in) - begin;
	fseek(in, begin, 0);
    }
    for (;;) {
	if (len > 0) {
	    while (len >= ((long) sizeof(buf))) {
		fread(buf, sizeof(buf), 1, in);
		fwrite(buf, sizeof(buf), 1, out);
		len -= sizeof(buf);
		org += sizeof(buf);
	    }
	    if (len) {
		fread(buf, (short)len, 1, in);
		fwrite(buf, (short)len, 1, out);
		org += len;
	    }
	}
	if (format == 2) {
	    if (maxseek < org - base)
		maxseek = org - base;
	    org = getwlh(in);
	    if (feof(in))
		break;
	    len = org - base;
	    if (len < 0) {
		puts("ERROR!  Reverse indexed to before beginning");
		puts("*Initial* origin must be the lowest address");
		return;
	    }
	    len -= maxseek;
	    if (len > 0)
		memset(buf, 255, sizeof(buf));
	    while (len > 0) {
		if (len >= ((long) sizeof(buf))) {
		    fwrite(buf, sizeof(buf), 1, out);
		    maxseek += sizeof(buf);
		    len -= sizeof(buf);
		} else {
		    fwrite(buf, (short)len, 1, out);
		    maxseek += len;
		    len = 0;
		}
	    }
	    fseek(out, (long)(org - base), 0);
	    len = getwlh(in);
	} else {
	    break;
	}
    }
}

static
uword
getwlh(FILE *in)
{
    uword result;

    result = getc(in);
    result += getc(in) << 8;
    return(result);
}

/* vim: set tabstop=4 softtabstop=4 expandtab shiftwidth=4 autoindent: */
