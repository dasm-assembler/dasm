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
 *  FTOHEX.C
 *
 *  FTOHEX format infile [outfile]
 *
 *  format: format used when assembling (asm705/asm65)
 *	    1,2,3	    -generate straight hex file
 *
 *  Note that int and char are not used as Lattice C on IBM PCs uses
 *  16 bit ints and unsigned chars.
 */

#include <stdio.h>
#include <stdlib.h>

#define PERLINE 16

void exiterr(const char *str);
void convert(int format, FILE *in, FILE *out);
unsigned int getwlh(FILE *in);
void puth(unsigned char c, FILE *out);

int
main(int ac, char **av)
{
    int format;
    FILE *infile;
    FILE *outfile;

    if (ac < 3) {
	puts("FTOHEX format infile [outfile]");
	puts("format 1 = DEFAULT, 2 = RAS, or 3 = RAW");
	puts("Copyright (c) 1988-2008 by various authors (see file AUTHORS).");
	exit(1);
    }
    format = atoi(av[1]);
    if (format < 1 || format > 3)
	exiterr("specify infile format 1, 2, or 3");
    infile = fopen(av[2], "r");
    if (infile == NULL)
	exiterr("unable to open input file");
    outfile = (av[3]) ? fopen(av[3], "w") : stdout;
    if (outfile == NULL)
	exiterr("unable to open output file");
    convert(format, infile, outfile);
    fclose(infile);
    fclose(outfile);

    return 0;
}

void
exiterr(const char *str)
{
    fputs(str, stderr);
    fputs("\n", stderr);
    exit(1);
}

/*
 *  Formats:
 *
 *  1:	  origin (word:lsb,msb) + data
 *  2:	  origin (word:lsb,msb) + length (word:lsb,msb) + data	(repeat)
 *  3:	  data
 *
 *  Hex output:
 *
 *  :lloooo00(ll bytes hex code)cc	  ll=# of bytes
 *		      oooo=origin
 *			cc=invert of checksum all codes
 */

void
convert(int format, FILE *in, FILE *out)
{
    unsigned int org = 0;
    unsigned int idx;
    long len;
    unsigned char buf[256];

    if (format < 3)
    org = getwlh(in);
    if (format == 2) {
	len = getwlh(in);
    } else {
	long begin = ftell(in);
	fseek(in, 0, SEEK_END);
	len = ftell(in) - begin;
	fseek(in, begin, 0);
    }
    for (;;) {
	while (len > 0) {
	    register unsigned char chk;
	    register unsigned int i;

	    idx = (len > PERLINE) ? PERLINE : len;
	    fread(buf, idx, 1, in);
	    putc(':', out);
	    puth(idx, out);
	    puth(org >> 8, out);
	    puth(org & 0xFF, out);
	    putc('0', out);
	    putc('0', out);
	    chk = idx + (org >> 8) + (org & 0xFF);
	    for (i = 0; i < idx; ++i) {
		chk += buf[i];
		puth(buf[i], out);
	    }
	    puth((unsigned char)-chk, out);
	    putc('\r', out);
	    putc('\n', out);
	    len -= idx;
	    org += idx;
	}
	if (format == 2) {
	    org = getwlh(in);
	    if (feof(in))
		break;
	    len = getwlh(in);
	} else {
	    break;
	}
    }
    fprintf(out, ":00000001FF\r\n");
}

unsigned int getwlh(FILE *in)
{
    unsigned int result;

    result = getc(in);
    result += getc(in) << 8;
    return result;
}

void
puth(unsigned char c, FILE *out)
{
    static char dig[] = { "0123456789ABCDEF" };
    putc(dig[c>>4], out);
    putc(dig[c&15], out);
}

