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
 *  OPS.C
 *
 *  Handle mnemonics and pseudo ops
 */

#include "asm.h"

unsigned char Gen[MAXLINE];
unsigned char OrgFill = DEFORGFILL;
int	 Glen;

extern MNEMONIC    Mne6502[];
extern MNEMONIC    Mne65c02[];
extern MNEMONIC    Mne6803[];
extern MNEMONIC    MneHD6303[];
extern MNEMONIC    Mne68705[];
extern MNEMONIC    Mne68HC11[];
extern MNEMONIC    MneF8[];
extern MNEMONIC    Mne68908[];

void generate(void);
void genfill(long fill, long bytes, int size);
void pushif(bool xbool);
int gethexdig(int c);

/*
*  An opcode modifies the SEGMENT flags in the following ways:
*/

void v_processor(char *str, MNEMONIC *dummy)
{
    static bool bCalled = false;
    unsigned long PreviousProcessor = Processor;

    Processor = 0;

    if (strcmp(str,"6502") == 0)
    {
        if ( !bCalled )
            addhashtable(Mne6502);

        MsbOrder = 0;	    /*	lsb,msb */
        Processor = 6502;
    }

    if (strcmp(str,"65c02") == 0 || strcmp(str, "65C02") == 0)
    {
        if ( !bCalled )
            addhashtable(Mne65c02);

        MsbOrder = 0;	    /*	lsb,msb */
        Processor = 16502;
    }
    if (strcmp(str,"6803") == 0)
    {
        if ( !bCalled )
            addhashtable(Mne6803);

        MsbOrder = 1;	    /*	msb,lsb */
        Processor = 6803;
    }

    if (strcmp(str,"HD6303") == 0 || strcmp(str, "hd6303") == 0)
    {
        if ( !bCalled )
        {
            addhashtable(Mne6803);
            addhashtable(MneHD6303);
        }

        MsbOrder = 1;	    /*	msb,lsb */
        Processor = 6303;
    }

    if (strcmp(str,"68705") == 0)
    {
        if ( !bCalled )
            addhashtable(Mne68705);

        MsbOrder = 1;	    /*	msb,lsb */
        Processor = 68705;
    }

    if (strcmp(str,"68HC11") == 0 || strcmp(str, "68hc11") == 0)
    {
        if ( !bCalled )
            addhashtable(Mne68HC11);

        MsbOrder = 1;	    /*	msb,lsb */
        Processor = 6811;
    }

    if (strcmp(str,"F8") == 0 || strcmp(str, "f8") == 0)
    {
		if ( !bCalled )
			addhashtable(MneF8);

		MsbOrder = 1;
        Processor = 0xf8;
    }
    if ((strcmp(str,"68908") == 0) || (strcmp(str,"68hc908") == 0)) {
       if ( !bCalled ) {
           addhashtable(Mne68908);
       }
       MsbOrder = 1;           /*  msb,lsb */
       Processor = 68908;
    }

    bCalled = true;

    if ( !Processor )
    {
        asmerr( ERROR_PROCESSOR_NOT_SUPPORTED, true, str );
    }

    if ( PreviousProcessor && Processor != PreviousProcessor )
    {
        asmerr( ERROR_ONLY_ONE_PROCESSOR_SUPPORTED, true, str );
    }

}

#define badcode(mne,adrmode)  (!(mne->okmask & (1L << adrmode)))

extern int pass;
extern int nMaxPasses;

