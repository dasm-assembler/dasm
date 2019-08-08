

	;   TEST ADDRESSING MODES

	processor   68HC11

	org	0

	aba
	abx
	aby
	adca	#1
	adca	1
	adca	1000
	adca	1,x
	adca	1,y
	adcb	#1
	adcb	1
	adcb	1000
	adcb	1,x
	adcb	1,y
	adda	#1
	adda	1
	adda	1000
	adda	1,x
	adda	1,y
	addb	#1
	addb	1
	addb	1000
	addb	1,x
	addb	1,y
	addd	#1000
	addd	1
	addd	1000
	addd	1,x
	addd	1,y
	anda	#1
	anda	1
	anda	1000
	anda	1,x
	anda	1,y
	andb	#1
	andb	1
	andb	1000
	andb	1,x
	andb	1,y
	asla
	aslb
	asl	1	;extended
	asl	1000
	asl	1,x
	asl	1,y
	asld
	asra
	asrb
	asr	1	;extended
	asr	1000
	asr	1,x
	asr	1,y
	bcc	.
	bclr	1, $11
	bclr	1,x ,$22
	bclr	1,y ,$33
	bcs	.
	beq	.
	bge	.
	bgt	.
	bhi	.
	bhs	.
	bita	#1
	bita	1
	bita	1000
	bita	1,x
	bita	1,y
	bitb	#1
	bitb	1
	bitb	1000
	bitb	1,x
	bitb	1,y
	ble	.
	blo	.
	bls	.
	blt	.
	bmi	.
	bne	.
	bpl	.
	bra	.
	brclr	1, $11, .	; br??? direct-location, mask, branch-to
	brclr	1,x , $22, .
	brclr	1,y , $33, .
	brn	.
	brset	1, $11, .
	brset	1,x , $22, .
	brset	1,y , $33, .
	bset	1, $11
	bset	1,x, $22
	bset	1,y, $33
	bsr	.
	bvc	.
	bvs	.
	cba
	clc
	cli
	clra
	clrb
	clr	1	    ;  extended
	clr	1000
	clr	1,x
	clr	1,y
	clv
	cmpa	#1
	cmpa	1
	cmpa	1000
	cmpa	1,x
	cmpa	1,y
	cmpb	#1
	cmpb	1
	cmpb	1000
	cmpb	1,x
	cmpb	1,y
	coma
	comb
	com	1	    ;	extended
	com	1000
	com	1,x
	com	1,y
	cpd	#1	    ;	16 bits
	cpd	#1000
	cpd	1
	cpd	1000
	cpd	1,x
	cpd	1,y
	cpx	#1	    ;	16 bits
	cpx	#1000
	cpx	1
	cpx	1000
	cpx	1,x
	cpx	1,y
	cpy	#1	    ;	16 bits
	cpy	#1000
	cpy	1
	cpy	1000
	cpy	1,x
	cpy	1,y
	daa
	deca
	decb
	dec	1	    ;	extended
	dec	1000
	dec	1,x
	dec	1,y
	des
	dex
	dey
	eora	#1
	eora	1
	eora	1000
	eora	1,x
	eora	1,y
	eorb	#1
	eorb	1
	eorb	1000
	eorb	1,x
	eorb	1,y
	fdiv
	idiv
	inca
	incb
	inc	1	;extended
	inc	1000
	inc	1,x
	inc	1,y
	ins
	inx
	iny
	jmp	1	;extended
	jmp	1000
	jmp	1,x
	jmp	1,y
	jsr	1
	jsr	1000
	jsr	1,x
	jsr	1,y
	ldaa	#1
	ldaa	1
	ldaa	1000
	ldaa	1,x
	ldaa	1,y
	ldab	#1
	ldab	1
	ldab	1000
	ldab	1,x
	ldab	1,y
	ldd	#1	;16 bits
	ldd	#1000
	ldd	1
	ldd	1000
	ldd	1,x
	ldd	1,y
	lds	#1	;16 bits
	lds	#1000
	lds	1
	lds	1000
	lds	1,x
	lds	1,y
	ldx	#1	;16 bits
	ldx	#1000
	ldx	1
	ldx	1000
	ldx	1,x
	ldx	1,y
	ldy	#1	;16 bits
	ldy	#1000
	ldy	1
	ldy	1000
	ldy	1,x
	ldy	1,y
	lsla
	lslb
	lsl	1	;extended
	lsl	1000
	lsl	1,x
	lsl	1,y
	lsld
	lsra
	lsrb
	lsr	1	;extended
	lsr	1000
	lsr	1,x
	lsr	1,y
	lsrd
	mul
	nega
	negb
	neg	1	;extended
	neg	1000
	neg	1,x
	neg	1,y
	nop
	oraa	#1
	oraa	1
	oraa	1000
	oraa	1,x
	oraa	1,y
	orab	#1
	orab	1
	orab	1000
	orab	1,x
	orab	1,y
	psha
	pshb
	pshx
	pshy
	pula
	pulb
	pulx
	puly
	rola
	rolb
	rol	1	;extended
	rol	1000
	rol	1,x
	rol	1,y
	rora
	rorb
	ror	1	;extended
	ror	1000
	ror	1,x
	ror	1,y
	rti
	rts
	sba
	sbca	#1
	sbca	1
	sbca	1000
	sbca	1,x
	sbca	1,y
	sbcb	#1
	sbcb	1
	sbcb	1000
	sbcb	1,x
	sbcb	1,y
	sec
	sei
	sev
	staa	1
	staa	1000
	staa	1,x
	staa	1,y
	stab	1
	stab	1000
	stab	1,x
	stab	1,y
	std	1
	std	1000
	std	1,x
	std	1,y
	stop
	sts	1
	sts	1000
	sts	1,x
	sts	1,y
	stx	1
	stx	1000
	stx	1,x
	stx	1,y
	sty	1
	sty	1000
	sty	1,x
	sty	1,y
	suba	#1
	suba	1
	suba	1000
	suba	1,x
	suba	1,y
	subb	#1
	subb	1
	subb	1000
	subb	1,x
	subb	1,y
	subd	#1	;extended
	subd	#1000
	subd	1
	subd	1000
	subd	1,x
	subd	1,y
	swi
	tab
	tap
	tba
	test
	tpa
	tsta
	tstb
	tst	1	;extended
	tst	1000
	tst	1,x
	tst	1,y
	tsx
	tsy
	txs
	tys
	wai
	xgdx
	xgdy














