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
 *  EXP.C
 *
 *  Handle expression evaluation and addressing mode decode.
 *
 *  NOTE! If you use the string field in an expression you must clear
 *  the SYM_MACRO and SYM_STRING bits in the flags before calling
 *  FreeSymbolList()!
 */

#include "asm.h"

#define UNION	0

#if UNION		/* warning: ANSI disallows cast to union type */
typedef void (unop)(long v1, int f1);
typedef void (binop)(long v1, long v2, int f1, int f2);

union unibin {
    unop *unary;
    binop *binary;
};

typedef union unibin opfunc_t;
#define _unary	.unary
#define _binary .binary
#else			/* warning: Calling functions without prototype */

typedef void (*opfunc_t)();
#define _unary
#define _binary

#endif

static void stackarg(long val, int flags, const char *ptr1);

void doop(opfunc_t, int pri);
void evaltop(void);
void	op_mult(long v1, long v2, int f1, int f2),
op_div(long v1, long v2, int f1, int f2),
op_mod(long v1, long v2, int f1, int f2),
op_add(long v1, long v2, int f1, int f2),
op_sub(long v1, long v2, int f1, int f2),
op_shiftleft(long v1, long v2, int f1, int f2),
op_shiftright(long v1, long v2, int f1, int f2),
op_greater(long v1, long v2, int f1, int f2),
op_greatereq(long v1, long v2, int f1, int f2),
op_smaller(long v1, long v2, int f1, int f2),
op_smallereq(long v1, long v2, int f1, int f2),
op_eqeq(long v1, long v2, int f1, int f2),
op_noteq(long v1, long v2, int f1, int f2),
op_andand(long v1, long v2, int f1, int f2),
op_oror(long v1, long v2, int f1, int f2),
op_xor(long v1, long v2, int f1, int f2),
op_and(long v1, long v2, int f1, int f2),
op_or(long v1, long v2, int f1, int f2),
op_question(long v1, long v2, int f1, int f2);

void	op_takelsb(long v1, int f1),
op_takemsb(long v1, int f1),
op_negate(long v1, int f1),
op_invert(long v1, int f1),
op_not(long v1, int f1);


const char *pushsymbol(const char *str);
const char *pushstr(const char *str);
const char *pushbin(const char *str);
const char *pushoct(const char *str);
const char *pushdec(const char *str);
const char *pushhex(const char *str);
const char *pushchar(const char *str);

int IsAlphaNum( int c );

/*
*  evaluate an expression.  Figure out the addressing mode:
*
*		implied
*    #val	immediate
*    val	zero page or absolute
*    val,x	zero,x or absolute,x
*    val,y	zero,y or absolute,y
*    val,sp	stack pointer indexed + offset
*    (val)	indirect
*    (val,x)	zero indirect x
*    (val),y	zero indirect y
*    val,val    zero page, relative
*
*    exp, exp,.. LIST of expressions
*
*  an absolute may be returned as zero page
*  a relative may be returned as zero page or absolute
*
*  unary:  - ~ ! < >
*  binary: (^)(* / %)(+ -)(>> <<)(& |)(`)(&& ||)(== != < > <= >=)
*
*  values: symbol, octal, decimal, $hex, %binary, 'c "str"
*
*/

#define MAXOPS	    32
#define MAXARGS     64

unsigned char Argflags[MAXARGS];
long  Argstack[MAXARGS];
char *Argstring[MAXARGS];
int Oppri[MAXOPS];
opfunc_t Opdis[MAXOPS];

int	Argi, Opi, Lastwasop;
int	Argibase, Opibase;

char ucasm_indexed_notation = false;

