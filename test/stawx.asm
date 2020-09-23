
	.TRACE	on

	.processor 	6502

	
RESP0 = $10
RESP1 = $100
	
	.ORG 	0

	sta.wx  RESP0,x
	sta.w	RESP0,x
	sta 	RESP1,x

	lda	(1),Y
	lda	(RESP0),Y
	




