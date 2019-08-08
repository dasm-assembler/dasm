/*
 * MNEMONICF8.C
 *
 * Fairchild F8 support code for DASM
 * 2004 by Thomas Mathys
 */
#include <ctype.h>
#include "asm.h"


/*
 * special registers. must use numbers from 16 and up,
 * since numbers below 16 are used for scratchpad registers.
 *
 * there is no REG_J, since J is really just scratchpad register 9.
 */
enum REGISTERS {
    REG_A = 16,
    REG_DC0,
    REG_H,
    REG_IS,
    REG_K,
    REG_KU,
    REG_KL,
    REG_PC0,
    REG_PC1,
    REG_Q,
    REG_QU,
    REG_QL,
    REG_W,
    REG_NONE,
};


/*
 * used to print error messages.
 * mnename and opstring are copied into a single error message,
 * which is passed to asmerr.
 *
 * err      : error code (ERROR_xxx constant, passed to asmerr)
 * mnename  : name of the mnemonic
 * opstring : operand string
 * abort    : false = don't abort assembly
 *            true = abort assembly
 */
static void f8err(int err, char *mnename, char *opstring, bool abort) {

    char *buf;

    buf = ckmalloc(strlen(mnename) + strlen(opstring) + 64);
    strcpy(buf, mnename);
    strcat(buf, " ");
    strcat(buf, opstring);
    asmerr(err, abort, buf);
    free(buf);
}


/*
 * emits a one byte opcode.
 */
static void emit_opcode1(unsigned char opcode) {
    Glen = 1;
    Gen[0] = opcode;
    generate();
}


/*
 * emits a two byte opcode
 *
 * byte0 : first byte (lower address)
 * byte1 : second byte (higher address)
 */
static void emit_opcode2(unsigned char byte0, unsigned char byte1) {
    Glen = 2;
    Gen[0] = byte0;
    Gen[1] = byte1;
    generate();
}


/*
 * emits a three byte opcode
 *
 * byte0 : first byte (lowest address)
 * byte1 : second byte (middle address)
 * byte2 : third byte (highest address)
 */
static void emit_opcode3(unsigned char byte0, unsigned char byte1, unsigned char byte2) {
    Glen = 3;
    Gen[0] = byte0;
    Gen[1] = byte1;
    Gen[2] = byte2;
    generate();
}



/*
 * check wether the current program counter is known.
 *
 * result : zero = current program counter is unknown
 *          nonzero = current program counter is known
 */
int isPCKnown(void) {
    unsigned char pcf;
    pcf= (Csegment->flags & SF_RORG) ? Csegment->rflags : Csegment->flags;
    return ((pcf & (SF_UNKNOWN|2)) == 0) ? 1 : 0;
}


/*
 * returns the current program counter
 */
static long getPC(void) {
    return (Csegment->flags & SF_RORG) ? Csegment->rorg : Csegment->org;
}


/*
 * attempts to parse a 32 bit unsigned value from a string.
 *
 * str    : string to parse the value from
 * value  : parsed value is stored here
 *
 * result : zero = ok or syntax error
 *          nonzero = unresolved expression
 */
static int parse_value(char *str, unsigned long *value) {

    SYMBOL *sym;
    int result = 0;

    *value = 0;
    sym = eval(str, 0);

    if (NULL != sym->next || AM_BYTEADR != sym->addrmode) {
        asmerr(ERROR_SYNTAX_ERROR, true, str);
    }
    else if (sym->flags & SYM_UNKNOWN) {
        ++Redo;
        Redo_why |= REASON_MNEMONIC_NOT_RESOLVED;
        result = 1;
    }
    else {
        *value = sym->value;
    }
    FreeSymbolList(sym);

    return result;
}


/*
 * attempts to parse a scratchpad register name.
 * register numbers are parsed as expressions.
 * if an expression is invalid, asmerr is called
 * and the assembly aborted.
 *
 * accepts the following input:
 *
 * - numbers 0..14 (as expressions, numbers 12-14 map to S, I and D)
 * - J  (alias for register  9)
 * - HU (alias for register 10)
 * - HL (alias for register 11)
 * - S and (IS)
 * - I and (IS)+
 * - D and (IS)-
 *
 * str    : string to parse the scratchpad register from
 * reg    : parsed scratchpad register is stored here.
 *          this is the value which will become the lower
 *          nibble of the opcodes.
 *
 * result : zero = ok or syntax error
 *          nonzero = unresolved expression
 */