SYMBOL *eval(const char *str, int wantmode)
{
    SYMBOL *base, *cur;
    int oldargibase = Argibase;
    int oldopibase = Opibase;
    int scr;
    
    const char *pLine = str;

    Argibase = Argi;
    Opibase = Opi;
    Lastwasop = 1;
    base = cur = allocsymbol();
    

    while (*str)
    {
        if (Xdebug)
            printf("char '%c'\n", *str);
        
        switch(*str)
        {
        case ' ':
        case '\n':
            ++str;
            break;

        case '~':
            if (Lastwasop)
                doop((opfunc_t)op_invert, 128);
            else
                asmerr( ERROR_SYNTAX_ERROR, false, pLine );
            ++str;
            break;

        case '*':
            if (Lastwasop) {
                pushsymbol(".");
            } else
                doop((opfunc_t)op_mult, 20);
            ++str;
            break;

        case '/':
            doop((opfunc_t)op_div, 20);
            ++str;
            break;

        case '%':
            if (Lastwasop) {
                str = pushbin(str+1);
            } else {
                doop((opfunc_t)op_mod, 20);
                ++str;
            }
            break;

        case '?':   /*  10      */
            doop((opfunc_t)op_question, 10);
            ++str;
            break;

        case '+':   /*  19      */
            doop((opfunc_t)op_add, 19);
            ++str;
            break;

        case '-':   /*  19: -   (or - unary)        */
            if (Lastwasop) {
                doop((opfunc_t)op_negate, 128);
            } else {
                doop((opfunc_t)op_sub, 19);
            }
            ++str;
            break;

        case '>':   /*  18: >> <<  17: > >= <= <    */
            
            if (Lastwasop)
            {
                doop((opfunc_t)op_takemsb, 128);
                ++str;
                break;
            }

            if (str[1] == '>')
            {
                doop((opfunc_t)op_shiftright, 18);
                ++str;
            }

            else if (str[1] == '=')
            {
                doop((opfunc_t)op_greatereq, 17);
                ++str;
            }
            else
            {
                doop((opfunc_t)op_greater, 17);
            }
            ++str;
            break;

        case '<':
            
            if (Lastwasop)
            {
                doop((opfunc_t)op_takelsb, 128);
                ++str;
                break;
            }

            if (str[1] == '<')
            {
                doop((opfunc_t)op_shiftleft, 18);
                ++str;
            }
            else if (str[1] == '=')
            {
                doop((opfunc_t)op_smallereq, 17);
                ++str;
            }
            else
            {
                doop((opfunc_t)op_smaller, 17);
            }
            ++str;
            break;

        case '=':   /*  16: ==  (= same as ==)      */
            
            if (str[1] == '=')
                ++str;
            doop((opfunc_t)op_eqeq, 16);
            ++str;
            break;

        case '!':   /*  16: !=                      */
            
            if (Lastwasop)
            {
                doop((opfunc_t)op_not, 128);
            }
            else
            {
                doop((opfunc_t)op_noteq, 16);
                ++str;
            }
            ++str;
            break;

        case '&':   /*  15: &   12: &&              */
            
            if (str[1] == '&')
            {
                doop((opfunc_t)op_andand, 12);
                ++str;
            }
            else
            {
                doop((opfunc_t)op_and, 15);
            }
            ++str;
            break;

        case '^':   /*  14: ^                       */
            
            doop((opfunc_t)op_xor, 14);
            ++str;
            break;

        case '|':   /*  13: |   11: ||              */
            
            if (str[1] == '|')
            {
                doop((opfunc_t)op_oror, 11);
                ++str;
            }
            else
            {
                doop((opfunc_t)op_or, 13);
            }
            ++str;
            break;

            
        case '(':
            
            if (wantmode)
            {
                cur->addrmode = AM_INDWORD;
                ++str;
                break;
            }
            
            /* fall thru OK */
            
        case '[':   /*  eventually an argument      */
	    ucasm_indexed_notation = false;
            
	    if ((((((str[1]|0x20) == 'x') || ((str[1]|0x20) == 'y')) && (str[2] == '+')) ||     // X- or Y-indexed address mode
	        (((str[1]|0x20) == 's') && ((str[2]|0x20) == 'p') && (str[3] == '+'))) &&	// SP-indexed address mode
		((Processor == 68705) || (Processor == 6811) || (Processor == 68908)))
	    {
		ucasm_indexed_notation = true;
		// UCASM compatibility, allow notations [X+255], [Y+3], [SP+5]
		switch(str[1]|0x20) {
		    case 'x': 
			cur->addrmode = AM_BYTEADRX; 
			//FIX: OPCODE.FORCE / Mnext adaption moved to ops.c
			break;

		    case 'y': cur->addrmode = AM_BYTEADRY; break;

		    case 's': 
			cur->addrmode = AM_BYTEADR_SP;
			//FIX: OPCODE.FORCE / Mnext adaption moved to ops.c
			break;
		}
		str += 3;	/* skip '[',{x,y,s},'+' */
		if ((cur->addrmode == AM_BYTEADR_SP) || (cur->addrmode == AM_WORDADR_SP)) {
		    ++str;	/* skip also 'p' */
		}
	    } else {
		ucasm_indexed_notation = false;
            if (Opi == MAXOPS)
                puts("too many ops");
            else
                Oppri[Opi++] = 0;
            ++str;
	    }
            break;
            
        case ')':
            
            if (wantmode)
            {
                if (cur->addrmode == AM_INDWORD && str[1] == ',' && (str[2]|0x20) == 'y')
                {
                    cur->addrmode = AM_INDBYTEY;
                    str += 2;
                }
		//FIX: detect illegal opc (zp),x syntax...
                if (cur->addrmode == AM_INDWORD && str[1] == ',' && (str[2]|0x20) == 'x')
                {
                   char sBuffer[128];
                   sprintf( sBuffer, "%s", str );
                   asmerr( ERROR_ILLEGAL_ADDRESSING_MODE,false, pLine );
                   ++Redo;
                   Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;

                   //we treat the opcode as valid to allow passes to continue, which should
                   //allow other errors (like phase errros) to resolve before our "++Redo"
                   //ultimately forces a failure.
                }

                if ((cur->addrmode == AM_INDWORD) && (str[1] == '\0') && (Processor == 16502))
                {
                    cur->addrmode = AM_INDBYTE;
                }
                ++str;
                break;
            }
            
            /* fall thru OK */
            
        case ']':
            
            while(Opi != Opibase && Oppri[Opi-1])
                evaltop();
            if (Opi != Opibase)
                --Opi;
            ++str;

	    if (ucasm_indexed_notation) {
		ucasm_indexed_notation = false;
	    } else {
            if (Argi == Argibase)
            {
                puts("']' error, no arg on stack");
                break;
            }
            }
            
            if (*str == 'd')
            {  /*  STRING CONVERSION   */
                char buf[32];
                ++str;
                if (Argflags[Argi-1] == 0)
                {
                    sprintf(buf,"%ld",Argstack[Argi-1]);
                    Argstring[Argi-1] = strcpy(ckmalloc(strlen(buf)+1),buf);
                }
            }
	    Lastwasop = 0;
            break;

        case '#':
            
            cur->addrmode = AM_IMM8;
            ++str;
            /*
            * No other addressing mode is possible from now on
            * so we might as well allow () instead of [].
            */
            wantmode = 0;
            break;
            
        case ',':
            
            while(Opi != Opibase)
                evaltop();
            Lastwasop = 1;
            scr = str[1]|0x20;	  /* to lower case */
            
            if (cur->addrmode == AM_INDWORD && scr == 'x' && !IsAlphaNum( str[2] ))
            {
                cur->addrmode = AM_INDBYTEX;
                ++str;
            }
            else if (cur->addrmode != AM_INDWORD && scr != 'x' && scr != 'y' && (Processor == 16502))
            {
                cur->addrmode = AM_BYTEREL;
                SYMBOL *pNewSymbol = allocsymbol();
                cur->next = pNewSymbol;
                --Argi;
                cur->value = Argstack[Argi];
                cur->flags = Argflags[Argi];

                if ((cur->string = (void *)Argstring[Argi]) != NULL)
                {
                    cur->flags |= SYM_STRING;
                    if (Xdebug)
                        printf("STRING: %s\n", cur->string);
                }
                cur = pNewSymbol;

                // Seems like this is not needed - not sure why
                // ++str;
            }
            //FIX: detect illegal opc (zp,y) syntax...
            else if ((cur->addrmode == AM_INDWORD && scr == 'y' && str[2]==')')&&(wantmode))
            {
                   char sBuffer[128];
                   sprintf( sBuffer, "%s", str );
                   asmerr( ERROR_ILLEGAL_ADDRESSING_MODE,false, pLine );
                   ++Redo;
                   Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;

                   //we treat the opcode as valid to allow passes to continue, which should
                   //allow other errors (like phase errros) to resolve before our "++Redo"
                   //ultimately forces a failure.
                   cur->addrmode = AM_0Y; 
                   ++str;

            }
            else if (scr == 'x' && !IsAlphaNum(str[2]))
            {
                cur->addrmode = AM_0X;
                ++str;
                //FIX: OPCODE.FORCE / Mnext adaption moved to ops.c
            }
            else if (scr == 'y' && !IsAlphaNum(str[2]))
            {
                cur->addrmode = AM_0Y;
                ++str;
                //FIX: OPCODE.FORCE / Mnext adaption moved to ops.c
            }
            else if ((scr == 's') && ((str[2]|0x20) == 'p') && !IsAlphaNum(str[3]))      // stack pointer indexed address mode
            {
                cur->addrmode = AM_BYTEADR_SP;
                ++str;
                ++str;
                //FIX: OPCODE.FORCE / Mnext adaption moved to ops.c
            }
            else
            {
                SYMBOL *pNewSymbol = allocsymbol();
                cur->next = pNewSymbol;
                --Argi;
                if (Argi < Argibase)
                    asmerr( ERROR_SYNTAX_ERROR, false, pLine );
                if (Argi > Argibase)
                    asmerr( ERROR_SYNTAX_ERROR, false, pLine );
                cur->value = Argstack[Argi];
                cur->flags = Argflags[Argi];
                
                if ((cur->string = (void *)Argstring[Argi]) != NULL)
                {
                    cur->flags |= SYM_STRING;
                    if (Xdebug)
                        printf("STRING: %s\n", cur->string);
                }
                cur = pNewSymbol;
            }
            ++str;
            break;

        case '$':
            str = pushhex(str+1);
            break;

        case '\'':
            str = pushchar(str+1);
            break;

        case '\"':
            str = pushstr(str+1);
            break;

        default:
            {
                const char *dol = str;
                while (*dol >= '0' && *dol <= '9')
                    dol++;
                if (*dol == '$')
                {
                    str = pushsymbol(str);
                    break;
                }
            }

            if (*str == '0')
            {
                if (str[1] == 'x') {                   // allow also '0xAA' notation for '$AA'
                    ++str;
                    str = (char *)pushhex(str+1);
                } else {
                str = pushoct(str);
                }
            }
            else
            {
                if (*str > '0' && *str <= '9')
                    str = pushdec(str);
                else
                    str = pushsymbol(str);
            }
            break;
        }
    }

    while(Opi != Opibase)
        evaltop();
    
    if (Argi != Argibase)
    {
        --Argi;
        cur->value = Argstack[Argi];
        cur->flags = Argflags[Argi];
        if ((cur->string = (void *)Argstring[Argi]) != NULL)
        {
            cur->flags |= SYM_STRING;
            if (Xdebug)
                printf("STRING: %s\n", cur->string);
        }
        if ((base->addrmode == 0) && (cur->addrmode != AM_BYTEREL))
            base->addrmode = AM_BYTEADR;

        if ((base->addrmode == 0) && (cur->addrmode == AM_BYTEREL))
            base->addrmode = AM_BYTEREL;
    }

    if (Argi != Argibase || Opi != Opibase)
        asmerr( ERROR_SYNTAX_ERROR, false, pLine );


    Argi = Argibase;
    Opi  = Opibase;
    Argibase = oldargibase;
    Opibase = oldopibase;
    return base;
}


