#!/usr/bin/env python3
"""
dasm.py — DASM-compatible 6502 macro assembler, Python edition.

Multi-pass macro assembler targeting the 6502/65C02.
Command-line compatible with DASM (https://github.com/dasm-assembler/dasm).
Designed to handle Atari 2600 CDFJ+ projects.

Original DASM: Copyright © 1988-2020 Matthew Dillon et al., GPL v2+.
This port carries the same licence.

Usage:
    python dasm.py source.asm -o output.bin [options]

Options:
    -o FILE     output binary  (default: a.out)
    -l FILE     listing file
    -s FILE     symbol dump file
    -I DIR      add include-search directory (repeatable)
    -D SYM[=V]  define symbol (EQU)
    -M SYM=EXP  define symbol (EQM / expression macro)
    -f N        output format: 1=raw (default), 2=raw+checksum, 3=raw
    -v N        verbosity 0-4 (default 0)
    -T N        symbol sort: 0=alpha (default), 1=value, 2=definition order
    -p N        maximum passes (default 10)
    -P N        maximum passes, skip convergence check
    -S          strict mode (error on duplicate macro definitions)
    -R          remove output file on error
"""

from __future__ import annotations

import os
import re
import sys
import struct
import argparse
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Tuple


# ─────────────────────────────────────────────────────────────────────────────
# Version
# ─────────────────────────────────────────────────────────────────────────────

VERSION = "2.20.14-py"

# ─────────────────────────────────────────────────────────────────────────────
# Symbol flags
# ─────────────────────────────────────────────────────────────────────────────

SYM_UNKNOWN = 0x01   # value not yet resolved
SYM_REF     = 0x04   # symbol has been referenced (not just defined)
SYM_STRING  = 0x08   # value is a string, not an integer
SYM_SET     = 0x10   # defined with SET (may be redefined)
SYM_MACRO   = 0x20   # EQM: value is a re-evaluated expression string
SYM_MASREF  = 0x40   # master-reference (EQU)

# ─────────────────────────────────────────────────────────────────────────────
# Segment flags
# ─────────────────────────────────────────────────────────────────────────────

SF_UNKNOWN = 0x01    # org not yet known
SF_BSS     = 0x10    # uninitialised (SEG.U): advance counter, emit nothing
SF_RORG    = 0x20    # rorg active: '*' returns rorg, not org

# ─────────────────────────────────────────────────────────────────────────────
# Addressing modes
# ─────────────────────────────────────────────────────────────────────────────

AM_IMP    =  0   # implied / accumulator
AM_IMM8   =  1   # #byte
AM_IMM16  =  2   # #word
AM_ZP     =  3   # zp
AM_ZPX    =  4   # zp,X
AM_ZPY    =  5   # zp,Y
AM_ABS    =  6   # abs
AM_ABSX   =  7   # abs,X
AM_ABSY   =  8   # abs,Y
AM_REL    =  9   # relative (branches)
AM_INDBX  = 10   # (zp,X)
AM_INDBY  = 11   # (zp),Y
AM_IND    = 12   # (abs)
AM_INDABSX= 13   # (abs,X) — 65C02
AM_INDZP  = 14   # (zp)   — 65C02

# Sentinel modes used only during operand parsing to carry index info
_AM_REGX  = 20   # ,X encountered (mode to be finalised)
_AM_REGY  = 21   # ,Y encountered

# Bytes needed per addressing mode (0 = no operand)
OPSIZE = {
    AM_IMP: 0, AM_IMM8: 1, AM_IMM16: 2,
    AM_ZP: 1, AM_ZPX: 1, AM_ZPY: 1,
    AM_ABS: 2, AM_ABSX: 2, AM_ABSY: 2,
    AM_REL: 1,
    AM_INDBX: 1, AM_INDBY: 1, AM_IND: 2,
    AM_INDABSX: 2, AM_INDZP: 1,
}

# Mode-widening chain: if opcode doesn't support a mode, try the wider one.
CVT = {
    AM_IMM8:  AM_IMM16,
    AM_ZP:    AM_ABS,
    AM_ZPX:   AM_ABSX,
    AM_ZPY:   AM_ABSY,
    AM_IND:   AM_IMM16,   # used for jmp (abs) auto-widen
}

# ─────────────────────────────────────────────────────────────────────────────
# 6502 opcode tables
# ─────────────────────────────────────────────────────────────────────────────
# Format:  mnemonic → {addr_mode: opcode_byte}

_OPCODES_6502: Dict[str, Dict[int, int]] = {
    "adc": {AM_IMM8:0x69, AM_ZP:0x65, AM_ZPX:0x75, AM_ABS:0x6D, AM_ABSX:0x7D, AM_ABSY:0x79, AM_INDBX:0x61, AM_INDBY:0x71},
    "and": {AM_IMM8:0x29, AM_ZP:0x25, AM_ZPX:0x35, AM_ABS:0x2D, AM_ABSX:0x3D, AM_ABSY:0x39, AM_INDBX:0x21, AM_INDBY:0x31},
    "asl": {AM_IMP:0x0A,  AM_ZP:0x06, AM_ZPX:0x16, AM_ABS:0x0E, AM_ABSX:0x1E},
    "bcc": {AM_REL:0x90},
    "bcs": {AM_REL:0xB0},
    "beq": {AM_REL:0xF0},
    "bit": {AM_ZP:0x24, AM_ABS:0x2C},
    "bmi": {AM_REL:0x30},
    "bne": {AM_REL:0xD0},
    "bpl": {AM_REL:0x10},
    "brk": {AM_IMP:0x00},
    "bvc": {AM_REL:0x50},
    "bvs": {AM_REL:0x70},
    "clc": {AM_IMP:0x18},
    "cld": {AM_IMP:0xD8},
    "cli": {AM_IMP:0x58},
    "clv": {AM_IMP:0xB8},
    "cmp": {AM_IMM8:0xC9, AM_ZP:0xC5, AM_ZPX:0xD5, AM_ABS:0xCD, AM_ABSX:0xDD, AM_ABSY:0xD9, AM_INDBX:0xC1, AM_INDBY:0xD1},
    "cpx": {AM_IMM8:0xE0, AM_ZP:0xE4, AM_ABS:0xEC},
    "cpy": {AM_IMM8:0xC0, AM_ZP:0xC4, AM_ABS:0xCC},
    "dec": {AM_ZP:0xC6, AM_ZPX:0xD6, AM_ABS:0xCE, AM_ABSX:0xDE},
    "dex": {AM_IMP:0xCA},
    "dey": {AM_IMP:0x88},
    "eor": {AM_IMM8:0x49, AM_ZP:0x45, AM_ZPX:0x55, AM_ABS:0x4D, AM_ABSX:0x5D, AM_ABSY:0x59, AM_INDBX:0x41, AM_INDBY:0x51},
    "inc": {AM_ZP:0xE6, AM_ZPX:0xF6, AM_ABS:0xEE, AM_ABSX:0xFE},
    "inx": {AM_IMP:0xE8},
    "iny": {AM_IMP:0xC8},
    "jmp": {AM_ABS:0x4C, AM_IND:0x6C},
    "jsr": {AM_ABS:0x20},
    "lda": {AM_IMM8:0xA9, AM_ZP:0xA5, AM_ZPX:0xB5, AM_ABS:0xAD, AM_ABSX:0xBD, AM_ABSY:0xB9, AM_INDBX:0xA1, AM_INDBY:0xB1},
    "ldx": {AM_IMM8:0xA2, AM_ZP:0xA6, AM_ZPY:0xB6, AM_ABS:0xAE, AM_ABSY:0xBE},
    "ldy": {AM_IMM8:0xA0, AM_ZP:0xA4, AM_ZPX:0xB4, AM_ABS:0xAC, AM_ABSX:0xBC},
    "lsr": {AM_IMP:0x4A,  AM_ZP:0x46, AM_ZPX:0x56, AM_ABS:0x4E, AM_ABSX:0x5E},
    "nop": {AM_IMP:0xEA},
    "ora": {AM_IMM8:0x09, AM_ZP:0x05, AM_ZPX:0x15, AM_ABS:0x0D, AM_ABSX:0x1D, AM_ABSY:0x19, AM_INDBX:0x01, AM_INDBY:0x11},
    "pha": {AM_IMP:0x48},
    "php": {AM_IMP:0x08},
    "pla": {AM_IMP:0x68},
    "plp": {AM_IMP:0x28},
    "rol": {AM_IMP:0x2A,  AM_ZP:0x26, AM_ZPX:0x36, AM_ABS:0x2E, AM_ABSX:0x3E},
    "ror": {AM_IMP:0x6A,  AM_ZP:0x66, AM_ZPX:0x76, AM_ABS:0x6E, AM_ABSX:0x7E},
    "rti": {AM_IMP:0x40},
    "rts": {AM_IMP:0x60},
    "sbc": {AM_IMM8:0xE9, AM_ZP:0xE5, AM_ZPX:0xF5, AM_ABS:0xED, AM_ABSX:0xFD, AM_ABSY:0xF9, AM_INDBX:0xE1, AM_INDBY:0xF1},
    "sec": {AM_IMP:0x38},
    "sed": {AM_IMP:0xF8},
    "sei": {AM_IMP:0x78},
    "sta": {AM_ZP:0x85, AM_ZPX:0x95, AM_ABS:0x8D, AM_ABSX:0x9D, AM_ABSY:0x99, AM_INDBX:0x81, AM_INDBY:0x91},
    "stx": {AM_ZP:0x86, AM_ZPY:0x96, AM_ABS:0x8E},
    "sty": {AM_ZP:0x84, AM_ZPX:0x94, AM_ABS:0x8C},
    "tax": {AM_IMP:0xAA},
    "tay": {AM_IMP:0xA8},
    "tsx": {AM_IMP:0xBA},
    "txa": {AM_IMP:0x8A},
    "txs": {AM_IMP:0x9A},
    "tya": {AM_IMP:0x98},
    # Undocumented / illegal opcodes
    "anc": {AM_IMM8:0x0B},
    "ane": {AM_IMM8:0x8B},
    "arr": {AM_IMM8:0x6B},
    "asr": {AM_IMM8:0x4B},
    "dcp": {AM_ZP:0xC7, AM_ZPX:0xD7, AM_ABS:0xCF, AM_ABSX:0xDF, AM_ABSY:0xDB, AM_INDBX:0xC3, AM_INDBY:0xD3},
    "isb": {AM_ZP:0xE7, AM_ZPX:0xF7, AM_ABS:0xEF, AM_ABSX:0xFF, AM_ABSY:0xFB, AM_INDBX:0xE3, AM_INDBY:0xF3},
    "jam": {AM_IMP:0x02},
    "las": {AM_ABSY:0xBB},
    "lax": {AM_ZP:0xA7, AM_ZPY:0xB7, AM_ABS:0xAF, AM_ABSY:0xBF, AM_INDBX:0xA3, AM_INDBY:0xB3},
    "lxa": {AM_IMM8:0xAB},
    "rla": {AM_ZP:0x27, AM_ZPX:0x37, AM_ABS:0x2F, AM_ABSX:0x3F, AM_ABSY:0x3B, AM_INDBX:0x23, AM_INDBY:0x33},
    "rra": {AM_ZP:0x67, AM_ZPX:0x77, AM_ABS:0x6F, AM_ABSX:0x7F, AM_ABSY:0x7B, AM_INDBX:0x63, AM_INDBY:0x73},
    "sax": {AM_ZP:0x87, AM_ZPY:0x97, AM_ABS:0x8F, AM_INDBX:0x83},
    "sbx": {AM_IMM8:0xCB},
    "sha": {AM_ABSY:0x9F, AM_INDBY:0x93},
    "shs": {AM_ABSY:0x9B},
    "shx": {AM_ABSY:0x9E},
    "shy": {AM_ABSX:0x9C},
    "slo": {AM_ZP:0x07, AM_ZPX:0x17, AM_ABS:0x0F, AM_ABSX:0x1F, AM_ABSY:0x1B, AM_INDBX:0x03, AM_INDBY:0x13},
    "sre": {AM_ZP:0x47, AM_ZPX:0x57, AM_ABS:0x4F, AM_ABSX:0x5F, AM_ABSY:0x5B, AM_INDBX:0x43, AM_INDBY:0x53},
}

