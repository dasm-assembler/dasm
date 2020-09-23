
	.TRACE	on

	.processor 	6502

RESP1 = $100
	
	.ORG 	0

	sta.by   RESP1,y	; wrong statement   forced(byteaddr_Y)   but  implied(wordaddr_Y)