void v_mnemonic(char *str, MNEMONIC *mne)
{
    int addrmode;
    SYMBOL *sym;
    unsigned int opcode;
    short opidx;
    SYMBOL *symbase;
    int     opsize;
    bool	byteRequested;
    char sBuffer[128];

    Csegment->flags |= SF_REF;
    programlabel();
    symbase = eval(str, 1);

    if ( bTrace )
        printf("PC: %04lx  MNEMONIC: %s  addrmode: %d  ", Csegment->org, mne->name, symbase->addrmode);

    for (sym = symbase; sym; sym = sym->next)
    {
        if (sym->flags & SYM_UNKNOWN)
        {
            ++Redo;
            Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;
            if (!F_verbose)
            {
                sprintf( sBuffer, "%s %s", mne->name, str );
                asmerr( ERROR_UNKNOWN_MNEMONIC, false, sBuffer );
            }
        }
    }
    sym = symbase;

    if (mne->flags & MF_IMOD)
    {
        if (sym->next)
        {
            sym->addrmode = AM_BITMOD;
            if ((mne->flags & MF_REL) && sym->next)
                sym->addrmode = AM_BITBRAMOD;
        }
    }
    addrmode = sym->addrmode;
    if ((sym->flags & SYM_UNKNOWN) || sym->value >= 0x100)
        opsize = 2;
    else
        opsize = (sym->value) ? 1 : 0;

    while (badcode(mne,addrmode) && Cvt[addrmode])
        addrmode = Cvt[addrmode];

    if (mne->opcode[addrmode] == 0x42 && addrmode == AM_INDBYTEX && Processor == 16502)
        addrmode = AM_INDWORDX;
    else if (mne->opcode[addrmode] == 0x42 && addrmode == AM_INDBYTE && Processor == 16502)
        addrmode = AM_INDWORD;

    if ( bTrace )
        printf("mnemask: %08lx adrmode: %d  Cvt[am]: %d   Mnext:%d   value: %ld\n", mne->okmask, addrmode, Cvt[addrmode], Mnext,  sym->value);

    if (badcode(mne,addrmode))
    {
        sprintf( sBuffer, "%s %s", mne->name, str );
        asmerr( ERROR_ILLEGAL_ADDRESSING_MODE, false, sBuffer );
        FreeSymbolList(symbase);
        //FIX
            ++Redo;
            Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;
            if (!F_verbose)
            {
                sprintf( sBuffer, "%s %s", mne->name, str );
                asmerr( ERROR_UNKNOWN_MNEMONIC, false, sBuffer );
            }
        return;
    }


    if (Mnext >= 0 && Mnext < NUMOC)            /*	Force	*/
    {
        addrmode = Mnext;

        //FIX: OPCODE.FORCE needs to be adjusted for x,y,sp indexing...
        switch(sym->addrmode) {
        	case AM_INDBYTEX:
        	case AM_INDBYTEY:
        		// there is only INDWORD to force indirect addressing mode but not with X or Y
        		// this must be an error
                asmerr( ERROR_ILLEGAL_FORCED_ADDRESSING_MODE, false, mne->name );
                FreeSymbolList(symbase);

                //FIX: Cause assembly to fail when an invalid mode is used for an opcode...
                ++Redo;
                Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;
                if (!F_verbose)
                {
                    sprintf( sBuffer, "%s %s", mne->name, str );
                    asmerr( ERROR_UNKNOWN_MNEMONIC, false, sBuffer );
                }
                return;

        	case AM_BYTEADR_SP:
        		addrmode = AM_BYTEADR_SP;
                if ((Mnext == AM_WORDADR) || (Mnext == AM_WORDADR_SP))
                	addrmode = AM_WORDADR_SP;
        		break;

        	case AM_0Y:
        		addrmode = AM_0Y;

            	if ((Mnext == AM_WORDADR) || (Mnext == AM_WORDADRY))
            		addrmode = AM_WORDADRY;

            	if ((Mnext == AM_BYTEADR) || (Mnext == AM_BYTEADRY))
            		addrmode = AM_BYTEADRY;
        		break;

        	case AM_0X:
        		addrmode = AM_0X;

                if ((Mnext == AM_WORDADR) || (Mnext == AM_WORDADRX))
                	addrmode = AM_WORDADRX;

                if ((Mnext == AM_BYTEADR) || (Mnext == AM_BYTEADRX))
                	addrmode = AM_BYTEADRX;
        		break;
        }


        if (badcode(mne,addrmode))
        {
            asmerr( ERROR_ILLEGAL_FORCED_ADDRESSING_MODE, false, mne->name );
            FreeSymbolList(symbase);

               //FIX: Cause assembly to fail when an invalid mode is used for an opcode...
               ++Redo;
               Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;
               if (!F_verbose)
               {
                   sprintf( sBuffer, "%s %s", mne->name, str );
                   asmerr( ERROR_UNKNOWN_MNEMONIC, false, sBuffer );
               }
            return;
        }
    }

    if ( bTrace )
        printf("final addrmode = %d, opsize:%d Opsize[%d]:%d\n", addrmode, opsize, addrmode, Opsize[addrmode]);

    byteRequested = false;
    switch(Mnext) {
    	case AM_IMM8:
    	case AM_BYTEADR:
    	case AM_BYTEADRX:
    	case AM_BYTEADRY:
    	case AM_BYTEADR_SP:
    		byteRequested = true;
    		if (opsize > 1) {
    			sprintf( sBuffer, "%s %s, user requested byte mode", mne->name, str );
    			asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
    		}
    		break;
    }

    switch(addrmode) {
    	case AM_IMM16:
    	case AM_WORDADR:
    	case AM_WORDADRX:
    	case AM_WORDADRY:
    	case AM_INDWORD:
    	case AM_WORDADR_SP:
    		if ( bStrictMode && ((sym->value > 0xFFFF) || (sym->value < -0xFFFF))) {	// isn't this our space ?
    			sprintf( sBuffer, "%s %s", mne->name, str );
    			asmerr( ERROR_ADDRESS_MUST_BE_LT_10000, false, sBuffer );
    		}
    		break;

    	case AM_IMM8:
    	case AM_BYTEADR:
    	case AM_BYTEADRX:
    	case AM_BYTEADRY:
    	case AM_BYTEADR_SP:
    		if (sym->value < -0xFF) {	// isn't this our space ?
    			sprintf( sBuffer, "negative %s %s", mne->name, str );
    			asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
    		}
    		break;
    }

    while (opsize > Opsize[addrmode])
    {
        if (Cvt[addrmode] == 0 || badcode(mne,Cvt[addrmode]))
        {
            if (sym->flags & SYM_UNKNOWN)
                break;

            //FIX: for negative operands...
            if ( (addrmode == AM_IMM8) && (sym->value <0) )
            {
                opsize=1;
                sym->value=(char)(sym->value & 255);
                break;
            }


            if ((sym->value > 255) && !byteRequested) {
                // automatically increasing address-mode only if user has not explicitly stated
            if (addrmode == AM_BYTEADRX) {
                if (! badcode(mne, AM_WORDADRX)) {
                    addrmode = AM_WORDADRX;
                    break;
                }
            }
            if (addrmode == AM_BYTEADRY) {
                if (! badcode(mne, AM_WORDADRY)) {
                    addrmode = AM_WORDADRY;
                    break;
                }
            }
            if (addrmode == AM_BYTEADR_SP) {
                if (! badcode(mne, AM_WORDADR_SP)) {
                    addrmode = AM_WORDADR_SP;
                    break;
                }
            }
	    }

        sprintf( sBuffer, "%s %s", mne->name, str );

        asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
        break;
        }
        addrmode = Cvt[addrmode];
    }
    opcode = mne->opcode[addrmode];
    opidx = 1 + (opcode > 0xFF);
    if (opidx == 2)
    {
        Gen[0] = opcode >> 8;
        Gen[1] = opcode;
    }
    else
    {
        Gen[0] = opcode;
    }

    switch(addrmode)
    {
    case AM_BITMOD:
        sym = symbase->next;
        if (!(sym->flags & SYM_UNKNOWN) && sym->value >= 0x100) {
            sprintf( sBuffer, "unknown %s %ld", mne->name, sym->value);
            asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
        }
        Gen[opidx++] = sym->value;

        if (!(symbase->flags & SYM_UNKNOWN))
        {
            if (symbase->value > 7)
                asmerr( ERROR_ILLEGAL_BIT_SPECIFICATION, false, str );
            else
                Gen[0] += symbase->value << 1;
        }
        break;

    case AM_BITBRAMOD:

        if (!(symbase->flags & SYM_UNKNOWN))
        {
            if (symbase->value > 7)
                asmerr( ERROR_ILLEGAL_BIT_SPECIFICATION, false, str );
            else
                Gen[0] += symbase->value << 1;
        }

        sym = symbase->next;

        if (!(sym->flags & SYM_UNKNOWN) && sym->value >= 0x100) {
            sprintf( sBuffer, "%s %ld", mne->name, sym->value);
            asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
        }
        Gen[opidx++] = sym->value;
        sym = sym->next;
        break;

    case AM_REL:
        break;

    default:
        if (Opsize[addrmode] > 0)
            Gen[opidx++] = sym->value;
        if (Opsize[addrmode] == 2)
        {
            if (MsbOrder)
            {
                Gen[opidx-1] = sym->value >> 8;
                Gen[opidx++] = sym->value;
            }
            else
            {
                Gen[opidx++] = sym->value >> 8;
            }
        }
        sym = sym->next;
        break;
    }

    if (mne->flags & MF_MASK)
    {
        if (sym)
        {
            if (!(sym->flags & SYM_UNKNOWN) && sym->value >= 0x100) {
                sprintf( sBuffer, "unknown && > 256 %s %ld", mne->name, sym->value);
                asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
            }
            Gen[opidx] = sym->value;
            sym = sym->next;
        }
        else
        {
            asmerr( ERROR_NOT_ENOUGH_ARGS, true, NULL );
        }

        ++opidx;
    }

    if ((mne->flags & MF_REL) || addrmode == AM_REL || addrmode == AM_BYTEREL)
    {
        ++opidx;		/*  to end of instruction   */

        if (!sym)
            asmerr( ERROR_NOT_ENOUGH_ARGS, true, NULL );
        else if (!(sym->flags & SYM_UNKNOWN))
        {
            long    pc;
            unsigned char pcf;
            long    dest;

            pc = (Csegment->flags & SF_RORG) ? Csegment->rorg : Csegment->org;
            pcf= (Csegment->flags & SF_RORG) ? Csegment->rflags : Csegment->flags;

            if ((pcf & (SF_UNKNOWN|2)) == 0)
            {
                dest = sym->value - pc - opidx;

                if (dest >= 128 || dest < -128)
                {
                    //FIX: sometimes zero page addressing will first be assumed to be absolute until
                    //     another pass. ERROR_BRANCH_OUT_OF_RANGE was made non-fatal, but we keep
                    //     pushing for Redo so assembly won't actually be succesfull until the branch
                    //     actually works.
                        sprintf( sBuffer, "%ld", dest );
                        asmerr( ERROR_BRANCH_OUT_OF_RANGE, false, sBuffer );
            		++Redo;
            		Redo_why |= REASON_BRANCH_OUT_OF_RANGE;
                        sym->flags=sym->flags | SYM_UNKNOWN;
                        dest = 0;
                }
            }
            else
            {
                /* Don't bother - we'll take another pass */
                dest = 0;
            }
            Gen[opidx-1] = dest & 0xFF;     /*	byte before end of inst.    */
        }
    }
    Glen = opidx;
    generate();
    FreeSymbolList(symbase);
}

