
	.PROCESSOR	68908

;
; variant specific defines
;

	#ifndef RAM_START
RAM_START       .EQU            $60
	#endif

	#ifndef RAM_SIZE
RAM_SIZE       .EQU             768
	#endif

	#ifndef FLASH_START
FLASH_START	.EQU            $8000
	#endif

	#ifndef FLASH_SIZE
FLASH_SIZE	.EQU            32256
	#endif

	#include	"reg68hc908mr32.asm"

	#include	"macros.asm"

	.macro	switch_to_pll_clock
		clr 	PCTL
		bset	5,PCTL	 		; switch PLL on
		bset	7,PBWC			; automatic bandwidth
PLL_lock_wait:
		brclr	6,PBWC,PLL_lock_wait
		bset	4,PCTL			; select PLL clock as system
	.endm

; ROM specific routines
WriteSerial		.EQU		0xFEC3


	.macro	fw_delay
		ldX	#{1}			;[2]
		ldA	#{2}			;[2]
		jsr	MoniRomDelayLoop	;[5]		// TODO: find this in the monitor code
	.endm

	.macro	declare_delay
Delay{1}:
		fw_delay {2},{3}
		rts				;[4]
	.endm


