; Test case for double macros 

	processor 6502

 MAC testmac		; the first declaration wins
	lda #1
 ENDM
 MAC testmac		; the second is silently ignored without the -S flag, but an error with -S
	ldx #1
 ENDM

	ORG $f800

	testmac

