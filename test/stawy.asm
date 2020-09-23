
	.TRACE	on

	.processor 	6502

	
RESP0 = $10
	
	.ORG 	0

	sta.w  (RESP0),y	; wrong statement  forced(wordaddr)  but coded(indirect_byte_Y)









