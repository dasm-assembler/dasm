
	.TRACE	on

	.processor 	6502

RESP0 = $10
RESP1 = $100
	
	.ORG 	0

	sta.wy   (RESP0),y	; wrong statement   forced(wordaddr_Y)   but  coded(indirect_byte_Y)













