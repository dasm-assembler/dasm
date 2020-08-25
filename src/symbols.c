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
 *  SYMBOLS.C
 */

#include "asm.h"

static unsigned int hash1(const char *str, int len);
SYMBOL *allocsymbol(void);

static SYMBOL org;
static SYMBOL special;
static SYMBOL specchk;

void setspecial(int value, int flags)
{
    special.value = value;
    special.flags = flags;
}

SYMBOL *findsymbol(const char *str, int len)
{
    unsigned int h1;
    SYMBOL *sym;
    char buf[MAX_SYM_LEN + 14];     /* historical */
    
    if ( len > MAX_SYM_LEN )
        len = MAX_SYM_LEN;
    
    if (str[0] == '.')
    {
        if (len == 1)
        {
            if (Csegment->flags & SF_RORG)
            {
                org.flags = Csegment->rflags & SYM_UNKNOWN;
                org.value = Csegment->rorg;
            }
            else
            {
                org.flags = Csegment->flags & SYM_UNKNOWN;
                org.value = Csegment->org;
            }
            return &org;
        }
        if (len == 2 && str[1] == '.')
            return &special;
        if (len == 3 && str[1] == '.' && str[2] == '.')
        {
            specchk.flags = 0;
            specchk.value = CheckSum;
            return &specchk;
        }
        sprintf(buf, "%ld%.*s", Localindex, len, str);
        len = strlen(buf);
        str = buf;
    }
    
    else if (str[len - 1] == '$')
    {
        sprintf(buf, "%ld$%.*s", Localdollarindex, len, str);
        len = strlen(buf);
        str = buf;
    }
    
    h1 = hash1(str, len);
    for (sym = SHash[h1]; sym; sym = sym->next)
    {
        if ((sym->namelen == len) && !memcmp(sym->name, str, len))
            break;
    }
    return sym;
}

SYMBOL *CreateSymbol( const char *str, int len )
{
    SYMBOL *sym;
    unsigned int h1;
    char buf[ MAX_SYM_LEN + 14 ];           /* historical */
    
    if (len > MAX_SYM_LEN )
        len = MAX_SYM_LEN;
    
    if (str[0] == '.')
    {
        sprintf(buf, "%ld%.*s", Localindex, len, str);
        len = strlen(buf);
        str = buf;
    }
    
    
    else if (str[len - 1] == '$')
    {
        sprintf(buf, "%ld$%.*s", Localdollarindex, len, str);
        len = strlen(buf);
        str = buf;
    }
    
    sym = allocsymbol();
    sym->name = permalloc(len+1);
    memcpy(sym->name, str, len);    /*	permalloc zeros the array for us */
    sym->namelen = len;
    h1 = hash1(str, len);
    sym->next = SHash[h1];
    sym->flags= SYM_UNKNOWN;
    SHash[h1] = sym;
    return sym;
}

static unsigned int hash1(const char *str, int len)
{
    uint8_t a = 0;
    uint8_t b = 0;
    while (len--) {	// this is Fletcher's checksum, better distribution, faster
    	a += *str++;
    	b += a;
    }
    return ((((a << 8) & 0xFF00) | (b & 0xFF))  ) & SHASHAND;
}

/*
*  Label Support Routines
*/

void programlabel(void)
{
    int len;
    SYMBOL *sym;
    SEGMENT *cseg = Csegment;
    char *str;
    unsigned char rorg = cseg->flags & SF_RORG;
    unsigned char cflags = (rorg) ? cseg->rflags : cseg->flags;
    unsigned long   pc = (rorg) ? cseg->rorg : cseg->org;
    
    Plab = cseg->org;
    Pflags = cseg->flags;
    str = Av[0];
    if (*str == 0)
        return;
    len = strlen(str);


    if (str[len-1] == ':')
        --len;
    
    if (str[0] != '.' && str[len-1] != '$')
    {
        Lastlocaldollarindex++;
        Localdollarindex = Lastlocaldollarindex;
    }
    
    /*
    *	Redo:	unknown and referenced
    *		referenced and origin not known
    *		known and phase error	 (origin known)
    */
    
    if ((sym = findsymbol(str, len)) != NULL)
    {
        if ((sym->flags & (SYM_UNKNOWN|SYM_REF)) == (SYM_UNKNOWN|SYM_REF))
        {
            ++Redo;
            Redo_why |= REASON_FORWARD_REFERENCE;
            if (Xdebug)
                printf("redo 13: '%s' %04x %04x\n", sym->name, sym->flags, cflags);
        }
        else if ((cflags & SYM_UNKNOWN) && (sym->flags & SYM_REF))
        {
            ++Redo;
            Redo_why |= REASON_FORWARD_REFERENCE;
        }
        else if (!(cflags & SYM_UNKNOWN) && !(sym->flags & SYM_UNKNOWN))
        {
            if (pc != sym->value)
            {

            /*
            * If we had an unevaluated IF expression in the
            * previous pass, don't complain about phase errors
            * too loudly.
                */

                //FIX: calling asmerr with ERROR_LABEL_MISMATCH is fatal. The clause
                //     below was causing aborts if verbosity was up, even when the
                //     phase errors were the result of unevaluated IF expressions in
                //     the previous pass.

                //if (F_verbose >= 1 || !(Redo_if & (REASON_OBSCURE))) 

                if (!(Redo_if & (REASON_OBSCURE)))
                {
                    char sBuffer[ MAX_SYM_LEN * 4 ];
                    sprintf( sBuffer, "%s %s", sym->name, sftos( sym->value, 0 ) );
                    asmerr( ERROR_LABEL_MISMATCH, false, sBuffer );
                }
                ++Redo;
                Redo_why |= REASON_PHASE_ERROR;
            }
        }
    }
    else
    {
        sym = CreateSymbol( str, len );
    }
    sym->value = pc;
    sym->flags = (sym->flags & ~SYM_UNKNOWN) | (cflags & SYM_UNKNOWN);
}

SYMBOL *SymAlloc;

SYMBOL *allocsymbol(void)
{
    SYMBOL *sym;
    
    if (SymAlloc)
    {
        sym = SymAlloc;
        SymAlloc = SymAlloc->next;
        memset(sym, 0, sizeof(SYMBOL));
    }
    else
    {
        sym = (SYMBOL *)permalloc(sizeof(SYMBOL));
    }
    return sym;
}

/* defined but not used [phf] */
/*
static void freesymbol(SYMBOL *sym)
{
    sym->next = SymAlloc;
    SymAlloc = sym;
}
*/



void FreeSymbolList(SYMBOL *sym)
{
    SYMBOL *next;
    
    while (sym)
    {
        next = sym->next;
        sym->next = SymAlloc;
        if (sym->flags & SYM_STRING)
            free(sym->string);
        SymAlloc = sym;
        sym = next;
    }
}

