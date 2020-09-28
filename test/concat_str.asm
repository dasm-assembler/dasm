
	PROCESSOR	6502

	ORG 0

	.macro concat_str ; {name},{size}
{1}		.set	str_var
str_var		.set	({1}{2})
	.endm

str_var		.set		""

	concat_str _1 "A"
	concat_str _2 "b"
	concar_str _3 "C"

    DC.b	str_var

    echo str_var