void v_trace(char *str, MNEMONIC *dummy)
{
    bTrace = (str[1] == 'n');
}

void v_list(char *str, MNEMONIC *dummy)
{
    programlabel();

    Glen = 0;		/*  Only so outlist() works */

    if (strncmp(str, "localoff", 7) == 0 || strncmp(str, "LOCALOFF", 7) == 0)
        pIncfile->flags |=  INF_NOLIST;
    else if (strncmp(str, "localon", 7) == 0 || strncmp(str, "LOCALON", 7) == 0)
        pIncfile->flags &= ~INF_NOLIST;
    else if (strncmp(str, "off", 2) == 0 || strncmp(str, "OFF", 2) == 0)
        ListMode = 0;
    else
        ListMode = 1;
}

static char *
getfilename(char *str)
{
    if (*str == '\"') {
        char	*buf;

        str++;
        buf = ckmalloc(strlen(str)+1);
        strcpy(buf, str);

        for (str = buf; *str && *str != '\"'; ++str);
        *str = 0;

        return buf;
    }
    return str;
}

void
v_include(char *str, MNEMONIC *dummy)
{
  SYMBOL *sym;

  programlabel();

  // only eval the string if it's compliant with symbol naming
  if ((*str<'0')||(*str>'9')) //check could be more comprehensive
    sym = eval(str, 0);
  else
    sym = NULL;

  if ( (sym) && (sym->flags & SYM_STRING ) )
  {
    pushinclude(sym->string);
  }
  else
  {
    char *buf;
    buf = getfilename(str);

    pushinclude(buf);

    if (buf != str)
      free(buf);
  }

  if (sym)
    FreeSymbolList(sym);
}


void
v_incbin(char *str, MNEMONIC *dummy)
{
    char    *buf;
    char    *fname;
    FILE    *binfile;

    programlabel();

    SYMBOL *sym = eval(str, 0);
    if ((sym->flags & SYM_STRING) == 0) {
        asmerr(ERROR_TYPE_MISMATCH, false, NULL);
        ++Redo;
        Redo_why |= REASON_INCBIN_NOT_RESOLVED;
        return;
    } else {
        fname = sym->string;
    }
    buf = getfilename(fname);

    long skip_bytes = 0;
    if (sym->next) {
        if (sym->next->flags & SYM_UNKNOWN) {
            asmerr(ERROR_VALUE_UNDEFINED, true, NULL);
        }
        skip_bytes = sym->next->value;
    }

    binfile = pfopen(buf, "rb");
    if (binfile) {
        if (Redo) {
            /* optimize: don't actually read the file if not needed */
            fseek(binfile, 0, SEEK_END);
            Glen = ftell(binfile);
            generate();     /* does not access Gen[] if Redo is set */
        }
        else
        {
            fseek(binfile, skip_bytes, SEEK_SET);
            for (;;) {
                Glen = fread(Gen, 1, sizeof(Gen), binfile);
                if (Glen <= 0)
                    break;
                generate();
            }
        }
        fclose(binfile);
    }
    else
    {
        asmerr( ERROR_FILE_ERROR, true, NULL );
    }

    if (buf != str)
        free(buf);
    Glen = 0;		    /* don't list hexdump */
}



void
v_seg(char *str, MNEMONIC *dummy)
{
    SEGMENT *seg;

    for (seg = Seglist; seg; seg = seg->next) {
        if (strcmp(str, seg->name) == 0) {
            Csegment = seg;
            programlabel();
            return;
        }
    }
    Csegment = seg = (SEGMENT *)zmalloc(sizeof(SEGMENT));
    seg->next = Seglist;
    seg->name = strcpy(ckmalloc(strlen(str)+1), str);
    seg->flags= seg->rflags = seg->initflags = seg->initrflags = SF_UNKNOWN;
    Seglist = seg;
    if (Mnext == AM_BSS)
        seg->flags |= SF_BSS;
    programlabel();
}

