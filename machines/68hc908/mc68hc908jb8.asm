
		.PROCESSOR	68908

;
; variant specific defines
;

	#ifndef RAM_START
RAM_START       .EQU            $40
	#endif
	#ifndef RAM_SIZE
RAM_SIZE       .EQU             256
	#endif

	#ifndef FLASH_START
FLASH_START		.EQU            $DC00
	#endif
	#ifndef FLASH_SIZE
FLASH_SIZE		.EQU            8192
	#endif

	#include	reg68hc908jb8.asm


PTE0	.EQU	(1 << 0)
PTE1	.EQU	(1 << 1)
PTE2	.EQU	(1 << 2)
PTE3	.EQU	(1 << 3)
PTE4	.EQU	(1 << 4)
PTE5	.EQU	(1 << 5)
PTE6	.EQU	(1 << 6)
PTE7	.EQU	(1 << 7)


;
;  Register Section Bits FLCR
;
PGM		.EQU		$01
ERASE		.EQU		$02
MASS		.EQU		$04
HVEN		.EQU		$08

;
; Firmware Monitor ROM specific section
;
ReadSerial	     	.EQU	$FC00
FlashRead       	.EQU    $FC03
FlashErase      	.EQU	$FC06
FlashProg	    	.EQU	$FC77	; $FC09
DelayUs         	.EQU	$FC0C	;

WriteSerial	     	.EQU	$FED6
MoniRomDelayLoop	.EQU	$FD21		; cycles = (((A-3)*3 +10)*X) + 7
										; longest wait time 58.6ms

fwCtrlByte      .EQU      RAM_START + $08
fwCpuSpeed      .EQU      RAM_START + $09
fwLastAddr      .EQU      RAM_START + $0A
fwDataBlock     .EQU      RAM_START + $0C


	#include	macros.asm

	.macro	fw_delay
		ldX		#{1}					;[2]
		ldA		#{2}					;[2]
		jsr		MoniRomDelayLoop		;[5]
	.endm

	.macro	declare_delay
Delay{1}:
		fw_delay {2},{3}
		rts								;[4]
	.endm

	; macro-names MUST be all lowercase
	.macro	fw_flash_erase_proc
		store_reg	fwCtrlByte,#$40
		store_reg	fwCpuSpeed,#(4*F_CPU)
		ldHX		#FLASH_START
		jsr			FlashErase
	.endm

	.macro	delay5us
		fw_delay	1,3			; ((((3-3)*3 +10)*1)+7) := 17 cycles  ~5.6 µs @ 3MHz
	.endm

	.macro	delay10us
		fw_delay	1,8			; ((((8-3)*3 +10)*1)+7) := 32 cycles  ~10µs @ 3MHz
	.endm

	.macro	delay100us
		fw_delay	1,98		; ((((98-3)*3 +10)*1)+7) := 302 cycles  100µs @ 3MHz
	.endm

	.macro	delay2ms
		fw_delay	8,250		; ((((250-3)*3 +10)*1)+7) := 6015 cycles  2ms @ 3MHz
	.endm

	.macro	delay4ms
		fw_delay	16,250		; ((((250-3)*3 +10)*16)+7) := 12023 cycles should make 4ms @ 3MHz
	.endm