# 65C02 additions / overrides
_OPCODES_65C02: Dict[str, Dict[int, int]] = {
    **_OPCODES_6502,
    "adc": {**_OPCODES_6502["adc"], AM_INDZP: 0x72},
    "and": {**_OPCODES_6502["and"], AM_INDZP: 0x32},
    "bit": {**_OPCODES_6502["bit"], AM_IMM8: 0x89, AM_ZPX: 0x34, AM_ABSX: 0x3C},
    "bra": {AM_REL: 0x80},
    "cmp": {**_OPCODES_6502["cmp"], AM_INDZP: 0xD2},
    "dec": {**_OPCODES_6502["dec"], AM_IMP: 0x3A},
    "eor": {**_OPCODES_6502["eor"], AM_INDZP: 0x52},
    "inc": {**_OPCODES_6502["inc"], AM_IMP: 0x1A},
    "jmp": {**_OPCODES_6502["jmp"], AM_INDABSX: 0x7C},
    "lda": {**_OPCODES_6502["lda"], AM_INDZP: 0xB2},
    "ora": {**_OPCODES_6502["ora"], AM_INDZP: 0x12},
    "phx": {AM_IMP: 0xDA},
    "phy": {AM_IMP: 0x5A},
    "plx": {AM_IMP: 0xFA},
    "ply": {AM_IMP: 0x7A},
    "sbc": {**_OPCODES_6502["sbc"], AM_INDZP: 0xF2},
    "sta": {**_OPCODES_6502["sta"], AM_INDZP: 0x92},
    "stp": {AM_IMP: 0xDB},
    "stz": {AM_ZP: 0x64, AM_ZPX: 0x74, AM_ABS: 0x9C, AM_ABSX: 0x9E},
    "trb": {AM_ZP: 0x14, AM_ABS: 0x1C},
    "tsb": {AM_ZP: 0x04, AM_ABS: 0x0C},
    "wai": {AM_IMP: 0xCB},
}

# ─────────────────────────────────────────────────────────────────────────────
# Data structures
# ─────────────────────────────────────────────────────────────────────────────

@dataclass
class Symbol:
    name:   str
    value:  int          = 0
    flags:  int          = SYM_UNKNOWN
    string: Optional[str] = None   # for SYM_STRING and SYM_MACRO
    order:  int          = 0       # definition order (for -T2 sort)


@dataclass
class Segment:
    name:   str
    org:    int  = 0           # physical write address
    rorg:   int  = 0           # logical reference address (for *)
    flags:  int  = SF_UNKNOWN  # org flags
    rflags: int  = SF_UNKNOWN  # rorg flags
    # Saved initial state for resetting between passes
    _init_org:    int = 0
    _init_flags:  int = SF_UNKNOWN


@dataclass
class IfFrame:
    active:    bool   # is this branch active (all outer frames also active)?
    this_true: bool   # is this particular IF branch true?
    had_true:  bool   # has any branch been true yet? (for ELSE handling)


@dataclass
class MacroDef:
    name:    str
    params:  List[str]      # named parameters (if declared)
    lines:   List[str]
    defpass: int = 0


@dataclass
class IncFrame:
    filename: str
    lines:    List[str]
    pos:      int  = 0
    lineno:   int  = 0
    is_macro: bool = False


# ─────────────────────────────────────────────────────────────────────────────
# Expression evaluator
# ─────────────────────────────────────────────────────────────────────────────

class ExprResult:
    """
    Result of evaluating an expression.

    `unknown` is True when the value could not be determined this pass
    (forward reference to an unresolved symbol).

    `string` holds a string value for SYM_STRING results.
    `addrmode` is set by operand parsing (see Evaluator.eval_operand).
    """
    __slots__ = ("value", "unknown", "string", "addrmode")

    def __init__(self, value: int = 0, unknown: bool = False,
                 string: Optional[str] = None, addrmode: int = AM_IMP):
        self.value    = value
        self.unknown  = unknown
        self.string   = string
        self.addrmode = addrmode


