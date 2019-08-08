/*
    DASM Assembler
    Portions of this code are Copyright (C)1988 Matthew Dillon
    and (C) 1995 Olaf Seibert, (C)2003 Andrew Davie 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*
*  MAIN.C
*  DASM   sourcefile
*  NOTE: must handle mnemonic extensions and expression decode/compare.
*/


#include "asm.h"


#define MAXLINE 1024
#define ISEGNAME    "INITIAL CODE SEGMENT"

char *cleanup(char *buf, bool bDisable);

MNEMONIC *parse(char *buf);
void panic(char *str);
MNEMONIC *findmne(char *str);
void clearsegs(void);
void clearrefs(void);

static unsigned int hash1(char *str);
static void outlistfile(char *);



/* Table encapsulates errors, descriptions, and fatality flags. */

ERROR_DEFINITION sErrorDef[] = {
    
    /* Error #, STOPEND, Description */
    
    { ERROR_NONE,                                   true,   "OK"   },
    { ERROR_COMMAND_LINE,                           true,   "Check command-line format."   },
    { ERROR_FILE_ERROR,                             true,   "Unable to open file."   },
    { ERROR_NOT_RESOLVABLE,                         true,   "Source is not resolvable."   },
    { ERROR_TOO_MANY_PASSES,                        true,   "Too many passes (%s)."   },
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
    { ERROR_BRANCH_OUT_OF_RANGE,                    true,   "Branch out of range (%s bytes)."   },
    { ERROR_ERR_PSEUDO_OP_ENCOUNTERED,              true,   "ERR pseudo-op encountered."  },
    { ERROR_ORIGIN_REVERSE_INDEXED,                 false,  "Origin Reverse-indexed."   },
    { ERROR_EQU_VALUE_MISMATCH,                     false,  "EQU: Value mismatch."   },
    { ERROR_ADDRESS_MUST_BE_LT_100,                 true,   "Value in '%s' must be <$100."  },
    { ERROR_ILLEGAL_BIT_SPECIFICATION,              true,   "Illegal bit specification."   },
    { ERROR_NOT_ENOUGH_ARGS,                        true,   "Not enough arguments."   },
    { ERROR_LABEL_MISMATCH,                         true,   "Label mismatch...\n --> %s"  },
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
    NULL
};

#define MAX_ERROR (( sizeof( sErrorDef ) / sizeof( ERROR_DEFINITION )))

bool bStopAtEnd = false;


char     *Extstr;
/*unsigned char     Listing = 1;*/
int     pass;

unsigned char     F_ListAllPasses = 0;



const char name[] = "DASM V2.20.10, Macro Assembler (C)1988-2004";



int CountUnresolvedSymbols()
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


int ShowUnresolvedSymbols()
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


int CompareAlpha( const void *arg1, const void *arg2 )
{
    /* Simple alphabetic ordering comparison function for quicksort */
    
    SYMBOL **sym1, **sym2;
    int nSym1Size, nSym2Size;
    char *pSym1LC, *pSym2LC;
    char *pSrc, *pDest;
    int nCompare;
    
    sym1 = (SYMBOL **) arg1;
    sym2 = (SYMBOL **) arg2;
    
    nSym1Size = strlen( (*sym1)->name ) + 1;
    nSym2Size = strlen( (*sym2)->name ) + 1;
    
    
    
    /* Primitive manual to lowercase conversion */
    
    pSym1LC = ckmalloc( nSym1Size );
    pDest = pSym1LC;
    pSrc = (*sym1)->name;
    while ( *pSrc )
    {
        if ( *pSrc >= 'A' && *pSrc <= 'Z' )
            *pDest = *pSrc - 'A' + 'a';
        else
            *pDest = *pSrc;
        
        *pDest++;
        *pSrc++;
    }
    *pDest = 0;         /* terminator */
    
    /* Primitive manual to lowercase conversion */
    
    pSym2LC = ckmalloc( nSym2Size );
    pDest = pSym2LC;
    pSrc = (*sym2)->name;
    while ( *pSrc )
    {
        if ( *pSrc >= 'A' && *pSrc <= 'Z' )
            *pDest = *pSrc - 'A' + 'a';
        else
            *pDest = *pSrc;
        
        *pDest++;
        *pSrc++;
    }
    *pDest = 0;         /* terminator */
    
    nCompare = strcmp( pSym1LC, pSym2LC );
    
    free( pSym2LC );
    free( pSym1LC );
    
    return nCompare;
}

