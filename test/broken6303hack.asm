; See ../src/BUGS for the details. Here's how to use macros
; to work around the issue.

	.processor HD6303

	.mac hack
	dc.b	{1}	; opcode
	dc.b	{2}	; immediate value
	dc.b	{3}	; zero-page address
	.endm

	.mac aimd
	hack	$71,{1},{2}
	.endm

	.mac aimx
	hack	$61,{1},{2}
	.endm

	.mac oimd
	hack	$72,{1},{2}
	.endm

	.mac oimx
	hack	$62,{1},{2}
	.endm

	.mac eimd
	hack	$75,{1},{2}
	.endm

	.mac eimx
	hack	$65,{1},{2}
	.endm

	.mac timd
	hack	$7b,{1},{2}
	.endm

	.mac timx
	hack	$6b,{1},{2}
	.endm

	.org $0

	; the broken DASM versions

	aim	$10	; assembles to 71 10
	aim	$10,x	; assembles to 61 10
	oim	$10	; assembles to 72 10
	oim	$10,x	; assembles to 62 10
	eim	$10	; assembles to 75 10
	eim	$10,x	; assembles to 65 10
	tim	$10	; assembles to 7B 10
	tim	$10,x	; assembles to 6B 10

	; the fixed macro versions

	aimd	$10,$20	; should be "aim #$10,$20"
	aimx	$10,$20	; should be "aim #$10,$20,x"
	oimd	$10,$20	; should be "oim #$10,$20"
	oimx	$10,$20	; should be "oim #$10,$20,x"
	eimd	$10,$20	; should be "eim #$10,$20"
	eimx	$10,$20	; should be "eim #$10,$20,x"
	timd	$10,$20	; should be "tim #$10,$20"
	timx	$10,$20	; should be "tim #$10,$20,x"

	.end
