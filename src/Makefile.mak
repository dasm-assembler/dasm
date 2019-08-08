#
#    DASM Assembler
#    Portions of this code are Copyright (C)1988 Matthew Dillon
#    and (C) 1995 Olaf Seibert, (C)2003 Andrew Davie 
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

#   DASM    -small systems cross assembler
#   DASM    -small systems cross assembler.  The source is compilable on
#	    Amiga's, UNIX systems, and any other system with ISO-C compiler.
#   (C)Copyright 1988-1989 Matthew Dillon, All Rights Reserved.
#
#   Modifications Copyright 1995 by Olaf Seibert. All Rights Reserved.



#GOPTIM= -O3
#GWARN=	-ansi -pedantic -Wall -Wstrict-prototypes
GDB=	# -g
#CC=	gcc $(GDB) $(GOPTIM) $(GWARN)
CC=	cl
CFLAGS=
# /O2 /WX
CFLAGS= /W0

OBJS=       main.o \
            ops.o \
            globals.o \
            exp.o \
            symbols.o \
            mne6303.o \
            mne6502.o \
            mne68705.o \
            mne6811.o \
            mnef8.o

SRCS= main.c ops.c globals.c exp.c symbols.c \
      mne6303.c mne6502.c mne68705.c mne6811.c mnef8.c


.c.o:

all: dasm #ftohex

dasm: 
	$(CC) $(CFLAGS) $(SRCS) /o..\bin\DOS\dasm.exe

ftohex: ftohex.o
	$(CC) ftohex.o -oftohex

example:
	dasm example.asm -oram:example.out -lram:example.list -f2
	ftohex 2 ram:example.out ram:example.hex

obj: $(OBJS)



$(OBJS): asm.h


DOS: $(OBJS)
    $(DOS_CC) $(CFLAGS) $(SRCS) -odasm.exe

AMIGA: $(OBJS)
    $(DOS_CC) $(CFLAGS) $(SRCS) -odasm.exe

LINUX: $(OBJS)
    $(DOS_CC) $(CFLAGS) $(SRCS) -odasm.exe

MAC_CLASSIC: $(OBJS)
    $(DOS_CC) $(CFLAGS) $(SRCS) -odasm.exe

MAC_OSX: $(OBJS)
    $(DOS_CC) $(CFLAGS) $(SRCS) -odasm.exe