int CompareAddress( const void *arg1, const void *arg2 )
{
    /* Simple numeric ordering comparison function for quicksort */
    
    SYMBOL **sym1, **sym2;
    
    sym1 = (SYMBOL **) arg1;
    sym2 = (SYMBOL **) arg2;
    
    return (*sym1)->value - (*sym2)->value;
}


void ShowSymbols( FILE *file, bool bTableSort )
{
    /* Display sorted (!) symbol table - if it runs out of memory, table will be displayed unsorted */
    
    SYMBOL **symArray;
    SYMBOL *sym;
    int i;
    int nSymbols = 0;
    
    fprintf( file, "--- Symbol List");
    
    /* Sort the symbol list either via name, or by value */
    
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
                
                if ( bTableSort )
                {
                    fprintf( file, " (sorted by address)\n" );
                    qsort( symArray, nPtr, sizeof( SYMBOL * ), CompareAddress );           /* Sort via address */
                }
                else
                {
                    fprintf( file, " (sorted by symbol)\n" );
                    qsort( symArray, nPtr, sizeof( SYMBOL * ), CompareAlpha );              /* Sort via name */
                }
                
                
                /* now display sorted list */
                
                for ( i = 0; i < nPtr; i++ )
                {
                    fprintf( file, "%-24s %-12s", symArray[ i ]->name,
                        sftos( symArray[ i ]->value, symArray[ i ]->flags ) );
                    if ( symArray[ i ]->flags & SYM_STRING )
                        fprintf( file, " \"%s\"", symArray[ i ]->string );                  /* If a string, display actual string */
                    fprintf( file, "\n" );
                }
                
                free( symArray );
        }
        
        fputs( "--- End of Symbol List.\n", file );
        
}



void ShowSegments()
{
    SEGMENT *seg;
    char *bss;
    char *sFormat = "%-24s %-3s %-8s %-8s %-8s %-8s\n\0";
    
    
    
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
    }
    
    printf( "\n" );
    
}



void DumpSymbolTable( bool bTableSort )
{
    if (F_symfile)
    {
        FILE *fi = fopen(F_symfile, "w");
        if (fi)
        {
            ShowSymbols( fi, bTableSort );
            fclose(fi);
        }
        else
        {
            printf("Warning: Unable to open Symbol Dump file '%s'\n", F_symfile);
        }
    }
    
}