static int parse_scratchpad_register(char *str, unsigned char *reg) {

    unsigned long regnum;

    /* parse special cases where ISAR is used as index */
    if (!stricmp("s", str) || !stricmp("(is)", str)) {
        *reg = 0x0c;
        return 0;
    }
    if (!stricmp("i", str) || !stricmp("(is)+", str)) {
        *reg = 0x0d;
        return 0;
    }
    if (!stricmp("d", str) || !stricmp("(is)-", str)) {
        *reg = 0x0e;
        return 0;
    }

    /* parse aliases for scratchpad registers */
    if (!stricmp("j", str)) {
        *reg = 0x09;
        return 0;
    }
    if (!stricmp("hu", str)) {
        *reg = 0x0a;
        return 0;
    }
    if (!stricmp("hl", str)) {
        *reg = 0x0b;
        return 0;
    }

    /* parse register number */
    if (parse_value(str, &regnum)) {
        return 1;       /* unresolved expr */
    } else {
        if (regnum > 14) {
            asmerr(ERROR_VALUE_MUST_BE_LT_F, true, str);
        }
        *reg = regnum;
        return 0;
    }
}


/*
 * attempts to parse a special register name from str
 *
 * result : one of the REG_xxx constants (possibly also REG_NONE)
 */
static int parse_special_register(char *str) {

    if (!stricmp("a", str)) {
        return REG_A;
    }
    if (!stricmp("dc0", str) || !stricmp("dc", str) ) {
        return REG_DC0;
    }
    if (!stricmp("h", str)) {
        return REG_H;
    }
    if (!stricmp("is", str)) {
        return REG_IS;
    }
    if (!stricmp("k", str)) {
        return REG_K;
    }
    if (!stricmp("ku", str)) {
        return REG_KU;
    }
    if (!stricmp("kl", str)) {
        return REG_KL;
    }
    if (!stricmp("pc0", str) || !stricmp("p0", str)) {
        return REG_PC0;
    }
    if (!stricmp("pc1", str) || !stricmp("p", str)) {
        return REG_PC1;
    }
    if (!stricmp("q", str)) {
        return REG_Q;
    }
    if (!stricmp("qu", str)) {
        return REG_QU;
    }
    if (!stricmp("ql", str)) {
        return REG_QL;
    }
    if (!stricmp("w", str)) {
        return REG_W;
    }
    else {
        return REG_NONE;
    }
}


static void v_ins_outs(char *str, MNEMONIC *mne) {

    unsigned long operand;

    programlabel();
    parse_value(str, &operand);
    if (operand > 15) {
        f8err(ERROR_VALUE_MUST_BE_LT_10, mne->name, str, false);
    }
    emit_opcode1(mne->opcode[0] | (operand & 15));
}


static void v_sl_sr(char *str, MNEMONIC *mne) {

    unsigned long operand;

    programlabel();

    if (parse_value(str, &operand)) {
        /* unresolved expression, reserve space */
        emit_opcode1(0);
    } else {
        switch (operand) {
            case 1:
                emit_opcode1(mne->opcode[0]);
                break;
            case 4:
                emit_opcode1(mne->opcode[0] + 2);
                break;
            default:
                f8err(ERROR_VALUE_MUST_BE_1_OR_4, mne->name, str, false);
                emit_opcode1(0);
                break;
        }
    }
}


static void v_lis(char *str, MNEMONIC *mne) {

    unsigned long operand;

    programlabel();
    parse_value(str, &operand);
    if (operand > 15) {
        f8err(ERROR_VALUE_MUST_BE_LT_10, mne->name, str, false);
    }
    emit_opcode1(0x70 | (operand & 15));
}


static void v_lisu_lisl(char *str, MNEMONIC *mne) {

    unsigned long operand;

    programlabel();
    parse_value(str, &operand);
    if (operand > 7) {
        f8err(ERROR_VALUE_MUST_BE_LT_8, mne->name, str, false);
    }
    emit_opcode1(mne->opcode[0] | (operand & 7));
}


/*
 * handles opcodes with a scratchpad register operand:
 * as, asd, ds, ns, xs
 */
static void v_sreg_op(char *str, MNEMONIC *mne) {

    unsigned char reg;

    programlabel();
    parse_scratchpad_register(str, &reg);
    emit_opcode1(mne->opcode[0] | reg);
}


