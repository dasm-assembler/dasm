; Test negative literals.
;
; This came about because of the "-1 bug" in 2.20.10 and
; will hopefully ensure that no such bug ever returns to
; haunt us again...
;
; Peter H. Froehlich
; phf at acm dot org

	.processor	6502
	.org		0

	lda	#1024	; should fail? does fail!

	.end
