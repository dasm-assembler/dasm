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
 *  MAIN.C
 *  DASM   sourcefile
 *  NOTE: must handle mnemonic extensions and expression decode/compare.
 */

#include <strings.h>
#include <unistd.h>

#include "version.h"
#include "asm.h"

static const char dasm_id[] = DASM_ID;

#define ISEGNAME    "INITIAL CODE SEGMENT"

/*
   replace old atoi() calls; I wanted to protect this using
   #ifdef strtol but the C preprocessor doesn't recognize
   function names, at least not GCC's; we should be safe
   since MS compilers document strtol as well... [phf]
*/
#define atoi(x) ((int)strtol(x, (char **)NULL, 10))

static const char *cleanup(char *buf, bool bDisable);

MNEMONIC *parse(char *buf);
void panic(const char *str);
MNEMONIC *findmne(char *str);
void clearsegs(void);
void clearrefs(void);

static unsigned int hash1(const char *str);
static void outlistfile(const char *);

// buffers to supress errors and messages until last pass
char *passbuffer [2] = { NULL, NULL};
#define ERRORBUF 0
#define MSGBUF 1
void passbuffer_clear(int);
void passbuffer_update(int,char *);
void passbuffer_output(int);
void passbuffer_cleanup(void);

int mlflag = 0; // multi-line comments

static char erroradd1[500]; // temp error holders
static char erroradd2[500];
static char erroradd3[500];

/* Table encapsulates errors, descriptions, and fatality flags. */

ERROR_DEFINITION sErrorDef[] = {

    /* Error #, STOPEND, Description */

    { ERROR_NONE,                                   true,   "OK"   },
    { ERROR_COMMAND_LINE,                           true,   "Check command-line format."   },
    { ERROR_FILE_ERROR,                             true,   "Unable to open file."   },
    { ERROR_NOT_RESOLVABLE,                         true,   "Source is not resolvable."   },
    { ERROR_TOO_MANY_PASSES,                        true,   "Too many passes (%s)."   },
    { ERROR_NON_ABORT,                              true,   "See previous output"   },
    { ERROR_SYNTAX_ERROR,                           true,   "Syntax Error '%s'."   },
    { ERROR_EXPRESSION_TABLE_OVERFLOW,              true,   "Expression table overflow."   },
    { ERROR_UNBALANCED_BRACES,                      true,   "Unbalanced Braces []."   },
    { ERROR_DIVISION_BY_0,                          true,   "Division by zero."  },
    { ERROR_UNKNOWN_MNEMONIC,                       true,   "Unknown Mnemonic '%s'."   },
    { ERROR_ILLEGAL_ADDRESSING_MODE,                false,  "Illegal Addressing mode '%s'."   },
    { ERROR_ILLEGAL_FORCED_ADDRESSING_MODE,         true,   "Illegal forced Addressing mode on '%s'."   },
    { ERROR_NOT_ENOUGH_ARGUMENTS_PASSED_TO_MACRO,   true,   "Not enough args passed to Macro."   },
    { ERROR_PREMATURE_EOF,                          false,  "Premature EOF."   },
    { ERROR_ILLEGAL_CHARACTER,                      true,   "Illegal character '%s'."   },
    { ERROR_BRANCH_OUT_OF_RANGE,                    false,   "Branch out of range (%s bytes)."   },
    { ERROR_ERR_PSEUDO_OP_ENCOUNTERED,              true,   "ERR pseudo-op encountered."  },
    { ERROR_ORIGIN_REVERSE_INDEXED,                 false,  "Origin Reverse-indexed."   },
    { ERROR_EQU_VALUE_MISMATCH,                     false,  "EQU: Value mismatch."   },
    { ERROR_ADDRESS_MUST_BE_LT_100,                 true,   "Value in '%s' must be <$100."  },
    { ERROR_ADDRESS_MUST_BE_LT_10000,               true,   "Value in '%s' must be <$10000."  },
    { ERROR_ILLEGAL_BIT_SPECIFICATION,              true,   "Illegal bit specification."   },
    { ERROR_NOT_ENOUGH_ARGS,                        true,   "Not enough arguments."   },
    { ERROR_LABEL_MISMATCH,                         false,   "Label mismatch...\n --> %s"  },
    { ERROR_MACRO_REPEATED,                         true,   "Macro \"%s\" definition is repeated."  },
    { ERROR_VALUE_UNDEFINED,                        true,   "Value Undefined."   },
    { ERROR_PROCESSOR_NOT_SUPPORTED,                true,   "Processor '%s' not supported."  },
    { ERROR_REPEAT_NEGATIVE,                        false,  "REPEAT parameter < 0 (ignored)."   },
    { ERROR_BADERROR,                               true,   "Bad error value (internal error)." },
    { ERROR_ONLY_ONE_PROCESSOR_SUPPORTED,           true,   "Only one processor type may be selected." },
    { ERROR_BAD_FORMAT,                             true,   "Bad output format specified." },
	{ ERROR_VALUE_MUST_BE_1_OR_4,					true,	"Value in '%s' must be 1 or 4." },
	{ ERROR_VALUE_MUST_BE_LT_10,					true,	"Value in '%s' must be <$10." },
	{ ERROR_VALUE_MUST_BE_LT_8,						true,	"Value in '%s' must be <$8." },
	{ ERROR_VALUE_MUST_BE_LT_F,						true,	"Value in '%s' must be <$f." },
	{ ERROR_VALUE_MUST_BE_LT_10000,					true,	"Value in '%s' must be <$10000." },
	{ ERROR_ILLEGAL_OPERAND_COMBINATION,			true,	"Illegal combination of operands '%s'" },
	{ ERROR_RECURSION_TOO_DEEP,                     true, "Recursion too deep in %s" },
	{ ERROR_AVOID_SEGFAULT,				true, "Internal error in %s" },
	{ ERROR_MISSING_ENDM,				true, "Unbalanced macro %s" },
	{ ERROR_MISSING_COMMENT_END,			true, "Multi-line comment not closed." },
	{ ERROR_SPURIOUS_COMMENT_CLOSE,			true, "Multi-line comment closed without open." },
    { ERROR_TYPE_MISMATCH,                          true,   "Type mismatch" },
    {-1, true, "Doh! Internal end-of-table marker, report the bug!"}
};

#define MAX_ERROR (( sizeof( sErrorDef ) / sizeof( ERROR_DEFINITION )))

bool *bStopAtEnd;
bool bRemoveOutBin  = false;

int nMaxPasses = 10;

char     *Extstr;
/*unsigned char     Listing = 1;*/

int     pass;

unsigned char     F_ListAllPasses = 0;

static int CountUnresolvedSymbols(void)
{
    SYMBOL *sym;
    int nUnresolved = 0;
    int i;

    /* Pre-count unresolved symbols */
    for (i = 0; i < SHASHSIZE; ++i)
        for (sym = SHash[i]; sym; sym = sym->next)
            if ( sym->flags & SYM_UNKNOWN )
                nUnresolved++;

    return nUnresolved;
}


