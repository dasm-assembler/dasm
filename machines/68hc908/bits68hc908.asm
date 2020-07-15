
	#ifndef	BITS_68HC908JB8

BITS_68HC908JB8 	.EQU 	1

; TODO: MOVE THIS OUT  TO regXXXXX.asm, remove all definitions that aren't on the board

PTA0	.EQU	(1 << 0)
PTA1	.EQU	(1 << 1)
PTA2	.EQU	(1 << 2)
PTA3	.EQU	(1 << 3)
PTA4	.EQU	(1 << 4)
PTA5	.EQU	(1 << 5)
PTA6	.EQU	(1 << 6)
PTA7	.EQU	(1 << 7)

PTB0	.EQU	1
PTB1	.EQU	2
PTB2	.EQU	4
PTB3	.EQU	8
PTB4	.EQU	16
PTB5	.EQU	32
PTB6	.EQU	64
PTB7	.EQU	128

PTC0	.EQU	(1 << 0)
PTC1	.EQU	(1 << 1)
PTC2	.EQU	(1 << 2)
PTC3	.EQU	(1 << 3)
PTC4	.EQU	(1 << 4)
PTC5	.EQU	(1 << 5)
PTC6	.EQU	(1 << 6)
PTC7	.EQU	(1 << 7)


PTD0	.EQU	(1 << 0)
PTD1	.EQU	(1 << 1)
PTD2	.EQU	(1 << 2)
PTD3	.EQU	(1 << 3)
PTD4	.EQU	(1 << 4)
PTD5	.EQU	(1 << 5)
PTD6	.EQU	(1 << 6)
PTD7	.EQU	(1 << 7)



; CONFIG bits
COPD	.EQU	(1 << 0)

; TSC bits
TOF		.EQU	(1 << 7)
TOIE	.EQU	(1 << 6)
TSTOP	.EQU	(1 << 5)
TRST	.EQU	(1 << 4)
PRS2	.EQU	(1 << 2)
PRS1	.EQU	(1 << 1)
PRS0	.EQU	(1 << 0)


; TSC0 bits
CH0F	.EQU	(1 << 7)
CH0IE	.EQU	(1 << 6)
MS0B	.EQU	(1 << 5)
MS0A	.EQU	(1 << 4)
ELS0B	.EQU	(1 << 3)
ELS0A	.EQU	(1 << 2)
TOV0	.EQU	(1 << 1)
CH0MAX	.EQU	(1 << 0)

; TSC1 bits
CH1F	.EQU	(1 << 7)
CH1IE	.EQU	(1 << 6)
; MS1B	.EQU	(1 << 5)  ;; does not exist
MS1A	.EQU	(1 << 4)
ELS1B	.EQU	(1 << 3)
ELS1A	.EQU	(1 << 2)
TOV1	.EQU	(1 << 1)
CH1MAX	.EQU	(1 << 0)

; INTSCR
ACKI	.EQU	(1 << 2)
IMASKI	.EQU	(1 << 1)
MODEI	.EQU	(1 << 0)

; KBSCR
ACKK	.EQU	(1 << 2)
IMASKK	.EQU	(1 << 1)
MODEK	.EQU	(1 << 0)

;
;  Register Section Bits FLCR
;
PGM		.EQU		$01
ERASE		.EQU		$02
MASS		.EQU		$04
HVEN		.EQU		$08


	#endif




