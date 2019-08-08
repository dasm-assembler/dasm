; Bug in string conversion.
; Reported 2007/08/25 by Eduardo Casino Almao <casino_e@movistar.es>

; I've found a bug in the string conversion feature ([exp]d) I'm using
; version 2.20.10b
;
; Basically, if exp is a not-yet-resolved label, the assembler aborts with
; "program.a: (XX): error Label mismatch...

	processor 6502

	org	$0801

	word	endprg			; pointer to next line
	word	1968			; line number
	byte	$9e			; SYS token
	byte	[start]d		; ASCII of SYS address
	byte	" TESTPROG BY TESTSOFT"	; Show copyright
endprg	byte	0,0,0			; End of BASIC program

start	rts

; The assembler outputs:
; 
; DASM V2.20.10, Macro Assembler (C)1988-2004
; testcase.a (13): error: Label mismatch...
;  --> endprg 081c
; testcase.a (16): error: Label mismatch...
;  --> start 081f
; Unrecoverable error(s) in pass, aborting assembly!
; Complete.
;
; The offending code is in symbols.c, lines 203 to 210. I think that there
; should be an extra condition, so if in the previous pass it has been a
; string conversion, just go for another pass.
;
; -----CUT-----
;            /*
;            * If we had an unevaluated IF expression in the
;            * previous pass, don't complain about phase errors
;            * too loudly.
;                */
; 203            if (F_verbose >= 1 || !(Redo_if & (REASON_OBSCURE)))
;                {
;                    char sBuffer[ MAX_SYM_LEN * 2 ];
;                    sprintf( sBuffer, "%s %s", sym->name, sftos( sym->value, 0 ) );
;                    /*, sftos(sym->value,
;                    sym->flags) ); , sftos(pc, cflags & 7));*/
;                    asmerr( ERROR_LABEL_MISMATCH, false, sBuffer );
; 210            }
;                ++Redo;
;                Redo_why |= REASON_PHASE_ERROR;
; -----CUT-----
