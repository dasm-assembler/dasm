; Test case for double macros 

	processor 6502

 .macro testmac
	lda #1
 .endm
 .macro testmac
	lda #1
 .endm

	ORG $f800

	testmac

