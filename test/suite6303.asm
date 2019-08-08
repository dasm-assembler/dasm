
    processor   hd6303
    org 0

    adda    #10    ;   8B 0A
    adda    10    ;   9B 0A
    adda    10,x    ;   AB 0A
    adda    1000    ;   BB 03 E8
    addb    #10    ;   CB 0A
    addb    10    ;   DB 0A
    addb    10,x    ;   EB 0A
    addb    1000    ;   FB 03 E8
    addd    #1000    ;   C3 03 E8
    addd    10    ;   D3 0A
    addd    10,x    ;   E3 0A
    addd    1000    ;   F3 03 E8
    aba        ;   1B
    adca    #10    ;   89 0A
    adca    10    ;   99 0A
    adca    10,x    ;   A9 0A
    adca    1000    ;   B9 03 E8
    adcb    #10    ;   C9
    adcb    10    ;   D9
    adcb    10,x    ;   E9
    adcb    1000    ;   F9 03 E8
    anda    #10    ;   84 0A
    anda    10    ;   94 0A
    anda    10,x    ;   A4 0A
    anda    1000    ;   B4 03 E8
    andb    #10    ;   C4 0A
    andb    10    ;   D4 0A
    andb    10,x    ;   E4 0A
    andb    1000    ;   F4 03 E8
    bita    #10    ;   85 0A
    bita    10    ;   95 0A
    bita    10,x    ;   A5 0A
    bita    1000    ;   B5 03 E8
    bitb    #10    ;   C5 0A
    bitb    10    ;   D5 0A
    bitb    10,x    ;   E5 0A
    bitb    1000    ;   F5 03 E8
    clr    10,x    ;   6F 0A
    clr    1000    ;   7F 03 E8
    clra        ;   4F
    clrb        ;   5F
    cmpa    #10    ;   81 0A
    cmpa    10    ;   91 0A
    cmpa    10,x    ;   A1 0A
    cmpa    1000    ;   B1 03 E8
    cmpb    #10    ;   C1 0A
    cmpb    10    ;   D1 0A
    cmpb    10,x    ;   E1 0A
    cmpb    1000    ;   F1 03 E8
    cba        ;   11
    com    10,x    ;   63 0A
    com    1000    ;   73 03 E8
    coma        ;   43
    comb        ;   53
    neg    10,x    ;   60 0A
    neg    1000    ;   70 03 E8
    nega        ;   40
    negb        ;   50
    daa        ;   19
    dec    10,x    ;   6A 0A
    dec    1000    ;   7A 03 E8
    deca        ;   4A
    decb        ;   5A
    eora    #10    ;   88 0A
    eora    10    ;   98 0A
    eora    10,x    ;   A8 0A
    eora    1000    ;   B8 03 E8
    eorb    #10    ;   C8 0A
    eorb    10    ;   D8 0A
    eorb    10,x    ;   E8 0A
    eorb    1000    ;   F8 03 E8
    inc    10,x    ;   6C 0A
    inc    1000    ;   7C 03 E8
    inca        ;   4C
    incb        ;   5C
    ldaa    #10    ;   86 0A
    ldaa    10    ;   96 0A
    ldaa    10,x    ;   A6 0A
    ldaa    1000    ;   B6 03 E8
    ldab    #10    ;   C6 0A
    ldab    10    ;   D6 0A
    ldab    10,x    ;   E6 0A
    ldab    1000    ;   F6 03 E8
    ldd    #1000    ;   CC 03 E8
    ldd    10    ;   DC 0A
    ldd    10,x    ;   EC 0A
    ldd    1000    ;   FC 03 E8
    mul        ;   3D
    oraa    #10    ;   8A 0A
    oraa    10    ;   9A 0A
    oraa    10,x    ;   AA 0A
    oraa    1000    ;   BA 03 E8
    orab    #10    ;   CA 0A
    orab    10    ;   DA 0A
    orab    10,x    ;   EA 0A
    orab    1000    ;   FA 03 E8
    psha        ;   36
    pshb        ;   37
    pula        ;   32
    pulb        ;   33
    rol    10,x    ;   69 0A
    rol    1000    ;   79 03 E8
    rola        ;   49
    rolb        ;   59
    ror    10,x    ;   66 0A
    ror    1000    ;   76 03 E8
    rora        ;   46
    rorb        ;   56
    asl    10,x    ;   68 0A
    asl    1000    ;   78 03 E8
    asla        ;   48
    aslb        ;   58
    asld        ;   05
    asr    10,x    ;   67 0A
    asr    1000    ;   77 03 E8
    asra        ;   47
    asrb        ;   57
    lsr    10,x    ;   64 0A
    lsr    1000    ;   74 03 E8
    lsra        ;   44
    lsrb        ;   54
    lsrd        ;   04
    staa    10    ;   97 0A
    staa    10,x    ;   A7 0A
    staa    1000    ;   B7 03 E8
    stab    10    ;   D7 0A
    stab    10,x    ;   E7 0A
    stab    1000    ;   F7 03 E8
    std    10    ;   DD 0A
    std    10,x    ;   ED 0A
    std    1000    ;   FD 03 E8
    suba    #10    ;   80 0A
    suba    10    ;   90 0A
    suba    10,x    ;   A0 0A
    suba    1000    ;   B0 03 E8
    subb    #10    ;   C0 0A
    subb    10    ;   D0 0A
    subb    10,x    ;   E0 0A
    subb    1000    ;   F0 03 E8
    subd    #1000    ;   83 03 E8
    subd    10    ;   93 0A
    subd    10,x    ;   A3 0A
    subd    1000    ;   B3 03 E8
    sba        ;   10
    sbca    #10    ;   82 0A
    sbca    10    ;   92 0A
    sbca    10,x    ;   A2 0A
    sbca    1000    ;   B2 03 E8
    sbcb    #10    ;   C2 0A
    sbcb    10    ;   D2 0A
    sbcb    10,x    ;   E2 0A
    sbcb    1000    ;   F2 03 E8
    tab        ;   16
    tba        ;   17
    tst    10,x    ;   6D 0A
    tst    1000    ;   7D 03 E8
    tsta        ;   4D
    tstb        ;   5D
    aim    10    ;   71 0A
    aim    10,x    ;   61 0A
    oim    10    ;   72 0A
    oim    10,x    ;   62 0A
    eim    10    ;   75 0A
    eim    10,x    ;   65 0A
    tim    10    ;   7B 0A
    tim    10,x    ;   6B 0A
    cpx    #1000    ;   8C 03 E8
    cpx    10    ;   9C 0A
    cpx    10,x    ;   AC 0A
    cpx    1000    ;   BC 03 E8
    dex        ;   09
    des        ;   34
    inx        ;   08
    ins        ;   31
    ldx    #1000    ;   CE 03 E8
    ldx    10    ;   DE 0A
    ldx    10,x    ;   EE 0A
    ldx    1000    ;   FE 03 E8
    lds    #1000    ;   8E 03 E8
    lds    10    ;   9E 0A
    lds    10,x    ;   AE 0A
    lds    1000    ;   BE 03 E8
    stx    10    ;   DF 0A
    stx    10,x    ;   EF 0A
    stx    1000    ;   FF 03 E8
    sts    10    ;   9F 0A
    sts    10,x    ;   AF 0A
    sts    1000    ;   BF 03 E8
    txs        ;   35
    tsx        ;   30
    abx        ;   3A
    pshx        ;   3C
    pulx        ;   38
    xgdx        ;   18
    bra    .    ;   20 FE
    brn    .    ;   21 FE
    bcc    .    ;   24 FE
    bcs    .    ;   25 FE
    beq    .    ;   27 FE
    bge    .    ;   2C FE
    bgt    .    ;   2E FE
    bhi    .    ;   22 FE
    ble    .    ;   2F FE
    bls    .    ;   23 FE
    blt    .    ;   2D FE
    bmi    .    ;   2B FE
    bne    .    ;   26 FE
    bvc    .    ;   28 FE
    bvs    .    ;   29 FE
    bpl    .    ;   2A FE
    bsr    .    ;   8D FE
    jmp    10,x    ;   6E 0A
    jmp    1000    ;   7E 03 E8
    jsr    10    ;   9D 0A
    jsr    10,x    ;   AD 0A
    jsr    1000    ;   BD 03 E8
    nop        ;   01
    rti        ;   3B
    rts        ;   39
    swi        ;   3F
    wai        ;   3E
    slp        ;   1A
    clc        ;   0C
    cli        ;   0E
    clv        ;   0A
    sec        ;   0D
    sei        ;   0F
    sev        ;   0B
    tap        ;   06
    tpa        ;   07

