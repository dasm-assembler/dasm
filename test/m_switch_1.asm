

	.processor 6502

	.org	0

	DC.w	37
	DC.w	47
	DC.w	59

	.ds	((64+4) * 1024)

	DC.w	37,47,59
    
	.end