void
v_hex(char *str, MNEMONIC *dummy)
{
    int i;
    int result;

    programlabel();
    Glen = 0;
    for (i = 0; str[i]; ++i) {
        if (str[i] == ' ')
            continue;
        result = (gethexdig(str[i]) << 4) + gethexdig(str[i+1]);
        if (str[++i] == 0)
            break;
        Gen[Glen++] = result;
    }
    generate();
}

int
gethexdig(int c)
{
    char sBuffer[64];

    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    sprintf( sBuffer, "Bad Hex Digit %c", c );
    asmerr( ERROR_SYNTAX_ERROR, false, sBuffer );

    puts("(Must be a valid hex digit)");
    if (F_listfile)
        fputs("(Must be a valid hex digit)\n", FI_listfile);

    return 0;
}

void
v_err(char *str, MNEMONIC *dummy)
{
    programlabel();
    asmerr( ERROR_ERR_PSEUDO_OP_ENCOUNTERED, true, NULL );
    exit(1);
}

void
v_dc(char *str, MNEMONIC *mne)
{
    SYMBOL *sym;
    SYMBOL *tmp;
    long  value;
    char *macstr = 0;		/* "might be used uninitialised" */
    char vmode = 0;
    char sBuffer[128];		/* verbose error messages*/

    Glen = 0;
    programlabel();


    /* for byte, .byte, word, .word, long, .long */
    if (mne->name[0] != 'd') {
        static char sTmp[4];
        strcpy(sTmp, "x.x");
        sTmp[2] = mne->name[0];
        findext(sTmp);
    }

	/* F8... */

    /* db, dw, dd */
    if ( (mne->name[0] == 'd') && (mne->name[1] != 'c') && (mne->name[1] != 'v')) {
        static char sTmp[4];
        strcpy(sTmp, "x.x");
        if ('d' == mne->name[1]) {
			sTmp[2] = 'l';
		} else {
            sTmp[2] = mne->name[1];
		}
        findext(sTmp);
    }

	/* ...F8 */



    if (mne->name[1] == 'v') {
        int i;
        vmode = 1;
        for (i = 0; str[i] && str[i] != ' '; ++i);
        tmp = findsymbol(str, i);
        str += i;
        if (tmp == NULL) {
            puts("EQM label not found");
            return;
        }
        if (tmp->flags & SYM_MACRO) {
            macstr = (void *)tmp->string;
        }
        else
        {
            puts("must specify EQM label for DV");
            return;
        }
    }
    sym = eval(str, 0);
    for (; sym; sym = sym->next) {
        value = sym->value;
        if (sym->flags & SYM_UNKNOWN) {
            ++Redo;
            Redo_why |= REASON_DC_NOT_RESOVED;
        }
        if (sym->flags & SYM_STRING) {
            unsigned char *ptr = (void *)sym->string;
            while ((value = *ptr) != 0) {
                if (vmode) {
                    setspecial(value, 0);
                    tmp = eval(macstr, 0);
                    value = tmp->value;
                    if (tmp->flags & SYM_UNKNOWN) {
                        ++Redo;
                        Redo_why |= REASON_DV_NOT_RESOLVED_PROBABLY;
                    }
                    FreeSymbolList(tmp);
                }
                switch(Mnext) {
                default:
                case AM_BYTE:
                    Gen[Glen++] = value & 0xFF;
                    break;
                case AM_WORD:
                    if (MsbOrder) {
                        Gen[Glen++] = (value >> 8) & 0xFF;
                        Gen[Glen++] = value & 0xFF;
                    }
                    else
                    {
                        Gen[Glen++] = value & 0xFF;
                        Gen[Glen++] = (value >> 8) & 0xFF;
                    }
                    break;
                case AM_OTHER_ENDIAN:
                    if (MsbOrder == 0) {
                        Gen[Glen++] = (value >> 8) & 0xFF;
                        Gen[Glen++] = value & 0xFF;
                    }
                    else
                    {
                        Gen[Glen++] = value & 0xFF;
                        Gen[Glen++] = (value >> 8) & 0xFF;
                    }
                	break;
                case AM_LONG:
                    if (MsbOrder) {
                        Gen[Glen++] = (value >> 24)& 0xFF;
                        Gen[Glen++] = (value >> 16)& 0xFF;
                        Gen[Glen++] = (value >> 8) & 0xFF;
                        Gen[Glen++] = value & 0xFF;
                    }
                    else
                    {
                        Gen[Glen++] = value & 0xFF;
                        Gen[Glen++] = (value >> 8) & 0xFF;
                        Gen[Glen++] = (value >> 16)& 0xFF;
                        Gen[Glen++] = (value >> 24)& 0xFF;
                    }
                    break;
                }
                ++ptr;
            }
        }
        else
        {
            if (vmode) {
                setspecial(value, sym->flags);
                tmp = eval(macstr, 0);
                value = tmp->value;
                if (tmp->flags & SYM_UNKNOWN) {
                    ++Redo;
                    Redo_why |= REASON_DV_NOT_RESOLVED_COULD;
                }
                FreeSymbolList(tmp);
            }
            switch(Mnext) {
            default:
            case AM_BYTE:
                //any value outside two's complement +ve and +ve byte representation is invalid...
                if ((value < -0xFF)||(value > 0xFF))
		{
                    sprintf( sBuffer, "byte %s %ld", mne->name, value);
                    asmerr( ERROR_ADDRESS_MUST_BE_LT_100, false, sBuffer );
		}
                Gen[Glen++] = value & 0xFF;
                break;
            case AM_WORD:
		//any value outside two's complement +ve and +ve word representation is invalid...
                if ( (bStrictMode) && ((value < -0xFFFF)||(value > 0xFFFF)) )
		{
                    sprintf( sBuffer, "word %s %ld", mne->name, value);
                    asmerr( ERROR_ADDRESS_MUST_BE_LT_10000, false, sBuffer );
		}

                if (MsbOrder) {
                    Gen[Glen++] = (value >> 8) & 0xFF;
                    Gen[Glen++] = value & 0xFF;
                }
                else
                {
                    Gen[Glen++] = value & 0xFF;
                    Gen[Glen++] = (value >> 8) & 0xFF;
                }
                break;
            case AM_OTHER_ENDIAN:
                if ( (bStrictMode) && ((value < -0xFFFF)||(value > 0xFFFF)) ) {
                    sprintf( sBuffer, "swapped %s %ld", mne->name, value);
                    asmerr( ERROR_ADDRESS_MUST_BE_LT_10000, false, sBuffer );
                }
                if (MsbOrder == 0) {
                    Gen[Glen++] = (value >> 8) & 0xFF;
                    Gen[Glen++] = value & 0xFF;
                }
                else
                {
                    Gen[Glen++] = value & 0xFF;
                    Gen[Glen++] = (value >> 8) & 0xFF;
                }
            	break;
            case AM_LONG:
                if (MsbOrder) {
                    Gen[Glen++] = (value >> 24)& 0xFF;
                    Gen[Glen++] = (value >> 16)& 0xFF;
                    Gen[Glen++] = (value >> 8) & 0xFF;
                    Gen[Glen++] = value & 0xFF;
                }
                else
                {
                    Gen[Glen++] = value & 0xFF;
                    Gen[Glen++] = (value >> 8) & 0xFF;
                    Gen[Glen++] = (value >> 16)& 0xFF;
                    Gen[Glen++] = (value >> 24)& 0xFF;
                }
                break;
            }
        }
    }
    generate();
    FreeSymbolList(sym);
}