class Evaluator:
    """
    Recursive-descent / Pratt expression parser matching DASM's operator set.

    Operator precedence (high→low):
        unary  -  ~  !  <  >          (priority 128, right-associative)
        *  /  %                        (20)
        +  -                           (19)
        >>  <<                         (18)
        >  >=  <  <=                   (17)
        ==  !=                         (16)
        &                              (15)
        ^                              (14)
        |                              (13)
        &&                             (12)
        ||                             (11)
        ?                              (10)

    Literals:
        $hex   %binary   0octal   decimal
        'c     "string"
        [expr]d  →  decimal string conversion

    Addressing modes are detected when `want_mode=True` (operand context):
        #value      →  AM_IMM8
        (zp,X)      →  AM_INDBX
        (zp),Y      →  AM_INDBY
        (abs)       →  AM_IND
        (abs,X)     →  AM_INDABSX
        (zp)        →  AM_INDZP  (65C02)
        value,X     →  _AM_REGX  (resolved to ZPX/ABSX later)
        value,Y     →  _AM_REGY
    """

    def __init__(self, asm: "Assembler"):
        self._asm   = asm
        self._text  = ""
        self._pos   = 0
        self._probe = False   # when True: no symbol creation, no redo increments

    # ── Public API ────────────────────────────────────────────────────────────

    def eval(self, text: str) -> ExprResult:
        """Evaluate a plain expression (no addressing mode detection)."""
        self._text = text.strip()
        self._pos  = 0
        result = self._pratt(0)
        return result

    def probe(self, text: str) -> ExprResult:
        """Like eval() but suppresses all side effects: no symbol creation,
        no redo counter increments, no _redo_reasons entries.
        Used by IFCONST/IFNCONST where 'unknown' just means 'not constant yet'."""
        old, self._probe = self._probe, True
        try:
            return self.eval(text)
        finally:
            self._probe = old

    def eval_operand(self, text: str) -> ExprResult:
        """
        Evaluate an instruction operand, detecting the addressing mode.
        Returns an ExprResult with `.addrmode` set.
        """
        self._text = text.strip()
        self._pos  = 0
        return self._parse_operand()

    # ── Operand mode parser ───────────────────────────────────────────────────

    def _parse_operand(self) -> ExprResult:
        self._skip()

        if self._pos >= len(self._text):
            return ExprResult(0, False, addrmode=AM_IMP)

        ch = self._text[self._pos]

        # Immediate: #expr
        if ch == '#':
            self._pos += 1
            r = self._pratt(0)
            r.addrmode = AM_IMM8
            return r

        # Indirect: (...)
        if ch == '(':
            self._pos += 1
            inner = self._pratt(0)
            self._skip()

            if self._skip_peek() == ',':
                # (zp,X) or (abs,X)
                self._pos += 1
                self._skip()
                reg = self._consume_reg()
                if reg == 'x':
                    self._skip()
                    if self._skip_peek() == ')':
                        self._pos += 1
                    inner.addrmode = AM_INDBX
                    return inner
                inner.addrmode = AM_IND
                return inner

            if self._skip_peek() == ')':
                self._pos += 1
                self._skip()
                if self._skip_peek() == ',':
                    self._pos += 1
                    self._skip()
                    reg = self._consume_reg()
                    if reg == 'y':
                        inner.addrmode = AM_INDBY
                        return inner
                    inner.addrmode = AM_IND
                    return inner
                # Just (abs) or (zp) — defer to 65C02 check in assembler
                inner.addrmode = AM_IND
                return inner

            inner.addrmode = AM_IND
            return inner

        # Everything else: value optionally followed by ,X or ,Y
        r = self._pratt(0)
        self._skip()

        if self._skip_peek() == ',':
            self._pos += 1
            self._skip()
            reg = self._consume_reg()
            r.addrmode = _AM_REGX if reg == 'x' else _AM_REGY
        else:
            # Default mode; assembler will narrow to ZP if value fits
            r.addrmode = AM_ABS

        return r

    # ── Pratt parser ──────────────────────────────────────────────────────────

    def _pratt(self, min_prec: int) -> ExprResult:
        left = self._unary()
        while True:
            self._skip()
            op, prec = self._peek_binop()
            if op is None or prec < min_prec:
                break
            self._pos += len(op)
            # Left-associative: next level is prec+1
            right = self._pratt(prec + 1)
            left  = self._apply_binop(op, left, right)
        return left

    def _peek_binop(self) -> Tuple[Optional[str], int]:
        if self._pos >= len(self._text):
            return None, 0
        t = self._text
        p = self._pos
        c  = t[p]
        c2 = t[p+1] if p+1 < len(t) else ''

        if c == '*': return '*',  20
        if c == '/': return '/',  20
        if c == '%': return '%',  20
        if c == '+': return '+',  19
        if c == '-': return '-',  19
        if c == '>' and c2 == '>': return '>>', 18
        if c == '<' and c2 == '<': return '<<', 18
        if c == '>' and c2 == '=': return '>=', 17
        if c == '<' and c2 == '=': return '<=', 17
        if c == '>' and c2 != '>': return '>',  17
        if c == '<' and c2 not in ('<', '='): return '<', 17
        if c == '=' and c2 == '=': return '==', 16
        if c == '!' and c2 == '=': return '!=', 16
        if c == '&' and c2 == '&': return '&&', 12
        if c == '&' and c2 != '&': return '&',  15
        if c == '^': return '^',  14
        if c == '|' and c2 == '|': return '||', 11
        if c == '|' and c2 != '|': return '|',  13
        if c == '?': return '?',  10
        return None, 0

    # Dispatch table for binary operators — avoids a 20-branch if-elif chain.
    # Each entry: op → lambda(lv, rv) → int
    # Division-by-zero and error cases are handled before the table lookup.
    _BINOPS: Dict[str, object] = {
        '*':  lambda l, r: l * r,
        '+':  lambda l, r: l + r,
        '-':  lambda l, r: l - r,
        '>>': lambda l, r: (l >> r) if r >= 0 else 0,
        '<<': lambda l, r: (l << r) if r >= 0 else 0,
        '>':  lambda l, r: int(l > r),
        '>=': lambda l, r: int(l >= r),
        '<':  lambda l, r: int(l < r),
        '<=': lambda l, r: int(l <= r),
        '==': lambda l, r: int(l == r),
        '!=': lambda l, r: int(l != r),
        '&':  lambda l, r: l & r,
        '^':  lambda l, r: l ^ r,
        '|':  lambda l, r: l | r,
        '&&': lambda l, r: int(bool(l) and bool(r)),
        '||': lambda l, r: int(bool(l) or  bool(r)),
        '?':  lambda l, r: r if l else 0,
    }

    def _apply_binop(self, op: str, L: ExprResult, R: ExprResult) -> ExprResult:
        unk = L.unknown or R.unknown

        if unk:
            # Short-circuit && / || even with unknown operands
            if op == '&&' and not L.unknown and not L.value:
                return ExprResult(0, False)
            if op == '||' and not L.unknown and L.value:
                return ExprResult(1, False)
            return ExprResult(0, True)

        lv, rv = L.value, R.value

        # Division handled separately to allow error reporting
        if op == '/':
            if not rv:
                self._error("Division by zero")
                return ExprResult(0, False)
            v = int(lv / rv)
        elif op == '%':
            v = lv % rv if rv else 0
        else:
            fn = self._BINOPS.get(op)
            v  = fn(lv, rv) if fn else 0

        # Clamp to signed 32-bit
        v = int(v) & 0xFFFFFFFF
        if v >= 0x80000000:
            v -= 0x100000000
        return ExprResult(v, False)

    # ── Unary / atom ──────────────────────────────────────────────────────────

    def _unary(self) -> ExprResult:
        self._skip()
        if self._pos >= len(self._text):
            return ExprResult(0, True)

        ch = self._text[self._pos]

        if ch == '-':
            self._pos += 1
            r = self._pratt(128)
            return ExprResult(-r.value & 0xFFFFFFFF, r.unknown)

        if ch == '~':
            self._pos += 1
            r = self._pratt(128)
            return ExprResult(~r.value & 0xFFFFFFFF, r.unknown)

        if ch == '!':
            self._pos += 1
            r = self._pratt(128)
            return ExprResult(int(not r.value) if not r.unknown else 0, r.unknown)

        if ch == '<':   # take LSB
            self._pos += 1
            r = self._pratt(128)
            return ExprResult(r.value & 0xFF, r.unknown)

        if ch == '>':   # take MSB
            self._pos += 1
            r = self._pratt(128)
            return ExprResult((r.value >> 8) & 0xFF, r.unknown)

        if ch == '(':
            self._pos += 1
            r = self._pratt(0)
            self._skip()
            if self._skip_peek() == ')':
                self._pos += 1
            # DASM supports both [expr]d and (expr)d for decimal string conversion
            if self._skip_peek() == 'd':
                self._pos += 1
                s = str(r.value) if not r.unknown else "????"
                return ExprResult(r.value, r.unknown, string=s)
            return r

        if ch == '[':
            # [expr]d  — evaluate expression, optionally convert to decimal string
            self._pos += 1
            r = self._pratt(0)
            self._skip()
            if self._skip_peek() == ']':
                self._pos += 1
            # 'd' suffix → decimal string result
            if self._skip_peek() == 'd':
                self._pos += 1
                s = str(r.value) if not r.unknown else "????"
                return ExprResult(r.value, r.unknown, string=s)
            return r

        return self._atom()

    def _atom(self) -> ExprResult:
        self._skip()
        if self._pos >= len(self._text):
            return ExprResult(0, True)

        ch = self._text[self._pos]

        # Hex: $ffff or 0x...
        if ch == '$':
            self._pos += 1
            return self._read_int(16, '0123456789abcdefABCDEF')

        if ch == '0' and self._pos+1 < len(self._text) and self._text[self._pos+1] in ('x','X'):
            self._pos += 2
            return self._read_int(16, '0123456789abcdefABCDEF')

        # Binary: %...
        if ch == '%':
            self._pos += 1
            return self._read_int(2, '01')

        # Octal: 0... (only if next char is digit)
        if ch == '0' and self._pos+1 < len(self._text) and self._text[self._pos+1].isdigit():
            return self._read_int(8, '01234567')

        # Decimal
        if ch.isdigit():
            return self._read_int(10, '0123456789')

        # Character literal 'x
        if ch == "'":
            self._pos += 1
            if self._pos < len(self._text):
                v = ord(self._text[self._pos])
                self._pos += 1
                return ExprResult(v, False)
            return ExprResult(0, True)

        # String literal "..."
        if ch == '"':
            self._pos += 1
            start = self._pos
            while self._pos < len(self._text) and self._text[self._pos] != '"':
                self._pos += 1
            s = self._text[start:self._pos]
            if self._pos < len(self._text):
                self._pos += 1  # consume closing "
            # Integer value = bytes packed MSB-first (DASM behaviour)
            v = 0
            for c in s:
                v = (v << 8) | ord(c)
            return ExprResult(v, False, string=s)

        # Symbol or current-PC
        if ch in ('*', '.') or ch.isalpha() or ch in ('_', '@'):
            return self._symbol()

        # Unknown character
        self._error(f"Unexpected character '{ch}' in expression")
        self._pos += 1
        return ExprResult(0, True)

    def _read_int(self, base: int, digits: str) -> ExprResult:
        start = self._pos
        while self._pos < len(self._text) and self._text[self._pos] in digits:
            self._pos += 1
        s = self._text[start:self._pos]
        if not s:
            return ExprResult(0, True)
        return ExprResult(int(s, base), False)

    def _symbol(self) -> ExprResult:
        """Look up a symbol (or . / * for current PC)."""
        # Handle '*' (current PC) — must consume the character before reading name
        if self._pos < len(self._text) and self._text[self._pos] == '*':
            self._pos += 1
            seg = self._asm.current_seg
            if seg.flags & SF_RORG:
                return ExprResult(seg.rorg, bool(seg.rflags & SF_UNKNOWN))
            return ExprResult(seg.org, bool(seg.flags & SF_UNKNOWN))

        start = self._pos
        # Allow: letters, digits, _, ., @, $ (dollar at end for local labels)
        while self._pos < len(self._text) and (
            self._text[self._pos].isalnum()
            or self._text[self._pos] in ('_', '.', '@', '$')
        ):
            self._pos += 1
        name = self._text[start:self._pos]

        # Current PC (bare '.')
        if name == '.':
            seg = self._asm.current_seg
            if seg.flags & SF_RORG:
                return ExprResult(seg.rorg, bool(seg.rflags & SF_UNKNOWN))
            return ExprResult(seg.org, bool(seg.flags & SF_UNKNOWN))

        # Resolve local label names
        name = self._asm._qualify_local(name)

        sym = self._asm.symbols.get(name)
        if sym is None:
            if not self._probe:
                # Forward reference: create placeholder
                sym = Symbol(name=name, value=0, flags=SYM_UNKNOWN | SYM_REF,
                             order=len(self._asm.symbols))
                self._asm.symbols[name] = sym
                self._asm._redo_forward += 1
                frame = self._asm._stack[-1] if self._asm._stack else None
                loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
                self._asm._redo_reasons.append(
                    f"{loc}forward reference to undefined symbol '{self._asm._sym_display_name(name)}'"
                )
            return ExprResult(0, True)

        if not self._probe:
            sym.flags |= SYM_REF

        if sym.flags & SYM_UNKNOWN:
            if not self._probe:
                self._asm._redo_forward += 1
                frame = self._asm._stack[-1] if self._asm._stack else None
                loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
                self._asm._redo_reasons.append(
                    f"{loc}symbol '{self._asm._sym_display_name(name)}' still unresolved"
                )
            return ExprResult(0, True)

        if sym.flags & SYM_MACRO:
            # EQM: re-evaluate the stored expression
            saved_text, saved_pos = self._text, self._pos
            result = self.eval(sym.string or "0")
            self._text, self._pos = saved_text, saved_pos
            return result

        if sym.flags & SYM_STRING:
            return ExprResult(sym.value, False, string=sym.string)

        return ExprResult(sym.value, False)

    # ── Helpers ───────────────────────────────────────────────────────────────

    def _skip(self):
        while self._pos < len(self._text) and self._text[self._pos] == ' ':
            self._pos += 1

    def _skip_peek(self) -> str:
        """Skip spaces then return (but do not consume) the next character."""
        while self._pos < len(self._text) and self._text[self._pos] == ' ':
            self._pos += 1
        return self._text[self._pos] if self._pos < len(self._text) else ''

    def _consume_reg(self) -> str:
        """Read a register name (x or y), lowercase."""
        self._skip()
        if self._pos < len(self._text):
            reg = self._text[self._pos].lower()
            self._pos += 1
            return reg
        return ''

    def _error(self, msg: str) -> int:
        self._asm._error(msg)
        return 0