static void v_lr(char *str, MNEMONIC *mne) {

    int i;
    int ncommas;
    int cindex;
    char *op1;
    char *op2;
    unsigned char reg_dst;
    unsigned char reg_src;
    int opcode;

    programlabel();

    /* a valid operand string must contain exactly one comma. find it. */
    ncommas = 0;
    cindex = 0;
    for (i=0; str[i]; i++) {
        if (',' == str[i]) {
        ncommas++;
        cindex = i;
        }
    }
    if (1 != ncommas) {
	f8err(ERROR_SYNTAX_ERROR, mne->name, str, false);
        return;
    }

    /* extract operand strings  */
    str[cindex] = 0;
    op1 = str;
    op2 = &str[cindex+1];
    if ( (0 != cindex) && (isspace(str[cindex-1])) ) {
        str[cindex-1] = 0;
    }
    if (isspace(*op2)) {
        op2++;
    }

    /* parse operand strings for register names */
    reg_dst = parse_special_register(op1);
    if (REG_NONE == reg_dst) {
        if (parse_scratchpad_register(op1, &reg_dst)) {
            /* unresolved expression, reserve space */
            emit_opcode1(0);
            return;
        }
    }
    reg_src = parse_special_register(op2);
    if (REG_NONE == reg_src) {
        if (parse_scratchpad_register(op2, &reg_src)) {
            /* unresolved expression, reserve space */
            emit_opcode1(0);
            return;
        }
    }

    /* restore operand string */
    str[cindex] = ',';
    if ( (0 != cindex) && (0 == str[cindex-1])) {
        str[cindex-1] = ' ';
    }

    /* generate opcode */
    opcode = -1;
    switch (reg_dst) {
        case REG_A:     /* lr a,xxx */
            switch (reg_src) {
                case REG_IS: opcode = 0x0a; break;
                case REG_KL: opcode = 0x01; break;
                case REG_KU: opcode = 0x00; break;
                case REG_QL: opcode = 0x03; break;
                case REG_QU: opcode = 0x02; break;
                default:
                    if (reg_src < 15) {
                        opcode = 0x40 | reg_src;
                    }
                    break;
                }
                break;
        case REG_DC0:
            switch (reg_src) {
                case REG_H: opcode = 0x10; break;
                case REG_Q: opcode = 0x0f; break;
            }
            break;
        case REG_H:
            if (REG_DC0 == reg_src) opcode = 0x11;
            break;
        case REG_IS:
            if (REG_A == reg_src) opcode = 0x0b;
            break;
        case REG_K:
            if (REG_PC1 == reg_src) opcode = 0x08;
            break;
        case REG_KL:
            if (REG_A == reg_src) opcode = 0x05;
            break;
        case REG_KU:
            if (REG_A == reg_src) opcode = 0x04;
            break;
        case REG_PC0:
            if (REG_Q == reg_src) opcode = 0x0d;
            break;
        case REG_PC1:
            if (REG_K == reg_src) opcode = 0x09;
            break;
        case REG_Q:
            if (REG_DC0 == reg_src) opcode = 0x0e;
            break;
        case REG_QL:
            if (REG_A == reg_src) opcode = 0x07;
            break;
        case REG_QU:
            if (REG_A == reg_src) opcode = 0x06;
            break;
        case REG_W:
            if (0x09 == reg_src) opcode = 0x1d;
            break;
        default:        /* lr sreg,xxx*/
            if ( (15 > reg_dst) && (REG_A == reg_src) ) {
                /* lr sreg,a */
                opcode = 0x50 | reg_dst;
            }
            else if ( (9 == reg_dst) && (REG_W == reg_src) ) {
                /* special case : lr j,w */
                opcode = 0x1e;
            }
            break;
    }
    if (opcode < 0) {
        f8err(ERROR_ILLEGAL_OPERAND_COMBINATION, mne->name, str, true);
    } else {
        emit_opcode1(opcode);
    }
}


/*
 * generates branch opcodes
 *
 * opcode : opcode of the branch (for instance 0x8f for BR7)
 * str    : operand string
 */
static void generate_branch(unsigned char opcode, char *str) {

    unsigned long target_adr;
    long disp;

    programlabel();

    /* get target address */
    if (parse_value(str, &target_adr)) {
        /* unresolved target address, reserve space */
        emit_opcode2(0, 0);
        return;
    }

    /* calculate displacement */
    if (isPCKnown()) {
        disp = target_adr - getPC() - 1;
        if (disp > 127 || disp < -128) {
            char buf[64];
            sprintf(buf, "%d", (int)disp);
            asmerr(ERROR_BRANCH_OUT_OF_RANGE, false, buf);
        }
    } else {
        /* unknown pc, will be (hopefully) resolved in future passes */
        disp = 0;
    }

    emit_opcode2(opcode, disp & 255);
}


