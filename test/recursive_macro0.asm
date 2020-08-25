
	.PROCESSOR	6502
	
	.MAC rec_test
		.IF ({1} > 0)
			rec_test [{1}-1]
		.ELSE
			lda	#{1}
		.ENDIF		
	.ENDM

	.ORG	0
	
	rec_test 5