static int ShowUnresolvedSymbols(void)
{
    SYMBOL *sym;
    int i;

    int nUnresolved = CountUnresolvedSymbols();
    if ( nUnresolved )
    {
        printf( "--- Unresolved Symbol List\n" );

        /* Display unresolved symbols */
        for (i = 0; i < SHASHSIZE; ++i)
            for (sym = SHash[i]; sym; sym = sym->next)
                if ( sym->flags & SYM_UNKNOWN )
                    printf( "%-24s %s\n", sym->name, sftos( sym->value, sym->flags ) );

        printf( "--- %d Unresolved Symbol%c\n\n", nUnresolved, ( nUnresolved == 1 ) ? ' ' : 's' );
    }

    return nUnresolved;
}


static int CompareAlpha( const void *arg1, const void *arg2 )
{
    /* Simple alphabetic ordering comparison function for quicksort */

    const SYMBOL *sym1 = *(SYMBOL * const *) arg1;
    const SYMBOL *sym2 = *(SYMBOL * const *) arg2;

    /*
       The cast above is wild, thank goodness the Linux man page
       for qsort(3) has an example explaining it... :-) [phf]

       TODO: Note that we compare labels case-insensitive here which
       is not quite right; I believe we should be case-sensitive as
       in other contexts where symbols (labels) are compared. But
       the old CompareAlpha() was case-insensitive as well, so I
       didn't want to change that right now... [phf]
    */

    return strcasecmp(sym1->name, sym2->name);
}

static int CompareAddress( const void *arg1, const void *arg2 )
{
    /* Simple numeric ordering comparison function for quicksort */

    const SYMBOL *sym1 = *(SYMBOL * const *) arg1;
    const SYMBOL *sym2 = *(SYMBOL * const *) arg2;

    return sym1->value - sym2->value;
}

static int CompareOrder( const void *arg1, const void *arg2 )
{
    /* Simple numeric ordering comparison function for quicksort */

    const SYMBOL *sym1 = *(SYMBOL * const *) arg1;
    const SYMBOL *sym2 = *(SYMBOL * const *) arg2;

    return sym1->order - sym2->order;
}

static void ShowSymbols( FILE *file, int sortMode )
{
    /* Display sorted (!) symbol table - if it runs out of memory, table will be displayed unsorted */

    SYMBOL **symArray;
    SYMBOL *sym;
    int i;
    int nSymbols = 0;

    fprintf( file, "--- Symbol List");

    /* Sort the symbol list either via name, by value or by order in code */

    /* First count the number of symbols */
    for (i = 0; i < SHASHSIZE; ++i)
        for (sym = SHash[i]; sym; sym = sym->next)
            nSymbols++;

    /* Malloc an array of pointers to data */

    symArray = (SYMBOL **)ckmalloc( sizeof( SYMBOL * ) * nSymbols );
    if ( !symArray )
    {
        fprintf( file, " (unsorted - not enough memory to sort!)\n" );

        /* Display complete symbol table */
        for (i = 0; i < SHASHSIZE; ++i)
            for (sym = SHash[i]; sym; sym = sym->next)
                fprintf( file, "%-24s %s\n", sym->name, sftos( sym->value, sym->flags ) );
    }
    else
    {
         /* Copy the element pointers into the symbol array */

         int nPtr = 0;

         for (i = 0; i < SHASHSIZE; ++i)
             for (sym = SHash[i]; sym; sym = sym->next)
                 symArray[ nPtr++ ] = sym;

         if ( sortMode == SORTMODE_ADDRESS )
         {
             fprintf( file, " (sorted by address)\n" );
             qsort( symArray, nPtr, sizeof( SYMBOL * ), CompareAddress );           /* Sort via address */
         }
         else if ( sortMode == SORTMODE_ALPHA )
         {
             fprintf( file, " (sorted by symbol)\n" );
             qsort( symArray, nPtr, sizeof( SYMBOL * ), CompareAlpha );              /* Sort via name */
         }
		 else
         {
             fprintf( file, " (sorted by order)\n" );
             qsort( symArray, nPtr, sizeof( SYMBOL * ), CompareOrder );              /* Sort via name */
         }

         /* now display sorted list */

         for ( i = 0; i < nPtr; i++ )
         {
             fprintf( file, "%-24s %-12s", symArray[ i ]->name,
                 sftos( symArray[ i ]->value, symArray[ i ]->flags ) );
             if ( symArray[ i ]->flags & SYM_STRING )
                 fprintf( file, " \"%s\"", symArray[ i ]->string );                  /* If a string, display actual string */
			 //fprintf( file, " %i", symArray[ i ]->order ); // enable for debugging -T2
             fprintf( file, "\n" );
         }

         free( symArray );
    }

    fputs( "--- End of Symbol List.\n", file );

}



static void ShowSegments(void)
{
    SEGMENT *seg;
    const char *bss;
    const char *sFormat = "%-24s %-3s %-8s %-8s %-8s %-8s\n\0";



    printf("\n----------------------------------------------------------------------\n");
    printf( sFormat, "SEGMENT NAME", "", "INIT PC", "INIT RPC", "FINAL PC", "FINAL RPC" );

    for (seg = Seglist; seg; seg = seg->next)
    {
        bss = (seg->flags & SF_BSS) ? "[u]" : "   ";

        printf( sFormat, seg->name, bss,
            sftos(seg->initorg, seg->initflags), sftos(seg->initrorg, seg->initrflags),
            sftos(seg->org, seg->flags), sftos(seg->rorg, seg->rflags) );
    }
    puts("----------------------------------------------------------------------");

    printf( "%d references to unknown symbols.\n", Redo_eval );
    printf( "%d events requiring another assembler pass.\n", Redo );

    if ( Redo_why )
    {
        if ( Redo_why & REASON_MNEMONIC_NOT_RESOLVED )
            printf( " - Expression in mnemonic not resolved.\n" );

        if ( Redo_why & REASON_OBSCURE )
            printf( " - Obscure reason - to be documented :)\n" );

        if ( Redo_why & REASON_DC_NOT_RESOVED )
            printf( " - Expression in a DC not resolved.\n" );

        if ( Redo_why & REASON_DV_NOT_RESOLVED_PROBABLY )
            printf( " - Expression in a DV not resolved (probably in DV's EQM symbol).\n" );

        if ( Redo_why & REASON_DV_NOT_RESOLVED_COULD )
            printf( " - Expression in a DV not resolved (could be in DV's EQM symbol).\n" );

        if ( Redo_why & REASON_DS_NOT_RESOLVED )
            printf( " - Expression in a DS not resolved.\n" );

        if ( Redo_why & REASON_ALIGN_NOT_RESOLVED )
            printf( " - Expression in an ALIGN not resolved.\n" );

        if ( Redo_why & REASON_ALIGN_RELOCATABLE_ORIGIN_NOT_KNOWN )
            printf( " - ALIGN: Relocatable origin not known (if in RORG at the time).\n" );

        if ( Redo_why & REASON_ALIGN_NORMAL_ORIGIN_NOT_KNOWN )
            printf( " - ALIGN: Normal origin not known	(if in ORG at the time).\n" );

        if ( Redo_why & REASON_EQU_NOT_RESOLVED )
            printf( " - EQU: Expression not resolved.\n" );

        if ( Redo_why & REASON_EQU_VALUE_MISMATCH )
            printf( " - EQU: Value mismatch from previous pass (phase error).\n" );

        if ( Redo_why & REASON_IF_NOT_RESOLVED )
            printf( " - IF: Expression not resolved.\n" );

        if ( Redo_why & REASON_REPEAT_NOT_RESOLVED )
            printf( " - REPEAT: Expression not resolved.\n" );

        if ( Redo_why & REASON_FORWARD_REFERENCE )
            printf( " - Label defined after it has been referenced (forward reference).\n" );

        if ( Redo_why & REASON_PHASE_ERROR )
            printf( " - Label value is different from that of the previous pass (phase error).\n" );

        if ( Redo_why & REASON_BRANCH_OUT_OF_RANGE )
            printf( " - Branch was out of range.\n" );

        if ( Redo_why & REASON_INCLUDE_FAILED )
            printf( " - INCLUDE: File not found or expression not resolved.\n" );

        if ( Redo_why & REASON_INCBIN_NOT_RESOLVED )
            printf( " - INCBIN: Expression not resolved.\n" );
    }

    printf( "\n" );

}