/*
 * handles the following branch mnemonics:
 * bc, bm, bnc, bno, bnz, bp, br, br7, bz
 */
static void v_branch(char *str, MNEMONIC *mne) {
    generate_branch(mne->opcode[0], str);
}


static void v_bf_bt(char *str, MNEMONIC *mne) {

    int ncommas;
    int cindex;
    int i;
    char *op1;
    char *op2;
    unsigned long value;

    /* a valid operand string must contain exactly one comma. find it. */
    ncommas = 0;
    cindex = 0;
    for (i=0; str[i]; i++) {
        if (',' == str[i]) {
        ncommas++;
        cindex = i;
        }
    }
    if (1 != ncommas) {
        f8err(ERROR_SYNTAX_ERROR, mne->name, str, false);
        return;
    }

    /* extract operands */
    str[cindex] = 0;
    op1 = str;
    op2 = &str[cindex+1];

    /* parse first operand*/
    if (parse_value(op1, &value)) {
        /* unresolved expression, reserve space */
        emit_opcode2(0, 0);
        return;
    }

    /* check first operand */
    str[cindex] = ',';		/* restore operand string */
    if ('f' == mne->name[1]) {
        /* bf */
        if (value > 15) {
            f8err(ERROR_VALUE_MUST_BE_LT_10, mne->name, str, false);
            value &= 15;
        }
    } else {
        /* bt */
        if (value > 7) {
            f8err(ERROR_VALUE_MUST_BE_LT_8, mne->name, str, false);
            value &= 7;
        }
    }

    generate_branch(mne->opcode[0] | value, op2);
}


/*
 * handles instructions that take a word operand:
 * dci, jmp, pi
 */
static void v_wordop(char *str, MNEMONIC *mne) {

    unsigned long value;

    programlabel();
    parse_value(str, &value);
    if (value > 0xffff) {
        f8err(ERROR_VALUE_MUST_BE_LT_10000, mne->name, str, false);
    }
    emit_opcode3(mne->opcode[0], (value >> 8) & 0xff, value & 0xff);
}


/*
 * handles instructions that take a byte operand:
 * ai, ci, in, li, ni, oi, out, xi
 */
static void v_byteop(char *str, MNEMONIC *mne) {

    unsigned long value;

    programlabel();
    parse_value(str, &value);
    if (value > 0xff) {
        f8err(ERROR_ADDRESS_MUST_BE_LT_100, mne->name, str, false);
    }
    emit_opcode2(mne->opcode[0], value & 0xff);
}


