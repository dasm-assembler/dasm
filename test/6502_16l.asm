	.PROCESSOR	6502
	.TRACE	on

	.ORG 0

	and.b	<400	; forced byte addressing and taking lower part should be OK
	and.w	<400

	ldy.b	<400,X
	ldy.w	<400,X



