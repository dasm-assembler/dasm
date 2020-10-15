;
;   regression test for all opcodes for 68hc908 controller 
;   
;   two opcodes with indirect X adressing and zero offset are not cleanly supported
;	dbnz		(X), jumpLabel	
;	cbeq		(X), jumpLabel
;			
;    one must add forced-address-mode extension ".ix" to the mnemonic and add a dummy variable
;    in order to get the correct opcodes 
;	dbnz.ix		0, jumpLabel
;	cbeq.ix		0, jumpLabel
;

	.PROCESSOR	68hc908

PTB	.EQU		1
PTC	.EQU		2
DDRB	.EQU		5

FIVE		.EQU	5
FIVE_HUNDRED	.EQU	500

RAM_START	.EQU	0x40

	.ORG		0xFB00

	.byte		0xba,0xdc,0x0d,0xe0

	adc	#6
	adc	RAM_START
	adc	[X+7]
	adc	constant0
	adc	[X+0x300]
	adc	,X
	adc	[SP+5]
	adc	[SP+500]

	add	#6
	add	RAM_START
	add	7,X
	add	constant0
	add	0x300,X
	add	,X
	add	FIVE,SP
	add	[SP+FIVE_HUNDRED]

	aiS		#5
	aiX		#6
	
	and	#6
	and	RAM_START
	and	7,X
	and	constant0
	and	[X+0x300]
	and	,X
	and	5,SP
	and	FIVE_HUNDRED,SP

	asl	RAM_START
	asl	3,X
	asl	,X
	asl	5,SP
	aslA
	aslX

	asr	RAM_START
	asr	3,X
	asr	,X
	asr	5,SP
	asrA
	asrX

	bcc	frontLoop
	bcs	frontLoop
	beq 	frontLoop
	bhcc	frontLoop
	bhcs	frontLoop
	bhi	frontLoop
	bhs	frontLoop
	bih	frontLoop
	bil	frontLoop

	bit	#6
	bit	RAM_START
	bit	7,X
	bit	constant0
	bit	0x300,X
	bit	,X
	bit	5,SP
	bit	500,SP
	
	blo	frontLoop
	bls	frontLoop
	bmc	frontLoop
	bmi	frontLoop
	bms	frontLoop
	bne	frontLoop
	bpl	frontLoop
	bra	frontLoop
	brn	frontLoop

subRoutine:
	rts    

	brset		0,PTB,frontLoop
	brclr		0,PTC,subRoutine
	brset           1,PTB,frontLoop
	brclr		1,PTC,subRoutine
	brset           2,PTB,frontLoop
	brclr		2,PTC,subRoutine
	brset           3,PTB,frontLoop
	brclr		3,PTC,subRoutine
	brset           4,PTB,frontLoop
	brclr		4,PTC,subRoutine
	brset           5,PTB,frontLoop
	brclr		5,PTC,subRoutine
	brset           6,PTB,frontLoop
	brclr		6,PTC,subRoutine
	brset           7,PTB,frontLoop
	brclr		7,PTC,subRoutine

frontLoop:

	bset		0,PTB
	bclr		0,PTC
	bset		1,PTB
	bclr		1,PTC
	bset		2,PTB
	bclr		2,PTC
	bset		3,PTB
	bclr		3,PTC
	bset		4,PTB
	bclr		4,PTC
	bset		5,PTB
	bclr		5,PTC
	bset		6,PTB
	bclr		6,PTC
	bset		7,PTB
	bclr		7,PTC

	bsr 	subRoutine

	cbeqA		$20,frontLoop	
	cbeq		9,frontLoop
	cbeq		(0xA,X),	frontLoop
	cbeq		[SP+0x0B],frontLoop
	cbeq.ix		0, frontLoop		; must add forced-address-mode extension to get opcode 0x71
	cbeqX		0x40,frontLoop	

	clc
	cli
