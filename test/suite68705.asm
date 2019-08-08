
    processor 68705

    org 0
    adc #10
    adc 10
    adc 1000
    adc ,x
    adc 1,x
    adc 1000,x
    add #10
    add 10
    add 1000
    add ,x
    add 1,x
    add 1000,x

    and #10
    and 10
    and 1000
    and ,x
    and 1,x
    and 1000,x

    asla
    aslx
    asl 10
    asl ,x
    asl 10,x

    asra
    asrx
    asr 10
    asr ,x
    asr 10,x

    bcc .
    bclr    1,23
    bcs .
    beq .
    bhcc .
    bhcs .
    bhi .
    bhs .
    bih .
    bil .

    bit #10
    bit 10
    bit 1000
    bit ,x
    bit 1,x
    bit 1000,x

    blo .
    bls .
    bmc .
    bmi .
    bms .
    bne .
    bpl .
    bra .
    brn .
    brclr    1,10,.
    brset    1,10,.
    bset    1,10
    bsr .

    clc
    cli

    clra
    clrx
    clr 10
    clr ,x
    clr 10,x

    cmp #10
    cmp 10
    cmp 1000
    cmp ,x
    cmp 1,x
    cmp 1000,x

    coma
    comx
    com 10
    com ,x
    com 10,x

    cpx #10
    cpx 10
    cpx 1000
    cpx ,x
    cpx 10,x
    cpx 1000,x

    deca
    decx
    dec 10
    dec ,x
    dec 10,x

    eor #10
    eor 10
    eor 1000
    eor ,x
    eor 10,x
    eor 1000,x

    inca
    incx
    inc 10
    inc ,x
    inc 10,x

    jmp 10
    jmp 1000
    jmp ,x
    jmp 10,x
    jmp 1000,x

    jsr 10
    jsr 1000
    jsr ,x
    jsr 10,x
    jsr 1000,x

    lda #10
    lda 10
    lda 1000
    lda ,x
    lda 10,x
    lda 1000,x

    ldx #10
    ldx 10
    ldx 1000
    ldx ,x
    ldx 10,x
    ldx 1000,x

    lsla
    lslx
    lsl 10
    lsl ,x
    lsl 10,x

    lsra
    lsrx
    lsr 10
    lsr ,x
    lsr 10,x

    nega
    negx
    neg 10
    neg ,x
    neg 10,x

    nop

    ora #10
    ora 10
    ora 1000
    ora ,x
    ora 10,x
    ora 1000,x

    rola
    rolx
    rol 10
    rol ,x
    rol 10,x

    rora
    rorx
    ror 10
    ror ,x
    ror 10,x

    rsp
    rti
    rts

    sbc #10
    sbc 10
    sbc 1000
    sbc ,x
    sbc 10,x
    sbc 1000,x

    sec
    sei

    sta 10
    sta 1000
    sta ,x
    sta 10,x
    sta 1000,x

    stx 10
    stx 1000
    stx ,x
    stx 10,x
    stx 1000,x

    sub #10
    sub 10
    sub 1000
    sub ,x
    sub 10,x
    sub 1000,x

    swi
    tax

    tsta
    tstx
    tst 10
    tst ,x
    tst 10,x

    txa