MNEMONIC MneF8[] = {

    /* ds is an f8 opcode, so we replace the ds directive by res */
    {NULL, v_ds, "res", 0, 0},

    /* add db/dw/dd directives for f8tool compatibility */
    {NULL, v_dc, "db", 0, 0},
    {NULL, v_dc, "dw", 0, 0},
    {NULL, v_dc, "dd", 0, 0},

    /*
     * f8 opcodes
     *
     * some instructions have AF_IMP in der addressflag, although
     * they are handled by own handlers and have explicit operands.
     * this is to keep dasm from clearing the opcode array when
     * adding the hashtable.
     *
     * the only instructions that are handled by v_mnemonic are
     * those with implicit operands.
     *
     * other f8 instructions have register operands, which v_mnemonic
     * can't handle.
     *
     * or they have byte and word operands (values or addresses).
     * these could theoretically be handled by v_mnemonic, but
     * we do it ourselves anyway, since this allows us to have
     * expressions with parentheses as operands.
     */
    {NULL, v_mnemonic, "adc", 0, AF_IMP, {0x8e}},
    {NULL, v_byteop,   "ai" , 0, AF_IMP, {0x24}},
    {NULL, v_mnemonic, "am" , 0, AF_IMP, {0x88}},
    {NULL, v_mnemonic, "amd", 0, AF_IMP, {0x89}},
    {NULL, v_sreg_op,  "as" , 0, AF_IMP, {0xc0}},       /* base opcode */
    {NULL, v_sreg_op,  "asd", 0, AF_IMP, {0xd0}},       /* base opcode */
    {NULL, v_branch,   "bc" , 0, AF_IMP, {0x82}},
    {NULL, v_bf_bt,    "bf" , 0, AF_IMP, {0x90}},       /* base opcode */
    {NULL, v_branch,   "bm" , 0, AF_IMP, {0x91}},
    {NULL, v_branch,   "bnc", 0, AF_IMP, {0x92}},
    {NULL, v_branch,   "bno", 0, AF_IMP, {0x98}},
    {NULL, v_branch,   "bnz", 0, AF_IMP, {0x94}},
    {NULL, v_branch,   "bp" , 0, AF_IMP, {0x81}},
    {NULL, v_branch,   "br" , 0, AF_IMP, {0x90}},
    {NULL, v_branch,   "br7", 0, AF_IMP, {0x8f}},
    {NULL, v_bf_bt,    "bt" , 0, AF_IMP, {0x80}},       /* base opcode */
    {NULL, v_branch,   "bz" , 0, AF_IMP, {0x84}},
    {NULL, v_byteop,   "ci" , 0, AF_IMP, {0x25}},
    {NULL, v_mnemonic, "clr", 0, AF_IMP, {0x70}},
    {NULL, v_mnemonic, "cm" , 0, AF_IMP, {0x8d}},
    {NULL, v_mnemonic, "com", 0, AF_IMP, {0x18}},
    {NULL, v_wordop,   "dci", 0, AF_IMP, {0x2a}},
    {NULL, v_mnemonic, "di" , 0, AF_IMP, {0x1a}},
    {NULL, v_sreg_op,  "ds" , 0, AF_IMP, {0x30}},       /* base opcode */
    {NULL, v_mnemonic, "ei" , 0, AF_IMP, {0x1b}},
    {NULL, v_byteop,   "in" , 0, AF_IMP, {0x26}},
    {NULL, v_mnemonic, "inc", 0, AF_IMP, {0x1f}},
    {NULL, v_ins_outs, "ins", 0, AF_IMP, {0xa0}},       /* base opcode */
    {NULL, v_wordop,   "jmp", 0, AF_IMP, {0x29}},
    {NULL, v_byteop,   "li" , 0, AF_IMP, {0x20}},
    {NULL, v_lis,      "lis", 0, 0},
    {NULL, v_lisu_lisl,"lisl",0, AF_IMP, {0x68}},       /* base opcode */
    {NULL, v_lisu_lisl,"lisu",0, AF_IMP, {0x60}},       /* base opcode */
    {NULL, v_mnemonic, "lm" , 0, AF_IMP, {0x16}},
    {NULL, v_mnemonic, "lnk", 0, AF_IMP, {0x19}},
    {NULL, v_lr,       "lr" , 0, 0},
    {NULL, v_byteop,   "ni" , 0, AF_IMP, {0x21}},
    {NULL, v_mnemonic, "nm" , 0, AF_IMP, {0x8a}},
    {NULL, v_mnemonic, "nop", 0, AF_IMP, {0x2b}},
    {NULL, v_sreg_op,  "ns" , 0, AF_IMP, {0xf0}},       /* base opcode */
    {NULL, v_byteop,   "oi" , 0, AF_IMP, {0x22}},
    {NULL, v_mnemonic, "om" , 0, AF_IMP, {0x8b}},
    {NULL, v_byteop,   "out", 0, AF_IMP, {0x27}},
    {NULL, v_ins_outs, "outs",0, AF_IMP, {0xb0}},       /* base opcode */
    {NULL, v_wordop,   "pi" , 0, AF_IMP, {0x28}},
    {NULL, v_mnemonic, "pk" , 0, AF_IMP, {0x0c}},
    {NULL, v_mnemonic, "pop", 0, AF_IMP, {0x1c}},
    {NULL, v_sl_sr,    "sl" , 0, AF_IMP, {0x13}},       /* base opcode for "sl 1" */
    {NULL, v_sl_sr,    "sr" , 0, AF_IMP, {0x12}},       /* base opcode for "sr 1" */
    {NULL, v_mnemonic, "st" , 0, AF_IMP, {0x17}},
    {NULL, v_mnemonic, "xdc", 0, AF_IMP, {0x2c}},
    {NULL, v_byteop,   "xi" , 0, AF_IMP, {0x23}},
    {NULL, v_mnemonic, "xm" , 0, AF_IMP, {0x8c}},
    {NULL, v_sreg_op,  "xs" , 0, AF_IMP, {0xe0}},       /* base opcode */
    NULL
};
