; Test case for the SETSYM pseudo-op

    PROCESSOR 6502
    ORG $C000
HERE = .
TESTSTR = "HERE"
TESTSYM setsym TESTSTR
    .word TESTSYM
    .word 0
