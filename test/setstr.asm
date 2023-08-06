; Test case for the SETSYM pseudo-op

    PROCESSOR 6502
    ORG $C000
TESTLABEL = .
TESTSTR setstr TESTLABEL
    .byte TESTSTR