static void DumpSymbolTable( int sortMode )
{
    if (F_symfile)
    {
        FILE *fi = fopen(F_symfile, "w");
        if (fi)
        {
            ShowSymbols( fi, sortMode );
            fclose(fi);
        }
        else
        {
            printf("Warning: Unable to open Symbol Dump file '%s'\n", F_symfile);
        }
    }

}


static int MainShadow(int ac, char **av, int *pSortMode )
{
    int nError = ERROR_NONE;
    bool bDoAllPasses = false;

    char buf[MAXLINE];
    int i;
    int argVal;
    MNEMONIC *mne;

    int oldredo = -1;
    unsigned long oldwhy = 0;
    int oldeval = 0;

    addhashtable(Ops);
    pass = 1;

    if (ac < 2)
    {

fail:
    puts(dasm_id);
    puts("Copyright (c) 1988-2020 by the DASM team.");
    puts("License GPLv2+: GNU GPL version 2 or later (see file LICENSE).");
    puts("DASM is free software: you are free to change and redistribute it.");
    puts("There is ABSOLUTELY NO WARRANTY, to the extent permitted by law.");
    puts("");
    puts("Usage: dasm sourcefile [options]");
    puts("");
    puts("-f#      output format 1-3 (default 1)");
    puts("-oname   output file name (else a.out)");
    puts("-lname   list file name (else none generated)");
    puts("-Lname   list file, containing all passes");
    puts("-sname   symbol dump file name (else none generated)");
    puts("-v#      verboseness 0-4 (default 0)");
    puts("-d       debug mode (for developers)");
    puts("-Dsymbol              define symbol, set to 0");
    puts("-Dsymbol=expression   define symbol, set to expression");
    puts("-Msymbol=expression   define symbol using EQM (same as -D)");
    puts("-Idir    search directory for INCLUDE and INCBIN");
    puts("-p#      maximum number of passes");
    puts("-P#      maximum number of passes, with fewer checks");
    puts("-T#      symbol table sorting (default 0 = alphabetical, 1 = address/value, 2 = order in code)");
    puts("-E#      error format (default 0 = MS, 1 = Dillon, 2 = GNU)");
    puts("-S       strict syntax checking");
    puts("-R       remove binary output-file in case of errors");
    puts("-m#      safety barrier to abort on recursions, max. allowed file-size in kB");
    puts("");
    puts("Report bugs on https://github.com/dasm-assembler/dasm please!");

    return ERROR_COMMAND_LINE;
    }

    for (i = 2; i < ac; ++i)
    {
        if ( ( av[i][0] == '-' ) || ( av[i][0] == '/' ) )
        {
            char *str = av[i]+2;
            switch(av[i][1])
            {
            /* TODO: need to improve option parsing and errors for it */
            case 'E':
                F_errorformat = atoi(str);
                if (F_errorformat < ERRORFORMAT_DEFAULT
                   || F_errorformat >= ERRORFORMAT_MAX )
                {
                    panic("Invalid error format for -E, must be 0, 1, 2");
                }
                break;

            case 'T':
                F_sortmode = atoi(str);
                if (F_sortmode < SORTMODE_DEFAULT
                   || F_sortmode >= SORTMODE_MAX )
                {
                    panic("Invalid sorting mode for -T option, must be 0, 1, 2");
                }
                /* TODO: refactor into regular configuration [phf] */
                *pSortMode = F_sortmode;
                break;

            case 'd':
                Xdebug = atoi(str) != 0;
                printf( "Debug trace %s\n", Xdebug ? "ON" : "OFF" );
                break;

            case 'M':
            case 'D':
                while (*str && *str != '=')
                    ++str;
                if (*str == '=')
                {
                    *str = 0;
                    ++str;
                }
                else
                {
                    str = "0";
                }
                Av[0] = av[i]+2;

                if (av[i][1] == 'M')
                    v_eqm(str, NULL);
                else
                    v_set(str, NULL);
                break;

            case 'f':   /*  F_format    */
                F_format = atoi(str);
                if (F_format < FORMAT_DEFAULT || F_format >= FORMAT_MAX )
                    panic("Illegal format specification");
                break;

            case 'o':   /*  F_outfile   */
                F_outfile = str;
nofile:
                if (*str == 0)
                    panic("-o Switch requires file name.");
                break;

            case 'L':
                F_ListAllPasses = 1;
                /* fall through to 'l' */

            case 'l':   /*  F_listfile  */
                F_listfile = str;
                goto nofile;

            case 'P':   /*  F_Passes   */
                bDoAllPasses = true;

                /* fall through to 'p' */
            case 'p':   /*  F_passes   */
                nMaxPasses = atoi(str);
                break;

            case 's':   /*  F_symfile   */
                F_symfile = str;
                goto nofile;
            case 'v':   /*  F_verbose   */
                F_verbose = atoi(str);
                break;

            case 'I':
                v_incdir(str, NULL);
                break;

            case 'S':
                bStrictMode = true;
                break;

            case 'R':
            	bRemoveOutBin = true;
                break;

            case 'm':   /*  F_passes   */
            	argVal = atol(str);
            	if (argVal <= 64) {
            		panic("-m Switch invalid argument, should be > 64");
            	} else {
            		maxFileSize = argVal;
            	}
                break;

            default:
                goto fail;
            }
            continue;
        }
        goto fail;
    }

    bStopAtEnd = malloc((nMaxPasses+1) * sizeof(bool));
    memset(bStopAtEnd, 0, nMaxPasses+1);	// we dont count from zero ! (for fewer code changes)

    /*    INITIAL SEGMENT */

    {
        SEGMENT *seg = (SEGMENT *)permalloc(sizeof(SEGMENT));
        seg->name = strcpy(permalloc(sizeof(ISEGNAME)), ISEGNAME);
        seg->flags= seg->rflags = seg->initflags = seg->initrflags = SF_UNKNOWN;
        Csegment = Seglist = seg;
    }
    /*    TOP LEVEL IF    */
    {
        IFSTACK *ifs = (IFSTACK *)zmalloc(sizeof(IFSTACK));
        ifs->file = NULL;
        ifs->flags = IFF_BASE;
        ifs->acctrue = 1;
        ifs->xtrue  = 1;
        Ifstack = ifs;
    }

    // ready error and message buffer...
    passbuffer_clear(ERRORBUF);
    passbuffer_clear(MSGBUF);


nextpass:


    if ( F_verbose )
    {
        puts("");
        printf("START OF PASS: %d\n", pass);
    }

    Localindex = Lastlocalindex = 0;

    Localdollarindex = Lastlocaldollarindex = 0;
	SymbolCount = 0;

    /*_fmode = 0x8000;*/
    FI_temp = fopen(F_outfile, "wb");
    /*_fmode = 0;*/
    Fisclear = 1;
    CheckSum = 0;
    if (FI_temp == NULL) {
        printf("Warning: Unable to [re]open '%s'\n", F_outfile);
        return ERROR_FILE_ERROR;
    }
    if (F_listfile) {

        FI_listfile = fopen(F_listfile,
            F_ListAllPasses && (pass > 1)? "ab" : "wb");

        if (FI_listfile == NULL) {
            printf("Warning: Unable to [re]open '%s'\n", F_listfile);
            return ERROR_FILE_ERROR;
        }
    }
    pushinclude(av[1]);

    while ( pIncfile )
    {
        for (;;) {
            const char *comment;
            if ( pIncfile->flags & INF_MACRO) {
                if ( pIncfile->strlist == NULL) {
                    Av[0] = "";
                    v_mexit(NULL, NULL);
                    continue;
                }
                strcpy(buf, pIncfile->strlist->buf);
                pIncfile->strlist = pIncfile->strlist->next;
            }
            else
            {
                if (fgets(buf, MAXLINE, pIncfile->fi) == NULL)
                    break;
            }

            if (Xdebug)
                printf("%08lx %s\n", (unsigned long) pIncfile, buf);

            comment = cleanup(buf, false);
            ++pIncfile->lineno;
            mne = parse(buf);

            if (Av[1][0])
            {
                if (mne)
                {
                    if ((mne->flags & MF_IF) || (Ifstack->xtrue && Ifstack->acctrue))
                        (*mne->vect)(Av[2], mne);
                }
                else
                {
                    if (Ifstack->xtrue && Ifstack->acctrue)
                        asmerr( ERROR_UNKNOWN_MNEMONIC, false, Av[1] );
                }

            }
            else
            {
                if (Ifstack->xtrue && Ifstack->acctrue)
                    programlabel();
            }

            if (F_listfile && ListMode)
                outlistfile(comment);
        }

        while (Reploop && Reploop->file == pIncfile)
            rmnode((void **)&Reploop, sizeof(REPLOOP));

        while (Ifstack->file == pIncfile)
            rmnode((void **)&Ifstack, sizeof(IFSTACK));

        fclose( pIncfile->fi );
        free( pIncfile->name );
        --Inclevel;
        rmnode((void **)&pIncfile, sizeof(INCFILE));

        if ( pIncfile )
        {
        	if (F_verbose > 3)
        		printf("back to: %s\n", pIncfile->name);

            if (F_listfile)
                fprintf(FI_listfile, "------- FILE %s\n", pIncfile->name);
        }
    }



    if ( F_verbose >= 1 )
        ShowSegments();

    if ( F_verbose >= 3 )
    {
        if ( !Redo || ( F_verbose == 4 ) )
            ShowSymbols( stdout, *pSortMode );

        ShowUnresolvedSymbols();
    }

    closegenerate();
    fclose(FI_temp);
    if (FI_listfile)
        fclose(FI_listfile);

    if (mlflag) // check if a multi-line comment is missing a terminator
        return ERROR_MISSING_COMMENT_END;

    if (Redo)
    {
        if ( !bDoAllPasses )
            if (Redo == oldredo && Redo_why == oldwhy && Redo_eval == oldeval)
            {
                ShowUnresolvedSymbols();
                return ERROR_NOT_RESOLVABLE;
            }

        oldredo = Redo;
        oldwhy = Redo_why;
        oldeval = Redo_eval;
        Redo = 0;
        Redo_why = 0;
        Redo_eval = 0;

        Redo_if <<= 1;
        ++pass;


        if ( pass > nMaxPasses )
        {
            char sBuffer[64];
            sprintf( sBuffer, "%d", pass );
            return asmerr( ERROR_TOO_MANY_PASSES, false, sBuffer );

        }
        else
        {
            passbuffer_clear(ERRORBUF);
            passbuffer_clear(MSGBUF);

            clearrefs();
            clearsegs();
            goto nextpass;
        }
    }
    // Do not print any errors if assembly is successful!!!!! -FXQ
    // only print messages from last pass and if there's no errors
    if (!bStopAtEnd[pass])
    {
        passbuffer_output(MSGBUF);
    }
    else
    {
        // Only print errors if assembly is unsuccessful!!!!!
        // by FXQ
	passbuffer_output(ERRORBUF);
        printf("Unrecoverable error(s) in pass, aborting assembly!\n");
	nError = ERROR_NON_ABORT;
    }

    if (nMacroClosings != nMacroDeclarations) {
        /* determine the file pointer to use */
        FILE *error_file = (F_listfile != NULL) ? FI_listfile : stdout;

    	fprintf(error_file, "premature end of file, macros opened:%d  closed:%d", nMacroDeclarations, nMacroClosings);
        fprintf(error_file, "Aborting assembly\n");

        exit(ERROR_MISSING_ENDM);
    }
    printf( "Complete. (%d)\n", nError);
    return nError;
}