int MainShadow(int ac, char **av, bool *pbTableSort )
{
    
    
    
    int nError = ERROR_NONE;
    bool bDoAllPasses = false;
    int nMaxPasses = 10;
    
    char buf[MAXLINE];
    int i;
    MNEMONIC *mne;
    
    int oldredo = -1;
    unsigned long oldwhy = 0;
    int oldeval = 0;
    
    addhashtable(Ops);
    pass = 1;
    
    if (ac < 2)
    {
        
fail:
    puts("redistributable for non-profit only");
    puts("");
    puts("DASM sourcefile [options]");
    puts(" -f#      output format");
    puts(" -oname   output file");
    puts(" -lname   list file");
    puts(" -Lname   list file, containing all passes");
    puts(" -sname   symbol dump");
    puts(" -v#      verboseness");
    puts(" -t#      Symbol Table sorting preference (#1 = by address.  default #0 = alphabetic)" );
    puts(" -Dname=exp   define label");
    puts(" -Mname=exp   define label as in EQM");
    puts(" -Idir    search directory for include and incbin");
    puts(" -p#      max number of passes");
    puts(" -P#      max number of passes, with less checks");
    
    return ERROR_COMMAND_LINE;
    }
    
    puts(name);
    
    for (i = 2; i < ac; ++i)
    {
        if ( ( av[i][0] == '-' ) || ( av[i][0] == '/' ) )
        {
            char *str = av[i]+2;
            switch(av[i][1])
            {
                
            case 'T':
                *pbTableSort = ( atoi( str ) != 0 );
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
                
            case 't':   /*  F_temppath  */
                break;
                
            case 'I':
                v_incdir(str, NULL);
                break;
                
            default:
                goto fail;
            }
            continue;
        }
        goto fail;
    }
    
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
    
    
nextpass:
    
    
    if ( F_verbose )
    {
        puts("");
        printf("START OF PASS: %d\n", pass);
    }
    
    Localindex = Lastlocalindex = 0;
    
    Localdollarindex = Lastlocaldollarindex = 0;
    
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
            F_ListAllPasses && (pass > 1)? "a" : "w");

        if (FI_listfile == NULL) {
            printf("Warning: Unable to [re]open '%s'\n", F_listfile);
            return ERROR_FILE_ERROR;
        }
    }
    pushinclude(av[1]);
    
    while ( pIncfile )
    {
        for (;;) {
            char *comment;
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
        /*
        if (F_verbose > 1)
        printf("back to: %s\n", Incfile->name);
            */
            if (F_listfile)
                fprintf(FI_listfile, "------- FILE %s\n", pIncfile->name);
        }
    }
    
    
    
    if ( F_verbose >= 1 )
        ShowSegments();
    
    if ( F_verbose >= 3 )
    {
        if ( !Redo || ( F_verbose == 4 ) )
            ShowSymbols( stdout, *pbTableSort );
        
        ShowUnresolvedSymbols();
    }
    
    closegenerate();
    fclose(FI_temp);
    if (FI_listfile)
        fclose(FI_listfile);
    
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
            
            if ( bStopAtEnd )
            {
                printf("Unrecoverable error(s) in pass, aborting assembly!\n");
            }
            else if ( pass > nMaxPasses )
            {
                char sBuffer[64];
                sprintf( sBuffer, "%d", pass );
                return asmerr( ERROR_TOO_MANY_PASSES, false, sBuffer );
                
            }
            else
            {
                clearrefs();
                clearsegs();
                goto nextpass;
            }
    }
    
    printf( "Complete.\n" );
    
    return nError;
}


int tabit(char *buf1, char *buf2)
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

