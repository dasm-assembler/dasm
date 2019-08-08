; Fairchild F8 test suite for DASM
; 2004 by Thomas Mathys
        processor f8

TWO     =       2

        org     0
start

        adc                     ; 8e
        ai      $f8             ; 24 f8
        am                      ; 88
        amd                     ; 89

        as      start           ; c0
        as      start+1         ; c1
        as      TWO             ; c2
        as      3               ; c3
        as      4               ; c4
        as      5               ; c5
        as      2*[1+2]         ; c6
        as      7               ; c7
        as      (2+2)*2         ; c8
        as      9               ; c9
        as      j               ; c9
        as      $a              ; ca
        as      hu              ; ca
        as      11              ; cb
        as      hl              ; cb
        as      s               ; cc
        as      (is)            ; cc
        as      i               ; cd
        as      (is)+           ; cd
        as      d               ; ce
        as      (is)-           ; ce

        asd     start           ; d0
        asd     start+1         ; d1
        asd     TWO             ; d2
        asd     3               ; d3
        asd     4               ; d4
        asd     5               ; d5
        asd     2*[1+2]         ; d6
        asd     7               ; d7
        asd     (2+2)*2         ; d8
        asd     9               ; d9
        asd     j               ; d9
        asd     $a              ; da
        asd     hu              ; da
        asd     11              ; db
        asd     hl              ; db
        asd     s               ; dc
        asd     (is)            ; dc
        asd     i               ; dd
        asd     (is)+           ; dd
        asd     d               ; de
        asd     (is)-           ; de

        bc      .               ; 82 ff
        bf      12,.            ; 9c ff
        bm      .               ; 91 ff
        bnc     .               ; 92 ff
        bno     .               ; 98 ff
        bnz     .               ; 94 ff
        bp      .               ; 81 ff
        br      .               ; 90 ff
        br7     .               ; 8f ff
        bt      5,.             ; 85 ff
        bz      .               ; 84 ff

        ci      4096/64-1       ; 25 3f
        clr                     ; 70
        cm                      ; 8d
        com                     ; 18
        dci     TWO             ; 2a 00 02
        dci     $1234           ; 2a 12 34
        di                      ; 1a

        ds      start           ; 30
        ds      start+1         ; 31
        ds      TWO             ; 32
        ds      3               ; 33
        ds      4               ; 34
        ds      5               ; 35
        ds      2*[1+2]         ; 36
        ds      7               ; 37
        ds      (2+2)*2         ; 38
        ds      9               ; 39
        ds      j               ; 39
        ds      $a              ; 3a
        ds      hu              ; 3a
        ds      11              ; 3b
        ds      hl              ; 3b
        ds      s               ; 3c
        ds      (is)            ; 3c
        ds      i               ; 3d
        ds      (is)+           ; 3d
        ds      d               ; 3e
        ds      (is)-           ; 3e

        ei                      ; 1b
        in      TWO             ; 26 02
        in      16*16-1         ; 26 ff
        inc                     ; 1f
        ins     5*3-15          ; a0
        ins     15+start        ; af
        jmp     $1234           ; 29 12 34
        li      TWO             ; 20 02
        lis     %111            ; 77
        lisl    (TWO+TWO)*TWO-1 ; 6f
        lisu    4               ; 64
        lm                      ; 16
        lnk                     ; 19

        lr      start,a         ; 50
        lr      start+1,a       ; 51
        lr      TWO,a           ; 52
        lr      3,a             ; 53
        lr      4,a             ; 54
        lr      5,a             ; 55
        lr      2*[1+2],a       ; 56
        lr      7,a             ; 57
        lr      (2+2)*2,a       ; 58
        lr      9,a             ; 59
        lr      j,a             ; 59
        lr      $a,a            ; 5a
        lr      hu,a            ; 5a
        lr      11,a            ; 5b
        lr      hl,a            ; 5b
        lr      s,a             ; 5c
        lr      (is),a          ; 5c
        lr      i,a             ; 5d
        lr      (is)+,a         ; 5d
        lr      d,a             ; 5e
        lr      (is)-,a         ; 5e

        lr      a,start         ; 40
        lr      a,start+1       ; 41
        lr      a,TWO           ; 42
        lr      a,3             ; 43
        lr      a,4             ; 44
        lr      a,5             ; 45
        lr      a,2*[1+2]       ; 46
        lr      a,7             ; 47
        lr      a,(2+2)*2       ; 48
        lr      a,9             ; 49
        lr      a,j             ; 49
        lr      a,$a            ; 4a
        lr      a,hu            ; 4a
        lr      a,11            ; 4b
        lr      a,hl            ; 4b
        lr      a,s             ; 4c
        lr      a,(is)          ; 4c
        lr      a,i             ; 4d
        lr      a,(is)+         ; 4d
        lr      a,d             ; 4e
        lr      a,(is)-         ; 4e

        lr      a,ku            ; 00
        lr      a,kl            ; 01
        lr      a,qu            ; 02
        lr      a,ql            ; 03
        lr      ku,a            ; 04
        lr      kl,a            ; 05
        lr      qu,a            ; 06
        lr      ql,a            ; 07
        lr      h,dc0           ; 11
        lr      h,dc            ; 11
        lr      q,dc0           ; 0e
        lr      q,dc            ; 0e
        lr      dc0,h           ; 10
        lr      dc,h            ; 10
        lr      dc0,q           ; 0f
        lr      dc,q            ; 0f
        lr      k,pc1           ; 08
        lr      k,p             ; 08
        lr      pc1,k           ; 09
        lr      p,k             ; 09
        lr      pc0,q           ; 0d
        lr      p0,q            ; 0d
        lr      a,is            ; 0a
        lr      is,a            ; 0b
        lr      w,j             ; 1d
        lr      w,(TWO+1)*3     ; 1d
        lr      9,w             ; 1e
        lr      j,w             ; 1e

        ni      %10000001       ; 21 81
        nm                      ; 8a
        nop                     ; 2b

        ns      start           ; f0
        ns      start+1         ; f1
        ns      TWO             ; f2
        ns      3               ; f3
        ns      4               ; f4
        ns      5               ; f5
        ns      2*[1+2]         ; f6
        ns      7               ; f7
        ns      (2+2)*2         ; f8
        ns      9               ; f9
        ns      j               ; f9
        ns      $a              ; fa
        ns      hu              ; fa
        ns      11              ; fb
        ns      hl              ; fb
        ns      s               ; fc
        ns      (is)            ; fc
        ns      i               ; fd
        ns      (is)+           ; fd
        ns      d               ; fe
        ns      (is)-           ; fe

        oi      (TWO+start)*64  ; 22 80
        om                      ; 8b

        out     TWO             ; 27 02
        out     16*16-1         ; 27 ff
        outs    5*3-15          ; b0
        outs    15+start        ; bf

        pi      start           ; 28 00 00
        pi      $1234           ; 28 12 34
        pk                      ; 0c
        pop                     ; 1c
        sl      start+1         ; 13
        sl      TWO+TWO         ; 15
        sr      1               ; 12
        sr      3+1             ; 14
        st                      ; 17
        xdc                     ; 2c
        xi      64              ; 23 40
        xm                      ; 8c

        xs      start           ; e0
        xs      start+1         ; e1
        xs      TWO             ; e2
        xs      3               ; e3
        xs      4               ; e4
        xs      5               ; e5
        xs      2*[1+2]         ; e6
        xs      7               ; e7
        xs      (2+2)*2         ; e8
        xs      9               ; e9
        xs      j               ; e9
        xs      $a              ; ea
        xs      hu              ; ea
        xs      11              ; ab
        xs      hl              ; eb
        xs      s               ; ec
        xs      (is)            ; ec
        xs      i               ; ed
        xs      (is)+           ; ed
        xs      d               ; ee
        xs      (is)-           ; ee

        xs      12              ; ec
        xs      13              ; ed
        xs      14              ; ee

        dc.b    0               ; 00
        dc.w    $1234           ; 12 34
        dc.l    $deadbeef       ; de ad be ef

        db      0               ; 00
        dw      $1234           ; 12 34
        dd      $deadbeef       ; de ad be ef

        byte    0               ; 00
        word    $1234           ; 12 34
        long    $deadbeef       ; de ad be ef

        .byte   0               ; 00
        .word   $1234           ; 12 34
        .long   $deadbeef       ; de ad be ef

        res.b   4, $33          ; 33 33 33 33
        res.w   2, $1234        ; 12 34 12 34
        res.l   2, $deadbeef    ; de ad be ef de ad be ef
