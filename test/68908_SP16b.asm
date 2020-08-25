	.processor	68hc908
	.org 	0	

	ldx.b	500,SP	; should trigger since the user requested byte mode but size is word

    