# ─────────────────────────────────────────────────────────────────────────────
# Assembler
# ─────────────────────────────────────────────────────────────────────────────

FILL_BYTE = 0xFF   # byte used to fill gaps between org regions

class Assembler:
    """
    Multi-pass 6502 macro assembler.

    Passes continue until either:
      (a) no symbols changed / no forward references remain (stable), or
      (b) a pass makes no progress (infinite loop → error).
    """

    # Directives where the label names the symbol being defined, not a PC label.
    # _define_label must NOT be called before dispatching these.
    _NO_PC_LABEL = frozenset({
        'equ', '=', 'set', 'eqm', 'mac', 'macro',
        'endm', 'if', 'ifconst', 'ifnconst',
        'ifdef', 'ifndef', 'else', 'endif', 'eif',
    })

    # Directives processed even inside a false IF block (to track nesting).
    _ALWAYS_DIRECTIVES = frozenset({
        'if', 'ifdef', 'ifndef', 'ifconst', 'ifnconst',
        'else', 'endif', 'eif',
        'macro', 'mac',   # track opens even when inactive
        'endm',
    })

    def __init__(self, opts: argparse.Namespace):
        self.opts = opts
        self.ev   = Evaluator(self)

        # Tables
        self.symbols:  Dict[str, Symbol]   = {}
        self.macros:   Dict[str, MacroDef] = {}
        self.opcodes:  Dict[str, Dict[int, int]] = {}
        self.inc_dirs: List[str] = list(opts.include_dirs)

        # Segments
        self._init_segment = Segment(name="__code__", flags=SF_UNKNOWN, rflags=SF_UNKNOWN)
        self.segments:    List[Segment] = [self._init_segment]
        self.current_seg: Segment       = self._init_segment

        # Output buffer: keyed by physical address
        self._output: Dict[int, int] = {}

        # Pass state
        self.pass_num      = 0
        self._redo_forward = 0   # count of unresolved forward refs this pass
        self._redo_phase   = 0   # count of phase errors
        self._redo_other   = 0   # other reasons to redo
        self._redo_reasons: List[str] = []  # human-readable redo causes
        self._errors:   List[str] = []
        self._messages: List[str] = []
        self._fatal = False

        # Include / macro stack
        self._stack: List[IncFrame] = []

        # Conditional assembly
        self._if_stack: List[IfFrame] = [IfFrame(active=True, this_true=True, had_true=True)]

        # Local label indexes
        self._local_idx:        int = 0
        self._local_dollar_idx: int = 0
        self._local_next:       int = 0
        self._local_dollar_next:int = 0

        # Multi-line comment tracking
        self._in_ml_comment = False

        # Macro definition tracking (for balance checking)
        self._macro_opens  = 0
        self._macro_closes = 0

        # Symbol order counter
        self._sym_order = 0

        # Directive dispatch table — built once, not per line.
        self._directives: Dict[str, object] = {
            'processor': self._d_processor,
            'org':       self._d_org,
            'rorg':      self._d_rorg,
            'rend':      self._d_rend,
            'seg':       self._d_seg,
            'seg.u':     lambda a, l: self._d_seg(a, l, bss=True),
            'ds':        self._d_ds,
            'dc':        self._d_dc,
            'byte':      lambda a, l: self._d_dc(a, l, size=1),
            'word':      lambda a, l: self._d_dc(a, l, size=2),
            'long':      lambda a, l: self._d_dc(a, l, size=4),
            'dv':        self._d_dc,
            'hex':       self._d_hex,
            'equ':       self._d_equ,
            '=':         self._d_equ,
            'set':       self._d_set,
            'eqm':       self._d_eqm,
            'include':   self._d_include,
            'incbin':    self._d_incbin,
            'incdir':    self._d_incdir,
            'if':        self._d_if,
            'ifdef':     self._d_ifdef,
            'ifconst':   self._d_ifconst,
            'ifndef':    self._d_ifndef,
            'ifnconst':  self._d_ifnconst,
            'else':      self._d_else,
            'endif':     self._d_endif,
            'eif':       self._d_endif,
            'mac':       self._d_macro,
            'macro':     self._d_macro,
            'endm':      self._d_endm,
            'mexit':     self._d_mexit,
            'repeat':    self._d_repeat,
            'repend':    self._d_repend,
            'align':     self._d_align,
            'subroutine':self._d_subroutine,
            'echo':      self._d_echo,
            'err':       self._d_err,
            'end':       self._d_end,
            'list':      lambda a, l: None,
            'trace':     lambda a, l: None,
            'format':    lambda a, l: None,
        }

        # Process -D defines
        for d in opts.defines:
            self._cmd_define(d, eqm=False)
        for d in opts.eqm_defines:
            self._cmd_define(d, eqm=True)

    # ── Properties ────────────────────────────────────────────────────────────

    @property
    def pc(self) -> int:
        """Current program counter: rorg if active, else org."""
        seg = self.current_seg
        return seg.rorg if (seg.flags & SF_RORG) else seg.org

    @property
    def pc_unknown(self) -> bool:
        seg = self.current_seg
        if seg.flags & SF_RORG:
            return bool(seg.rflags & SF_UNKNOWN)
        return bool(seg.flags & SF_UNKNOWN)

    # ── Assembly entry point ──────────────────────────────────────────────────

    def assemble(self, source: str) -> int:
        """Run the multi-pass assembler.  Returns 0 on success."""
        max_passes = self.opts.max_passes
        prev_redo  = -1

        for self.pass_num in range(1, max_passes + 1):
            self._begin_pass()

            if self.opts.verbose:
                print(f"\n--- Pass {self.pass_num}")

            self._push_file(source)
            self._run()

            total_redo = self._redo_forward + self._redo_phase + self._redo_other

            if self.opts.verbose >= 2 and total_redo:
                print(
                    f"  redo: forward={self._redo_forward} "
                    f"phase={self._redo_phase} other={self._redo_other}",
                    file=sys.stderr,
                )

            # Write output after every pass (DASM does this)
            rc = self._write_output()
            if rc:
                return rc

            if self._in_ml_comment:
                self._print_errors()
                print("error: Unterminated /* comment", file=sys.stderr)
                return 41

            if self._fatal:
                self._print_errors()
                return 5

            if total_redo == 0:
                break   # Stable — done!

            # Detect infinite loop (no progress)
            if not self.opts.do_all_passes and total_redo == prev_redo:
                self._report_stuck()
                return 3

            prev_redo = total_redo
        else:
            print(f"error: Too many passes ({max_passes}).", file=sys.stderr)
            return 4

        # Check macro balance
        if self._macro_opens != self._macro_closes:
            print(
                f"error: Unbalanced MACRO/ENDM: "
                f"{self._macro_opens} opened, {self._macro_closes} closed",
                file=sys.stderr,
            )
            return 40

        self._print_errors()
        if self._errors:
            return 1

        # Print accumulated echo/info messages
        for m in self._messages:
            print(m)

        if self.opts.verbose:
            print(f"\nComplete. (pass {self.pass_num})")
        else:
            print("Complete. (0)")

        return 0

    def _begin_pass(self):
        """Reset per-pass state."""
        self._redo_forward  = 0
        self._redo_phase    = 0
        self._redo_other    = 0
        self._redo_reasons  = []
        self._errors        = []
        self._messages      = []
        self._fatal         = False
        self._in_ml_comment = False
        self._macro_opens   = 0
        self._macro_closes  = 0
        self._output        = {}
        self._stack         = []
        self._if_stack      = [IfFrame(active=True, this_true=True, had_true=True)]
        self._local_idx         = 0
        self._local_next        = 0
        self._local_dollar_idx  = 0
        self._local_dollar_next = 0

        # Symbols KEEP their values from the previous pass.
        # We only clear the REF flag (used for unused-symbol reporting).
        # SYM_UNKNOWN is cleared on definition; it is NOT re-set here —
        # that would cause every backward reference to look like a forward
        # reference and prevent convergence.
        for sym in self.symbols.values():
            sym.flags &= ~SYM_REF

        # Reset segment physical counters (keep names, BSS flag etc.)
        for seg in self.segments:
            seg.org    = seg._init_org
            seg.flags  = seg._init_flags | SF_UNKNOWN
            seg.rorg   = 0
            seg.rflags = SF_UNKNOWN

        self.current_seg = self.segments[0]

    # ── Main processing loop ──────────────────────────────────────────────────

    def _run(self):
        while self._stack:
            frame = self._stack[-1]
            if frame.pos >= len(frame.lines):
                self._stack.pop()
                continue
            raw = frame.lines[frame.pos]
            frame.pos    += 1
            frame.lineno += 1
            self._process_line(raw)
            if self._fatal:
                return

    def _process_line(self, raw: str):
        line, _ = self._strip_comment(raw)
        label, opcode, operands = self._tokenise(line)

        if not opcode and not label:
            return

        op_lc = opcode.lower()

        # Determine whether this block is active
        active = self._if_stack[-1].active

        if not active and op_lc not in self._ALWAYS_DIRECTIVES:
            return   # skip inactive lines (except label definitions for IF tracking)

        self._dispatch(label, op_lc, operands, opcode)

    def _dispatch(self, label: str, op_lc: str, operands: str, op_orig: str):
        """Route a parsed line to the right handler."""

        # Strip leading dot from directives (.byte → byte, .word → word)
        base_op = op_lc.lstrip('.')

        # Parse DASM-style extension suffix: mne.XX
        # Explicitly skip 'seg.u' — it's a compound directive, not a sized mnemonic.
        #
        # Two kinds of extension:
        #   forced_mode  — two-char (e.g. .wx, .bx): full AM_* override, ignores operand register
        #   forced_size  — one-char (e.g. .w, .b):   width-only; operand's ,X/,Y still applies
        forced_mode: Optional[int] = None   # AM_* override for opcode addressing (two-char ext)
        forced_size: Optional[int] = None   # 1=ZP 2=ABS width hint (one-char ext)
        dc_size:     Optional[int] = None   # byte count for dc directives
        swap_endian: bool          = False
        if '.' in base_op and base_op != 'seg.u':
            base, _, ext_raw = base_op.partition('.')
            # Multi-word ext aliases (dc.byte, dc.word, dc.long) — handle first
            _word_aliases = {'byte': (1,), 'word': (2,), 'long': (4,)}
            if ext_raw in _word_aliases:
                dc_size = _word_aliases[ext_raw][0]
                base_op = base
            elif ext_raw:
                c0 = ext_raw[0].lower()
                c1 = ext_raw[1].lower() if len(ext_raw) > 1 else ''
                base_op = base
                if c0 == 's':                        # .s  swap endian (dc only)
                    dc_size = 2; swap_endian = True
                elif c0 in ('i', '0'):               # .i / .0  implied
                    forced_mode = {
                        'x': AM_ABSX,
                        'y': AM_ABSY,
                    }.get(c1, AM_IMP)
                elif c0 in ('b', 'z', 'd'):          # .b / .z / .d  zero-page
                    if c1 in ('x', 'y'):             # .bx / .by — full override
                        forced_mode = AM_ZPX if c1 == 'x' else AM_ZPY
                    else:                            # .b alone — width only
                        forced_size = 1
                    dc_size = 1
                elif c0 in ('w', 'a', 'e'):          # .w / .a / .e  absolute
                    if c1 in ('x', 'y'):             # .wx / .wy — full override
                        forced_mode = AM_ABSX if c1 == 'x' else AM_ABSY
                    else:                            # .w alone — width only
                        forced_size = 2
                    dc_size = 2
                elif c0 == 'l':                      # .l  long
                    dc_size = 4
                elif c0 == 'r':                      # .r  relative
                    forced_mode = AM_REL
                elif c0 == 'u':                      # .u  BSS / uninitialised
                    pass  # handled by seg.u; ignore on opcodes
                else:
                    pass  # Unknown extension — base_op already stripped; falls through to error

        if dc_size is not None and base_op in {'dc', 'dv', 'byte', 'word', 'long'}:
            # dc.b / dc.w / dc.l with explicit size
            self._define_label(label)
            self._d_dc(operands, label, size=dc_size, swap_endian=swap_endian)
            return

        if base_op in self._directives:
            # EQU/SET/EQM/MACRO: the label is the symbol being *defined* by the
            # directive, not a PC label — don't assign PC to it first.
            if base_op not in self._NO_PC_LABEL:
                self._define_label(label)
            self._directives[base_op](operands, label)
            return

        # Mnemonic?
        if base_op in self.opcodes:
            self._define_label(label)
            self._emit_instruction(base_op, operands, forced_mode, forced_size)
            return

        # Macro call?
        if base_op in self.macros:
            self._define_label(label)
            self._expand_macro(base_op, operands)
            return

        # If there's only a label (no opcode) that's fine
        if not base_op:
            self._define_label(label)
            return

        if self._if_stack[-1].active:
            self._error(f"Unknown mnemonic or directive: '{op_orig}'")

    # ── Directives ────────────────────────────────────────────────────────────

    def _d_processor(self, args: str, _label: str):
        proc = args.strip().lower()
        if proc in ('6502',):
            self.opcodes = dict(_OPCODES_6502)
        elif proc in ('65c02', '65816'):
            self.opcodes = dict(_OPCODES_65C02)
        else:
            self._error(f"Unsupported processor '{args.strip()}'")

    def _d_org(self, args: str, _label: str):
        """ORG addr — set the physical write address."""
        r = self.ev.eval(args.strip())
        seg = self.current_seg
        if r.unknown:
            seg.flags |= SF_UNKNOWN
            self._redo_other += 1
            return
        val = r.value

        # Guard against reversing origin within a pass (usually a bug)
        if not (seg.flags & SF_UNKNOWN) and val < seg.org:
            msg = f"ORG reversed in '{seg.name}': was {seg.org:#06x}, now {val:#06x}"
            self._error(msg)
            self._redo_phase += 1
            frame = self._stack[-1] if self._stack else None
            loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
            self._redo_reasons.append(f"{loc}{msg}")

        seg.org   = val
        seg.flags = (seg.flags & ~SF_UNKNOWN) | (seg.flags & SF_BSS)


    def _d_rorg(self, args: str, _label: str):
        """RORG addr — set the logical reference address (changes what * returns)."""
        r = self.ev.eval(args.strip())
        seg = self.current_seg
        seg.flags |= SF_RORG
        if r.unknown:
            seg.rflags |= SF_UNKNOWN
            self._redo_other += 1
            return
        seg.rorg   = r.value
        seg.rflags = seg.rflags & ~SF_UNKNOWN

    def _d_rend(self, _args: str, _label: str):
        """REND — end the rorg section; * reverts to org."""
        self.current_seg.flags &= ~SF_RORG

    def _d_seg(self, args: str, _label: str, bss: bool = False):
        """SEG [.U] name — switch to a named segment (create if new)."""
        raw = args.strip()

        # Allow 'SEG.U' as argument suffix too (e.g. 'SEG VARS.U')
        if raw.upper().endswith('.U'):
            bss = True
            raw = raw[:-2].strip()
        if raw.upper() in ('.U', 'U'):
            bss = True
            raw = ''

        name = raw if raw else "__unnamed__"

        for seg in self.segments:
            if seg.name.lower() == name.lower():
                self.current_seg = seg
                if bss:
                    seg.flags |= SF_BSS
                return

        new_seg = Segment(
            name=name,
            flags=SF_UNKNOWN | (SF_BSS if bss else 0),
            rflags=SF_UNKNOWN,
            _init_org=0,
            _init_flags=SF_UNKNOWN | (SF_BSS if bss else 0),
        )
        self.segments.append(new_seg)
        self.current_seg = new_seg

    def _d_ds(self, args: str, _label: str):
        """DS count[,fill] — define space."""
        parts = self._split_args(args, maxsplit=1)
        r_count = self.ev.eval(parts[0])
        fill    = 0
        if len(parts) > 1:
            r_fill = self.ev.eval(parts[1])
            fill = r_fill.value & 0xFF if not r_fill.unknown else 0

        if r_count.unknown:
            self._redo_other += 1
            return
        count = r_count.value
        if count < 0:
            self._warn(f"DS with negative count {count}, ignored")
            return

        seg = self.current_seg
        if seg.flags & SF_BSS:
            seg.org  += count
            if seg.flags & SF_RORG:
                seg.rorg += count
        else:
            self._write_bytes(bytes([fill & 0xFF]) * count)

    def _d_dc(self, args: str, _label: str, size: int = 1, swap_endian: bool = False):
        """DC[.b/.w/.l/.s] val[,val,...] — define constant(s).
        swap_endian (.s) reverses byte order within each element."""
        for part in self._split_args(args):
            if not part:
                continue
            r = self.ev.eval(part)
            if r.unknown:
                self._redo_other += 1
                self._write_bytes(bytes(size))
            elif r.string is not None and size == 1:
                # String literal in dc.b: emit each character byte
                for ch in r.string:
                    self._write_byte(ord(ch) & 0xFF)
            else:
                v = r.value
                if size == 1:
                    self._write_byte(v)
                elif size == 2:
                    if swap_endian:
                        self._write_bytes(struct.pack('>H', v & 0xFFFF))
                    else:
                        self._write_word(v)
                elif size == 4:
                    if swap_endian:
                        self._write_bytes(struct.pack('>I', v & 0xFFFFFFFF))
                    else:
                        self._write_long(v)

    def _d_hex(self, args: str, _label: str):
        """HEX hexbytes — emit literal hex bytes (no $, no commas)."""
        h = ''.join(args.split())
        for i in range(0, len(h) - 1, 2):
            try:
                self._write_byte(int(h[i:i+2], 16))
            except ValueError:
                self._error(f"Invalid hex pair '{h[i:i+2]}' in HEX directive")

    def _d_equ(self, args: str, label: str):
        """label EQU expr — define an immutable symbol."""
        if not label:
            self._error("EQU without a label")
            return
        r = self.ev.eval(args.strip())
        if r.unknown:
            self._redo_other += 1
            # Store as unknown so other passes can see it
            self._set_symbol(label, 0, flags=SYM_UNKNOWN,
                             string=r.string)
        else:
            self._set_symbol(label, r.value, flags=0, string=r.string)

    def _d_set(self, args: str, label: str):
        """label SET expr — define a mutable symbol (may be redefined)."""
        if not label:
            self._error("SET without a label")
            return
        r = self.ev.eval(args.strip())
        flags = SYM_SET | (SYM_UNKNOWN if r.unknown else 0)
        if r.string is not None:
            flags |= SYM_STRING
        self._set_symbol(label, r.value, flags=flags, string=r.string, allow_redef=True)
        if r.unknown:
            self._redo_other += 1

    def _d_eqm(self, args: str, label: str):
        """label EQM expr — expression macro (re-evaluated each use)."""
        if not label:
            self._error("EQM without a label")
            return
        key = self._qualify_local(label)
        sym = Symbol(name=key, value=0, flags=SYM_MACRO, string=args.strip(),
                     order=self._sym_order)
        self._sym_order += 1
        self.symbols[key] = sym

    def _d_include(self, args: str, _label: str):
        """INCLUDE "file" — push a source file onto the include stack."""
        name = args.strip().strip('"').strip("'")
        path = self._find_file(name)
        if path is None:
            self._error(f"Cannot find include file: '{name}'")
            self._redo_other += 1
            return
        if self.opts.verbose >= 2:
            indent = "  " * len(self._stack)
            print(f"{indent}Including: {path}")
        self._push_file(path)

    def _d_incbin(self, args: str, _label: str):
        """INCBIN "file"[, skip] — embed a binary file."""
        parts = self._split_args(args, maxsplit=1)
        name  = parts[0].strip().strip('"').strip("'")
        skip  = 0
        if len(parts) > 1:
            r = self.ev.eval(parts[1])
            skip = r.value if not r.unknown else 0

        path = self._find_file(name)
        if path is None:
            self._error(f"Cannot find binary file: '{name}'")
            return

        try:
            with open(path, 'rb') as f:
                f.seek(skip)
                data = f.read()
            self._write_bytes(data)
        except OSError as e:
            self._error(f"Error reading '{path}': {e}")

    def _d_incdir(self, args: str, _label: str):
        """INCDIR "dir" — add a directory to the include search path."""
        d = args.strip().strip('"').strip("'")
        if d and d not in self.inc_dirs:
            self.inc_dirs.append(d)

    # ── Conditional assembly ──────────────────────────────────────────────────

    def _d_if(self, args: str, _label: str):
        outer = self._if_stack[-1]
        r     = self.ev.probe(args.strip())   # probe: no placeholder, no _redo_forward
        if r.unknown:
            self._redo_other += 1             # still need a redo if expr not yet known
        truth   = (not r.unknown) and bool(r.value)
        active  = outer.active and truth
        self._if_stack.append(IfFrame(active=active, this_true=truth, had_true=truth))

    def _d_ifdef(self, args: str, _label: str):
        """IFDEF — true if symbol exists and is fully resolved.
        Does NOT create a forward-reference placeholder; an absent symbol
        simply means 'not defined', which is a valid false condition."""
        outer = self._if_stack[-1]
        name  = args.strip()
        sym   = self.symbols.get(name)
        truth = sym is not None and not (sym.flags & SYM_UNKNOWN)
        self._if_stack.append(IfFrame(active=outer.active and truth,
                                      this_true=truth, had_true=truth))

    def _d_ifndef(self, args: str, _label: str):
        """IFNDEF — true if symbol is absent or unresolved.
        Same non-creating semantics as IFDEF."""
        outer = self._if_stack[-1]
        name  = args.strip()
        sym   = self.symbols.get(name)
        truth = sym is None or bool(sym.flags & SYM_UNKNOWN)
        self._if_stack.append(IfFrame(active=outer.active and truth,
                                      this_true=truth, had_true=truth))

    def _d_ifconst(self, args: str, _label: str):
        """IFCONST — true if expression is fully resolved.
        Uses probe() so undefined symbols don't create placeholders or redo."""
        outer = self._if_stack[-1]
        r     = self.ev.probe(args.strip())
        truth = not r.unknown
        self._if_stack.append(IfFrame(active=outer.active and truth,
                                      this_true=truth, had_true=truth))

    def _d_ifnconst(self, args: str, _label: str):
        """IFNCONST — true if expression is unresolved / undefined.
        Uses probe() so undefined symbols don't create placeholders or redo."""
        outer = self._if_stack[-1]
        r     = self.ev.probe(args.strip())
        truth = r.unknown
        self._if_stack.append(IfFrame(active=outer.active and truth,
                                      this_true=truth, had_true=truth))

    def _d_else(self, _args: str, _label: str):
        if len(self._if_stack) <= 1:
            self._error("ELSE without IF")
            return
        frame = self._if_stack[-1]
        outer = self._if_stack[-2]
        truth   = not frame.had_true
        active  = outer.active and truth
        self._if_stack[-1] = IfFrame(active=active, this_true=truth,
                                      had_true=frame.had_true or truth)

    def _d_endif(self, _args: str, _label: str):
        if len(self._if_stack) <= 1:
            self._error("ENDIF without IF")
            return
        self._if_stack.pop()

    # ── Macros ────────────────────────────────────────────────────────────────

    def _d_macro(self, args: str, label: str):
        """MACRO name / label MACRO — define a macro."""
        name = (label or args.strip()).lower()
        if not name:
            self._error("MACRO without a name")
            return

        self._macro_opens += 1

        # Collect body lines until ENDM (tracking nested macros)
        frame = self._stack[-1] if self._stack else None
        if frame is None:
            return

        body: List[str] = []
        depth = 1
        while frame.pos < len(frame.lines):
            raw  = frame.lines[frame.pos]
            frame.pos    += 1
            frame.lineno += 1
            line, _ = self._strip_comment(raw)
            _, op, _ = self._tokenise(line)
            op_l = op.lower().lstrip('.')
            if op_l in ('mac', 'macro'):
                depth += 1
            if op_l == 'endm':
                depth -= 1
                if depth == 0:
                    self._macro_closes += 1
                    break
            body.append(raw)

        if self.opts.strict and name in self.macros and self.macros[name].defpass == self.pass_num:
            self._error(f"Macro '{name}' redefined in same pass")

        self.macros[name] = MacroDef(name=name, params=[], lines=body, defpass=self.pass_num)

    def _d_endm(self, _args: str, _label: str):
        self._error("ENDM outside a MACRO definition")

    def _d_mexit(self, _args: str, _label: str):
        """MEXIT — exit current macro expansion."""
        # Skip to end of current macro frame
        if self._stack and self._stack[-1].is_macro:
            self._stack[-1].pos = len(self._stack[-1].lines)

    def _d_repeat(self, args: str, _label: str):
        """REPEAT n ... REPEND — repeat a block n times."""
        r = self.ev.eval(args.strip())
        if r.unknown:
            self._redo_other += 1
            count = 0
        else:
            count = max(0, r.value)

        # Collect body up to matching REPEND
        frame = self._stack[-1] if self._stack else None
        if frame is None:
            return

        body: List[str] = []
        depth = 1
        while frame.pos < len(frame.lines):
            raw  = frame.lines[frame.pos]
            frame.pos    += 1
            frame.lineno += 1
            line, _ = self._strip_comment(raw)
            _, op, _ = self._tokenise(line)
            op_l = op.lower().lstrip('.')
            if op_l == 'repeat':
                depth += 1
            if op_l == 'repend':
                depth -= 1
                if depth == 0:
                    break
            body.append(raw)

        if count > 0:
            self._stack.append(IncFrame(
                filename=frame.filename + "[REPEAT]",
                lines=body * count,
                is_macro=True,
            ))

    def _d_repend(self, _args: str, _label: str):
        self._error("REPEND without REPEAT")

    def _expand_macro(self, name: str, args_str: str):
        """Expand a macro call, substituting {1}, {2}, ... arguments."""
        mac = self.macros.get(name)
        if mac is None:
            self._error(f"Undefined macro '{name}'")
            return

        args = [a.strip() for a in self._split_args(args_str)] if args_str.strip() else []

        def substitute(line: str) -> str:
            def repl(m: re.Match) -> str:
                idx = int(m.group(1)) - 1
                return args[idx] if idx < len(args) else ''
            return re.sub(r'\{(\d+)\}', repl, line)

        expanded = [substitute(ln) for ln in mac.lines]
        self._stack.append(IncFrame(
            filename=f"<macro:{name}>",
            lines=expanded,
            is_macro=True,
        ))

    # ── Misc directives ───────────────────────────────────────────────────────

    def _d_align(self, args: str, _label: str):
        """ALIGN n — pad to n-byte boundary with $FF."""
        r = self.ev.eval(args.strip())
        if r.unknown or r.value < 1:
            return
        n   = r.value
        rem = self.current_seg.org % n
        if rem:
            self._write_bytes(bytes([FILL_BYTE]) * (n - rem))

    def _d_subroutine(self, _args: str, _label: str):
        """SUBROUTINE — advance the local label index."""
        self._local_next        += 1
        self._local_idx          = self._local_next
        self._local_dollar_next += 1
        self._local_dollar_idx   = self._local_dollar_next

    def _d_echo(self, args: str, _label: str):
        """ECHO expr[,expr,...] — print values to stdout at assembly time."""
        parts = self._split_args(args)
        tokens: List[str] = []
        for part in parts:
            part = part.strip()
            if not part:
                continue
            r = self.ev.eval(part)
            if r.string is not None:
                tokens.append(r.string)
            elif r.unknown:
                tokens.append("????")
            else:
                # Bare numbers print as hex $xxxx
                tokens.append(f"${r.value:04X}")
        self._messages.append(" ".join(tokens))

    def _d_err(self, args: str, _label: str):
        """ERR — force an assembly error."""
        self._error(f"ERR: {args.strip()}", fatal=True)

    def _d_end(self, _args: str, _label: str):
        """END — stop processing the current file."""
        if self._stack:
            self._stack[-1].pos = len(self._stack[-1].lines)

    # ── Instruction emission ──────────────────────────────────────────────────

    def _emit_instruction(self, mne: str, operands: str,
                          forced_mode: Optional[int],
                          forced_size: Optional[int] = None):
        omap = self.opcodes.get(mne)
        if omap is None:
            self._error(f"Unknown mnemonic '{mne}'")
            return

        operands = operands.strip()

        if not operands:
            mode  = AM_IMP
            val   = 0
            unk   = False
        else:
            r    = self.ev.eval_operand(operands)
            val  = r.value
            unk  = r.unknown
            mode = r.addrmode

        # Resolve _AM_REGX / _AM_REGY to concrete modes
        if mode == _AM_REGX:
            mode = AM_ZPX   # will widen to ABSX if val >= 256
        elif mode == _AM_REGY:
            mode = AM_ZPY

        if forced_mode is not None:
            # Two-char extension (.wx, .bx, .wy …) — fully overrides mode,
            # ignores anything from the operand.
            mode = forced_mode
        elif forced_size is not None:
            # One-char extension (.w, .b) — controls width only; register
            # index (,X / ,Y) from the operand is preserved.
            if forced_size == 1:
                if mode in (AM_ABS, AM_ZP):   mode = AM_ZP
                if mode in (AM_ABSX, AM_ZPX): mode = AM_ZPX
                if mode in (AM_ABSY, AM_ZPY): mode = AM_ZPY
            elif forced_size == 2:
                if mode in (AM_ZP, AM_ABS):   mode = AM_ABS
                if mode in (AM_ZPX, AM_ABSX): mode = AM_ABSX
                if mode in (AM_ZPY, AM_ABSY): mode = AM_ABSY
        elif not unk:
            # Auto: use ZP when value fits, ABS otherwise
            if mode == AM_ABS  and 0 <= val <= 0xFF: mode = AM_ZP
            if mode == AM_ABSX and 0 <= val <= 0xFF: mode = AM_ZPX
            if mode == AM_ABSY and 0 <= val <= 0xFF: mode = AM_ZPY

            if mode == AM_ZP  and val > 0xFF: mode = AM_ABS
            if mode == AM_ZPX and val > 0xFF: mode = AM_ABSX
            if mode == AM_ZPY and val > 0xFF: mode = AM_ABSY

        # Walk the widening chain until we find a supported mode
        attempt = mode
        while attempt not in omap and attempt in CVT:
            attempt = CVT[attempt]

        if attempt not in omap:
            # Special case: branch instructions only have AM_REL but operands are
            # given as absolute addresses.  If the opcode supports AM_REL and we
            # have an absolute address, use AM_REL (offset computed at emit time).
            if AM_REL in omap and mode in (AM_ABS, AM_ZP):
                attempt = AM_REL
            elif unk:
                # Unknown value — try common modes as placeholders for this pass
                for fallback in (AM_REL, AM_ABS, AM_ABSX, AM_ABSY, AM_ZP, AM_IMM8, AM_IMP):
                    if fallback in omap:
                        attempt = fallback
                        break
                else:
                    self._error(f"No valid addressing mode for '{mne}' (operand: '{operands}')")
                    return
                self._redo_forward += 1
            else:
                self._error(
                    f"Illegal addressing mode for '{mne}' "
                    f"(operand: '{operands}', value: {val:#x})"
                )
                return

        mode   = attempt
        opsize = OPSIZE.get(mode, 0)
        opbyte = omap[mode]

        self._write_byte(opbyte)

        if mode == AM_REL:
            if unk:
                self._write_byte(0)
                self._redo_forward += 1
            else:
                seg = self.current_seg
                # cur_pc is the address of the offset byte (opcode already written).
                # The 6502 adds the offset to PC *after* the full instruction,
                # so the base is cur_pc+1 (address of next instruction).
                cur_pc = (seg.rorg if (seg.flags & SF_RORG) else seg.org)
                offset = val - cur_pc - 1
                if offset < -128 or offset > 127:
                    self._error(
                        f"Branch out of range: {offset:+d} bytes "
                        f"(target {val:#x}, PC {cur_pc:#x})"
                    )
                    self._redo_phase += 1
                    offset = 0
                self._write_byte(offset & 0xFF)
        elif opsize == 1:
            self._write_byte(0 if unk else val)
            if unk: self._redo_forward += 1
        elif opsize == 2:
            self._write_word(0 if unk else val)
            if unk: self._redo_forward += 1

    # ── Symbol management ─────────────────────────────────────────────────────

    def _qualify_local(self, name: str) -> str:
        """Expand .local and local$ to their fully-qualified internal names."""
        if name.startswith('.'):
            return f"\x01loc{self._local_idx}{name}"
        if name.endswith('$'):
            return f"\x01dloc{self._local_dollar_idx}{name}"
        return name

    def _define_label(self, name: str):
        """Assign the current PC to a label."""
        if not name:
            return

        # Advance dollar-local scope for non-local labels
        if not name.startswith('.') and not name.endswith('$'):
            self._local_dollar_next += 1
            self._local_dollar_idx   = self._local_dollar_next

        key = self._qualify_local(name)
        pc  = self.pc
        pcu = self.pc_unknown

        existing = self.symbols.get(key)
        if existing is None:
            sym = Symbol(name=key, value=pc,
                         flags=SYM_UNKNOWN if pcu else 0,
                         order=self._sym_order)
            self._sym_order += 1
            self.symbols[key] = sym
        else:
            old_val = existing.value
            old_unk = bool(existing.flags & SYM_UNKNOWN)

            # Phase error: symbol was known last pass but value changed this pass
            if not pcu and not old_unk and old_val != pc:
                self._redo_phase += 1
                frame = self._stack[-1] if self._stack else None
                loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
                reason = f"{loc}phase error: '{self._sym_display_name(name)}' was {old_val:#x}, now {pc:#x}"
                self._redo_reasons.append(reason)
                if self.opts.verbose >= 2:
                    self._warn(f"Phase error: '{name}' was {old_val:#x}, now {pc:#x}")

            existing.value = pc
            if pcu:
                existing.flags |= SYM_UNKNOWN
            else:
                existing.flags &= ~SYM_UNKNOWN

    def _set_symbol(self, name: str, value: int, flags: int = 0,
                    string: Optional[str] = None, allow_redef: bool = False):
        key      = self._qualify_local(name)
        existing = self.symbols.get(key)

        if existing and not (existing.flags & SYM_SET) and not allow_redef:
            # EQU redefinition check
            if not (existing.flags & SYM_UNKNOWN) and existing.value != value:
                self._redo_phase += 1
                frame = self._stack[-1] if self._stack else None
                loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
                reason = (
                    f"{loc}phase error: '{self._sym_display_name(name)}' "
                    f"was {existing.value:#x}, now {value:#x}"
                )
                self._redo_reasons.append(reason)
                if self.opts.verbose:
                    self._warn(f"EQU mismatch: '{name}' was {existing.value:#x}, now {value:#x}")

        sym = Symbol(
            name=key, value=value,
            flags=flags | (SYM_STRING if string is not None else 0),
            string=string,
            order=(existing.order if existing else self._sym_order),
        )
        if not existing:
            self._sym_order += 1
        self.symbols[key] = sym

    def _cmd_define(self, spec: str, eqm: bool):
        """Process a -D or -M command-line define."""
        if '=' in spec:
            name, _, expr = spec.partition('=')
        else:
            name, expr = spec, '1'
        name = name.strip()
        if eqm:
            sym = Symbol(name=name, value=0, flags=SYM_MACRO, string=expr.strip(),
                         order=self._sym_order)
        else:
            r   = self.ev.eval(expr.strip())
            sym = Symbol(name=name, value=r.value,
                         flags=0 if not r.unknown else SYM_UNKNOWN,
                         order=self._sym_order)
        self._sym_order += 1
        self.symbols[name] = sym

    # ── Output buffer ─────────────────────────────────────────────────────────

    def _write_byte(self, v: int):
        seg = self.current_seg
        if seg.flags & SF_BSS:
            seg.org  += 1
            if seg.flags & SF_RORG: seg.rorg += 1
            return
        if not (seg.flags & SF_UNKNOWN):
            self._output[seg.org] = v & 0xFF
        seg.org  += 1
        if seg.flags & SF_RORG: seg.rorg += 1

    def _write_bytes(self, data: bytes):
        seg = self.current_seg
        n   = len(data)
        if seg.flags & SF_BSS:
            seg.org += n
            if seg.flags & SF_RORG:
                seg.rorg += n
        elif not (seg.flags & SF_UNKNOWN):
            self._output.update(enumerate(data, seg.org))
            seg.org += n
            if seg.flags & SF_RORG:
                seg.rorg += n
        else:
            # ORG not yet known — fall back to per-byte (advances counter only)
            for b in data:
                self._write_byte(b)

    def _write_word(self, v: int):
        """Emit 16-bit little-endian word."""
        self._write_byte(v & 0xFF)
        self._write_byte((v >> 8) & 0xFF)

    def _write_long(self, v: int):
        """Emit 32-bit little-endian long."""
        for i in range(4):
            self._write_byte((v >> (8*i)) & 0xFF)

    def _write_output(self) -> int:
        if not self._output:
            try:
                with open(self.opts.outfile, 'wb'):
                    pass
            except OSError:
                pass
            return 0

        lo = min(self._output)
        hi = max(self._output)
        size = hi - lo + 1
        buf  = bytearray([FILL_BYTE] * size)
        for addr, byte in self._output.items():
            buf[addr - lo] = byte

        fmt = getattr(self.opts, 'format', 1)
        try:
            with open(self.opts.outfile, 'wb') as f:
                f.write(bytes(buf))
                if fmt == 2:
                    f.write(struct.pack('>I', sum(buf) & 0xFFFFFFFF))
        except OSError as e:
            print(f"error: Cannot write output: {e}", file=sys.stderr)
            return 2
        return 0

    # ── Line preprocessing ────────────────────────────────────────────────────

    def _strip_comment(self, line: str) -> Tuple[str, str]:
        """
        Strip trailing comments from a line, handling:
          ; line comments
          /* ... */  multi-line C-style comments (may span lines)

        Returns (clean_line, comment_text).
        """
        if self._in_ml_comment:
            end = line.find('*/')
            if end >= 0:
                self._in_ml_comment = False
                # Continue processing the rest of the line after */
                return self._strip_comment(line[end+2:])
            return '', line   # entire line is comment

        result = []
        i = 0
        while i < len(line):
            ch = line[i]

            # Start of multi-line comment
            if ch == '/' and i+1 < len(line) and line[i+1] == '*':
                end = line.find('*/', i+2)
                if end >= 0:
                    # Single-line /* ... */ — skip it and continue
                    i = end + 2
                    continue
                else:
                    # Comment runs to end of file (multi-line)
                    self._in_ml_comment = True
                    return ''.join(result).rstrip(), line[i:]

            # Line comment
            if ch == ';':
                return ''.join(result).rstrip(), line[i+1:]

            # Skip character literals (so ';' inside doesn't end the line)
            if ch == "'":
                result.append(ch)
                i += 1
                if i < len(line):
                    result.append(line[i])
                    i += 1
                continue

            # Skip string literals
            if ch == '"':
                result.append(ch)
                i += 1
                while i < len(line) and line[i] != '"':
                    result.append(line[i])
                    i += 1
                if i < len(line):
                    result.append(line[i])
                    i += 1
                continue

            result.append(ch)
            i += 1

        return ''.join(result).rstrip(), ''

    _LABEL_RE = re.compile(
        r'^([A-Za-z_.$@][A-Za-z0-9_.$@]*)(\s|:|=|$)'
    )

    def _tokenise(self, line: str) -> Tuple[str, str, str]:
        """
        Split a comment-stripped line into (label, opcode, operands).

        DASM layout:
            [label[:]] [opcode [operands]]

        A line starting with whitespace has no label.
        A label-only line (no opcode) is valid.
        'label = value' is treated as EQU.
        """
        label = ''
        opcode = ''
        operands = ''

        if not line.strip():
            return label, opcode, operands

        # '^' prefix forces a label regardless of whitespace (rarely used)
        if line.startswith('^'):
            line = line[1:].lstrip()

        # If line starts with non-whitespace it has a label (or a bare opcode)
        if line and line[0] not in (' ', '\t'):
            m = self._LABEL_RE.match(line)
            if m:
                label = m.group(1)
                rest  = line[len(label):]
                # Consume optional colon
                rest = rest.lstrip()
                if rest.startswith(':'):
                    rest = rest[1:]
                line = rest.lstrip()
            else:
                # Label-like but with unusual chars; treat whole token as label
                i = 0
                while i < len(line) and line[i] not in (' ', '\t', ':'):
                    i += 1
                label = line[:i]
                line  = line[i:].lstrip(':').lstrip()

        # Handle 'label = value' (EQU-shorthand)
        if label and line.startswith('=') and (len(line) == 1 or line[1] != '='):
            opcode   = '='
            operands = line[1:].lstrip()
            return label, opcode, operands

        # Next token is the opcode
        parts = line.split(None, 1)
        if parts:
            opcode   = parts[0]
            operands = parts[1].strip() if len(parts) > 1 else ''

        return label, opcode, operands

    # ── Include file stack ────────────────────────────────────────────────────

    def _push_file(self, path: str):
        try:
            with open(path, 'r', errors='replace') as f:
                lines = f.read().splitlines()
        except OSError as e:
            self._error(f"Cannot open '{path}': {e}", fatal=True)
            return
        self._stack.append(IncFrame(filename=path, lines=lines))

    def _find_file(self, name: str) -> Optional[str]:
        """Search for a file in include directories."""
        # Try relative to current source file
        if self._stack:
            base = os.path.dirname(self._stack[-1].filename)
            p = os.path.join(base, name)
            if os.path.exists(p):
                return p

        # Try cwd
        if os.path.exists(name):
            return name

        # Search include dirs
        for d in self.inc_dirs:
            p = os.path.join(d, name)
            if os.path.exists(p):
                return p

        return None

    # ── Argument splitting ────────────────────────────────────────────────────

    def _split_args(self, text: str, maxsplit: int = 0) -> List[str]:
        """
        Split a comma-separated argument list, respecting parentheses, brackets,
        and quoted strings.
        """
        parts: List[str] = []
        depth   = 0
        current: List[str] = []
        in_str  = False
        str_ch  = ''
        splits  = 0

        for ch in text:
            if in_str:
                current.append(ch)
                if ch == str_ch:
                    in_str = False
                continue
            if ch in ('"', "'"):
                in_str = True
                str_ch = ch
                current.append(ch)
                continue
            if ch in ('(', '['):
                depth += 1
                current.append(ch)
                continue
            if ch in (')', ']'):
                depth -= 1
                current.append(ch)
                continue
            if ch == ',' and depth == 0:
                if maxsplit and splits >= maxsplit:
                    current.append(ch)
                else:
                    parts.append(''.join(current).strip())
                    current = []
                    splits += 1
                continue
            current.append(ch)

        if current or not parts:
            parts.append(''.join(current).strip())

        return parts

    # ── Error/warning reporting ───────────────────────────────────────────────

    def _error(self, msg: str, fatal: bool = False):
        frame = self._stack[-1] if self._stack else None
        loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
        self._errors.append(f"{loc}error: {msg}")
        if fatal:
            self._fatal = True

    def _warn(self, msg: str):
        frame = self._stack[-1] if self._stack else None
        loc   = f"{frame.filename}:{frame.lineno}: " if frame else ""
        self._errors.append(f"{loc}warning: {msg}")

    def _print_errors(self):
        for e in self._errors:
            print(e, file=sys.stderr)

    def _report_stuck(self):
        """
        Called when the assembler makes no progress between two passes.
        Prints a structured diagnostic: pass errors, redo breakdown, root causes.
        """
        err = sys.stderr

        # ── Any hard errors from the last pass ────────────────────────────────
        if self._errors:
            print("--- Errors from last pass ---", file=err)
            for e in self._errors:
                print(f"  {e}", file=err)

        # ── Redo breakdown ────────────────────────────────────────────────────
        print(
            f"\nerror: Source is not resolvable after pass {self.pass_num}.",
            file=err,
        )
        parts = []
        if self._redo_forward: parts.append(f"{self._redo_forward} unresolved forward reference(s)")
        if self._redo_phase:   parts.append(f"{self._redo_phase} phase error(s) (symbol value changed)")
        if self._redo_other:   parts.append(f"{self._redo_other} unknown ORG/DS/expression(s)")
        if parts:
            print("Reason: " + ", ".join(parts) + ".", file=err)

        # ── Most useful: what's actually causing it ───────────────────────────
        if self._redo_reasons:
            unique = list(dict.fromkeys(self._redo_reasons))
            print(f"\n--- Root causes ({len(unique)} unique) ---", file=err)
            for r in unique[:20]:   # cap at 20 so it doesn't flood
                print(f"  {r}", file=err)
            if len(unique) > 20:
                print(f"  ... and {len(unique)-20} more", file=err)

        # ── Unresolved symbols ────────────────────────────────────────────────
        unresolved = [s for s in self.symbols.values() if s.flags & SYM_UNKNOWN]
        if unresolved:
            print(f"\n--- Unresolved symbols ({len(unresolved)}) ---", file=err)
            for sym in unresolved[:20]:
                name_clean = self._sym_display_name(sym.name)
                print(f"  {name_clean}", file=err)
            if len(unresolved) > 20:
                print(f"  ... and {len(unresolved)-20} more", file=err)

    # ── Symbol dump ───────────────────────────────────────────────────────────

    @staticmethod
    def _sym_display_name(internal: str) -> str:
        """Strip internal prefixes to get the user-visible symbol name.
        Internal format: \\x01loc<N>.label  or  \\x01dloc<N>label$"""
        if not internal.startswith('\x01'):
            return internal
        s = internal[1:]           # strip sentinel
        if s.startswith('loc'):    # .local labels:   loc5.foo  → 5.foo
            s = s[3:]
        elif s.startswith('dloc'): # dollar-local:    dloc3bar$ → 3bar$
            s = s[4:]
        return s

    def write_symbols(self, path: str, sort_mode: int = 0):
        """Write symbol table in DASM-compatible format.

        Column layout (matches real DASM sftos / ShowSymbols):
          %-24s  symbol name (left-padded to 24 chars)
          ' '    separator
          %04x   value (4 lowercase hex digits)
          ' '
          '???? ' or '     '   unknown flag
          'str ' or '    '     string flag
          'eqm ' or '    '     macro/eqm flag
          '(R )' or '    '     referenced flag
        """
        syms = list(self.symbols.values())
        _sort_labels = {0: 'symbol', 1: 'address', 2: 'order'}
        if sort_mode == 0:
            syms.sort(key=lambda s: s.name.lower())
        elif sort_mode == 1:
            syms.sort(key=lambda s: s.value)
        else:
            syms.sort(key=lambda s: s.order)

        try:
            with open(path, 'w') as f:
                print(f"--- Symbol List (sorted by {_sort_labels.get(sort_mode, 'symbol')})", file=f)
                for sym in syms:
                    name_clean = self._sym_display_name(sym.name)
                    val_str  = f"{sym.value & 0xFFFF:04x}"
                    unk_str  = "???? " if (sym.flags & SYM_UNKNOWN) else "     "
                    str_str  = "str " if (sym.flags & SYM_STRING) else "    "
                    eqm_str  = "eqm " if (sym.flags & SYM_MACRO)  else "    "
                    if sym.flags & SYM_REF:
                        ref_str = "(R )"
                    elif sym.flags & SYM_SET:
                        ref_str = "( S)"
                    else:
                        ref_str = "    "
                    print(f"{name_clean:<24} {val_str} {unk_str}{str_str}{eqm_str}{ref_str}", file=f)
                print("--- End of Symbol List.", file=f)
        except OSError as e:
            print(f"Warning: cannot write symbol file: {e}", file=sys.stderr)