void
v_ds(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym;
    int mult = 1;
    long filler = 0;

    if (Mnext == AM_WORD)
        mult = 2;
    if (Mnext == AM_LONG)
        mult = 4;
    programlabel();
    if ((sym = eval(str, 0)) != NULL) {
        if (sym->next)
            filler = sym->next->value;
        if (sym->flags & SYM_UNKNOWN) {
            ++Redo;
            Redo_why |= REASON_DS_NOT_RESOLVED;
        }
        else
        {
            if (sym->next && sym->next->flags & SYM_UNKNOWN) {
                ++Redo;
                Redo_why |= REASON_DS_NOT_RESOLVED;
            }
            genfill(filler, sym->value, mult);
        }
        FreeSymbolList(sym);
    }
}

void
v_org(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym;

    sym = eval(str, 0);
    Csegment->org = sym->value;

    if (sym->flags & SYM_UNKNOWN)
        Csegment->flags |= SYM_UNKNOWN;
    else
        Csegment->flags &= ~SYM_UNKNOWN;

    if (Csegment->initflags & SYM_UNKNOWN)
    {
        Csegment->initorg = sym->value;
        Csegment->initflags = sym->flags;
    }

    if (sym->next)
    {
        OrgFill = sym->next->value;
        if (sym->next->flags & SYM_UNKNOWN)
            asmerr( ERROR_VALUE_UNDEFINED, true, NULL );
    }

    programlabel();
    FreeSymbolList(sym);
}

void
v_rorg(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym = eval(str, 0);

    Csegment->flags |= SF_RORG;
    if (sym->addrmode != AM_IMP) {
        Csegment->rorg = sym->value;
        if (sym->flags & SYM_UNKNOWN)
            Csegment->rflags |= SYM_UNKNOWN;
        else
            Csegment->rflags &= ~SYM_UNKNOWN;
        if (Csegment->initrflags & SYM_UNKNOWN) {
            Csegment->initrorg = sym->value;
            Csegment->initrflags = sym->flags;
        }
    }
    programlabel();
    FreeSymbolList(sym);
}

void
v_rend(char *str, MNEMONIC *dummy)
{
    programlabel();
    Csegment->flags &= ~SF_RORG;
}

void
v_align(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym = eval(str, 0);
    unsigned char fill = 0;
    unsigned char rorg = Csegment->flags & SF_RORG;

    if (rorg)
        Csegment->rflags |= SF_REF;
    else
        Csegment->flags |= SF_REF;
    if (sym->next) {
        if (sym->next->flags & SYM_UNKNOWN) {
            ++Redo;
            Redo_why |= REASON_ALIGN_NOT_RESOLVED;
        }
        else
        {
            fill = sym->next->value;
        }
    }
    if (rorg) {
        if ((Csegment->rflags | sym->flags) & SYM_UNKNOWN) {
            ++Redo;
            Redo_why |= REASON_ALIGN_RELOCATABLE_ORIGIN_NOT_KNOWN;
        }
        else
        {
            long n = sym->value - (Csegment->rorg % sym->value);
            if (n != sym->value)
                genfill(fill, n, 1);
        }
    }
    else
    {
        if ((Csegment->flags | sym->flags) & SYM_UNKNOWN) {
            ++Redo;
            Redo_why |= REASON_ALIGN_NORMAL_ORIGIN_NOT_KNOWN;
        }
        else
        {
            long n = sym->value - (Csegment->org % sym->value);
            if (n != sym->value)
                genfill(fill, n, 1);
        }
    }
    FreeSymbolList(sym);
    programlabel();
}

void
v_subroutine(char *str, MNEMONIC *dummy)
{
    ++Lastlocalindex;
    Localindex = Lastlocalindex;
    programlabel();
}