void addmsg(char *message) // add to message buffer (FXQ)
{
  passbuffer_update(MSGBUF,message);
}


static int tabit(char *buf1, char *buf2)
{
    char *bp, *ptr;
    int j, k;

    bp = buf2;
    ptr= buf1;
    for (j = 0; *ptr && *ptr != '\n'; ++ptr, ++bp, j = (j+1)&7) {
        *bp = *ptr;
        if (*ptr == '\t') {
            /* optimize out spaces before the tab */
            while (j > 0 && bp[-1] == ' ') {
                bp--;
                j--;
            }
            j = 0;
            *bp = '\t';         /* recopy the tab */
        }
        if (j == 7 && *bp == ' ' && bp[-1] == ' ') {
            k = j;
            while (k-- >= 0 && *bp == ' ')
                --bp;
            *++bp = '\t';
        }
    }
    while (bp != buf2 && (bp[-1] == ' ' || bp[-1] == '\t'))
        --bp;
    *bp++ = '\n';
    *bp = '\0';
    return (int)(bp - buf2);
}

static void outlistfile(const char *comment)
{
    char xtrue;
    char c;
    static char buf1[MAXLINE+32];
    static char buf2[MAXLINE+32];
    const char *ptr;
    const char *dot;
    int i, j;


    if ( pIncfile->flags & INF_NOLIST )
        return;

    xtrue = (Ifstack->xtrue && Ifstack->acctrue) ? ' ' : '-';
    c = (Pflags & SF_BSS) ? 'U' : ' ';
    ptr = Extstr;
    dot = "";
    if (ptr)
        dot = ".";
    else
        ptr = "";

    sprintf(buf1, "%7ld %c%s", pIncfile->lineno, c, sftos(Plab, Pflags & 7));
    j = strlen(buf1);
    for (i = 0; i < Glen && i < 4; ++i, j += 3)
        sprintf(buf1+j, "%02x ", Gen[i]);
    if (i < Glen && i == 4)
        xtrue = '*';
    for (; i < 4; ++i) {
        buf1[j] = buf1[j+1] = buf1[j+2] = ' ';
        j += 3;
    }
    sprintf(buf1+j-1, "%c%-10s %s%s%s\t%s\n",
        xtrue, Av[0], Av[1], dot, ptr, Av[2]);
    if (comment[0]) { /*  tab and comment */
        j = strlen(buf1) - 1;
        sprintf(buf1+j, "\t;%s", comment);
    }
    fwrite(buf2, tabit(buf1,buf2), 1, FI_listfile);
    Glen = 0;
    Extstr = NULL;
}

