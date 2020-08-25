; segfault

str	set	"BB"
str	set	str + "AAAA"

    .PROCESSOR 6502

    .ORG 0

    DS		str

    lda		#1

; EOF

