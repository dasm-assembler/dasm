
		;   FATROM.ASM
		;
		;-----------------------------------------------------
		;			MEMORY MAP	 	     -
		;-----------------------------------------------------
		;
		;   We throw away some internal ram (96 bytes) in order
		;   to use direct addressing for the majority of memory
		;   accesses by mapping the registers at 0.
		;
		;   MC68HC11F1 port and bit assignments
		;
		;   We setup a very fast OC1 interrupt.. around 208uS, to
		;   handle main system timing.  This also allows us to have
		;   a WAI instruction in our serial poll loop to lower power
		;   consumption.

RAM		equ	$0000		;   1K byte of RAM
LORAMBEG	equ	$0060		;   96 unusable due to ctl regs
	       ifconst INEEPROM
HIRAMBEG	equ	$0300		;   hardwired for RTU software
	       else
HIRAMBEG	equ	$0320
	       eif
REGS		equ	$0000		;   96 control regs (after INIT)
EEPROM		equ	$FE00		;   512 bytes EEPROM

PORTA		equ	REGS+$00	;   i/o port A
PORTA_24V	equ	$80		;   24V power switch

DDRA		equ	REGS+$01	;

PORTG		equ	REGS+$02
CSPROG		equ	$80
CSGEN		equ	$40
CS101		equ	$20
CS102		equ	$10
PORTG_RESERVED7 equ	$80
PORTG_MUXSEL3	equ	$40
PORTG_MUXSEL2	equ	$20
PORTG_MUXSEL1	equ	$10
PORTG_MUXSEL0	equ	$08
PORTG_ADSTROBHI equ	$04

DDRG		equ	REGS+$03

PORTB		equ	REGS+$04	;   i/o port B

PORTF		equ     REGS+$05	;   PORTF l.e.d's and digout 1 & 2
PORTF_DIGOUT2	equ	$80
PORTF_DIGOUT1	equ	$40
PORTF_GRN	equ	$20
PORTF_RED	equ	$10

PORTC		equ	REGS+$06

DDRC		equ	REGS+$07

PORTD		equ	REGS+$08

DDRD		equ	REGS+$09

PORTE		equ	REGS+$0A

CFORC		equ	REGS+$0B
CFORC_FOC1	equ	$80	; FORCE OUTPUT COMPARE 1-4
CFORC_FOC2	equ	$40	;	....OR'D WITH TOC'S
CFORC_FOC3	equ	$20	;
CFORC_FOC4	equ	$10	;
CFORC_FOC5	equ	$08	;

OC1M		equ	REGS+$0C	; OUTPUT COMPARE MASK
OC1_M7		equ	$80	; ALLOWS COUNTER COMPARE OUTPUTS ESTABLISHED BY
OC1_M6		equ	$40	; OC1D6-D3 ON PA
OC1_M5		equ	$20	;   EXCEPT FOR OC1M7 WHICH ALLOWS EITHER
OC1_M4		equ	$10	; INTERNAL DATA (0) OR OC1D7 DURING WRITES TO
OC1_M3		equ	$08	; PA7

OC1D		equ	REGS+$0D
OC1_d7		equ	$80	; STATES WHICH WILL BE PRESENT ON PA6-PA3
OC1_d6		equ	$40	; WHEN PERMITTED BY OC1_M6-M3
OC1_d5		equ	$20	;
OC1_d4		equ	$10	;
OC1_d3		equ	$08	;

TCNT		equ	REGS+$0E

TIC1		equ	REGS+$10

TIC2		equ	REGS+$12

TIC3		equ	REGS+$14

TOC1		equ	REGS+$16

TOC2		equ	REGS+$18

TOC3		equ	REGS+$1A

TOC4		equ	REGS+$1C

TOC5		equ	REGS+$1E

