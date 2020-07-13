	.PROCESSOR	68hc908
	.ORG 0
backLoop:
	cbeq	(500,SP),backLoop	; word offset is not allowed with this mnemonic