char *sftos(long val, int flags)
{
    static char buf[ MAX_SYM_LEN + 14 ];
    static char c;
    char *ptr = (c) ? buf : buf + sizeof(buf) / 2;

    memset( buf, 0, sizeof( buf ) );

    c = 1 - c;

    sprintf(ptr, "%04lx ", val);

    if (flags & SYM_UNKNOWN)
        strcat( ptr, "???? ");
    else
        strcat( ptr, "     " );

    if (flags & SYM_STRING)
        strcat( ptr, "str ");
    else
        strcat( ptr, "    " );

    if (flags & SYM_MACRO)
        strcat( ptr, "eqm ");
    else
        strcat( ptr, "    " );


    if (flags & (SYM_MASREF|SYM_SET))
    {
        strcat( ptr, "(" );
    }
    else
        strcat( ptr, " " );

    if (flags & (SYM_MASREF))
        strcat( ptr, "R" );
    else
        strcat( ptr, " " );


    if (flags & (SYM_SET))
        strcat( ptr, "S" );
    else
        strcat( ptr, " " );

    if (flags & (SYM_MASREF|SYM_SET))
    {
        strcat( ptr, ")" );
    }
    else
        strcat( ptr, " " );


    return ptr;
}

void clearsegs(void)
{
    SEGMENT *seg;

    for (seg = Seglist; seg; seg = seg->next) {
        seg->flags = (seg->flags & SF_BSS) | SF_UNKNOWN;
        seg->rflags= seg->initflags = seg->initrflags = SF_UNKNOWN;
    }
}


void clearrefs(void)
{
    SYMBOL *sym;
    short i;

    for (i = 0; i < SHASHSIZE; ++i)
        for (sym = SHash[i]; sym; sym = sym->next)
            sym->flags &= ~SYM_REF;
}




static const char *cleanup(char *buf, bool bDisable)
{
    char *str;
    STRLIST *strlist;
    int arg, add;
    const char *comment = "";

    char *mlstart, *mlend, *semistart;
    mlstart=strstr(buf,"/*");
    mlend=strstr(buf,"*/");
    semistart=strstr(buf,";");

    if (mlflag) // a previous multi-line comment is in progress...
    {
        if ( mlend )
        {
            mlflag=0; // turn off multiline comments
	    char tempbuf[MAXLINE];
            char *tmpc;
            *mlend = 0;
            tmpc = mlend+1;
            while(*tmpc!=0) // we need to purge any newlines before we reorder the parts of the line
            {
                if((*tmpc == '\r')||(*tmpc == '\n'))
                    *tmpc=0;
                tmpc++;
            }
	    snprintf(tempbuf,MAXLINE,"%s;*/%s",mlend+2,buf); // put the comment at the end of the line
            strncpy(buf,tempbuf,MAXLINE);
            return(cleanup(buf,bDisable)); // repeat for any single-line comments that may follow
        }
        else
        {
            memmove(buf+1,buf,strlen(buf)+1); // make room for the last comment
            buf[0]=';';
        }
    }
    else // we're not presently in the middle of a multi-line comment...
    {
        // check for spurious comment close
        if (mlend && ( (!semistart) || (mlend < semistart) ) && ( (!mlstart) ||  (mlend < mlstart) ) )
            asmerr( ERROR_SPURIOUS_COMMENT_CLOSE, false, NULL );
        if (mlstart && ((!semistart) || (mlstart < semistart)))
        {
            if (mlend && (mlstart < mlend) && ((!semistart)||(mlend < semistart))) // single line /* */
            {
	        char tempbuf[MAXLINE];
                char *tmpc;
                *mlstart = 0;
                *(mlend+1)=0;
                tmpc = mlend+2;
                while(*tmpc!=0) // we need to purge any newlines before we reorder the parts of the line
                {
                    if((*tmpc == '\r')||(*tmpc == '\n'))
                        *tmpc=0;
                    tmpc++;
                }
	        snprintf(tempbuf,MAXLINE,"%s%s;/%s",buf,mlend+2,mlstart+1); // move the first comment to the end of the line
                strcpy(buf,tempbuf);
                return(cleanup(buf,bDisable)); // repeat for any single-line comments that may follow
            }
            mlflag=1; // turn on multiline comments
            memmove(mlstart+1,mlstart,strlen(mlstart)+1); // make room for a comment
            *mlstart=';';
        }
    }

    for (str = buf; *str; ++str)
    {
        switch(*str)
        {
        case ';':
            comment = (char *)str + 1;
            /*    FALL THROUGH    */
        case '\r':
        case '\n':
            goto br2;
        case TAB:
            *str = ' ';
            break;
        case '\'':
            ++str;
            if (*str == TAB)
                *str = ' ';
            if (*str == '\n' || *str == 0)
            {
                str[0] = ' ';
                str[1] = 0;
            }
            if (str[0] == ' ')
                str[0] = '\x80';
            break;
        case '\"':
            ++str;
            while (*str && *str != '\"')
            {
                if (*str == ' ')
                    *str = '\x80';
                ++str;
            }
            if (*str != '\"')
            {
                asmerr( ERROR_SYNTAX_ERROR, false, buf );
                --str;
            }
            break;
        case '{':
            if ( bDisable )
                break;

            if (Xdebug)
                printf("macro tail: '%s'\n", str);

            arg = atoi(str+1);
            for (add = 0; *str && *str != '}'; ++str)
                --add;
            if (*str != '}')
            {
                puts("end brace required");
                --str;
                break;
            }
            --add;
            ++str;


            if (Xdebug)
                printf("add/str: %d '%s'\n", add, str);

            for (strlist = pIncfile->args; arg && strlist;)
            {
                --arg;
                strlist = strlist->next;
            }

            if (strlist)
            {
                add += strlen(strlist->buf);

                if (Xdebug)
                    printf("strlist: '%s' %zu\n", strlist->buf, strlen(strlist->buf));

                if (str + add + strlen(str) + 1 > buf + MAXLINE)
                {
                    if (Xdebug)
                        printf("str %8ld buf %8ld (add/strlen(str)): %d %ld\n",
                        (unsigned long)str, (unsigned long)buf, add, (long)strlen(str));
                    panic("failure1");
                }

                memmove(str + add, str, strlen(str)+1);
                str += add;
                if (str - strlen(strlist->buf) < buf)
                    panic("failure2");
                memmove(str - strlen(strlist->buf), strlist->buf, strlen(strlist->buf));
                str -= strlen(strlist->buf);
                if (str < buf || str >= buf + MAXLINE)
                    panic("failure 3");
                --str;      /*  for loop increments string    */
            }
            else
            {
                asmerr( ERROR_NOT_ENOUGH_ARGUMENTS_PASSED_TO_MACRO, false, NULL );
                goto br2;
            }
            break;
        }
    }

br2:
    while(str != buf && *(str-1) == ' ')
        --str;
    *str = 0;

    return comment;
}

