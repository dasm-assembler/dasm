
		.PROCESSOR	6803


FAR_ADDR	.EQU		$F01A
DIRECT_ADDR	.EQU		$40

	.ORG		0

	jsr.b		DIRECT_ADDR
	jsr.w		FAR_ADDR
	
	jsr		DIRECT_ADDR
	jsr		FAR_ADDR

	jsr.w		DIRECT_ADDR
	
	jsr.b		>FAR_ADDR
	jsr.b		<FAR_ADDR

	jsr.w		>FAR_ADDR
	jsr.w		<FAR_ADDR



	
