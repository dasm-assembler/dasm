
	.PROCESSOR	6502
	

ME	.EQU		(OTHER +1)
OTHER	.EQU		(ME -1)

	.ORG	0
	
	DC.b 	ME
	DC.b	OTHER