void panic(const char *str)
{
    puts(str);
    exit(1);
}

/*
*  .dir    direct              x
*  .ext    extended              x
*  .r          relative              x
*  .x          index, no offset          x
*  .x8     index, byte offset          x
*  .x16    index, word offset          x
*  .bit    bit set/clr
*  .bbr    bit and branch
*  .imp    implied (inherent)          x
*  .b                      x
*  .w                      x
*  .l                      x
*  .u                      x
*  .s       swapped short, force other endian with DC
*/


void findext(char *str)
{
    Mnext = -1;
    Extstr = NULL;

    if (str[0] == '.') {    /* Allow .OP for OP */
        return;
    }

    while (*str && *str != '.')
        ++str;
    if (*str) {
        *str = 0;
        ++str;
        Extstr = str;
        switch(str[0]|0x20) {
	     case 's':
                Mnext = AM_OTHER_ENDIAN;
                return;

        case '0':
        case 'i':
            Mnext = AM_IMP;
            switch(str[1]|0x20) {
            case 'x':
                Mnext = AM_0X;
                break;
            case 'y':
                Mnext = AM_0Y;
                break;
            case 'n':
                Mnext = AM_INDWORD;
                break;
            }
            return;
            case 'd':
            case 'b':
            case 'z':
                switch(str[1]|0x20) {
                case 'x':
                    Mnext = AM_BYTEADRX;
                    break;
                case 'y':
                    Mnext = AM_BYTEADRY;
                    break;
                case 'i':
                    Mnext = AM_BITMOD;
                    break;
                case 'b':
                    Mnext = AM_BITBRAMOD;
                    break;
                default:
                    Mnext = AM_BYTEADR;
                    break;
                }
                return;
                case 'e':
                case 'w':
                case 'a':
                    switch(str[1]|0x20) {
                    case 'x':
                        Mnext = AM_WORDADRX;
                        break;
                    case 'y':
                        Mnext = AM_WORDADRY;
                        break;
                    default:
                        Mnext = AM_WORDADR;
                        break;
                    }
                    return;
                    case 'l':
                        Mnext = AM_LONG;
                        return;
                    case 'r':
                        Mnext = AM_REL;
                        return;
                    case 'u':
                        Mnext = AM_BSS;
                        return;
        }
    }
}

/*
*  bytes arg will eventually be used to implement a linked list of free
*  nodes.
*  Assumes *base is really a pointer to a structure with .next as the first
*  member.
*/

void rmnode(void **base, int bytes)
{
    void *node;

    if ((node = *base) != NULL) {
        *base = *(void **)node;
        free(node);
    }
}

/*
*  Parse into three arguments: Av[0], Av[1], Av[2]
*/
MNEMONIC *parse(char *buf)
{
    int i, j;
    MNEMONIC *mne = NULL;
    int labelundefined = 0;

    i = 0;
    j = 1;

    /*
        If the first non-space is a ^, skip all further spaces too.
        This means what follows is a label.
        If the first non-space is a #, what follows is a directive/opcode.
    */
    while (buf[i] == ' ')
        ++i;
    if (buf[i] == '^') {
        ++i;
        while (buf[i] == ' ')
            ++i;
    } else if (buf[i] == '#') {
        buf[i] = ' ';   /* label separator */
    } else
        i = 0;

    Av[0] = Avbuf + j;
    while (buf[i] && buf[i] != ' ' && buf[i] != '=') {

        if (buf[i] == ':') {
            i++;
            break;
        }

        if (buf[i] == ',')  // we have label arguments
        {
            if(buf[i+1]=='"') // is it a string constant?
            {
                i=i+2; // advance to string contents
                while (buf[i] && buf[i] != '"' && buf[i] != ' ' && buf[i] != ',' && buf[i] != ':')
                {
                    Avbuf[j++] = buf[i++];
                }
                if (buf[i] && buf[i]=='"')
                {
                    i++;
                    continue;
                }
                else
                {
                    labelundefined++;
                    asmerr( ERROR_SYNTAX_ERROR, false, buf );
                    continue;
                }
            }
            else // or else it's a symbol to be evaluated, and added to the label
            {
                int t;
                char tempbuf[257];
                char tempval[257];
                SYMBOL *symarg;
                strncpy(tempbuf,buf+i+1,256);
                tempbuf[256]=0;
                for(t=0;t<strlen(tempbuf);t++)
                {
                    if(tempbuf[t] == ',')
                        tempbuf[t]=0;
                }
                symarg = eval(tempbuf,0);
                if(symarg)
                {
                    if (symarg->flags & SYM_UNKNOWN) // one of the arguments isn't defined yet
			labelundefined++; // ensure the label we're creating doesn't get used
                    else
                    {
                        snprintf(tempval,256,"%d",(unsigned)symarg->value);
                        strcpy(Avbuf+j,tempval);
			j=j+strlen(tempval);
                    }
                }
                i++;
                while (buf[i] && buf[i] != ' ' && buf[i] != '=' && buf[i] != ','&& buf[i] != ':')
                    i++;
            }
            continue;
        }

        if ((unsigned char)buf[i] == 0x80)
            buf[i] = ' ';
        Avbuf[j++] = buf[i++];
    }
    Avbuf[j++] = 0;

    // if the label has arguments that aren't defined, we need to scuttle it
    // to avoid a partial label being used.
    if(labelundefined)
    {
      j=1;
      Avbuf[j++] = 0;
    }

    /* Parse the second word of the line */
    while (buf[i] == ' ')
        ++i;
    Av[1] = Avbuf + j;
    if (buf[i] == '=') {
        /* '=' directly seperates Av[0] and Av[2] */
        Avbuf[j++] = buf[i++];
    } else while (buf[i] && buf[i] != ' ') {
        if ((unsigned char)buf[i] == 0x80)
            buf[i] = ' ';
        Avbuf[j++] = buf[i++];
    }
    Avbuf[j++] = 0;
    /* and analyse it as an opcode */
    findext(Av[1]);
    mne = findmne(Av[1]);

    /* Parse the rest of the line */
    while (buf[i] == ' ')
        ++i;
    Av[2] = Avbuf + j;
    while (buf[i]) {
        if (buf[i] == ' ') {
            while(buf[i+1] == ' ')
                ++i;
        }
        if ((unsigned char)buf[i] == 0x80)
            buf[i] = ' ';
        Avbuf[j++] = buf[i++];
    }
    Avbuf[j] = 0;

    if (mne != NULL) {
    	if (mne->flags & MF_BEGM) {
    		nMacroDeclarations++;
    	}
    	if (mne->flags & MF_ENDM) {
        	nMacroClosings++;
    	}
    }
    return mne;
}



