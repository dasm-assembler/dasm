;
; Test various directives that "fill" memory.
;
; This came about because Dennis Debro posted about .align not
; doing the right thing to the Stella list. Thomas Mathys was
; able to trace this to v_align, which uses the first value
; instead of the second as the actual fill byte. Fixed as of
; release 2.20.12 I hope. :-)
;
; Peter H. Froehlich
; phf at acm dot org

	.processor	6502
	.org		0

	; .ds fills with $a5 as required, works
	.ds	33, $a5		; always filled $a5
	; .align fills with *align* value prior to 2.20.12
	.align	64, $f5		; filled $40 <2.20.11, fills $f5 now
	.align	256, $5f	; filled $0 <2.20.11, fills $5f now

	.end