# ─────────────────────────────────────────────────────────────────────────────
# CLI
# ─────────────────────────────────────────────────────────────────────────────

def parse_args(argv: List[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(
        prog="dasm.py",
        description="DASM-compatible 6502 macro assembler (Python port)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        add_help=True,
    )
    p.add_argument("source",                help="Source file (.asm)")
    p.add_argument("-o", metavar="FILE",    dest="outfile",     default="a.out")
    p.add_argument("-l", metavar="FILE",    dest="listfile",    default=None)
    p.add_argument("-s", metavar="FILE",    dest="symfile",     default=None)
    p.add_argument("-v", metavar="N",       dest="verbose",     type=int, default=0)
    p.add_argument("-f", metavar="N",       dest="format",      type=int, default=1,
                   choices=[1, 2, 3])
    p.add_argument("-T", metavar="N",       dest="sort_mode",   type=int, default=0,
                   choices=[0, 1, 2])
    p.add_argument("-p", metavar="N",       dest="max_passes",  type=int, default=10)
    p.add_argument("-P", metavar="N",       dest="max_passes_lenient", type=int, default=None)
    p.add_argument("-I", metavar="DIR",     dest="include_dirs", action="append", default=[])
    p.add_argument("-D", metavar="SYM[=V]", dest="defines",     action="append", default=[])
    p.add_argument("-M", metavar="SYM=EXP", dest="eqm_defines", action="append", default=[])
    p.add_argument("-S",                    dest="strict",       action="store_true", default=False)
    p.add_argument("-R",                    dest="remove_on_error", action="store_true", default=False)

    opts = p.parse_args(argv)

    opts.do_all_passes = False
    if opts.max_passes_lenient is not None:
        opts.max_passes    = opts.max_passes_lenient
        opts.do_all_passes = True

    return opts


def main(argv: Optional[List[str]] = None) -> int:
    opts = parse_args(argv if argv is not None else sys.argv[1:])

    print(f"dasm.py {VERSION}  (DASM 6502 assembler — Python port)")

    asm = Assembler(opts)
    rc  = asm.assemble(opts.source)

    if opts.symfile and not rc:
        asm.write_symbols(opts.symfile, opts.sort_mode)

    if rc and opts.remove_on_error:
        try:
            os.unlink(opts.outfile)
        except FileNotFoundError:
            pass

    return rc


if __name__ == "__main__":
    sys.exit(main())
