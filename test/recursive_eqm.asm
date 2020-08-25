; segfault

str	eqm	"BB"

    .MAC strcat		; {symbol},{string_trailer}
strcat_tmp	eqm	{1} + {2}
{1}		eqm	strcat_tmp
    .ENDM

    strcat	str,"\nAAA"
    strcat	str,"\nCC"

    .PROCESSOR 6502

    .ORG 0

    DS		str

    lda		#1

; EOF