MNEMONIC *findmne(char *str)
{
	int h,k;
    int i;
    char c;
    MNEMONIC *mne;
    char buf[64];


    if (str[0] == '.') {    /* Allow .OP for OP */
        str++;
    }

    for (i = 0; (c = str[i]); ++i) {
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        buf[i] = c;
    }
    buf[i] = 0;
    h = hash1(buf);
    mne = MHash[h];
    k = 0;
    while (mne != NULL) {
        if (strcmp(buf, mne->name) == 0)
            break;

        k++;
        mne = mne->next;
        if (mne != NULL) {
        	if ((mne == mne->next) && (k > 5)) { // any number bigger than 1 would do
        		// should not happen at all, I'm not paranoid, I've had this problem really
        		fprintf(stderr,"[%s] [%s] %08lx %08lx\n", buf, mne->name, (long)mne, (long)mne->next);
        		fprintf(stderr,"BUG: %s:%d: chained list looped to itself and no match, would lock up endlessly\n", __FILE__, __LINE__);
        		return NULL; // we need to return NULL here or the program will get stuck in an endless loop
        					 // the BUG vanished with increased hashtable
        	}
        }
    }
    return mne;
}

void v_macro(char *str, MNEMONIC *dummy)
{
    STRLIST *base;
    int defined = 0;
    STRLIST **slp, *sl;
    MACRO *mac;    /* slp, mac: might be used uninitialised */
    MNEMONIC   *mne;
    unsigned int i;
    char buf[MAXLINE];
    int skipit = !(Ifstack->xtrue && Ifstack->acctrue);

    strlower(str);
    mne = findmne(str);

    if (skipit) {
        defined = 1;
    } else {
        defined = (mne != NULL);
        if (F_listfile && ListMode)
            outlistfile("");
    }
    if (!defined) {
        base = NULL;
        slp = &base;
        mac = (MACRO *)permalloc(sizeof(MACRO));
        i = hash1(str);
        mac->next = (MACRO *)MHash[i];
        mac->vect = v_execmac;
        mac->name = strcpy(permalloc(strlen(str)+1), str);
        mac->flags = MF_MACRO;
        mac->defpass = pass;
        if (mac == mac->next) {
        	// should not happen
        	fprintf(stderr,"BUG: %s:%d: permalloc() returned the same value twice, expect severe problems\n", __FILE__, __LINE__);
        }
        MHash[i] = (MNEMONIC *)mac;
    }
    else {
        mac = (MACRO *)mne;
        if( (bStrictMode) && (mac != NULL) && (mac->defpass == pass) )
            asmerr( ERROR_MACRO_REPEATED, true, str );
    }
    while (fgets(buf, MAXLINE, pIncfile->fi)) {
        const char *comment;

        if (Xdebug)
            printf("%08lx %s\n", (unsigned long) pIncfile, buf);

        ++pIncfile->lineno;


        comment = cleanup(buf, true);

        mne = parse(buf);
        if (Av[1][0]) {
            if (mne != NULL) {	// for some reason I got a segfault while accessing mne->flags, should not happen but gdb said it was here
            	if (mne->flags & MF_ENDM) {
                if (!defined)
                    mac->strlist = base;
                return;
            	}
            }
        }
        if (!skipit && F_listfile && ListMode)
            outlistfile(comment);
        if (!defined) {
            sl = (STRLIST *)permalloc(STRLISTSIZE+1+strlen(buf));
            strcpy(sl->buf, buf);
            *slp = sl;
            slp = &sl->next;
        }
    }
    asmerr( ERROR_PREMATURE_EOF, true, NULL );
}


void addhashtable(MNEMONIC *mne)
{
    int i, j;
    unsigned int opcode[NUMOC];

    for (; mne->vect; ++mne) {
        memcpy(opcode, mne->opcode, sizeof(mne->opcode));
        for (i = j = 0; i < NUMOC; ++i) {
            mne->opcode[i] = 0;     /* not really needed */
            if (mne->okmask & (1L << i))
                mne->opcode[i] = opcode[j++];
        }
        i = hash1(mne->name);
        mne->next = MHash[i];
        MHash[i] = mne;
    }
}


static unsigned int hash1(const char *str)
{
    uint8_t a = 0;
    uint8_t b = 0;
    while (*str) {	// this is Fletcher's checksum, better distribution, faster
    	a += *str++;
    	b += a;
    }
    return ((((a << 8) & 0xFF00) | (b & 0xFF))  ) & MHASHAND;
}

void pushinclude(char *str)
{
    INCFILE *inf;
    FILE *fi;

    if ((fi = pfopen(str, "rb")) != NULL) {
        if (F_verbose > 1 && F_verbose != 5 )
            printf("%.*s Including file \"%s\"\n", Inclevel*4, "", str);
        ++Inclevel;

        if (F_listfile)
            fprintf(FI_listfile, "------- FILE %s LEVEL %d PASS %d\n", str, Inclevel, pass);

        inf = (INCFILE *)zmalloc(sizeof(INCFILE));
        inf->next    = pIncfile;
        inf->name    = strcpy(ckmalloc(strlen(str)+1), str);
        inf->fi = fi;
        inf->lineno = 0;
        pIncfile = inf;
        return;
    }
    asmerr( ERROR_FILE_ERROR, false, NULL );
    ++Redo;
    Redo_why |= REASON_INCLUDE_FAILED;
    return;
}