void
v_equ(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym = eval(str, 0);
    SYMBOL *lab;


    /*
    * If we encounter a line of the form
    *   . = expr	; or . EQU expr
    * treat it as one of
    *     org expr
    *     rorg expr
    * depending on whether we have a relocatable origin now or not.
    */
    if (strlen(Av[0]) == 1 && (Av[0][0] == '.'
        || (Av[0][0] == '*' && (Av[0][0] = '.') && 1)           /*AD: huh?*/
        )) {
        /* Av[0][0] = '\0'; */
        if (Csegment->flags & SF_RORG)
        {
            v_rorg(str, dummy);
        }
        else
        {
            v_org(str, dummy);
        }
        return;
    }


    lab = findsymbol(Av[0], strlen(Av[0]));
    if (!lab)
        lab = CreateSymbol( Av[0], strlen(Av[0]), true );
    if (!(lab->flags & SYM_UNKNOWN))
    {
        if (sym->flags & SYM_UNKNOWN)
        {
            ++Redo;
            Redo_why |= REASON_EQU_NOT_RESOLVED;
        }
        else
        {
            if (lab->value != sym->value)
            {
                asmerr( ERROR_EQU_VALUE_MISMATCH, false, NULL );
				if ( bTrace )
					printf("INFO: Label '%s' changed from $%04lx to $%04lx\n",
						Av[0], lab->value, sym->value);
                ++Redo;
                Redo_why |= REASON_EQU_VALUE_MISMATCH;
            }
        }
    }

    lab->value = sym->value;
    lab->flags = sym->flags & (SYM_UNKNOWN|SYM_STRING);
    lab->string = sym->string;
    sym->flags &= ~(SYM_STRING|SYM_MACRO);

    /* List the value */
    {
        unsigned long v = lab->value;

        Glen = 0;
        if (v > 0x0000FFFF)
        {
            Gen[Glen++] = v >> 24;
            Gen[Glen++] = v >> 16;
        }
        Gen[Glen++] = v >>  8;
        Gen[Glen++] = v;
    }


    FreeSymbolList(sym);
}

void
v_eqm(char *str, MNEMONIC *dummy)
{
    SYMBOL *lab;
    int len = strlen(Av[0]);

    if ((lab = findsymbol(Av[0], len)) != NULL) {
        if (lab->flags & SYM_STRING)
            free(lab->string);
    }
    else
    {
        lab = CreateSymbol( Av[0], len, true );
    }
    lab->value = 0;
    lab->flags = SYM_STRING | SYM_SET | SYM_MACRO;
    lab->string = strcpy(ckmalloc(strlen(str)+1), str);
}

void
v_echo(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym = eval(str, 0);
    SYMBOL *s;
    char buf[256];

    for (s = sym; s; s = s->next) {
        if (!(s->flags & SYM_UNKNOWN)) {
            if (s->flags & (SYM_MACRO|SYM_STRING))
                sprintf(buf,"%s", s->string);
            else
                sprintf(buf,"$%lx", s->value);
            if (FI_listfile)
                fprintf(FI_listfile, " %s", buf);
            //printf(" %s", buf);
            addmsg(" "); // -FXQ supress output until final pass
            addmsg(buf);
        }
    }
    //puts("");
    addmsg("\n");
    if (FI_listfile)
        putc('\n', FI_listfile);
}

void v_set(char *str, MNEMONIC *dummy)
{

    SYMBOL *sym, *lab;
    char dynamicname[257];
    int i = 0,j;
    int setundefined = 0;

    while (str[i] && str[i] != '"' && str[i] != ' ' && str[i] != ',' )
        i++;

    if(str[i] && str[i] == ',') // is this SET is using the "," eval-concat operator?
    {
        strncpy(dynamicname, str,256);
        if(i<256)
            dynamicname[i]=0;
        dynamicname[256]=0;
        j=strlen(dynamicname);

        // eval-concat argument processing loop...
        while (str[i] && str[i] != '"' && str[i] != ' ')
        {
            if( str[i] == 0 || str[i] == ' ' ) // leave if we've processed all arguments
            {
                break;
            }
            if(str[i+1]=='"') // is this a string constant?
            {
                i=i+2; // advance to string contents
                while (str[i] && str[i] != '"' && str[i] != ' ' && str[i] != ',')
                {
                    dynamicname[j++]=str[i++];
                }
                if (str[i] && str[i]=='"')
                {
                    dynamicname[j]=0;
                    i++;
                    continue;
                }
                else
                {
                    asmerr( ERROR_SYNTAX_ERROR, false, str);
                    continue;
                }
            } // argument was string constant
            else // this argument is a symbol to be evaluated
            {
                int t;
                char tempbuf[257];
                char tempval[257];
                SYMBOL *symarg;
                strncpy(tempbuf,str+i+1,256);
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
                        setundefined++; // ensure the set doesn't actually happen
                    else
                    {
                        snprintf(tempval,256,"%d",(unsigned)symarg->value);
                        strcpy(dynamicname+j,tempval);
                        j=j+strlen(tempval);
                    }
                }
                i++;
                while (str[i] && str[i] != ' ' && str[i] != ',')
                    i++;
            } // argument was symbol

            continue; // process any remaining arguments
        }
        dynamicname[i++] = 0;
        if (setundefined) // not all of the arguments are defined yet, so skip this SET
        {
            return;
        }
        sym = eval(dynamicname,0);
    }
    else // traditional SET behavior
        sym = eval(str, 0);

    lab = findsymbol(Av[0], strlen(Av[0]));
    if (!lab)
        lab = CreateSymbol( Av[0], strlen(Av[0]), true );
    lab->value = sym->value;
    lab->flags = sym->flags & (SYM_UNKNOWN|SYM_STRING);
    lab->string = sym->string;
    sym->flags &= ~(SYM_STRING|SYM_MACRO);
    FreeSymbolList(sym);
}

void v_setstr(char *symstr, MNEMONIC *dummy)
{
    char str[1024];
    snprintf(str,1024,"\"%s\"",symstr);
    v_set(str,dummy);
}

void v_setsym(char *symstr, MNEMONIC *dummy)
{
    char str[1024];
    SYMBOL *sym;
    sym = eval(symstr,0);
    if (sym->flags & SYM_UNKNOWN)
    {
        FreeSymbolList(sym);
        return;
    }

    if (sym->flags & SYM_STRING)
        strcpy(str,sym->string);

    FreeSymbolList(sym);

    v_set(str,dummy);
}

