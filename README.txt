
    DASM    Version 2.20.10

    DASM was created in 1988 by Matthew Dillon (dillon@apollo.backplane.com)
    and modified in 1995 by Olaf 'Rhialto' Seibert (rhialto@polderland.nl), and
    is now updated and maintained by Andrew Davie (atari2600@taswegian.com)

    DASM's homepage is http://www.atari2600.org/dasm

    DASM is freely re-distributable, and provided with full source-code to
    allow recompilation for other target-platforms.  This package may not be
    sold on any media, or distributed in any way other than totally free.

    Directory structure...

	BIN\DOS                 Binary for DOS/Windows
	BIN\Mac\Classic		Binary for Mac Classic (2.20.07 onwards)
	BIN\Mac\OSX		Binary for Mac OSX (2.20.07 onwards)
	BIN\Linux               Binaries (static and dynamic) (2.20.09 onwards)
	DOC                     Documentation and manuals
	SRC                     DASM Source code
	TEST                    Suite of test code

    This file (README.TXT) will always list the most recent updates for
    the DASM disributable.


VERSION HISTORY

    2.20.10     - Fairchild Channel F (processor F8) support added.  Thanks to Thomas Mathys!
		- Please review the readme file in the new F8 directory.
		- There have been a LOT of code modifications to improve readability/maintainability.
		- ONLY the DOS binary is updated in this release;  if you are able to provide binaries for
		  other platforms, please send to me so I can update.

    2.20.09     - various cleanups and code modifications.  Switched from typedefs to actual type usage to allow
                  Linux compatibility (Linux had ulong defined).  Various minor changes.
                - Linux versions compiled by Brian Wilson.

    2.20.08     - started to do some source code changes - I think.
                - // comments changed/removed
		- changed DOC files to text format.  We may go to HTML soon, but Word is definitely
		  a thing of the past!

    2.20.07     - (3rd posting, Mac OSX version included!  Thanks Jeremy Penner)

    2.20.07     - (2nd posting, Amiga Version included!  Thanks, Peter Gordon)

    2.20.07     - Rewrote symbol table sort to NOT use string functions

                - Removed '_fmode' code altogether

                - Conditional compilation around 

                - Mac binary provided by Jason Rein

		- I appear to have misplaced the Amiga binary???


    2.20.05	- modified the file open from using _fmode (what was that FOR?!) to a "wb"
		  (as suggested by ES and PG)
		
		- changed itoa(...) usage to sprintf (PG - Amiga)

		- changed _stricmp(...) to stricmp (again, PG - amiga)

		- fixed Symbol Table Dump so it outputs symbol table even if errors occur
		
		- symbol table now displays string symbols in string form, too

		- further modifications to code (removed REGISTER keywords).

		- AMIGA support (binary included! - thanks Peter Gordon)



    2.20.04	- Significant reformatting of code, prelude to full documentation.

		- Internal error numbers now equated (more maintanable code).

		- Command-line switches now allow / as prefix (and -).

		- Divide by 0 (generated equates, etc.) now treated as an unrecoverable error.

		- Error messages now contain more information about the error, including
                  the source-line where possible.  Message templates allow inclusion of select
		  data on error output.

                - reworking of the output data.  Typically, the -v switch is now un-necessary
                  as the appropriate messages to allow you to find errors will be output.  
                  Back compatibility with the -v options has been maintained, though there
                  may be a few minor changes.

                - Branch out of range errors now list the 'distance' to the branch.

                - Undefined symbols are now *always* listed in a separate table.  If an 
		  unresolved symbol causes an error, then the table will be automatically
                  displayed at the end of assembly.

                - new command-line switch -T# where # is 0 or 1
		    0 = sort symbol table alphabetically (default)
		    1 = sort symbol table by address
		  If available memory precludes a sort, then the table will be output in a
		  non-sorted order.

		- code now in transient state between old and new formatting.  More modular, but
		  still all over the place as far as consistency.  It'll improve as we go along.


    2.20.02     - Inclusion of full (?) illegal opcode support using MNE6502.C by
                  John Saeger.

    2.20.01     - This version combines the modifications made by Olaf in 1995
                  into an official release, with minor additions.  The source-code
                  is currently being commented and/or rewritten to make future
                  maintenance easier.

                  For Olaf's changes, see the #ifdef'ed parts in DASM.DOC.
                  These modifications have now become a part of the main code-line
                  of DASM, so they are not optional.  Documentation will be merged
                  in a later version.

                - Machine-specific support is now provided for Atari 2600.
                  Other machines may be added at later dates.

                - 6502/7 illegal opcodes NOP zp and LAX (zp),y added

    2.12.15     Matt's ultimate release, updates a memory-allocation problem on
                SPARC systems.  This fix appears to have been fixed in Olaf's
                version, so 2.12.14 (*not* 2.12.15) was used as the code-base
                for 2.20.01

    2.12.14     Olaf's release, including many additions - many of which are
                conditionally-compiled.


COMPILATION

    The source code assumes that integers are LONG WORDS.  All expression
    computation is carried out with 32 bit ints.  Additionaly, the
    correct implementation of STDIO functions is assumed (no translation).

    This code will compile with little or no modification on DOS, Amiga,
    Mac and UNIX systems.


FEATURES

    -relatively fast
    -processor selection (use same assembler for many processors)
    -multi-pass (more than two passes if required)
    -conditional assembly
    -local labels (VERY local labels)
    -macro capability (no stacking limit)
    -symbolic expression capability
    -addressing mode overides
    -arbitrary number of named segments
    -many pseudo-ops for repeat loops, data generation, etc....
    -automatic checksumming accessed via the '...' symbol.


PROCESSORS

      --------------- CURRENTLY SUPPORTED MICROPROCESSORS ---------------


6502:	    ORDER LSB,MSB   A.b    X.b	  Y.b	    RelAddr:   .+ilen+offset
68705:	    ORDER MSB,LSB   A.b    X.b		    RelAddr:   .+ilen+offset
6803/HD6303:ORDER MSB,LSB   A.b    B.b	  X.w	    RelAddr:   .+ilen+offset
6811:	    ORDER MSB,LSB   A.b    B.b	  X.w  Y.w  RelAddr:   .+ilen+offset


ADDRESSING MODES		6502	68705	6803	6811
BYTES						HD6303

 2  implied			x	x	x	x
 2  immediate.8     #byte	x	x	x	x
 3  immediate.16    #word			x	x
 2  byteaddr	    byte	x	x	x	x
 2  byteaddr,x	    byte,x	x	x	x	x
 2  byteaddr,y	    byte,y	x			x
 3  wordaddr	    word	x	x	x	x
 3  wordaddr,x	    word,x	x	x
 3  wordaddr,y	    word,y	x
 2  relative	    byte	x	x	x	x
 2  ind.byte.x	    (byte,x)    x
 2  ind.byte.y	    (byte),y    x
 3  ind.word	    (word)      x
 1  0,x 	    [0],x		x
 2  bitmod	    #no,badr		x		    baseinst + 2*bitno
 3  bitbramod	    #no,badr,rel	x		    baseinst + 2*bitno

NOTE:	HD6303 instruction extensions over the 6803 are:
	    AIM OIM EIM TIM XGDX SLP

I believe the 6811 is a superset of the 6803.
For 6507 (Atari 2600) use 6502 as the selected processor.


;EOF
