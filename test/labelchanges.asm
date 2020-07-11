; labelchanges.asm - test various cases of labels changing location in 
; successive passes. Test code provided by Thomas Jentzsch.

    processor 6502

;===============================================================================

  ; automatically decide whether to use a branch or a jump
  .macro bnej
    #if ({1} - .) >= -126 && ({1} - .) <= 129
      bne    {1}
    #else
      beq    skip$
      jmp    {1}
skip$
    #endif
  .endm

;===============================================================================

    ORG     $1000

BackOk
    ds      126, $ea
    bnej    BackOk

BackJmp
    ; this has to use "jmp"
    ; and is detected as such
    ds      126+1, $ea
    bnej    BackJmp

    ; this can use "bne"
    bnej    ForwardOk
    ds      127, $ea
ForwardOk

    ; this has to use "jmp"
    ; and is detected as such
    bnej    ForwardJmp
    ds      127+3, $ea
ForwardJmp

    ; this has to use "jmp"
    ; and is detected as such
    bnej    ForwardJmp2
    ds      127+2, $ea
ForwardJmp2