int IsAlphaNum( int c )
{
    return ((c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9'));
}

void evaltop(void)
{
    if (Xdebug)
        printf("evaltop @(A,O) %d %d\n", Argi, Opi);
    
    if (Opi <= Opibase) {
        asmerr( ERROR_SYNTAX_ERROR, false, NULL );
        Opi = Opibase;
        return;
    }
    --Opi;

    if (Opdis[Opi] == NULL) {
    	asmerr( ERROR_AVOID_SEGFAULT, true, "operator function table" );
    	return;
    }

    if (Oppri[Opi] == 128) {
        if (Argi < Argibase + 1) {
            asmerr( ERROR_SYNTAX_ERROR, false, NULL );
            Argi = Argibase;
            return;
        }
        --Argi;
        (*Opdis[Opi]_unary)(Argstack[Argi], Argflags[Argi]);
    }
    else
    {
        if (Argi < Argibase + 2)
        {
            asmerr( ERROR_SYNTAX_ERROR, false, NULL );
            Argi = Argibase;
            return;
        }

        Argi -= 2;
        (*Opdis[Opi]_binary)(Argstack[Argi], Argstack[Argi+1],
            Argflags[Argi], Argflags[Argi+1]);
    }
}

static void stackarg(long val, int flags, const char *ptr1)
{
    char *str = NULL;
    
    if (Xdebug)
        printf("stackarg %ld (@%d)\n", val, Argi);
    
    Lastwasop = 0;
    if (flags & SYM_STRING)
    {
        /*
           Why unsigned char? Looks like we're converting to
           long in a very strange way... [phf]
        */
        const unsigned char *ptr = (const unsigned char *)ptr1;
        char *new;
        int len;
        val = len = 0;
        while (*ptr && *ptr != '\"')
        {
            val = (val << 8) | *ptr;
            ++ptr;
            ++len;
        }
        new = ckmalloc(len + 1);
        memcpy(new, ptr1, len);
        new[len] = 0;
        flags &= ~SYM_STRING;
        str = new;
    }
    Argstack[Argi] = val;
    Argstring[Argi] = str;
    Argflags[Argi] = flags;
    if (++Argi == MAXARGS) {
        puts("stackarg: maxargs stacked");
        Argi = Argibase;
    }
    while (Opi != Opibase && Oppri[Opi-1] == 128)
        evaltop();
}

void doop(opfunc_t func, int pri)
{
    if (Xdebug)
        puts("doop");
    
    Lastwasop = 1;
    
    if (Opi == Opibase || pri == 128)
    {
        if (Xdebug)
            printf("doop @ %d unary\n", Opi);

	if (Opi >= MAXOPS) {
		fprintf(stderr,"doop: error: operator index(%d) > MAXOPS(%d), probably too deep recursion", Opi, MAXOPS);
		asmerr(ERROR_RECURSION_TOO_DEEP, true, "doop()");
		return;
	}
        Opdis[Opi] = func;
        Oppri[Opi] = pri;
        ++Opi;
        return;
    }
    
    while (Opi != Opibase && Oppri[Opi-1] && pri <= Oppri[Opi-1])
        evaltop();
    
    if (Xdebug)
        printf("doop @ %d\n", Opi);
    
    Opdis[Opi] = func;
    Oppri[Opi] = pri;
    ++Opi;
    
    if (Opi == MAXOPS)
    {
        puts("doop: too many operators");
        Opi = Opibase;
    }
    return;
}

void op_takelsb(long v1, int f1)
{
    stackarg(v1 & 0xFFL, f1, NULL);
}

void op_takemsb(long v1, int f1)
{
    stackarg((v1 >> 8) & 0xFF, f1, NULL);
}

void op_negate(long v1, int f1)
{
    stackarg(-v1, f1, NULL);
}

void op_invert(long v1, int f1)
{
    stackarg(~v1, f1, NULL);
}

void op_not(long v1, int f1)
{
    stackarg(!v1, f1, NULL);
}

void op_mult(long v1, long v2, int f1, int f2)
{
    stackarg(v1 * v2, f1|f2, NULL);
    Lastwasop = 1;
}

void op_div(long v1, long v2, int f1, int f2)
{
    Lastwasop = 1;
    if (f1|f2) {
        stackarg(0L, f1|f2, NULL);
        return;
    }
    if (v2 == 0)
    {
        asmerr( ERROR_DIVISION_BY_0, true, NULL );
        stackarg(0L, 0, NULL);
    }
    else
    {
        stackarg(v1 / v2, 0, NULL);
    }
}

void op_mod(long v1, long v2, int f1, int f2)
{
    if (f1|f2) {
        stackarg(0L, f1|f2, NULL);
        return;
    }
    if (v2 == 0)
        stackarg(v1, 0, NULL);
    else
        stackarg(v1 % v2, 0, NULL);
    Lastwasop = 1;
}

void op_question(long v1, long v2, int f1, int f2)
{
    if (f1)
        stackarg(0L, f1, NULL);
    else
        stackarg((long)((v1) ? v2 : 0), ((v1) ? f2 : 0), NULL);
}

void op_add(long v1, long v2, int f1, int f2)
{
    stackarg(v1 + v2, f1|f2, NULL);
    Lastwasop = 1;
}

void op_sub(long v1, long v2, int f1, int f2)
{
    stackarg(v1 - v2, f1|f2, NULL);
    Lastwasop = 1;
}

void op_shiftright(long v1, long v2, int f1, int f2)
{
    if (f1|f2)
        stackarg(0L, f1|f2, NULL);
    else
        stackarg((long)(v1 >> v2), 0, NULL);
}

void op_shiftleft(long v1, long v2, int f1, int f2)
{
    if (f1|f2)
        stackarg(0L, f1|f2, NULL);
    else
        stackarg((long)(v1 << v2), 0, NULL);
}

void op_greater(long v1, long v2, int f1, int f2)
{
    stackarg((long)(v1 > v2), f1|f2, NULL);
}

void op_greatereq(long v1, long v2, int f1, int f2)
{
    stackarg((long)(v1 >= v2), f1|f2, NULL);
}

void op_smaller(long v1, long v2, int f1, int f2)
{
    stackarg((long)(v1 < v2), f1|f2, NULL);
}

void op_smallereq(long v1, long v2, int f1, int f2)
{
    stackarg((long)(v1 <= v2), f1|f2, NULL);
}

void op_eqeq(long v1, long v2, int f1, int f2)
{
    stackarg((long)(v1 == v2), f1|f2, NULL);
}

void op_noteq(long v1, long v2, int f1, int f2)
{
    stackarg((long)(v1 != v2), f1|f2, NULL);
}

void op_andand(long v1, long v2, int f1, int f2)
{
    if ((!f1 && !v1) || (!f2 && !v2)) {
        stackarg(0L, 0, NULL);
        return;
    }
    stackarg(1L, f1|f2, NULL);
}

void op_oror(long v1, long v2, int f1, int f2)
{
    if ((!f1 && v1) || (!f2 && v2)) {
        stackarg(1L, 0, NULL);
        return;
    }
    stackarg(0L, f1|f2, NULL);
}

void op_xor(long v1, long v2, int f1, int f2)
{
    stackarg(v1^v2, f1|f2, NULL);
}

void op_and(long v1, long v2, int f1, int f2)
{
    stackarg(v1&v2, f1|f2, NULL);
}

void op_or(long v1, long v2, int f1, int f2)
{
    stackarg(v1|v2, f1|f2, NULL);
}

const char *pushchar(const char *str)
{
    if (*str) {
        stackarg((long)*str, 0, NULL);
        ++str;
    } else {
        stackarg((long)' ', 0, NULL);
    }
    return str;
}

const char *pushhex(const char *str)
{
    long val = 0;
    for (;; ++str) {
        if (*str >= '0' && *str <= '9') {
            val = (val << 4) + (*str - '0');
            continue;
        }
        if ((*str >= 'a' && *str <= 'f') || (*str >= 'A' && *str <= 'F')) {
            val = (val << 4) + ((*str&0x1F) + 9);
            continue;
        }
        break;
    }
    stackarg(val, 0, NULL);
    return str;
}

const char *pushoct(const char *str)
{
    long val = 0;
    while (*str >= '0' && *str <= '7') {
        val = (val << 3) + (*str - '0');
        ++str;
    }
    stackarg(val, 0, NULL);
    return str;
}

const char *pushdec(const char *str)
{
    long val = 0;
    while (*str >= '0' && *str <= '9') {
        val = (val * 10) + (*str - '0');
        ++str;
    }
    stackarg(val, 0, NULL);
    return str;
}

const char *pushbin(const char *str)
{
    long val = 0;
    while (*str == '0' || *str == '1') {
        val = (val << 1) | (*str - '0');
        ++str;
    }
    stackarg(val, 0, NULL);
    return str;
}

const char *pushstr(const char *str)
{
    stackarg(0, SYM_STRING, str);
    while (*str && *str != '\"')
        ++str;
    if (*str == '\"')
        ++str;
    return str;
}

static int symbolRecursionCount = 0;

const char *pushsymbol(const char *str)
{
    SYMBOL *sym;
    const char *ptr;
    unsigned char macro = 0;

    symbolRecursionCount++;

    if (symbolRecursionCount > 1000) {
    	fprintf(stderr, "error: %s:%d: recursion > 1000, too deep, aborting\n",__FILE__,__LINE__);
    	asmerr(ERROR_RECURSION_TOO_DEEP, true, "pushsymbol()");
    }

    for (ptr = str;
    *ptr == '_' ||
        *ptr == '.' ||
        (*ptr >= 'a' && *ptr <= 'z') ||
        (*ptr == '@') ||                         // UCASM compatibility, allow at-sign to apear in label names
        (*ptr >= 'A' && *ptr <= 'Z') ||
        (*ptr >= '0' && *ptr <= '9');
    ++ptr
        );
    if (ptr == str) {
        asmerr( ERROR_ILLEGAL_CHARACTER, false, str );
        printf("char = '%c' %d (-1: %d)\n", *str, *str, *(str-1));
        if (F_listfile)
            fprintf(FI_listfile, "char = '%c' code %d\n", *str, *str);
        return str+1;
    }

    if (*ptr == '$')
        ptr++;

    if ((sym = findsymbol(str, ptr - str)) != NULL)
    {
        if (sym->flags & SYM_UNKNOWN)
            ++Redo_eval;
        
        if (sym->flags & SYM_MACRO)
        {
            macro = 1;
            sym = eval(sym->string, 0);
        }
        
        if (sym->flags & SYM_STRING)
            stackarg(0, SYM_STRING, sym->string);
        
        else
            stackarg(sym->value, sym->flags & SYM_UNKNOWN, NULL);
        
        sym->flags |= SYM_REF|SYM_MASREF;
        
        if (macro)
            FreeSymbolList(sym);
    }
    else
    {
        stackarg(0L, SYM_UNKNOWN, NULL);
        sym = CreateSymbol( str, ptr - str );
        sym->flags = SYM_REF|SYM_MASREF|SYM_UNKNOWN;
        ++Redo_eval;
    }
    symbolRecursionCount--;
    return ptr;
}


