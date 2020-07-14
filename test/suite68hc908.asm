;
;   regression test for all opcodes for 68hc908 controller 
;   
;   two opcodes with indirect X adressing are not yet implemented
;	dbnz	(X), jumpLabel	
;	cbeq	(X), jumpLabel
;			
;    one must add 0 to circumvent this problem, the code will become one (that zero) byte larger
;	dbnz	(0,X), jumpLabel
;	cbeq	(0,X), jumpLabel
;

	.PROCESSOR	68hc908

PTB	.EQU		1
DDRB	.EQU		5


RAM_START	.EQU	0x40

	.ORG		0xFB00

	.byte		0xba,0xdc,0x0d,0xe0

	adc	#6
	adc.b	RAM_START
	adc	7,X
	adc	constant0
	adc	0x300,X
	adc	,X
	adc	5,SP
	adc.w	500,SP

	add	#6
	add.b	RAM_START
	add	7,X
	add	constant0
	add	0x300,X
	add	,X
	add	5,SP
	add.w	500,SP

	aiS		#5
	aiX		#6
	
	and	#6
	and.b	RAM_START
	and	7,X
	and	constant0
	and	0x300,X
	and	,X
	and	5,SP
	and.w	500,SP

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
	bit.b	RAM_START
	bit	7,X
	bit	constant0
	bit	0x300,X
	bit	,X
	bit	5,SP
	bit.w	500,SP
	
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
	brset           1,PTB,frontLoop
	brset           2,PTB,frontLoop
	brset           3,PTB,frontLoop
	brset           4,PTB,frontLoop
	brset           5,PTB,frontLoop
	brset           6,PTB,frontLoop
	brset           7,PTB,frontLoop

	bset		0,PTB
	bset		1,PTB
	bset		2,PTB
	bset		3,PTB
	bset		4,PTB
	bset		5,PTB
	bset		6,PTB
	bset		7,PTB

	bsr 	subRoutine

	cbeqA		$20,frontLoop	
	cbeq		9,frontLoop
	cbeq		(0xA,X),frontLoop
	cbeq		0xB,SP,frontLoop
	cbeqX		0x40,frontLoop	

	clc
	cli
frontLoop:

	clr	RAM_START
	clr	3,X
	clr	,X
	clr	5,SP
	clrA
	clrH
	clrX
	
	cmp	#6
	cmp.b	RAM_START
	cmp	7,X
	cmp	constant0
	cmp	0x300,X
	cmp	,X
	cmp	5,SP
	cmp.w	500,SP

	com	RAM_START
	com	3,X
	com	,X
	com	5,SP
	comA
	comX
	
	cpHX	RAM_START
	cpHX	#constant0

	cpX	#6
	cpX.b	RAM_START
	cpX	7,X
	cpX	constant0
	cpX	0x300,X
	cpX	,X
	cpX	5,SP
	cpX.w	500,SP
	
backLoop:
	dbnzA   backLoop
	dbnz	4,backLoop
	dbnz	5,X,backLoop
;	dbnz.x	backLoop
	dbnz	6,SP,backLoop
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
	eor.b	RAM_START
	eor	7,X
	eor	constant0
	eor	0x300,X
	eor	,X
	eor	5,SP
	eor.w	500,SP

	inc	RAM_START
	inc	3,X
	inc	,X
	inc	5,SP
	incA
	incX

	jmp.8 	RAM_START
	jmp 	vector0	
	jmp	3,X
	jmp 	$300,X
	jmp	,X

	jsr.8	RAM_START
	jsr	vector0
	jsr	3,X
	jsr	0x300,X
	jsr	,X

	ldA	#6
	ldA.b	RAM_START
	ldA	7,X
	ldA	constant0
	ldA	0x300,X
	ldA	,X
	ldA	5,SP
	ldA.w	500,SP
	
	ldHX	#500
	ldHX	RAM_START

	ldX	#55
	ldX.b	RAM_START
	ldX	7,X
	ldX	constant0
	ldX	0x300,X
	ldX	,X
	ldX	5,SP
	ldX.w	500,SP
	
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
	orA.b	RAM_START
	orA	7,X
	orA	constant0
	orA	0x300,X
	orA	,X
	orA	5,SP
	orA.w	500,SP
	
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
	sbc.b	RAM_START
	sbc	7,X
	sbc	constant0
	sbc	0x300,X
	sbc	,X
	sbc	5,SP
	sbc.w	500,SP
	
	sec	
	sei	

	stA.b	RAM_START
	stA	7,X
	stA	constant0
	stA	0x300,X
	stA	,X
	stA	5,SP
	stA.w	500,SP
	
	stHX	RAM_START
	
	stX.b	RAM_START
	stX	7,X
	stX	constant0
	stX	0x300,X
	stX	,X
	stX	5,SP
	stX.w	500,SP	

	sub	#6
	sub.b 	RAM_START
	sub	7,X
	sub	constant0
	sub	0x300,X
	sub	,X
	sub	5,SP
	sub.w	500,SP

	swi
	tAP
	tAX
	tHXS
	tPA
	tSHX

	tst.8	RAM_START
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


