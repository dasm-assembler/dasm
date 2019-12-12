; addressexpression - adapted from the "jentzsch3" address expression bug test.
; Original test courtesy Thomas Jentzsch. Output result code generation 
; added by Mike Saarna.

	processor 6502
	ORG $100

TEST_1 = (.) 
TEST_2 = (*)
TEST_3 = (1 - .)
TEST_4 = (1 - *)
TEST_5 = (1 - . - 1)
TEST_6 = (1 - * - 1)
TEST_7 = (1 - 1 - .)
TEST_8 = (1 - 1 - *)
TEST_9 = (1 + *)
TEST_10 = (1 + * + 1)
TEST_11 = (1 + 1 + *)

 if TEST_1 =  256
	.byte $01
 endif
 if TEST_2 = 256
	.byte $02
 endif
 if TEST_3 = -255
	.byte $03
 endif
 if TEST_4 = -255
	.byte $04
 endif
 if TEST_5 = -256
	.byte $05
 endif
 if TEST_6 = -256
	.byte $06
 endif
 if TEST_7 = -256
	.byte $07
 endif
 if TEST_8 = -256
	.byte $08
 endif
 if TEST_9 = 257
	.byte $09
 endif
 if TEST_10 = 258
	.byte $0A
 endif
 if TEST_11 = 258
	.byte $0B
 endif