TCTL1		equ	REGS+$20	;ACTION TAKEN CODES  M	L
TCTL1_OM2	equ	$80	;		     0	0 -DISABLE
TCTL1_OL2	equ	$40	;		     0	1 -TOGGLE
TCTL1_OM3	equ	$20	;		     1	0 -CLEAR
TCTL1_OL3	equ	$10	;		     1	1 -SET
TCTL1_OM4	equ	$08	;
TCTL1_OL4	equ	$04	;  OR, WHAT 1 OF 4 THINGS HAPPEN TO OUT.CMP
TCTL1_OM5	equ	$02	;  FLAGS IN TFLG1 2-5 (OCF'S 2-5)
TCTL1_OL5	equ	$01	;

TCTL2		equ	REGS+$21	; B  A
TCTL2_EDG1B	equ	$20	; 0  0 CAPTURE DISABLED
TCTL2_EDG1A	equ	$10	; 0  1	 ''    ON RISING
TCTL2_EDG2B	equ	$08	; 1  0   ''      ON FALLING
TCTL2_EDG2A	equ	$04	; 1  1   ''    ON ANY
TCTL2_EDG3B	equ	$02
TCTL2_EDG3A	equ	$01

TMSK1		equ	REGS+$22
TMSK1_OC1I	equ	$80	;
TMSK1_OC2I	equ	$40	; ENABLE OUTPUT COMPARE INTERRUPT WHEN SET
TMSK1_OC3I	equ	$20	;
TMSK1_OC4I	equ	$10	;
TMSK1_OC5I	equ	$08	;
TMSK1_IC1I	equ	$04	; ENABLE INTERUPTS ON CAPTURE FLAG WHEN SET
TMSK1_IC2I	equ	$02	;
TMSK1_IC3I	equ	$01	;

TFLG1		equ	REGS+$23
TFLG1_OC1F	equ	$80	; CORRESPONDING FLAGS TO THE ABOVE
TFLG1_OC2F	equ	$40	;
TFLG1_OC3F	equ	$20	;
TFLG1_OC4F	equ	$10	;
TFLG1_OC5F	equ	$08	;
TFLG1_IC1F	equ	$04	;
TFLG1_IC2F	equ	$02	;
TFLG1_IC3F	equ	$01	;

		;NOTE: PR1 & PR0 MUST BE WRITTEN IMMEDIATELY AFTER RESET!!

TMSK2		equ	REGS+$24
TMSK2_TOI	equ	$80	; Timer overflow interrupt enable
TMSK2_RTII	equ	$40	; Real time interrupt
TMSK2_PAOVI	equ	$20	; Pulse accumulator overflow
TMSK2_PAII	equ	$10	; Pulse accumulator count
TMSK2_PR1	equ	$02	; Timer prescale select
TMSK2_PR0	equ	$01	;  0,0=/1  0,1=/4  1,0=/8  1,1=/16



TFLG2		equ	REGS+$25
TFLG2_TOF	equ	$80	; FLAGS FOR THE ABOVE 1ST 4 INTERRUPTS
TFLG2_RTIF	equ	$40	;
TFLG2_PAOVF	equ	$20	;
TFLG2_PAIF	equ	$10	;


PACTL		equ	REGS+$26
PACTL_DDRA7	equ	$80	; 0=INPUT ONLY, 1=OUTPUT
PACTL_PAEN	equ	$40	; ENABLE PA
PACTL_PAMOD	equ	$20	; 0=COUNT EXTERNAL EVENTS,1=GATED TIME ACCUM.
PACTL_PEDGE	equ	$10	; 0=INC ON FALLING OR INHIBITS CNT;1=OPPOSITE
PACTL_RTR1	equ	$02 	; Divider for real time interrupt
PACTL_RTR0	equ	$01 	; Divide by 2^13, 2^14, 2^15, or 2^16


PACNT		equ	REGS+$27

SPCR		equ	REGS+$28
SPCR_SPIE	equ	$80	; serial PERIPH INTERRUPT ENABLE
SPCR_SPE	equ	$40	; serial periph system enable
SPCR_DWOM	equ	$20	; PORT D WIRED-OR 0=TOTEM,1=OPEN DRAIN
SPCR_MSTR	equ	$10	; 0=SLAVE, 1=MASTER
SPCR_CPOL	equ	$08	; 0=ACT. HIGH CLKS;SCK IDLES LOW--1 IS OPPOSITE
SPCR_CPHA	equ	$04	; CLOCK PHASE
SPCR_SPR1	equ	$02	;	  E CLOCK /  2	 4   16   32
SPCR_SPR0	equ	$01	;		     00  01  10   11

SPSR		equ	REGS+$29
SPSR_SPIF	equ	$80	; TXFR COMPLETE
SPSR_WCOL	equ	$40	; WRITE COLLISION
SPSR_MODF	equ	$10	; MODE-FAULT ERROR FLAG

SPDR		equ	REGS+$2A

BAUD		equ	REGS+$2B
BAUD_TCLR	equ	$80 	; 0
BAUD_SCP1	equ	$20 	; divide e by 1  3  4   13
BAUD_SCP0	equ	$10 	;	      00 01 10  11
BAUD_RCKB	equ	$08 	; 0
BAUD_SCR2	equ	$04 	; divide prescaler out by 1,  2   4,.....128
BAUD_SCR1	equ	$02 	;
BAUD_SCR0	equ	$01 	; scr0,1,2 =    000 001 010.....111
BAUD_4800	equ	BAUD_SCP1|BAUD_SCR1
BAUD_9600	equ	BAUD_SCR0|BAUD_SCR1
BAUD_19200	equ	BAUD_SCR1


SCCR1		equ	REGS+$2C
SCCR1_R8	equ	$80 	; get the ninth bit here, if 'M' bit is set
SCCR1_T8	equ	$40	; set this bit for the 9th tx bit if 'M' is set
SCCR1_M 	equ	$10	; 'M'
SCCR1_WAKE	equ	$08	; 0=idle line, 1=address mark

SCCR2		equ	REGS+$2D
SCCR2_TIE	equ	$80	; Transmit interrupt enable
SCCR2_TCIE	equ	$40	; Transmit Complete interr en
SCCR2_RIE	equ	$20	; Receiver Interrupt enable
SCCR2_ILIE	equ	$10	; Idle Line ''       ''
SCCR2_TE	equ	$08	; tx enable
SCCR2_RE	equ	$04	; rx """
SCCR2_RWU	equ	$02	; wake up when set
SCCR2_SBK	equ	$01	; send break


SCSR		equ	REGS+$2E
SCSR_TDRE	equ	$80	; Transmit Data Register Empty
SCSR_TC 	equ	$40	; Transmit Complete
SCSR_RDRF	equ	$20	; Receiver Data Ready
SCSR_IDLE	equ	$10	; Idle Line Detect
SCSR_OR 	equ	$08	; overrun error
SCSR_NF 	equ	$04	; noise flag
SCSR_FE 	equ	$02	; framing error


SCDR		equ	REGS+$2F	; sc data reg

ADCTL		equ	REGS+$30
ADCTL_CCF	equ	$80 	; Conversion complete, cleared on ADCTL write
ADCTL_SCAN	equ	$20	; Continuous scan mode, else single scan
ADCTL_MULT	equ	$10	; Scan all four channels or just one channel
ADCTL_CD	equ	$08	;
ADCTL_CC	equ	$04	;
ADCTL_CB	equ	$02	;
ADCTL_CA	equ	$01	;

ADR1		equ	REGS+$31	; ADC register 1
ADR2		equ	REGS+$32
ADR3		equ	REGS+$33
ADR4		equ	REGS+$34


BPROT		equ	REGS+$35
BPROT_PTCON	equ	$10
BPROT_BPRT3	equ	$08
BPROT_BPRT2	equ	$04
BPROT_BPRT1	equ	$02
BPROT_BPRT0	equ	$01

OPT2		equ	REGS+$38	; config options 2 register
OPT2_GWOM	equ	$80
OPT2_CWOM	equ	$40
OPT2_CLK4X	equ	$20

OPTION		equ	REGS+$39
OPTION_ADPU	equ	$80	; A/D power
OPTION_CSEL	equ	$40
OPTION_IRQE	equ	$20	; IRQ edge sensitive enable, else level sens.
OPTION_DLY	equ	$10	; delay 4000 E clks after stop, else 4
OPTION_CME	equ	$08
OPTION_CR1	equ	$02	; cop timer rates of E/2^15/(1,4,16,64)
OPTION_CR0	equ	$01	; for 00,01,10,11


COPRST		equ	REGS+$3A

		; EEPROM programming register.
		;
		; BYTE ROW	(only valid when erase bit is set)
		;  0    0	bulk erase
		;  0	1	row erase
		;  1	0	byte erase
		;  1	1	byte erase
		;
		; erase sequence is:
		;	+BYTE+ERASE+EELAT, write eeprom, +PGM, delay 10 ms,
		;	-PGM-ERASE-EELAT
		; program sequence is:  
		;	+EELAT, write eeprom, +PGM, delay 10 ms, -PGM-EELAT
		;
		; you do not need to program to zero before erasing to 1's.

PPROG		equ	REGS+$3B
PPROG_ODD	equ	$80	; Factory reserved
PPROG_EVEN	equ	$40	; Factory reserved
PPROG_BYTE	equ	$10	; Byte erase mode
PPROG_ROW	equ	$08	; Row erase mode
PPROG_ERASE	equ	$04	; Erase enable
PPROG_EELAT	equ	$02	; arm eeprom write (then write to eeprom)
PPROG_EEPGM	equ	$01	; apply programming voltage


HPRIO		equ	REGS+$3C
HPRIO_RBOOT	equ	$80	; Set in special test mode
HPRIO_SMOD	equ	$40	; ~MODB pin latched at reset
HPRIO_MDA	equ	$20	; MODB pin latched at reset, can be clrd in
				; special tes/tbootstrap mode ?
HPRIO_IRV	equ	$10	; internal read visibility
HPRIO_PSEL3	equ	$08	; Select highest priority interrupt, only
HPRIO_PSEL2	equ	$04	; writable when interrupts are masked.
HPRIO_PSEL1	equ	$02	;
HPRIO_PSEL0	equ	$01	;


INIT		equ	REGS+$3D
INIT_RAM3	equ	$80	; Internal ram base address in 4K chunks,
INIT_RAM2	equ	$40	; default $0000 (0000)
INIT_RAM1	equ	$20	;
INIT_RAM0	equ	$10	;
INIT_REG3	equ	$08	; Control register base address in 4K chunks,
INIT_REG2	equ	$04	; default $1000 (0001)
INIT_REG1	equ	$02	;
INIT_REG0	equ	$01	;

TEST1		equ	REGS+$3E
TEST1_TILOP	equ	$80	; FACTORY TEST STUFF ONLY WHEN SMOD=1
TEST1_OCCR	equ	$20
TEST1_CBYP	equ	$10
TEST1_DISR	equ	$08
TEST1_FCM	equ	$04
TEST1_FCOP	equ	$02
TEST1_TCON	equ	$01


CONFIG		equ	REGS+$3F
CONFIG_NOSEC	equ	$08	; no eeprom security
CONFIG_NOCOP	equ	$04	; no hardware watchdog
CONFIG_ROMON	equ	$02	; enable rom
CONFIG_EEON	equ	$01	; enable eeprom

		; In bootstrap mode vectors go through ram
		; 

BsSCIJmp	equ	$C4
BsSPIJmp	equ	$C7
BsPAIJmp	equ	$CA
BsPAOVJmp	equ	$CD
BsTOVJmp	equ	$D0
BsOC5Jmp	equ	$D3
BsOC4Jmp	equ	$D6
BsOC3Jmp	equ	$D9
BsOC2Jmp	equ	$DC
BsOC1Jmp	equ	$DF
BsIC3Jmp	equ	$E2
BsIC2Jmp	equ	$E5
BsIC1Jmp	equ	$E8
BOOTSTRAPORG	equ	$EB	; bootstrap origin
BsRTIJmp	equ	$EB
BsIRQJmp	equ	$EE
BsXIRQJmp	equ	$F1
BsSWIJmp	equ	$F4
BsILLJmp	equ	$F7
BsCOPJmp	equ	$FA
BsCMJmp		equ	$FD

		processor 68hc11

		;-----------------------------------------------------
		;			RAM VARS 	 	     -
		;-----------------------------------------------------
		;
		; most/all ram variables can be accessed with direct
		; addressing for the EEPROM version.

		seg.u	bss
		org	LORAMBEG

RTUConfig	ds.b	1	; RTU Flags reg, cleared on reset
BoardAddr	ds.b	1	; 1 byte board address
Counter1	ds.w	1	; 2 bytes counter input
Counter2	ds.w	1	; 2 bytes counter input
RTIRedCnt	ds.b	1	; real time interrupt red l.e.d. countdown
RTIGrnCnt	ds.b	1	; real time interrupt grn l.e.d. countdown
RTIWDog		ds.w	1	; 2 byte wdog ctr, reset by packet reception
RTIAvrCnt	ds.b	1	; 1 16 sample countdown

		; note: packet size for ram-dl version must be smaller so
		; program fits in ram

	       ifconst INEEPROM
PacketBuf	ds.b	256+8	; maximum packet size plus slop
	       else
PacketBuf	ds.b	32+8	; maximum packet size plus slop in bootstrap
	       eif

		; note: MaxScanTable location hardwired by ioscan for normal
		; operation
		;

		org	HIRAMBEG

MaxScanTable	ds.b	96	; 16 channels x 3 (holdingregx1, accumulatorx2)
Stack		ds.b	64	; Reserve 64 bytes for the stack
StackEnd	ds.b	0

		;-----------------------------------------------------
		;			CODE SEGMENT		     -
		;-----------------------------------------------------
		;
		; For EEPROM version code starts at $FE00.  For RAM bootstrap
		; version code starts at zero from which we immediately jump
		; forward past all registers and ram variables
		;

		seg	code

	       ifconst INEEPROM
		org	$FE00
	       else
		org	$0000
		jmp	Reset
		org	BOOTSTRAPORG
	       eif

		;-----------------------------------------------------
		;			ROM VARS 	 	     -
		;-----------------------------------------------------
		;
		; Calibration constants are at the beginning of the rom,
		; consisting of two 16 bit base constants for analog channel
		; 1 and two 8 bit constants for each of analog channels
		; 2 through 7, two 8 bit constants for the two temp.
		; sensors, two 8 bit constants for the board temp. sensor.
		;
		; The RTU will interpret these constants

		dc.w	$FFFF,$FFFF	; Mux channel 0, 4 and 20mA	AI01
		dc.b	$FF,$FF		; Mux channel 1 (delta)		AI02
		dc.b	$FF,$FF		; Mux channel 2 (delta)		AI03
		dc.b	$FF,$FF		; Mux channel 3 (delta)		AI04
		dc.b	$FF,$FF		; Mux channel 4 (delta) (unused)
		dc.b	$FF,$FF		; Mux channel 5 (delta)		AI05
		dc.b	$FF,$FF		; Mux channel 6 (delta)		AI06
		dc.b	$FF,$FF		; Mux channel 7 (delta)		AI07
		dc.b	$FF,$FF		; External temp. sensor 1
		dc.b	$FF,$FF		; External temp. sensor 2
		dc.b	$FF,$FF		; On-Board temp. sensor

		;-----------------------------------------------------
		;			RESET 		 	     -
		;-----------------------------------------------------
		;
		; Platform reset.  Interrupts are disabled and the stack
		; pointer is/becomes invalid.

Reset		subroutine
ResetCmd
BadInt
		; disable interrupts, setup protected control registers
		;	INIT - relocate ram and hardware regs to 0
		;	OPT2 - disable clk4x output (save power)
		;	OPTION - enable ADC
		;
		; note: due to ram/regs overlap, first 96 bytes of ram
		; is unaccessable.  For bootstrapped programs, the 
		; jmp to Reset becomes unaccessable after it is executed.

		sei
		clra			; .A = 0
		staa	INIT-REGS+$1000	; map ram & regs to base address 0
		staa	OPT2		; disable clk4x & C,G wire-or modes
		staa	BPROT		; allow eeprom to be written
		ldab	#OPTION_ADPU	; enable ADC
		stab	OPTION

		; SETUP PORTS AND DDRS
		;
		;	    DDR	       DATA
		; PORTA	 10000000    0xxxxxxx	(+24V power switch)
		; PORTB  <OUTPUT>    11111111	(LED's)
		; PORTC  00000000    xxxxxxxx
		; PORTD  xx110000    xx00xxxx   (~ss, sck)
		; PORTE  <INPUT>     xxxxxxxx	(addr h, l, brdid, temp)
		; PORTF  <OUTPUT>    00111111	(digout, LED's)
		; PORTG  11111100    000000xx	(filter, mux sel, startconv, 
		;				digin)
		; note: PD5 must be an output so as not to interfere with
		; SPI ~SS hardware.
		;
		; NOTE: PORTA_24V is off (0) by default.  This is to prevent
		; the RTU's isolated power supply from getting jerked around
		; when several boards are in the system.  Analog and digital
		; inputs will not be valid until the RTU turns on this line.
		;
		; NOTE: PORTD ~SS output must be set to 1 before configuring
		; the SPI as a master (???)
		;
		; .A is still 0

		lds	#DDRD
		ldab	#%00111000	; DDRD = 00111000
		pshb
		pshb			; PORTD= 00111000
		psha			; DDRC = 00000000
		des			; RESERVED06
		ldab	#$3F		; PORTF= 00111111
		pshb
		ldab	#$FF		; PORTB= 11111111
		pshb
		ldab	#%11111100	; DDRG = 11111100
		pshb
		psha			; PORTG= 0
		ldab	#PORTA_24V	; DDRA = PORTA_24V
		pshb
		psha			; PORTA= 0

		; Setup remaining hardware, .A still 0
		;
		;	ADC	setup for continuous scan channels 4-7
		;	SCI	9600bps 8N1, IDLE wake detection
		;	SPI	Setup clock phase, etc..

		ldab	#ADCTL_MULT|ADCTL_SCAN|ADCTL_CC
		stab	ADCTL

		staa	SCCR1
		ldab	#SCCR2_TE|SCCR2_RE
		stab	SCCR2
		ldab	#BAUD_9600
		stab	BAUD

		; SPI, .A still 0
		;
		; CPOL=0, CPHA=0, clkrate= E/2 (614KHz)

		ldab	#SPCR_MSTR	; make master, disable
		stab	SPCR
		bset	SPCR,SPCR_SPE	; enable

		; ICR - Input capture registers
		;
		; Setup input capture registers for counter inputs and setup
		; OC1 to handle system timing.
		;

		bset	TCTL2,TCTL2_EDG1A|TCTL2_EDG2A|TCTL2_EDG3A
		bset	TMSK1,TMSK1_IC1I|TMSK1_IC2I|TMSK1_OC1I

		; bootstrap interrupt vectors (.A still 0)
		;
		; When bootstrapping to ram, the 68HC11F1 implements pseudo
		; vectors and we have to create a jump table for the vectors
		; we use.

	       ifnconst INEEPROM
		ldab	#$7E		; JMP extended instruction
		stab	BsOC1Jmp
		stab    BsIC1Jmp
		stab    BsIC2Jmp
		ldx	#OC1Int		; address of OC1 interrupt
		stx	BsOC1Jmp+1
		ldx	#IC1Int
		stx	BsIC1Jmp+1
		ldx	#IC2Int
		stx	BsIC2Jmp+1
	       eif

		; Setup stack and various ram variables, .A still 0
		;
		; We do not have to clear RTIWDog since it is reset by packet
		; reception and the relays are off anyway before then.  We
		; do not have to clear the red/grn count either because the
		; l.e.d's are already off.
		;

		lds	#StackEnd-1	; reset stack pointer
		staa	RTUConfig
		staa	RTIAvrCnt
		cli			; enable ints (note: stop disble is 1)

		;-----------------------------------------------------
		;		  MAIN PACKET WAIT LOOP		     -
		;-----------------------------------------------------
		;
		; Wait for an incomming packet 
		;
		; The RED led is lit when an error is detected
		;
		; The GRN led is lit whenever we receive a valid packet
		; whos address is for us.

PKT_LEN		equ	0
PKT_ADR		equ	1
PKT_CMD		equ	2
PKT_DATA	equ	3

		subroutine

WaitPacketRed	
		ldaa	#56		; 3 seconds
		staa	RTIRedCnt
		bclr	PORTF,PORTF_RED

		; Wait for a packet.  We keep a running checksum while
		; bringing in the packet and setup the loop such that
		; .X will point to the CHK byte when done rather then
		; one past the CHK byte.
		;
		;	AA
		;	LEN		\
		;	ADR		|
		;	CMD		| LEN bytes
		;	DATA[LEN-4]	|
		;	CHK		/
		;	55
		;
		; CHK does not include AA or 55.  CHK is formulated such
		; that it all adds to zero.

WaitPacket
		bsr	GetB		; get protocol/synch
		cmpb	#$AA		; ignore if not $AA packet
		bne	WaitPacketRed
		clra			; checksum accumulation
		ldx	#PacketBuf-1
.1		bsr	GetB		; get byte (first byte is length)
		inx
		stab	0,x		; store
		aba			; accumulate checksum
		dec	PacketBuf
		bne	.1
		tsta
		bne	WaitPacketRed
		bsr	GetB		; obtain endsynch and check
		cmpb	#$55
		bne	WaitPacketRed
					; check address
		ldaa	PacketBuf+PKT_ADR
		bmi	.10
		cmpa	BoardAddr
		bne	WaitPacket
.10
		; Now have a valid packet, setup for return and
		; execute command
		;

		ldab	PacketBuf+PKT_CMD
		ldaa	#22		; aprox set wdog 5 min (sto msb)
		staa	RTIWDog	

		ldx	#CommandTable	; lookup the command
		andb	#$0E		; force command to be valid
		abx
		ldx	0,x		; get pointer to command

					; setup part of response
		ldaa	#$20
		staa	PacketBuf+PKT_ADR
		ldaa	RTUConfig
		staa	PacketBuf+PKT_CMD

		jsr	0,x		; Execute the command
		bsr	TransmitPacket
		bra	WaitPacket

		; Upon completion transmit back .B bytes of data.. that is,
		; a packet:
		;    AA LEN ADR+$20 FLG DATA[.B] CHK $55	(LEN=.B+4)

TransmitPacket
		ldaa	#9		; set green led 0.5 sec
		staa	RTIGrnCnt
		bclr	PORTF,PORTF_GRN

		addb	#4
		stab	PacketBuf+PKT_LEN
		ldx	#PacketBuf
		clra			; checksum
		bclr	SCCR2,SCCR2_RE	; disable receiver
		ldab	#$AA		; sync
.110		bsr	SendB		; send loop, everything except checksum
		ldab	0,x
		inx
		sba
		dec	PacketBuf+PKT_LEN
		bne	.110
		aba
		tab			; send checksum
		bsr	SendB
		ldab	#$55
		bsr	SendB
.120		brclr	SCSR,SCSR_TC,.120
		bset	SCCR2,SCCR2_RE	; reenable receiver
		rts

		; GETB - receive a character from the serial port
		;

		subroutine
GetBLoop	wai
GetB 		brclr	SCSR,SCSR_RDRF,GetBLoop
		ldab	SCDR
		rts

		; SENDB - transmit a character to the serial port
		;

		subroutine
SendBLoop	wai
SendB 		brclr	SCSR,SCSR_TDRE,SendBLoop
		stab	SCDR
		rts

		;-----------------------------------------------------
		;		     	COMMANDS
		;-----------------------------------------------------
		;
		;  A command must return the response length in .B, 0
		;  if no data (returns a packet with no data)
		;
		;  00	IDENTIFY
		;  02   READ BYTES
		;  04   WRITE BYTES
		;  06   PROGRAM EEPROM BYTE
		;  08   RESET
		;  0A   CONFIGURE
		;  0C   BAD CMD
		;  0E   BAD CMD

CommandTable
		dc.w	IdentifyCmd
		dc.w	ReadBytesCmd
		dc.w	WriteBytesCmd
	       ifconst INEEPROM
		dc.w	BadCmd
	       else
		dc.w	ProgramByteCmd
	       eif
		dc.w	ResetCmd
		dc.w	ConfigureCmd
		dc.w	BadCmd
		dc.w	BadCmd

		;  BADCMD
		;
		;  Bad command, simply ignore... return an empty packet

BadCmd		subroutine
		clrb
		rts

		;  CONFIGURECMD
		;
		;  Configure an I/O board, copy data to RTUConfig and PORTA

ConfigureCmd	subroutine
		ldd	PacketBuf+PKT_DATA
		staa	RTUConfig
		stab	PORTA
		clrb
		rts

		;   IDENTIFY	- identify board
		;
		;   returns board type, board address, board temp, board id
		;
		;   IdentifyUncCmd only identifies unconfigured boards

		subroutine
IdentifyCmd
		ldaa	#1
		staa	PacketBuf+PKT_DATA
		ldaa	BoardAddr
		staa	PacketBuf+PKT_DATA+1
		ldd	ADR1
		std	PacketBuf+PKT_DATA+2
		ldab	#4
		rts

		;   READ BYTES - read data bytes
		;
		;

ReadBytesCmd	subroutine
		ldx	PacketBuf+PKT_DATA	; source address
		ldy	#PacketBuf+PKT_DATA+3	; destination address
memCpy		clrb
memCpyLoop	ldaa	0,x
		staa	0,y
		inx
		iny
		incb
		cmpb	PacketBuf+PKT_DATA+2
		bne	memCpyLoop
		addb	#3
		rts

		;   WRITE BYTES - write data bytes, written data returned
		;

WriteBytesCmd	subroutine
		ldx	#PacketBuf+PKT_DATA+3	; source address
		ldy	PacketBuf+PKT_DATA	; destination address
		bra	memCpy

	       ifnconst INEEPROM
		;   PROGRAM BYTE - write one byte of data to eeprom
		;
		;   Command:	addr cmd ADRH ADRL LEN DATA[]
		;   Response:	40+X err 

ProgramByteCmd	subroutine
		ldx	PacketBuf+PKT_DATA	; destination address
		ldab	PacketBuf+PKT_DATA+2	; # of bytes to write
		ldy	#PacketBuf+PKT_DATA+3	; source address
pbloop		pshb
		ldab	0,y
		bsr	eepromWrite
		pulb
		inx
		iny
		decb
		bne	pbloop
		; clrb
		rts

		; eepromWrite - write data in .B to eeprom address .X
		;
		; .B, .X left alone
		;

eepromWrite	subroutine
		ldaa	#PPROG_BYTE|PPROG_ERASE|PPROG_EELAT
		bsr	.10
		ldaa	#PPROG_EELAT
.10		staa	PPROG
		stab	0,x
		bset	PPROG,PPROG_EEPGM

		pshb
		ldd	TCNT
		addd	#12228
.11		cpd	TCNT		; D-TCNT
		bpl	.11
		pulb

		clra
		staa	PPROG
		rts

	       eif


		;-----------------------------------------------------
		;		   COUNTER INTERRUPTS
		;-----------------------------------------------------
		;
		; IC1 - digital input counter 2 
		; IC2 - digital input counter 1

IC1Int
		ldx	Counter2
		inx
		stx	Counter2
		bclr	TFLG1,<~TFLG1_IC1F
		rti

IC2Int
		ldx	Counter1
		inx
		stx	Counter1
		bclr	TFLG1,<~TFLG1_IC2F
		rti

		;-----------------------------------------------------
		;		     GET BOARD ADDRESS		     -
		;-----------------------------------------------------
		;
		; GetBoardAddr - calculate board network address 00-0F
		;		 and return in .B
		;
		;	ADR4 (msb) and ADR3 (lsb) contain the address.  
		;	Each A/D is valued as shown below.  We conver the
		;	A/D value to a two bit address simply by multiplying
		;	it by x3 and extracting the top two bits.  The
		;	result is also flexible enough to allow variations in
		;	ADC values.
		;
		;		**xxxxxx
		;
		; 00 x 3 =  00  00000000
		; 1E x 3 =  5A  01011010
		; 37 x 3 =  A5  10100101
		; 4B x 3 =  E1  11100001

GetBoardAddr	subroutine
		clrb			; address accumulator
		ldaa	ADR4
		bsr	oneOfFour
		ldaa	ADR3
oneOfFour
		pshb
		tab
		asla
		aba
		pulb

		asla
		rolb
		asla
		rolb
		rts

		;-----------------------------------------------------
		;		     OC1 INTERRUPT
		;-----------------------------------------------------
		;
		; The OC1 interrupt is set to occur at 3840Hz (~260uS).  Each
		; interrupt switches channel so each individual channel is
		; effectively sampled at 240Hz.  256 samples are accumulated
		; yielding a 20 bit sum which is normalized to 16 bits and
		; transfered to another ram address for reading by the RTU.
		;
		; Working the math, you will also notice that 256 samples at
		; 240 Hz winds up being an integral number of 60Hz cycles,
		; therefore any 60Hz hum (and most harmonics of 60Hz) are 
		; automatically removed.
		;
		; The OC1 interrupt also serves the following functions:
		;    * Break WAI instruction in main loop for serial poll
		;    * Handle LEDs and watchdog countdown
		;    * Ensure processor runs long enough for ADC to convert

OC1Int		subroutine

		; 1.2288MHzx320 (3840Hz) Max176 scan
		;


		ldab	PORTG
		lsrb
		lsrb
		ldx	#MaxScanTable	; +0 primary, +32 acclsb, +64 accmsb
		abx
		bsr	ScanMax16	; fmt ssdddddd.ddddddxx
		andb	#$FC
		aslb
		rola
		aslb
		rola			
		bcc	.4		; if negative, msb is all 1's
		dec	64,x
.4		addd	32,x		; fmt dddddddd.dddd0000
		bcc	.5		; carry
		inc	64,x
.5		tst	RTIAvrCnt
		bne	.10		; fmt 0ddddddd.dddddddd.dddd0000
		aslb			; cvt to dddddddd.dddddddd
		tab
		rolb
		ldaa	64,x
		rola
		std	0,x
		clra			; setup to clear msb, lsb
		clrb
		staa	64,x
.10		std	32,x
		ldab	PORTG
		addb	#$08
		bpl	.20
		ldaa	RTIAvrCnt
		adda	#$02		; $10=16 samp/avr, $02=128 samp avr
		staa	RTIAvrCnt
		clrb
.20		stab	PORTG

		; Setup for next interrupt
		;
		;

		bclr	TFLG1,<~TFLG1_OC1F

		ldd	TOC1
		addd	#320
		std	TOC1
		bcc	.200

		; x65536 (18.75 Hz) code
		;

		dec	RTIRedCnt
		bne	.100
		bset	PORTF,PORTF_RED
.100		
		dec	RTIGrnCnt
		bne	.101
		bset	PORTF,PORTF_GRN
.101		
		ldx	RTIWDog
		dex
		stx	RTIWDog
		beq	.102r
.102
		bsr	GetBoardAddr	; obtain board address
		cmpb	BoardAddr
		beq	.103
		stab	BoardAddr
.102r
		clr	RTUConfig
		bclr	PORTF,PORTF_DIGOUT1|PORTF_DIGOUT2
.103

		;
		;

.200		
		rti

		; SCANMAX16 - scan Max176
		;
		; This routine may not be interrupted due to sample-and-hold
		; droop in the Max176.  We use the SPI to generate 16 clocks
		; to the Max176 after the rising edge of strobe.  The falling
		; edge of strobe must occur before the 12th bit and can occur
		; as soon as the second bit.

ScanMax16	subroutine
		bset	PORTG,PORTG_ADSTROBHI
		bsr	Get8B			; 13 from store
		bsr	Get8A			; 16 to load, 13 from store
Get8A		bclr	PORTG,PORTG_ADSTROBHI	; 6
		tba				; 2
Get8B		tst	SPSR			; 3
		ldab	SPDR			; 3
		stab	SPDR			; 3
		asla				; 2
		asla				; 2
		asra				; 2
		asra				; 2
		rts				; 5

		;-----------------------------------------------------
		;		    INTERRUPT VECTORS	     	     -
		;-----------------------------------------------------
		;
		; Interrupt Vectors
		;
		; note: if we need the space we can use some of this space
		; for code (for those interrupt vectors we do not use)

	       ifconst INEEPROM
		org	$FFD6+9*2

		;;dc.w	BadInt		; SCI serial
		;;dc.w	BadInt		; SPIE
		;;dc.w	BadInt		; PAII Pulse Accumulator
		;;dc.w	BadInt		; PAOVI Pulse Accumulator Overflow
		;;dc.w	BadInt		; TOI Timer overflow
		;;dc.w	BadInt		; OC5
		;;dc.w	BadInt		; OC4
		;;dc.w	BadInt		; OC3
		;;dc.w	BadInt		; OC2
		dc.w	OC1Int		; OC1
		dc.w	BadInt		; IC3
		dc.w	IC2Int		; IC2
		dc.w	IC1Int		; IC1

		dc.w	BadInt		; RTI Realtime interrupt
		dc.w	BadInt		; IRQ
		dc.w	BadInt		; XIRQ
		dc.w	BadInt		; SWI
		dc.w	Reset		; Illegal OpCode Trap
		dc.w	Reset		; COP failure (reset)
		dc.w	Reset		; Clock Monitor Failure
		dc.w	Reset		; RESET

	       eif

