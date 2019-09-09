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

	lda	#0
;	lda	#+0	;syntax error? needs a FIX as well :-)
	lda	#-0

	lda	#1
	lda	#-1

	lda	#127
	lda	#128
	lda	#129

	lda	#-127
	lda	#-128
	lda	#-129

	lda	#254
	lda	#255
	lda	#256	; should fail? does fail!

	lda	#-254
	lda	#-255
	lda	#-256	; should fail? does NOT yet fail?

	lda	#1024	; should fail? does fail!
	lda	#-1024	; should fail? does NOT yet fail?

	.end
