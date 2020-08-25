	.PROCESSOR 68908

	.ORG 0

    DC.w $EE00		; native is big endian

    DC.s $FF00		; the opposite

    DC.w "Multibyte String"

    DC.s "Little Endian"

