	.PROCESSOR	6502
	.TRACE	on

RAM_START	.equ   $80
addr_offs	.equ    (RAM_START + 192)

	.ORG 0

	rol.b	addr_offs,X	; forced byte addressing and too large value should trigger