void
v_execmac(char *str, MACRO *mac)
{
    INCFILE *inc;
    STRLIST *base;
    STRLIST **psl, *sl;
    char *s1;

    programlabel();

    if (Mlevel == MAXMACLEVEL) {
        char errMsg[256];
        sprintf(errMsg, " macro [%s] recursion > %d", mac->name, MAXMACLEVEL);
        asmerr( ERROR_RECURSION_TOO_DEEP, true, errMsg );
        return;
    }
    ++Mlevel;
    base = (STRLIST *)ckmalloc(STRLISTSIZE+strlen(str)+1);
    base->next = NULL;
    strcpy(base->buf, str);
    psl = &base->next;
    while (*str && *str != '\n') {
        s1 = str;
        while (*str && *str != '\n' && *str != ',')
            ++str;
        sl = (STRLIST *)ckmalloc(STRLISTSIZE+1+(str-s1));
        sl->next = NULL;
        *psl = sl;
        psl = &sl->next;
        memcpy(sl->buf, s1, (str-s1));
        sl->buf[str-s1] = 0;
        if (*str == ',')
            ++str;
        while (*str == ' ')
            ++str;
    }

    inc = (INCFILE *)zmalloc(sizeof(INCFILE));
    inc->next = pIncfile;
    inc->name = mac->name;
    inc->fi   = pIncfile->fi;	/* garbage */
    inc->lineno = 0;
    inc->flags = INF_MACRO;
    inc->saveidx = Localindex;

    inc->savedolidx = Localdollarindex;

    inc->strlist = mac->strlist;
    inc->args	  = base;
    pIncfile = inc;

    ++Lastlocalindex;
    Localindex = Lastlocalindex;

    ++Lastlocaldollarindex;
    Localdollarindex = Lastlocaldollarindex;

}

void v_end(char *str, MNEMONIC *dummy)
{
    /* Only ENDs current file and any macro calls within it */

    while ( pIncfile->flags & INF_MACRO)
        v_endm(NULL, NULL);

    fseek( pIncfile->fi, 0, SEEK_END);
}

void
v_endm(char *str, MNEMONIC *dummy)
{
    INCFILE *inc = pIncfile;
    STRLIST *args, *an;

    /* programlabel(); contrary to documentation */
    if (inc->flags & INF_MACRO) {
        --Mlevel;
        for (args = inc->args; args; args = an) {
            an = args->next;
            free(args);
        }
        Localindex = inc->saveidx;

        Localdollarindex = inc->savedolidx;

        pIncfile = inc->next;
        free(inc);
        return;
    }
    puts("not within a macro");
}

void
v_mexit(char *str, MNEMONIC *dummy)
{
    v_endm(NULL, NULL);
}

void
v_ifconst(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym;

    programlabel();
    sym = eval(str, 0);
    pushif(sym->flags == 0);
    FreeSymbolList(sym);
}

void
v_ifnconst(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym;

    programlabel();
    sym = eval(str, 0);
    pushif(sym->flags != 0);
    FreeSymbolList(sym);
}

void
v_if(char *str, MNEMONIC *dummy)
{
    SYMBOL *sym;

    if (!Ifstack->xtrue || !Ifstack->acctrue) {
        pushif(0);
        return;
    }
    programlabel();
    sym = eval(str, 0);
    if (sym->flags) {
        ++Redo;
        Redo_why |= REASON_IF_NOT_RESOLVED;
        pushif(0);
        Ifstack->acctrue = 0;

        Redo_if |= 1;

    }
    else
    {
        pushif(!!sym->value);
    }
    FreeSymbolList(sym);
}

void v_else(char *str, MNEMONIC *dummy)
{
    if (Ifstack->acctrue && !(Ifstack->flags & IFF_BASE)) {
        programlabel();
        Ifstack->xtrue = !Ifstack->xtrue;
    }
}

void
v_endif(char *str, MNEMONIC *dummy)
{
    IFSTACK *ifs = Ifstack;

    if (!(ifs->flags & IFF_BASE)) {
        if (ifs->acctrue)
            programlabel();
        if (ifs->file != pIncfile) {
            puts("too many endif's");
        }
        else
        {
            Ifstack = ifs->next;
            free(ifs);
        }
    }
}

void v_repeat(char *str, MNEMONIC *dummy)
{
    REPLOOP *rp;
    SYMBOL *sym;

    if (!Ifstack->xtrue || !Ifstack->acctrue) {
        pushif(0);
        return;
    }
    programlabel();
    sym = eval(str, 0);
    if (sym->value == 0) {
        pushif(0);
        FreeSymbolList(sym);
        return;
    }

#ifdef DAD

    /* Don't allow negative values for REPEAT loops */

    if ( sym->value < 0 )
    {
        pushif( 0 );
        FreeSymbolList( sym );

        asmerr( ERROR_REPEAT_NEGATIVE, false, NULL );
        return;
    }

#endif

    rp = (REPLOOP *)zmalloc(sizeof(REPLOOP));
    rp->next = Reploop;
    rp->file = pIncfile;
    if (pIncfile->flags & INF_MACRO)
        rp->seek = (long)pIncfile->strlist;
    else
        rp->seek = ftell(pIncfile->fi);
    rp->lineno = pIncfile->lineno;
    rp->count = sym->value;
    if ((rp->flags = sym->flags) != 0) {
        ++Redo;
        Redo_why |= REASON_REPEAT_NOT_RESOLVED;
    }
    Reploop = rp;
    FreeSymbolList(sym);
    pushif(1);
}

void
v_repend(char *str, MNEMONIC *dummy)
{
    if (!Ifstack->xtrue || !Ifstack->acctrue) {
        v_endif(NULL,NULL);
        return;
    }
    if (Reploop && Reploop->file == pIncfile) {
        if (Reploop->flags == 0 && --Reploop->count) {
            if (pIncfile->flags & INF_MACRO)
                pIncfile->strlist = (STRLIST *)Reploop->seek;
            else
                fseek(pIncfile->fi,Reploop->seek,0);
            pIncfile->lineno = Reploop->lineno;
        }
        else
        {
            rmnode((void **)&Reploop, sizeof(REPLOOP));
            v_endif(NULL,NULL);
        }
        return;
    }
    puts("no repeat");
}



STRLIST *incdirlist;

void
v_incdir(char *str, MNEMONIC *dummy)
{
    STRLIST **tail;
    char *buf;
    int found = 0;

    buf = getfilename(str);

    for (tail = &incdirlist; *tail; tail = &(*tail)->next) {
        if (strcmp((*tail)->buf, buf) == 0)
            found = 1;
    }

    if (!found) {
        STRLIST *newdir;

        newdir = (STRLIST *)permalloc(STRLISTSIZE + 1 + strlen(buf));
        strcpy(newdir->buf, buf);
        *tail = newdir;
    }

    if (buf != str)
        free(buf);
}

