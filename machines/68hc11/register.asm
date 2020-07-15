

RAM_END			.EQU    (DOWNLOAD_SIZE-1)
FLASH_END		.EQU<-->(FLASH_START + FLASH_SIZE)


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~;
;     Registernames                ;  
;__________________________________;
;
;	with this architecture you can shift the IO register base-address by programming the config register 
;	within 64 cycles after reboot.
;	if you don't, you loose some bytes of RAM since the IO registers and RAM share the zero page
;	if you do, you consume some more bytes of FLASH_ROM since you will need extended addressing mode
;	a good compromise is to shift base register address to 0x1000, load 0x1000 to X register and use 
;	X-indexed addressing mode. Using Y-indexed addressing for register-IO is no advantage at all, 
;	it has both more opcodes and longer execution time addressing mode for register IO
;
	.IFNCONST	REGBASE
REGBASE		.EQU		0
	.ENDIF


PA0	.EQU	$01
PA1	.EQU	$02
PA2	.EQU	$04
PA3	.EQU	$08
PA4	.EQU	$10
PA5	.EQU	$20
PA6	.EQU	$40
PA7	.EQU	$80
PA3_4	.EQU	$18
PA7_5	.EQU	$a0


CSSTRH   .EQU   REGBASE+$5c
HPRIO	 .EQU	REGBASE+$3c


INIT	.EQU 	$103d
PORTA	.EQU 	REGBASE+$00
PORTB	.EQU 	REGBASE+$04 
PORTC	.EQU 	REGBASE+$03
PORTD	.EQU 	REGBASE+$08
PORTE	.EQU 	REGBASE+$0a
DDRC	.EQU 	REGBASE+$07
DDRD	.EQU 	REGBASE+$09
CFORC	.EQU 	REGBASE+$0b
OC1M	.EQU	REGBASE+$0c
OC1D	.EQU	REGBASE+$0d
TCNT	.EQU	REGBASE+$0e

TIC1	.EQU	REGBASE+$10
TIC2	.EQU	REGBASE+$12
TIC3	.EQU	REGBASE+$14

TOC1	.EQU 	REGBASE+$16
TOC2	.EQU	REGBASE+$18
TOC3	.EQU	REGBASE+$1a
TOC4	.EQU	REGBASE+$1c
TOC5	.EQU	REGBASE+$1e

TCTL1	.EQU 	REGBASE+$20
TCTL2	.EQU 	REGBASE+$21
TFLG1	.EQU 	REGBASE+$23
TMSK1	.EQU 	REGBASE+$22
TMSK2	.EQU 	REGBASE+$24
TFLG2	.EQU 	REGBASE+$25
PACTL	.EQU 	REGBASE+$26
PACNT	.EQU	REGBASE+$27

SPCR	.EQU 	REGBASE+$28
BAUD	.EQU 	REGBASE+$2b
SCCR1   .EQU 	REGBASE+$2c
SCCR2	.EQU 	REGBASE+$2d 
SCSR	.EQU 	REGBASE+$2e
SCDR	.EQU 	REGBASE+$2f


ADCTL   .EQU 	REGBASE+$30
ADR1    .EQU 	ADCTL+1
ADR2    .EQU 	ADCTL+2
ADR3    .EQU 	ADCTL+3
ADR4    .EQU 	ADCTL+4

BPROT	.EQU	REGBASE+$35
OPTION  .EQU	REGBASE+$39
EEPCTL	.EQU	REGBASE+$3b
CONFIG	.EQU	REGBASE+$3f





