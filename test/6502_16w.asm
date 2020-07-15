	.PROCESSOR	6502
	.TRACE	on

RAM_START	.equ   $80000
my_addr		.equ    (RAM_START + 192)

	.ORG 0

	and	my_addr
	and.w	my_addr
	and.b	my_addr	; any op should trigger since we have 64k space



