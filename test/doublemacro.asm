; Test case for double macros 

	processor 6502

 MAC testmac
	lda #1
 ENDM
 MAC testmac
	lda #1
 ENDM

	ORG $f800

	testmac

