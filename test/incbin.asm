; Test case for the INCBIN pseudo-op

fname = "incbinexample1.bin"

    PROCESSOR 6502
    ORG $C000
    ; Regression test - see if it works as it used to
    INCBIN "incbinexample1.bin"
    DC.B 0
    ; Skip first two bytes
    INCBIN fname, 2
    DC.B 0
    ; Number bigger than file length - include nothing
    INCBIN fname, 100 
    DC.B 0