; Thomas Jentzsch <tjentzsch@web.de> test case for bug
; in error handling.

	processor 6502

	ORG $f800
	ldz #$00 ; illegal mnemonic

