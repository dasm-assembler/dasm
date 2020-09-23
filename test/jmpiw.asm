
	.TRACE	on

	.processor 	6502

	.ORG 	0
RESP0 = $10		; (curious) why is (only) listing output big endian ordered when processor is little ?
	DC.w	RESP0

RESP1 = $100		;       ... but that's a different construction site
	DC.w	RESP1


	jmp	RESP0
	jmp	RESP1
	jmp	(RESP1)		

	jmp.in	(RESP0)		; forced(indirect_word)  and coded(indirect_word)
	jmp.in	RESP0		; forced(indirect_word)  and coded(byteaddr)		; should this be an error with strict mode ?







