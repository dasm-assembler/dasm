
        .PROCESSOR 68hc908

FIVE_HUNDRED	.EQU	500

_v@0041		.EQU		$0041	; disassembler generated variable name 

	.ORG	$FC00

F_@fc00:				; disassembler detected function label 
	lda	_v@fc1a
	sta	_v@0041
	bne	_L@fc11
        and	300,X
        and	[X+300]
        adc	300,SP
_L@fc11:				; disassembler detected branch or jump label
        adc	[SP+FIVE_HUNDRED]
        adc	[SP+500]
	rts

_v@fc1a:				; disassembler generated variable name
	DC.b	$0C