static void
addpart(char *dest, const char *dir, const char *file)
{
#if 0	/* not needed here */
    if (strchr(file, ':')) {
        strcpy(dest, file);
    }
    else
#endif
    {
        int pos;

        strcpy(dest, dir);
        pos = strlen(dest);
        if (pos > 0 && dest[pos-1] != ':' && dest[pos-1] != '/') {
            dest[pos] = '/';
            pos++;
        }
        strcpy(dest + pos, file);
    }
}

FILE *
pfopen(const char *name, const char *mode)
{
    FILE *f;
    STRLIST *incdir;
    char *buf;

    f = fopen(name, mode);
    if (f)
        return f;

    /* Don't use the incdirlist for absolute pathnames */
    if (strchr(name, ':'))
        return NULL;

    buf = zmalloc(512);

    for (incdir = incdirlist; incdir; incdir = incdir->next) {
        addpart(buf, incdir->buf, name);

        f = fopen(buf, mode);
        if (f)
            break;
    }

    free(buf);
    return f;
}


static long Seglen;
static long Seekback;

void
generate(void)
{
    long seekpos;
    static unsigned long org;
    int i;

    if (!Redo)
    {
        if (!(Csegment->flags & SF_BSS))
        {
            for (i = Glen - 1; i >= 0; --i)
                CheckSum += Gen[i];

            if (Fisclear)
            {
                Fisclear = 0;
                if (Csegment->flags & SF_UNKNOWN)
                {
                    ++Redo;
                    Redo_why |= REASON_OBSCURE;
                    return;
                }

                org = Csegment->org;

                if ( F_format < FORMAT_RAW )
                {
                    putc((org & 0xFF), FI_temp);
                    putc(((org >> 8) & 0xFF), FI_temp);

                    if ( F_format == FORMAT_RAS )
                    {
                        Seekback = ftell(FI_temp);
                        Seglen = 0;
                        putc(0, FI_temp);
                        putc(0, FI_temp);
                    }
                }
            }

            switch(F_format)
            {

            default:

                asmerr( ERROR_BAD_FORMAT, true, "Unhandled internal format specifier" );
                break;

            case FORMAT_RAW:
            case FORMAT_DEFAULT:

                if (Csegment->org < org)
                {
                    printf("segment: %s %s  vs current org: %04lx\n",
                        Csegment->name, sftos(Csegment->org, Csegment->flags), org);
                    asmerr( ERROR_ORIGIN_REVERSE_INDEXED, true, NULL );
                    exit(1);
                }

                while (Csegment->org != org)
                {
                    putc(OrgFill, FI_temp);
                    ++org;
                }

                fwrite(Gen, Glen, 1, FI_temp);
                break;

            case FORMAT_RAS:

                if (org != Csegment->org)
                {
                    org = Csegment->org;
                    seekpos = ftell(FI_temp);
                    fseek(FI_temp, Seekback, 0);
                    putc((Seglen & 0xFF), FI_temp);
                    putc(((Seglen >> 8) & 0xFF), FI_temp);
                    fseek(FI_temp, seekpos, 0);
                    putc((org & 0xFF), FI_temp);
                    putc(((org >> 8) & 0xFF), FI_temp);
                    Seekback = ftell(FI_temp);
                    Seglen = 0;
                    putc(0, FI_temp);
                    putc(0, FI_temp);
                }

                fwrite(Gen, Glen, 1, FI_temp);
                Seglen += Glen;
                break;

            }
            org += Glen;
        }
    }

    Csegment->org += Glen;

    if (Csegment->flags & SF_RORG)
        Csegment->rorg += Glen;

    if (Csegment->org > maxFileSize) {
    	char errMsg[128];
    	sprintf(errMsg, "code segment growing larger (%ld) than max. allowed file size (%ld)\n"
    					,Csegment->org, maxFileSize);
    	asmerr( ERROR_RECURSION_TOO_DEEP, true, errMsg );
    	return;
    }
}

void closegenerate(void)
{
    if (!Redo)
    {
        if ( F_format == FORMAT_RAS )
        {
            fseek(FI_temp, Seekback, 0);
            putc((Seglen & 0xFF), FI_temp);
            putc(((Seglen >> 8) & 0xFF), FI_temp);
            fseek(FI_temp, 0L, 2);
        }
    }
}

void
genfill(long fill, long entries, int size)
{
    long bytes = entries;  /*	multiplied later    */
    int i;
    unsigned char c3,c2,c1,c0;

    if (bytes == 0) {
    	// nothing to do
        return;
    }
    if (bytes < 0) {
    	asmerr( ERROR_ORIGIN_REVERSE_INDEXED, true, NULL );
    	return;
    }

    c3 = fill >> 24;
    c2 = fill >> 16;
    c1 = fill >> 8;
    c0 = fill;
    switch(size)
    {
    case 1:
        memset(Gen, c0, sizeof(Gen));
        break;

    case 2:
        bytes <<= 1;
        for (i = 0; i < sizeof(Gen); i += 2)
        {
            if (MsbOrder)
            {
                Gen[i+0] = c1;
                Gen[i+1] = c0;
            }
            else
            {
                Gen[i+0] = c0;
                Gen[i+1] = c1;
            }
        }
        break;

    case 4:
        bytes <<= 2;
        for (i = 0; i < sizeof(Gen); i += 4)
        {
            if (MsbOrder)
            {
                Gen[i+0] = c3;
                Gen[i+1] = c2;
                Gen[i+2] = c1;
                Gen[i+3] = c0;
            }
            else
            {
                Gen[i+0] = c0;
                Gen[i+1] = c1;
                Gen[i+2] = c2;
                Gen[i+3] = c3;
            }
        }
        break;
    }

    for (Glen = sizeof(Gen); bytes > sizeof(Gen); bytes -= sizeof(Gen))
        generate();
    Glen = bytes;
    generate();
}

void
pushif(bool xbool)
{
    IFSTACK *ifs = (IFSTACK *)zmalloc(sizeof(IFSTACK));
    ifs->next = Ifstack;
    ifs->file = pIncfile;
    ifs->flags = 0;
    ifs->xtrue  = xbool;
    ifs->acctrue = Ifstack->acctrue && Ifstack->xtrue;
    Ifstack = ifs;
}