int asmerr(int err, bool bAbort, const char *sText )
{
    const char *str;
    INCFILE *pincfile;
    /* file pointer we print error messages to */
    FILE *error_file = NULL;

    if ( err >= MAX_ERROR || err < 0 )
    {
        return asmerr( ERROR_BADERROR, true, "Bad error ERROR!" );
    }
    else
    {

        if (sErrorDef[err].bFatal)
            bStopAtEnd[pass] = true;

	pincfile = pIncfile;
	while(1) {
		if (pincfile == NULL) {
			fprintf(stderr, "%s:%d: error: pincfile is NULL, err:%d, [%s]: %s\n", __FILE__, __LINE__
						, err, sText, sErrorDef[err].sDescription);
			bAbort = true;
			break;
		}
		if (pincfile->flags & INF_MACRO) {
			pincfile = pincfile->next;
			continue;
		} else {
			break;
		}
	}
        str = sErrorDef[err].sDescription;

        /*
            New error format selection for 2.20.11 since some
            people *don't* use MS products. For historical
            reasons we currently send errors to stdout when
            they should really go to stderr, but we'll switch
            eventually I hope... [phf]
        */

        /* determine the file pointer to use */
        error_file = (F_listfile != NULL) ? FI_listfile : stdout;

        /* print first part of message, different formats offered */
	if (pincfile != NULL)
        switch (F_errorformat)
        {
            case ERRORFORMAT_WOE:
                /*
                    Error format for MS VisualStudio and relatives:
                    "file (line): error: string"
                */
                if(error_file!=stdout)
                    fprintf(error_file, "%s (%lu): error: ",
                        pincfile->name, pincfile->lineno);
                sprintf(erroradd1, "%s (%lu): error: ",
                    pincfile->name, pincfile->lineno); // -FXQ
                break;
            case ERRORFORMAT_DILLON:
                /*
                    Matthew Dillon's original format, except that
                    we don't distinguish writing to the terminal
                    from writing to the list file for now. Matt's
                    2.16 uses these:

                      "*line %4ld %-10s %s\n" (list file)
                      "line %4ld %-10s %s\n" (terminal)
                */
                if(error_file!=stdout)
                    fprintf(error_file, "line %7ld %-10s ",
                        pincfile->lineno, pincfile->name);
                sprintf(erroradd1, "line %7ld %-10s ",
                    pincfile->lineno, pincfile->name); // -FXQ
                break;
            case ERRORFORMAT_GNU:
                /*
                    GNU format error messages, from their coding
                    standards.
                */
                if(error_file!=stdout)
                    fprintf(error_file, "%s:%lu: error: ",
                        pincfile->name, pincfile->lineno);
                sprintf(erroradd1, "%s:%lu: error: ",
                    pincfile->name, pincfile->lineno); // -FXQ
                break;
            default:
                /* TODO: really panic here? [phf] */
                panic("Invalid error format, internal error!");
                break;
        }

        if(error_file!=stdout)
        {
            /* print second part of message, always the same for now */
            fprintf(error_file, str, sText ? sText : "");
            fprintf(error_file, "\n");
        }
        sprintf(erroradd2, str, sText ? sText : "");
        sprintf(erroradd3, "\n");

	passbuffer_update(ERRORBUF,erroradd1);
	passbuffer_update(ERRORBUF,erroradd2);
	passbuffer_update(ERRORBUF,erroradd3);

        if ( bAbort )
        {
            passbuffer_output(MSGBUF); // dump messages from this pass
            fprintf(error_file, "Aborting assembly\n");
            passbuffer_output(ERRORBUF); // time to dump the errors from this pass!
            exit(err);
        }
    }

    return err;
}

char *zmalloc(int bytes)
{
    char *ptr = ckmalloc(bytes);
    if ( ptr )
        memset(ptr, 0, bytes);
    return ptr;
}

char *ckmalloc(int bytes)
{
    char *ptr = malloc(bytes);
    if (ptr)
    {
        return ptr;
    }
    panic("unable to malloc");
    return NULL;
}

char *permalloc(int bytes)
{
    static char *buf;
    static int left;
    char *ptr;

    /* Assume sizeof(union align) is a power of 2 */

    union align
    {
        long l;
        void *p;
        void (*fp)(void);
    };

    bytes = (bytes + sizeof(union align)-1) & ~(sizeof(union align)-1);
    if (bytes > left)
    {
        if ((buf = malloc(ALLOCSIZE)) == NULL)
            panic("unable to malloc");
        memset(buf, 0, ALLOCSIZE);
        left = ALLOCSIZE;
        if (bytes > left)
            panic("software error");
    }
    ptr = buf;
    buf += bytes;
    left -= bytes;
    return ptr;
}

char *strlower(char *str)
{
    char c;
    char *ptr;

    for (ptr = str; (c = *ptr); ++ptr)
    {
        if (c >= 'A' && c <= 'Z')
            *ptr = c | 0x20;
    }
    return str;
}

int main(int ac, char **av)
{
	int sortMode = SORTMODE_DEFAULT;
    int nError = MainShadow( ac, av, &sortMode);

    if ( nError && (nError != ERROR_NON_ABORT) )
    {
	// dump messages when aborting due to errors
        passbuffer_output(MSGBUF);

        // Only print errors if assembly is unsuccessful
        passbuffer_output(ERRORBUF);

        printf( "Fatal assembly error: %s\n", sErrorDef[nError].sDescription );
    }

    DumpSymbolTable( sortMode );

    passbuffer_cleanup();

    if (nError != 0) {
    	if (bRemoveOutBin) {
    		unlink(F_outfile);
    	}
    }
    return nError;
}

void passbuffer_clear(int mbindex)
{
    // ensure the buffer is initialized before we attempt to clear it,
    // just in case no messages have been stored prior to this clear.
    if(passbuffer[mbindex] == NULL)
        passbuffer_update(mbindex,"");
    // clear the requested guffer
    passbuffer[mbindex][0] = 0;
}

void passbuffer_update(int mbindex,char *message)
{
    int newsizerequired;

    // allocate 16k buffers to start...
    static int passbuffersize[2] = {16384,16384};


    // check if the buffer we're working with needs initialization
    if(passbuffer[mbindex] == NULL)
    {
        passbuffer[mbindex] = malloc(passbuffersize[mbindex]);
        if(passbuffer[mbindex] == NULL)
            panic("couldn't allocate memory for message buffer.");
        passbuffer[mbindex][0] = 0; // empty string
    }

    // check if we need to grow the buffer...
    newsizerequired=strlen(passbuffer[mbindex])+strlen(message);
    if( newsizerequired > passbuffersize[mbindex])
    {
        char *tmpalloc;
        // double the current buffer size, if sufficient, so we don't continually reallocate memory...
        newsizerequired = ( newsizerequired < (passbuffersize[mbindex]*2) ) ? passbuffersize[mbindex]*2 : newsizerequired;

        tmpalloc = realloc(passbuffer[mbindex], newsizerequired);
        if(tmpalloc == NULL)
           strcpy(passbuffer[0],"Insufficient memeory to extend the pass buffer. Some output was lost.\n");
        else
        {
            passbuffer[mbindex] = tmpalloc;
            passbuffersize[mbindex]=newsizerequired;
        }
    }

    // update the buffer with the message...
    strcat(passbuffer[mbindex],message);
}

void passbuffer_output(int mbindex)
{
    // ensure the buffer is initialized before we attempt to clear it,
    // just in case no messages have been stored yet.
    if(passbuffer[mbindex] == NULL)
        passbuffer_update(mbindex,"");
    printf("%s\n",passbuffer[mbindex]); // ...do we really still need to put this through stdout, instead stderr?
}

void passbuffer_cleanup()
{
    int t;
    for(t=0;t<2;t++)
        if(passbuffer[t]!=NULL)
            free(passbuffer[t]);
}