static void outlistfile(char *comment)
{
    char xtrue;
    char c;
    static char buf1[MAXLINE+32];
    static char buf2[MAXLINE+32];
    char *ptr;
    char *dot;
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




char *cleanup(char *buf, bool bDisable)
{
    char *str;
    STRLIST *strlist;
    int arg, add;
    char *comment = "";
    
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
                    printf("strlist: '%s' %d\n", strlist->buf, strlen(strlist->buf));
                
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

void panic(char *str)
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
    
    i = 0;
    j = 1;

#if OlafFreeFormat
    /* Skip all initial spaces */
    while (buf[i] == ' ')
        ++i;
#endif

#if OlafHashFormat
        /*
        * If the first non-space is a ^, skip all further spaces too.
        * This means what follows is a label.
        * If the first non-space is a #, what follows is a directive/opcode.
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
#endif

    Av[0] = Avbuf + j;
    while (buf[i] && buf[i] != ' ') {

        if (buf[i] == ':') {
            i++;
            break;
        }

        if ((unsigned char)buf[i] == 0x80)
            buf[i] = ' ';
        Avbuf[j++] = buf[i++];
    }
    Avbuf[j++] = 0;

#if OlafFreeFormat
    /* Try if the first word is an opcode */
    findext(Av[0]);
    mne = findmne(Av[0]);
    if (mne != NULL) {
    /* Yes, it is. So there is no label, and the rest
    * of the line is the argument
        */
        Avbuf[0] = 0;    /* Make an empty string */
        Av[1] = Av[0];    /* The opcode is the previous first word */
        Av[0] = Avbuf;    /* Point the label to the empty string */
    } else
#endif

    {    /* Parse the second word of the line */
        while (buf[i] == ' ')
            ++i;
        Av[1] = Avbuf + j;
        while (buf[i] && buf[i] != ' ') {
            if ((unsigned char)buf[i] == 0x80)
                buf[i] = ' ';
            Avbuf[j++] = buf[i++];
        }
        Avbuf[j++] = 0;
        /* and analyse it as an opcode */
        findext(Av[1]);
        mne = findmne(Av[1]);
    }
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
    
    return mne;
}



MNEMONIC *findmne(char *str)
{
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
    for (mne = MHash[hash1(buf)]; mne; mne = mne->next) {
        if (strcmp(buf, mne->name) == 0)
            break;
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
    if (skipit) {
        defined = 1;
    } else {
        defined = (findmne(str) != NULL);
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
        MHash[i] = (MNEMONIC *)mac;
    }
    while (fgets(buf, MAXLINE, pIncfile->fi)) {
        char *comment;
        
        if (Xdebug)
            printf("%08lx %s\n", (unsigned long) pIncfile, buf);
        
        ++pIncfile->lineno;
        
        
        comment = cleanup(buf, true);
        
        mne = parse(buf);
        if (Av[1][0]) {
            if (mne && mne->flags & MF_ENDM) {
                if (!defined)
                    mac->strlist = base;
                return;
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


static unsigned int hash1(char *str)
{
    unsigned int result = 0;
    
    while (*str)
        result = (result << 2) ^ *str++;
    return result & MHASHAND;
}

void pushinclude(char *str)
{
    INCFILE *inf;
    FILE *fi;
    
    if ((fi = pfopen(str, "r")) != NULL) {
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
    printf("Warning: Unable to open '%s'\n", str);
    return;
}




int asmerr(int err, bool abort, char *sText )
{
    char *str;
    INCFILE *pincfile;
    
    if ( err >= MAX_ERROR || err < 0 )
    {
        return asmerr( ERROR_BADERROR, true, "Bad error ERROR!" );
    }
    else
    {
        
        if (sErrorDef[err].bFatal)
            bStopAtEnd = true;
        
        for ( pincfile = pIncfile; pincfile->flags & INF_MACRO; pincfile=pincfile->next);
        str = sErrorDef[err].sDescription;
        
#ifdef DAD
        
        /* Error output format changed to be Visual-Studio compatible.
        Output now file (line): error: string
        */
        
        if (F_listfile)
        {
            fprintf(FI_listfile, "%s (%d): error: ", pincfile->name, pincfile->lineno );
            fprintf(FI_listfile, str, sText ? sText : "" );
            fprintf(FI_listfile, "\n" );
        }
        printf( "%s (%d): error: ", pincfile->name, pincfile->lineno );
        printf( str, sText ? sText : "" );
        printf( "\n" );
        
        
#else
        
        if (F_listfile)
            fprintf(FI_listfile, "*line %7ld %-10s %s\n", pincfile->lineno, pincfile->name, str);
        printf("line %7ld %-10s %s\n", pincfile->lineno, pincfile->name, str);
        
#endif
        
        if ( abort )
        {
            puts("Aborting assembly");
            if (F_listfile)
                fputs("Aborting assembly\n", FI_listfile);
            
            exit( 1 );
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
    bool bTableSort = false;
    int nError = MainShadow( ac, av, &bTableSort );
    
    if ( nError )
        printf( "Fatal assembly error: %s\n", sErrorDef[nError].sDescription );
    
    DumpSymbolTable( bTableSort );
    
    return nError;
}

