
	.TRACE on

	.PROCESSOR 68908

	.MAC	ldx		; we cant overwrite any of the processor ops
		lda	{1}
	.ENDM

	.MAC	LDHX		; also not with different case
		ldx	<{1}
		lda	>{1}
		psha
		pulh
	.ENDM

	.MAC M1_postfix7
		ldhx	#1
	.ENDM

	.MAC M2_postfix7
		ldx 	#1
		clrh
	.ENDM

M1_postfix7		.EQU	0xA1		; symbols can share the same name with macros or processor ops
M2_postfix7		.EQU	0xB2
LDA			.EQU	0xDA

	.MAC M3_postfix7
		pshh
		pulx
	.ENDM


	.ORG 0

; call to macro1
	M1_postfix7

; call to macro2
	M2_postfix7

	DC.b	M1_postfix7	
	DC.b	M2_postfix7	
	DC.b	LDA

	lda	#0x0a
	LDA	#0x0b

; call to macro3
	M3_postfix7

	ldx	#1
	ldhx	#2
	LDHX	#3

	.END	


