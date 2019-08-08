;;;;
;
;   Test local symbols
;
	processor 6502

	mac	test1
	bne	.end
test$	=	1
	test2
.end
	nop
	endm

	mac	test2
	bne	.end
test$	=	1
	test3
.end
	nop
	endm

	mac	test3
	bne	.end
test$	=	1
	nop
.end
	nop
	endm

	org	0

	test1
	test2
	test3

.end

loop	set	0
	
