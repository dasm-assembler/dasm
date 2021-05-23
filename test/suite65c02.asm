
	;   TEST ADDRESSING MODES

	processor   65c02

	org	0

	adc	#1
	adc	1
	adc	1,x
	adc	1,y	    ;absolute
	adc	1000
	adc	1000,x
	adc	1000,y
	adc	(1,x)
	adc	(1),y
	adc (1)

	and	#1
	and	1
	and	1,x
	and	1,y	    ;absolute
	and	1000
	and	1000,x
	and	1000,y
	and	(1,x)
	and	(1),y
	and (1)

	asl
	asl	1
	asl	1,x
	asl	1000
	asl	1000,x

	bcc	.
	bcs	.
	beq	.
	bit #$1
	bit	1
	bit 1,x
	bit	1000
	bit 1000,x
	bmi	.
	bne	.
	bpl	.
	bra .
	brk
	bvc	.
	bvs	.

	bbr0 1,.
	bbr1 1,.
	bbr2 1,.
	bbr3 1,.
	bbr4 1,.
	bbr5 1,.
	bbr6 1,.
	bbr7 1,.
	bbs0 1,.
	bbs1 1,.
	bbs2 1,.
	bbs3 1,.
	bbs4 1,.
	bbs5 1,.
	bbs6 1,.
	bbs7 1,.
	
	clc
	cld
	cli
	clv

	cmp	#1
	cmp	1
	cmp	1,x
	cmp	1,y	    ;absolute
	cmp	1000
	cmp	1000,x
	cmp	1000,y
	cmp	(1,x)
	cmp	(1),y
	cmp (1)

	cpx	#1
	cpx	1
	cpx	1000

	cpy	#1
	cpy	1
	cpy	1000

	dec	1
	dec	1,x
	dec	1000
	dec	1000,x

	dea
	dex
	dey

	eor	#1
	eor	1
	eor	1,x
	eor	1,y	    ;absolute
	eor	1000
	eor	1000,x
	eor	1000,y
	eor	(1,x)
	eor	(1),y
	eor (1)

	inc	1
	inc	1,x
	inc	1000
	inc	1000,x

	ina
	inx
	iny

	jmp	1	    ;absolute
	jmp	1000
	jmp	(1)         ;absolute
	jmp	(1000)
	jmp (1000,x)

	jsr	1	    ;absolute
	jsr	1000

	lda	#1
	lda	1
	lda	1,x
	lda	1,y	    ;absolute
	lda	1000
	lda	1000,x
	lda	1000,y
	lda	(1,x)
	lda	(1),y
	lda (1)

	ldx	#1
	ldx	1
	ldx	1,y
	ldx	1000
	ldx	1000,y

	ldy	#1
	ldy	1
	ldy	1,x
	ldy	1000
	ldy	1000,x

	lsr
	lsr	1
	lsr	1,x
	lsr	1000
	lsr	1000,x

	nop

	ora	#1
	ora	1
	ora	1,x
	ora	1,y	    ;absolute
	ora	1000
	ora	1000,x
	ora	1000,y
	ora	(1,x)
	ora	(1),y
	ora (1)

	pha
	php
	phx
	phy
	pla
	plp
	plx
	ply

	rol
	rol	1
	rol	1,x
	rol	1000
	rol	1000,x

	ror
	ror	1
	ror	1,x
	ror	1000
	ror	1000,x

	rmb0 1
	rmb1 1
	rmb2 1
	rmb3 1
	rmb4 1
	rmb5 1
	rmb6 1
	rmb7 1

	rti
	rts

	sbc	#1
	sbc	1
	sbc	1,x
	sbc	1,y	    ;absolute
	sbc	1000
	sbc	1000,x
	sbc	1000,y
	sbc	(1,x)
	sbc	(1),y
	sbc (1)

	sec
	sed
	sei

	smb0 1
	smb1 1
	smb2 1
	smb3 1
	smb4 1
	smb5 1
	smb6 1
	smb7 1

	sta	1
	sta	1,x
	sta	1,y	    ;absolute
	sta	1000
	sta	1000,x
	sta	1000,y
	sta	(1,x)
	sta	(1),y
	sta (1)

	stp

	stx	1
	stx	1,y
	stx	1000

	sty	1
	sty	1,x
	sty	1000

	stz 1
	stz 1,x
	stz 1000
	stz 1000,x

	tax
	tay
	tsx
	txa
	txs
	tya

	trb 1
	trb 1000
	tsb 1
	tsb 1000

	wai


