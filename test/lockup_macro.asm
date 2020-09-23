
	.PROCESSOR 68908

 	.macro	fw_delay
		ldx		#{1}				;[2]
		lda		#{2}				;[2]
 		jsr		MoniRomDelayLoop		;[5]
	.endm

	.macro	declare_delay
Delay{1}:
		fw_delay {2},{3}
 		rts						;[4]
	.endm

	.ORG	0

	.word	$B,$AD,$C0,$DE

	declare_delay 100us,1,40                        ; 128 cycles,

	declare_delay 4ms,7,234                         ; 4928 cycles,


	#include "file-with-dash.asm"


;MoniRomDelayLoop:
;	rts