;frontLoop:

	clr	RAM_START
	clr	3,X
	clr	,X
	clr	5,SP
	clrA
	clrH
	clrX
	
	cmp	#6
	cmp	RAM_START
	cmp	7,X
	cmp	constant0
	cmp	0x300,X
	cmp	,X
	cmp	5,SP
	cmp	500,SP

	com	RAM_START
	com	3,X
	com	,X
	com	5,SP
	comA
	comX
	
	cpHX	RAM_START
	cpHX	#constant0

	cpX	#6
	cpX	RAM_START
	cpX	7,X
	cpX	constant0
	cpX	0x300,X
	cpX	,X
	cpX	5,SP
	cpX	FIVE_HUNDRED,SP

	daa			; decimal adjust after BCD calculation

backLoop:
	dbnzA   backLoop
	dbnz	4,backLoop
	dbnz	[X+5],backLoop
	dbnz	[SP+6],backLoop
	dbnz.ix		0,backLoop	; must add forced-address-mode extension and dummy 0 to get opcode 0x7B
	dbnzX	backLoop

	dec	RAM_START
	dec	3,X
	dec	,X
	dec	5,SP
	decA
	decX

	div
	
	mul
	nsa
	ble	backLoop
	bgt	backLoop
	blt	backLoop
	bge	backLoop
	wait
	stop

	eor	#6
	eor	RAM_START
	eor	7,X
	eor	constant0
	eor	0x300,X
	eor	,X
	eor	5,SP
	eor	500,SP

	inc	RAM_START
	inc	3,X
	inc	,X
	inc	5,SP
	incA
	incX

	jmp 	RAM_START
	jmp 	vector0	
	jmp	3,X
	jmp 	$300,X
	jmp	,X

	jsr	RAM_START
	jsr	vector0
	jsr	3,X
	jsr	0x300,X
	jsr	,X

	ldA	#6
	ldA	RAM_START
	ldA	7,X
	ldA	constant0
	ldA	0x300,X
	ldA	,X
	ldA	5,SP
	ldA	500,SP
	
	ldHX	#500
	ldHX	RAM_START

	ldX	#55
	ldX	RAM_START
	ldX	7,X
	ldX	constant0
	ldX	0x300,X
	ldX	,X
	ldX	5,SP
	ldX	500,SP
	
	lsl	RAM_START
	lsl	3,X
	lsl	,X
	lsl	5,SP	
	lslA
	lslX
	
	lsr	RAM_START
	lsr	3,X
	lsr	,X
	lsr	5,SP
	lsrA
	lsrX
	
	
	mov	#5, RAM_START
	movp	RAM_START,X
	mov	10, RAM_START
	xmov	RAM_START
	
	neg	RAM_START
	neg	3,X
	neg	,X
	neg	5,SP	
	negA
	negX

	nop
	
	orA	#6
	orA	RAM_START
	orA	7,X
	orA	constant0
	orA	0x300,X
	orA	,X
	orA	5,SP
	orA	500,SP
	
	pshA
	pshH
	pshX
	pulA
	pulH
	pulX

	rol	RAM_START
	rol	3,X
	rol	,X
	rol	5,SP
	rolA
	rolX

	ror	RAM_START
	ror	3,X
	ror	,X
	ror	5,SP
	rorA
	rorX

	rsp
    
dummyISR:	
	rti
	
	sbc	#6
	sbc	RAM_START
	sbc	7,X
	sbc	constant0
	sbc	0x300,X
	sbc	,X
	sbc	5,SP
	sbc	500,SP
	
	sec	
	sei	

	stA	RAM_START
	stA	7,X
	stA	constant0
	stA	0x300,X
	stA	,X
	stA	5,SP
	stA	500,SP
	
	stHX	RAM_START
	
	stX	RAM_START
	stX	7,X
	stX	constant0
	stX	0x300,X
	stX	,X
	stX	5,SP
	stX	500,SP	

	sub	#6
	sub	RAM_START
	sub	7,X
	sub	constant0
	sub	0x300,X
	sub	,X
	sub	5,SP
	sub	500,SP

	swi
	tAP
	tAX
	tHXS
	tPA
	tSHX

	tst	RAM_START
	tst	3,X
	tst	,X
	tst	4,SP

	tstA
	tstX
	tXA

;	.ORG		0xFE00
constant0:
	.word		0
vector0:
	.word		dummyISR


