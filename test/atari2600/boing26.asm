
        processor 6502

; TIA (Stella) write-only registers
;
Vsync		equ	$00
Vblank		equ	$01
Wsync		equ	$02
Rsync		equ	$03
Nusiz0		equ	$04
Nusiz1		equ	$05
ColuP0          equ     $06
ColuP1          equ     $07
Colupf		equ	$08
ColuBK          equ     $09
Ctrlpf		equ	$0A
Refp0		equ	$0B
Refp1		equ	$0C
Pf0             equ     $0D
Pf1             equ     $0E
Pf2             equ     $0F
RESP0           equ     $10
RESP1           equ     $11
Resm0		equ	$12
Resm1		equ	$13
Resbl		equ	$14
Audc0		equ	$15
Audc1		equ	$16
Audf0		equ	$17
Audf1		equ	$18
Audv0		equ	$19
Audv1		equ	$1A
GRP0            equ     $1B
GRP1            equ     $1C
Enam0		equ	$1D
Enam1		equ	$1E
Enabl		equ	$1F
HMP0            equ     $20
HMP1            equ     $21
Hmm0		equ	$22
Hmm1		equ	$23
Hmbl		equ	$24
VdelP0          equ     $25
VdelP1          equ     $26
Vdelbl		equ	$27
Resmp0		equ	$28
Resmp1		equ	$29
HMOVE           equ     $2A
Hmclr		equ	$2B
Cxclr		equ	$2C
;
; TIA (Stella) read-only registers
;
Cxm0p		equ	$00
Cxm1p		equ	$01
Cxp0fb		equ	$02
Cxp1fb		equ	$03
Cxm0fb		equ	$04
Cxm1fb		equ	$05
Cxblpf		equ	$06
Cxppmm		equ	$07
Inpt0		equ	$08
Inpt1		equ	$09
Inpt2		equ	$0A
Inpt3		equ	$0B
Inpt4		equ	$0C
Inpt5		equ	$0D
;
; RAM definitions
; Note: The system RAM maps in at 0080-00FF and also at 0180-01FF. It is
; used for variables and the system stack. The programmer must make sure
; the stack never grows so deep as to overwrite the variables.
;
RamStart	equ	$0080
RamEnd		equ	$00FF
StackBottom	equ	$00FF
StackTop	equ	$0080
;
; 6532 (RIOT) registers
;
SWCHA           equ     $0280
Swacnt		equ	$0281
SWCHB		equ	$0282
Swbcnt		equ	$0283
Intim		equ	$0284
Tim1t		equ	$0294
Tim8t		equ	$0295
Tim64t		equ	$0296
T1024t		equ	$0297
;
; ROM definitions
;
RomStart        equ     $F000
RomEnd          equ     $FFFF
IntVectors      equ     $FFFA
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

s1              EQU     $80
s2              EQU     $82
s3              EQU     $84
s4              EQU     $86
s5              EQU     $88
s6              EQU     $8A
DelayPTR        EQU     $8C
LoopCount       EQU     $8E
TopDelay        EQU     $8F
BottomDelay     EQU     $90
MoveCount       EQU     $91
Temp            EQU     $92
RotateDir       EQU     $93
SkipFrame       EQU     $94
VerticalDir     EQU     $95
HorizontalDir   EQU     $96
VerticalPos     EQU     $97
HorizontalPos   EQU     $98
SoundQ          EQU     $99
SkipMove        EQU     $9a
EggMode         EQU     $9b
FrameCycle      EQU     $9c
EggMusic        EQU     $9d


       ORG $F000


Cart_Init:
		SEI				; Disable interrupts.:
		CLD				; Clear "decimal" mode.

		LDX	#$FF
		TXS				; Clear the stack

Common_Init:
		LDX	#$28		; Clear the TIA registers ($04-$2C)
		LDA	#$00
TIAClear:
		STA	$04,X
		DEX
                BPL     TIAClear        ; loop exits with X=$FF
	
		LDX	#$FF
RAMClear:
		STA	$00,X		; Clear the RAM ($FF-$80)
		DEX
                BMI     RAMClear        ; loop exits with X=$7F
	
		LDX	#$FF
		TXS				; Reset the stack
 
IOClear:
		STA	Swbcnt		; console I/O always set to INPUT
		STA	Swacnt		; set controller I/O to INPUT

DemoInit:       LDA     #$01
                STA     VdelP0
                STA     VdelP1
                LDA     #$03
                STA     Nusiz0
                STA     Nusiz1
                LDA     #$36      ; a nice shade of red
                STA     ColuP0
                STA     ColuP1
                LDA     #$ff      ; page to get gfx from initially
                STA     s1+1
                STA     s2+1
                STA     s3+1
                STA     s4+1
                STA     s5+1
                STA     s6+1
                LDA     #0        ; offset in the gfx data
                STA     s1
                LDA     #50       ; offset in the gfx data
                STA     s2
                LDA     #100      ; offset in the gfx data
                STA     s3
                LDA     #150      ; offset in the gfx data
                STA     s4
                LDA     #200      ; offset in the gfx data
                STA     s5
                LDA     #0        ; vestigial!
                STA     s6
                LDA     #$01      ; +1 or -1, rotating the ball
                STA     RotateDir
                LDA     #$01      ; Vertical direction, +1 or -1
                STA     VerticalDir
                LDA     #$01      ; Same for horizontal
                STA     HorizontalDir
                STA     SoundQ    ; Start out by making a noise
                LDA     #1
                STA     SkipFrame
                STA     SkipMove
                LDA     #0
                STA     TopDelay
                STA     MoveCount
                LDA     #120
                STA     BottomDelay
                LDA     #$f2
                STA     DelayPTR+1
                LDA     #$1d+36 ;?????
                STA     DelayPTR
                STA     Wsync
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                STA     RESP0
                STA     RESP1
                LDA     #$50    ;?????
                STA     HMP1
                LDA     #$40    ;?????
                STA     HMP0
                STA     Wsync
                STA     HMOVE
                STA     Wsync
                LDA     #$0f
                STA     ColuBK

NewScreen:
                LDA     #$02
		STA	Wsync		; Wait for horizontal sync
		STA	Vblank		; Turn on Vblank
                STA	Vsync		; Turn on Vsync
		STA	Wsync		; Leave Vsync on for 3 lines
		STA	Wsync
		STA	Wsync
                LDA     #$00
		STA	Vsync		; Turn Vsync off

                LDA     #43             ; Vblank for 37 lines
                                        ; changed from 43 to 53 for 45 lines PAL
		STA	Tim64t		; 43*64intvls=2752=8256colclks=36.2lines

                JSR     DoSound         ; was too big to leave inline :P
                
                INC     FrameCycle      ; we'll use this for color cycling and
                                        ; possibly for sound

                DEC     SkipFrame
                BNE     Movement        ; skip the animation most of the time
                LDA     #3              ; number of frames to skip
                STA     SkipFrame       ; if it's zero, reset it

SkipSkip:       LDA     EggMode         ; check for easter egg :)
                CMP     #0              ; no? how sad.
                BNE     Movement
                LDA     RotateDir       ; which direction to rotate it in?
                CLC
                ADC     s1+1            ; add that to the gfx page
                ORA #$F8                ; there are only 8 so mask the rest
                STA s1+1
                STA s2+1
                STA s3+1
                STA s4+1
                STA s5+1
                
                


Movement:       LDA     #$1
                BIT     SWCHB           ; is someone pushing reset?
                BNE     NoReset         ; no? how sad.
                STA     EggMode         ; set egg mode
                JMP     MoveDelay       
NoReset:        LDA     #0
                STA     EggMode         ; no select = no easter egg.
                                        ; yes, I could have made it harder to find.
                                        ; gimme a break, I'm feeling good ;)

MoveDelay:      JSR     CheckEgg        ; override animation if necessary
                LDA     SkipMove        
                INC     SkipMove
                AND     #1              ; basically i lamed out and said 
                BNE     MoveHorizontal  ; "skip every other frame"
                JMP     VblankLoop

MoveHorizontal: LDA     HorizontalPos   ; i couldn't figure out how to use HMOVE 
                CLC                     ; without blowing up yet, so let's glom 
                ADC     HorizontalDir   ; onto the joystick routines
                STA     HorizontalPos
                LDA     HorizontalDir
                CMP     #0
                BMI     GoLeft
GoRight:        JSR     Right
                LDA     HorizontalPos
                CMP     #112            ; i also haven't figured out how to make the
                BNE     MoveVertical    ; sprite go all the way to the right edge!
                LDA     HorizontalDir   ; since we're not using the 6th copy
                LDA     #$FF
                STA     HorizontalDir
		LDA     #1              ; if we're reversing direction, we've hit a wall
                STA     SoundQ          ; so make a sound
                LDA     RotateDir
                EOR     #$FE
                STA     RotateDir       ; and change 1 into -1 (255)
                JMP     MoveVertical
GoLeft:         JSR     Left
                LDA     HorizontalPos
                CMP     #1
                BNE     MoveVertical
                LDA     #$01
                STA     HorizontalDir
                STA     SoundQ
                LDA     RotateDir
                EOR     #$FE
                STA     RotateDir
MoveVertical:   LDA     VerticalPos
                CLC
                ADC     VerticalDir
                STA     VerticalPos
                LDA     VerticalDir
                CMP     #0
                BMI     GoUp
GoDown:         JSR     Down
                LDA     VerticalPos
                CMP     #120             ; kind of a rough approximation, yeah
                BNE     EndMove
                LDA     #$FF
                STA     VerticalDir
		LDA     #1
                STA     SoundQ
                LDA     RotateDir
                EOR     #$FE
                STA     RotateDir
                JMP     EndMove
GoUp:           JSR     UP
                LDA     VerticalPos
                CMP     #1
                BNE     EndMove
                LDA     #$01
                STA     VerticalDir
                STA     SoundQ
                LDA     RotateDir
                EOR     #$FE
                STA     RotateDir
EndMove:        JMP     VblankLoop
                                


UP:             LDA     TopDelay
                BEQ     U1
                DEC     TopDelay
                INC     BottomDelay
U1:             RTS ; was JMP     VblankLoop

Down:           LDA     BottomDelay
                BEQ     D1
                INC     TopDelay
                DEC     BottomDelay
D1:             RTS; was JMP     VblankLoop

Right:          LDX     MoveCount
                INX 
                STX     MoveCount
                CPX     #3
                BNE     R2
                LDX     DelayPTR
                DEX
                STX     DelayPTR
                CPX     #$1c ;?????
                BNE     R1
                LDA     #$1d ;?????
                STA     DelayPTR
                LDA     #2
                STA     MoveCount
                RTS; was JMP     VblankLoop
R1:             LDA     #0
                STA     MoveCount
R2:             LDA     #$f0
                STA     HMP0
                STA     HMP1
                STA     Wsync
                STA     HMOVE
                RTS; was JMP     VblankLoop

Left:           LDX     MoveCount
                DEX
                STX     MoveCount
                CPX     #$ff
                BNE     L2
                LDX     DelayPTR
                INX
                STX     DelayPTR
                CPX     #$1d+37 ; indexing into a code segment with a literal - naughty
                BNE     L1
                LDA     #$1d+36 ; indexing into a code segment with a literal - naughty
                STA     DelayPTR
                LDA     #0
                STA     MoveCount
                RTS; was JMP     VblankLoop
L1:             LDA     #2
                STA     MoveCount
L2:             LDA     #$10
                STA     HMP0
                STA     HMP1
                STA     Wsync
                STA     HMOVE
                RTS; was JMP     VblankLoop

                ORG     $F200
VblankLoop:
		LDA	Intim
		BNE	VblankLoop	; wait for vblank timer
		STA	Wsync		; finish waiting for the current line
		STA	Vblank		; Turn off Vblank

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ScreenStart:
                LDY     TopDelay
                INY     ;?????
X1:             STA     Wsync
                DEY
                BNE     X1
                LDY     #4 ;?????
X2:             DEY
                BPL     X2
                LDA     #49 ; 50 pixels high
                STA     LoopCount
                JMP     (DelayPTR)
JNDelay:        .byte   $c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9
                .byte   $c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9
                .byte   $c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9
                .byte   $c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9,$c9,$c5
                NOP
X3:             NOP
                NOP
                NOP
                LDY     LoopCount
                LDA     (s1),Y
                STA     GRP0
                LDA     (s2),Y
                STA     GRP1
                LDA     (s3),Y
                STA     GRP0
;                LDA     (s6),Y
                lda     $00
                LDA     #$00
                STA     Temp
                LDA     (s5),Y
                TAX
                LDA     (s4),Y
                LDY     Temp
                STA     GRP1
                STX     GRP0
                STY     GRP1
                STA     GRP0
                DEC     LoopCount
                BPL     X3
                LDA     #0
                STA     GRP0
                STA     GRP1
                STA     GRP0
                STA     GRP1
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                LDY     BottomDelay
                INY     ;?????
X4:             STA     Wsync
                DEY
                BNE     X4
                LDA     #$02
                STA     Vblank
                STA     Wsync
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
OverscanStart:  LDA     #34             ;skip 30 lines (overscan)
		STA	Tim64t

OverscanLoop:
		LDA	Intim
		BNE	OverscanLoop	; wait for Overscan timer

OverscanDone:	STA	Wsync		; finish waiting for the current line


                JMP     NewScreen


; sound routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DoSound:        LDA     EggMode         ; if egg mode is set, we do it differently
                CMP     #0
                BEQ     StartSound
                LDA     EggMusic
                AND     #$7F            ; we're doing 16 bars of 8th notes
                TAY
                LDA     FrameCycle
                AND     #1
                BEQ     UseSet2
UseSet1:        LDA     EggMusic1,Y     ; channel 1 notes
                CMP     #0
                BEQ     SetVol1
                STA     Audf0
                LDA     #5              ; I guess we'll try this for now
                STA     Audc0
                LDA     #3             
SetVol1:        STA     Audv0
                LDA     EggMusic2,Y     ; channel 2 notes
                CMP     #0
                BEQ     SetVol2
                STA     Audf1
                LDA     #8             ; I guess we'll try this for now
                STA     Audc1
                LDA     #5             ; left hand s/b quieter
SetVol2:        STA     Audv1
                JMP     ChangeNote
UseSet2:        LDA     EggMusic3,Y     ; channel 1 notes
                CMP     #0
                BEQ     SetVol1a
                STA     Audf0
                LDA     #5              ; I guess we'll try this for now
                STA     Audc0
                LDA     #15             
SetVol1a:       STA     Audv0
                LDA     EggMusic4,Y     ; channel 2 notes
                CMP     #0
                BEQ     SetVol2a
                STA     Audf1
                LDA     #1             ; I guess we'll try this for now
                STA     Audc1
                LDA     #12             ; left hand s/b quieter
SetVol2a:       STA     Audv1
ChangeNote:     LDA     FrameCycle      ; so we can skip every 3, 7 or 15 frames
                AND     #$07            ; let's try 15
                BNE     EndSound
                INY
                STY     EggMusic
                BNE     EndSound
StartSound:     LDA     FrameCycle      ; so we can skip every 3, 7 or 15 frames
                AND     #$03            ; let's try 15
                BNE     EndSound
                LDA     SoundQ          ; is there sound to be played?
                CMP     #0
                BEQ     EndSound        ; no? how sad.
                TAY
                CPY     #1              ; if it's note #1 we can't do the cheezy echo.
                BEQ     DoVoice1
                DEY
DoVoice2:       LDA     SoundFData,Y    ; basically you just set SoundQ to an
                STA     Audf1           ; offset and put frequency, control and
                LDA     SoundCData,Y    ; volume data in the data segment below
                STA     Audc1           ; with zero termination.  I was gonna do
                LDA     SoundVData,Y    ; a channel multiplexing music thing 
                                        ; but I'm too lame.
                LSR                     ; Divide volume in half for the cheezy echo
                STA     Audv1
                INY
DoVoice1:       LDA     SoundFData,Y    ; see above
                STA     Audf0
                LDA     SoundCData,Y
                STA     Audc0
                LDA     SoundVData,Y
                STA     Audv0
                CMP     #0
                BNE     NextNote        ; if it's not zero there's more
                STA     Audf0
                STA     Audc0
                STA     Audv1
                STA     SoundQ          ; otherwise we turn off the sound and empty the Q
                JMP     EndSound
NextNote:       INC     SoundQ
EndSound:       RTS


CheckEgg:       LDA     EggMode
                CMP     #0
                BEQ     NoEgg           ; no egg?  how sad!
                LDA     FrameCycle      ; hey, it works in adventure
                STA     ColuP0          
                STA     ColuP1          
                LDA     #$F6            
                STA     s1+1
                STA     s2+1
                STA     s3+1
                STA     s4+1
                STA     s5+1
                LDA     #1
                STA     SoundQ
                RTS
NoEgg:          LDA     #$36            ; otherwise set the color back to the
                STA     ColuP0          ; lovely shade of red.  this egg works much
                STA     ColuP1          ; more smoothly than I expected.
                LDA     #0
                STA     EggMusic
                RTS


; egg sound data 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

                ORG     $F400
EggMusic3: 
 .byte 23,0,23,0,23,23,0,0
 .byte 23,0,23,0,23,23,0,0
 .byte 23,0,19,0,29,0,26,0
 .byte 23,23,23,23,0,0,0,0
 .byte 22,0,22,0,22,0,22,0
 .byte 22,0,23,0,23,0,23,0
 .byte 23,0,26,0,26,0,23,0
 .byte 26,26,26,26,19,19,19,19
 .byte 23,0,23,0,23,23,0,0
 .byte 23,0,23,0,23,23,0,0
 .byte 23,0,19,0,29,0,26,0
 .byte 23,23,23,23,0,0,0,0
 .byte 22,0,22,0,22,0,22,0
 .byte 22,0,23,0,23,0,23,0
 .byte 19,0,19,0,22,0,28,0
 .byte 29,29,29,29,14,14,14,14

EggMusic4:
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 11,11,$00,$00,15,15,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 21,21,$00,$00,13,13,$00,$00
 .byte 21,21,$00,$00,18,18,16,16
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 11,11,$00,$00,15,15,$00,$00
 .byte 15,15,$00,$00,21,21,$00,$00
 .byte 21,21,$00,$00,13,13,$00,$00
 .byte 15,15,21,21,18,18,16,16

                ORG     $F500
EggMusic1:  
 .byte 19,0,19,0,19,19,0,0
 .byte 19,0,19,0,19,19,0,0
 .byte 19,0,14,0,23,0,22,0
 .byte 19,19,19,19,0,0,0,0
 .byte 17,0,17,0,17,0,17,0
 .byte 17,0,19,0,19,0,19,0
 .byte 19,0,20,0,20,0,17,0
 .byte 19,19,19,19,15,15,15,15
 .byte 19,0,19,0,19,19,0,0
 .byte 19,0,19,0,19,19,0,0
 .byte 19,0,14,0,23,0,22,0
 .byte 19,19,19,19,0,0,0,0
 .byte 17,0,17,0,17,0,17,0
 .byte 17,0,19,0,19,0,19,0
 .byte 15,0,15,0,17,0,22,0
 .byte 23,23,23,23,23,23,23,23


EggMusic2:  
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00
 .byte 28,0,1,$00,28,0,1,$00

; graphics data (boing ball)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                ORG     $F600
; first column

 .byte   $33,$37,$3e,$3c,$36,$33,$31,$00
 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $03,$03,$03,$03,$03,$03,$03,$00
 .byte   $3f,$00,$1f,$06,$06,$06,$06,$0e
 .byte   $06,$00,$03,$03,$01,$00,$01,$03
 .byte   $03,$00,$31,$31,$31,$3f,$31,$31
 .byte   $31,$00

; second column

 .byte   $9f,$31,$31,$31,$31,$31,$b1,$00
 .byte   $33,$37,$3e,$33,$31,$31,$3f,$00
 .byte   $03,$03,$03,$f3,$03,$03,$fb,$00
 .byte   $ff,$00,$9e,$03,$01,$1f,$31,$31
 .byte   $1f,$00,$1b,$bb,$f3,$e3,$f3,$bb
 .byte   $1b,$00,$b1,$b1,$bf,$b1,$b1,$9b
 .byte   $8e,$00

; third column

 .byte   $3e,$b3,$b1,$b1,$b1,$b3,$be,$00
 .byte   $9f,$31,$31,$b1,$b1,$b1,$1f,$00
 .byte   $39,$73,$e3,$3b,$1b,$1b,$f1,$00
 .byte   $ff,$00,$1e,$03,$81,$9f,$b1,$b1
 .byte   $1f,$00,$1b,$1b,$5b,$fb,$fb,$b9
 .byte   $18,$00,$b0,$b0,$bf,$b1,$b1,$31
 .byte   $3f,$00

; fourth column

 .byte   $3f,$30,$b0,$b0,$b0,$30,$30,$00
 .byte   $3f,$b1,$b1,$bf,$b1,$b1,$3f,$00
 .byte   $f3,$1b,$1b,$1b,$1b,$1b,$f3,$00
 .byte   $ff,$00,$1e,$03,$81,$9f,$b1,$b1
 .byte   $1f,$00,$19,$1b,$f8,$19,$1b,$b3
 .byte   $e1,$00,$30,$30,$3f,$b1,$b1,$b1
 .byte   $3f,$00

; fifth column

 .byte   $63,$63,$7f,$63,$63,$36,$1c,$00
 .byte   $00,$80,$80,$00,$80,$80,$00,$00
 .byte   $18,$18,$58,$f8,$f8,$b8,$18,$00
 .byte   $ff,$00,$20,$00,$90,$98,$9c,$8e
 .byte   $0e,$00,$f0,$18,$18,$f0,$00,$30
 .byte   $e0,$00,$0c,$0c,$0c,$9e,$b3,$b3
 .byte   $33,$00

; sound data (bounce noise)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                ORG     $F700
SoundFData: .byte $1f,$19,$1a,$1b,$1c,$1d,$1e,$1f
                ORG     $F750
SoundCData: .byte $07,$06,$06,$06,$06,$06,$06,$06
                ORG     $F7A0
SoundVData: .byte $0f,$0b,$0a,$08,$06,$04,$02,$00


                ORG     $F800
; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $01,$01,$03,$06,$06,$0d,$03,$23
 .byte   $23,$23,$23,$27,$47,$46,$48,$f8
 .byte   $38,$38,$38,$3c,$3c,$3c,$3c,$3d
 .byte   $19,$01,$21,$20,$10,$10,$18,$08
 .byte   $0c,$0d,$01,$00,$00,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$03,$03,$0e,$0e,$8e,$8e
 .byte   $9f,$1f,$18,$00,$60,$e0,$e0,$e0
 .byte   $e0,$e0,$e3,$ef,$9f,$1f,$1f,$1f
 .byte   $1f,$0f,$0f,$0f,$0e,$10,$38,$f8
 .byte   $f8,$fc,$fc,$fc,$fe,$fe,$79,$60
 .byte   $40,$e0,$e0,$f0,$78,$3c,$1d,$08
 .byte   $00,$00

; third column

 .byte   $00,$40,$78,$3e,$1f,$0f,$06,$08
 .byte   $3c,$fc,$fc,$fe,$fe,$7f,$7e,$78
 .byte   $60,$40,$c0,$c0,$c0,$e0,$e0,$e0
 .byte   $e1,$e7,$ff,$0f,$0f,$0f,$0f,$07
 .byte   $07,$07,$02,$04,$3c,$fc,$fe,$fe
 .byte   $fe,$7e,$7f,$38,$00,$70,$f8,$78
 .byte   $1c,$0e

; fourth column

 .byte   $00,$00,$00,$00,$98,$fc,$1e,$1f
 .byte   $0f,$07,$03,$00,$0e,$7e,$fe,$7f
 .byte   $7f,$7f,$3f,$3f,$3f,$3c,$00,$60
 .byte   $e0,$e0,$e0,$e0,$f0,$f0,$f0,$f1
 .byte   $ff,$cf,$0f,$0f,$07,$07,$07,$06
 .byte   $08,$38,$f0,$f1,$72,$70,$60,$60
 .byte   $40,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $80,$d0,$b8,$18,$18,$0c,$0c,$06
 .byte   $06,$06,$84,$9c,$7c,$7c,$7c,$3c
 .byte   $3c,$3c,$3e,$3e,$3c,$22,$42,$c2
 .byte   $c2,$c4,$c4,$c4,$c0,$88,$f0,$60
 .byte   $c0,$c0,$80,$80,$00,$00,$00,$00
 .byte   $00,$00


                ORG   $F900

; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $01,$01,$02,$06,$04,$0b,$17,$27
 .byte   $27,$27,$07,$0f,$4f,$4e,$40,$b0
 .byte   $70,$70,$30,$30,$30,$30,$38,$39
 .byte   $3f,$27,$07,$23,$01,$11,$11,$00
 .byte   $08,$09,$03,$01,$00,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$01,$01,$18,$1c,$dc,$9c
 .byte   $3c,$3c,$3b,$61,$01,$81,$81,$81
 .byte   $81,$81,$83,$8f,$ff,$7f,$7f,$7f
 .byte   $3f,$3f,$3f,$3f,$3e,$20,$00,$e0
 .byte   $e0,$e0,$f0,$f0,$f8,$f8,$ff,$e3
 .byte   $81,$81,$c0,$e0,$f0,$78,$3f,$10
 .byte   $00,$00

; third column

 .byte   $00,$01,$c0,$fc,$7e,$3f,$1e,$00
 .byte   $30,$f0,$f8,$f8,$f8,$fc,$fd,$fb
 .byte   $e3,$81,$01,$01,$00,$00,$00,$00
 .byte   $81,$87,$9f,$7f,$7f,$3f,$3f,$3f
 .byte   $1f,$1f,$1e,$08,$30,$f0,$f0,$f8
 .byte   $f8,$f8,$fc,$fb,$61,$41,$f0,$f8
 .byte   $1c,$00

; fourth column

 .byte   $00,$80,$60,$18,$00,$7c,$7c,$3e
 .byte   $3f,$1f,$1f,$0c,$08,$78,$fc,$fc
 .byte   $fc,$fe,$fe,$fe,$ff,$fc,$60,$00
 .byte   $80,$80,$80,$c0,$c0,$c0,$c0,$c1
 .byte   $ef,$df,$1f,$1f,$1f,$1f,$1f,$1e
 .byte   $10,$21,$e1,$e1,$e3,$e4,$e0,$c0
 .byte   $80,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $00,$90,$b8,$38,$38,$1c,$1c,$0e
 .byte   $0e,$0e,$00,$19,$f8,$f8,$f8,$f8
 .byte   $f8,$f8,$f8,$fc,$ff,$66,$06,$86
 .byte   $86,$84,$8c,$0c,$0c,$00,$60,$c0
 .byte   $c0,$80,$80,$00,$00,$00,$00,$00
 .byte   $00,$00



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                ORG     $FA00
; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $01,$02,$06,$04,$08,$0b,$17,$2f
 .byte   $2f,$0f,$0f,$0e,$5e,$5f,$10,$a0
 .byte   $60,$60,$60,$60,$60,$60,$60,$31
 .byte   $37,$2f,$0f,$07,$07,$03,$13,$01
 .byte   $01,$09,$03,$01,$00,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$00,$03,$18,$38,$38,$38
 .byte   $38,$78,$7f,$e7,$87,$07,$07,$07
 .byte   $07,$03,$00,$0c,$7c,$fc,$fc,$fe
 .byte   $fe,$fe,$fe,$fe,$ff,$60,$40,$80
 .byte   $80,$c0,$c0,$c0,$e0,$e0,$e7,$ef
 .byte   $87,$03,$83,$81,$c1,$60,$33,$0b
 .byte   $00,$00

; third column

 .byte   $00,$06,$c1,$f0,$f8,$7c,$7f,$30
 .byte   $20,$c0,$e0,$e0,$e0,$f0,$f1,$f7
 .byte   $e7,$87,$07,$07,$03,$03,$03,$03
 .byte   $00,$06,$1e,$fe,$fe,$ff,$ff,$ff
 .byte   $7f,$7f,$7e,$78,$00,$c0,$e0,$e0
 .byte   $f0,$f0,$f0,$ff,$e7,$83,$c1,$e1
 .byte   $f1,$10

; fourth column

 .byte   $00,$00,$80,$60,$10,$70,$f8,$fc
 .byte   $fc,$7e,$3f,$3c,$10,$60,$e0,$f0
 .byte   $f0,$f0,$f8,$f8,$fc,$ff,$e3,$83
 .byte   $03,$01,$01,$01,$01,$01,$01,$00
 .byte   $0e,$be,$7e,$7e,$7e,$7e,$7e,$3f
 .byte   $31,$03,$c3,$c3,$c2,$e4,$e0,$e0
 .byte   $c0,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$80,$40
 .byte   $30,$00,$70,$f0,$78,$78,$38,$3c
 .byte   $3c,$1e,$18,$11,$f1,$f1,$f1,$f1
 .byte   $f1,$f0,$f0,$f8,$fa,$e6,$8e,$0e
 .byte   $0c,$0c,$0c,$0c,$18,$10,$40,$c0
 .byte   $80,$80,$00,$00,$00,$00,$00,$00
 .byte   $00,$00


                ORG   $FB00

; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$01
 .byte   $01,$02,$06,$04,$09,$12,$1e,$0e
 .byte   $1e,$1c,$1c,$1c,$1c,$3d,$33,$03
 .byte   $c3,$c3,$c3,$41,$41,$61,$61,$60
 .byte   $66,$7f,$0f,$0f,$0f,$07,$07,$03
 .byte   $03,$00,$07,$03,$01,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$01,$03,$01,$11,$70,$70
 .byte   $70,$70,$e7,$ff,$9f,$1f,$1f,$1f
 .byte   $1f,$1f,$1c,$10,$70,$f0,$f0,$f0
 .byte   $f8,$f8,$f8,$f8,$f9,$e7,$c3,$03
 .byte   $03,$01,$01,$01,$80,$80,$87,$df
 .byte   $bf,$1f,$0f,$87,$c3,$61,$33,$13
 .byte   $00,$00

; third column

 .byte   $00,$02,$e3,$e1,$f0,$f0,$f9,$f3
 .byte   $c3,$01,$81,$80,$80,$80,$c1,$c7
 .byte   $df,$9f,$1f,$1f,$1f,$0f,$0f,$0f
 .byte   $0e,$00,$18,$f8,$f8,$f8,$fc,$fc
 .byte   $fc,$fc,$ff,$f9,$c1,$01,$00,$80
 .byte   $80,$c0,$e0,$e7,$ef,$87,$07,$c3
 .byte   $f3,$32

; fourth column

 .byte   $00,$00,$c0,$f0,$62,$41,$e0,$f0
 .byte   $f8,$fc,$fe,$fd,$70,$00,$80,$c0
 .byte   $c0,$c0,$e0,$e0,$e0,$f3,$ef,$8f
 .byte   $0f,$0f,$0f,$07,$07,$07,$07,$02
 .byte   $0c,$3c,$fc,$fc,$fc,$fc,$fc,$fd
 .byte   $f3,$47,$87,$87,$86,$8e,$c8,$d0
 .byte   $80,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$80,$c0
 .byte   $70,$20,$60,$e0,$f0,$f0,$f8,$78
 .byte   $78,$7c,$3a,$23,$c3,$c3,$c3,$c1
 .byte   $e1,$e1,$e1,$e1,$e2,$fe,$9c,$1c
 .byte   $1c,$1c,$3c,$38,$38,$30,$48,$90
 .byte   $90,$a0,$40,$00,$00,$00,$00,$00
 .byte   $00,$00


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                ORG     $FC00
; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $02,$00,$01,$01,$0b,$12,$0c,$18
 .byte   $18,$18,$38,$38,$38,$31,$3f,$4f
 .byte   $87,$c7,$c7,$47,$47,$47,$47,$46
 .byte   $44,$5c,$1e,$1e,$0e,$0f,$0f,$07
 .byte   $07,$02,$06,$03,$01,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$01,$03,$07,$23,$61,$61
 .byte   $e1,$e1,$c6,$de,$be,$3f,$3f,$3f
 .byte   $3f,$3f,$3c,$30,$40,$c0,$c0,$c0
 .byte   $c0,$c0,$e0,$e0,$e1,$ff,$df,$0f
 .byte   $0f,$07,$07,$07,$03,$03,$04,$9e
 .byte   $fe,$3f,$1f,$1f,$8f,$47,$24,$0f
 .byte   $01,$00

; third column

 .byte   $00,$08,$07,$83,$c0,$e0,$f1,$ff
 .byte   $c7,$07,$07,$03,$03,$01,$00,$06
 .byte   $1f,$7f,$7f,$7f,$7f,$7f,$7f,$3f
 .byte   $3e,$38,$20,$e0,$e0,$e0,$e0,$f0
 .byte   $f0,$f0,$f1,$ff,$c7,$07,$03,$03
 .byte   $03,$01,$01,$86,$9f,$9f,$0f,$87
 .byte   $e3,$3c

; fourth column

 .byte   $00,$00,$80,$f0,$e6,$43,$c1,$c0
 .byte   $e0,$e0,$f0,$fb,$f3,$83,$03,$01
 .byte   $01,$01,$00,$80,$80,$83,$df,$bf
 .byte   $3f,$3f,$3f,$3f,$1f,$1f,$1f,$1e
 .byte   $10,$20,$e0,$f0,$f0,$f0,$f0,$f1
 .byte   $ff,$cf,$0e,$0e,$0e,$0e,$88,$80
 .byte   $40,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$80,$c0
 .byte   $f0,$60,$40,$c0,$e0,$e0,$f0,$f0
 .byte   $f8,$f8,$fe,$e6,$07,$03,$83,$83
 .byte   $83,$83,$83,$83,$82,$dc,$bc,$38
 .byte   $78,$78,$78,$78,$70,$78,$08,$90
 .byte   $30,$20,$40,$00,$00,$00,$00,$00
 .byte   $00,$00


                ORG  $FD00

; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $00,$00,$01,$01,$03,$14,$08,$18
 .byte   $18,$10,$30,$30,$30,$31,$6f,$5f
 .byte   $8f,$8f,$8f,$0f,$0f,$0f,$0f,$4e
 .byte   $48,$58,$38,$38,$1c,$1c,$0e,$0e
 .byte   $0e,$06,$04,$02,$01,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$00,$03,$07,$23,$23,$43
 .byte   $c3,$c3,$84,$9c,$fc,$7c,$7c,$7c
 .byte   $7c,$fc,$ff,$f1,$81,$01,$81,$81
 .byte   $80,$80,$80,$80,$81,$9f,$bf,$3f
 .byte   $3f,$3f,$1f,$1f,$1f,$0f,$08,$18
 .byte   $7c,$fc,$7e,$3e,$1f,$1f,$04,$0e
 .byte   $01,$00

; third column

 .byte   $00,$0c,$0f,$07,$83,$81,$c1,$ef
 .byte   $df,$1f,$0f,$0f,$0f,$07,$06,$00
 .byte   $1c,$7c,$fc,$fc,$fe,$fe,$fe,$ff
 .byte   $fe,$f8,$e0,$00,$00,$80,$80,$80
 .byte   $c0,$c0,$e1,$e7,$df,$1f,$0f,$0f
 .byte   $0f,$07,$07,$00,$1c,$fe,$3e,$0f
 .byte   $85,$7c

; fourth column

 .byte   $00,$00,$00,$c0,$fe,$8f,$07,$83
 .byte   $81,$c0,$e0,$e3,$ff,$8f,$0f,$07
 .byte   $07,$03,$03,$03,$01,$02,$1e,$7e
 .byte   $fe,$fe,$fe,$7f,$7f,$7f,$7f,$7e
 .byte   $70,$00,$c0,$c0,$c0,$c0,$c0,$c1
 .byte   $ee,$de,$1e,$1c,$1c,$1e,$1c,$10
 .byte   $20,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$80,$c0
 .byte   $e0,$e0,$08,$80,$80,$c4,$c0,$e2
 .byte   $e0,$f0,$f6,$ee,$0e,$06,$06,$06
 .byte   $06,$07,$07,$07,$05,$19,$78,$f8
 .byte   $f0,$f2,$f0,$f0,$f0,$f8,$90,$30
 .byte   $20,$40,$40,$80,$00,$00,$00,$00
 .byte   $00,$00




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                ORG     $FE00
; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$01
 .byte   $01,$03,$03,$07,$07,$04,$08,$10
 .byte   $10,$30,$30,$31,$21,$20,$6f,$5f
 .byte   $9f,$9f,$9f,$1f,$1f,$1f,$1f,$4e
 .byte   $48,$50,$30,$38,$18,$1c,$0c,$0e
 .byte   $0e,$07,$04,$00,$00,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$01,$03,$07,$07,$47,$cf
 .byte   $cf,$8f,$88,$18,$78,$f8,$f8,$f8
 .byte   $f8,$fc,$ff,$f3,$83,$03,$03,$01
 .byte   $01,$01,$01,$01,$00,$9f,$bf,$7f
 .byte   $7f,$3f,$3f,$3f,$1f,$1f,$18,$10
 .byte   $78,$fc,$fc,$7e,$3e,$1f,$0c,$00
 .byte   $00,$00

; third column

 .byte   $00,$3c,$9e,$8f,$07,$83,$80,$8f
 .byte   $ff,$3f,$1f,$1f,$1f,$0f,$0e,$08
 .byte   $18,$78,$f8,$f8,$fc,$fc,$fc,$fc
 .byte   $ff,$f9,$e1,$01,$01,$00,$00,$00
 .byte   $80,$80,$81,$87,$ff,$3f,$1f,$1f
 .byte   $0f,$0f,$0f,$00,$18,$7c,$3e,$1e
 .byte   $0f,$06

; fourth column

 .byte   $00,$00,$00,$80,$fe,$8f,$07,$03
 .byte   $03,$81,$c0,$c3,$ef,$9f,$1f,$0f
 .byte   $0f,$0f,$07,$07,$03,$00,$1c,$7c
 .byte   $fc,$fe,$fe,$fe,$fe,$fe,$fe,$ff
 .byte   $f1,$41,$81,$81,$81,$81,$81,$c0
 .byte   $ce,$fc,$3c,$3c,$3c,$38,$38,$30
 .byte   $20,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$00,$80
 .byte   $c0,$f0,$88,$08,$80,$84,$c4,$c2
 .byte   $c2,$e0,$e6,$ee,$0e,$0e,$0e,$0e
 .byte   $0e,$0f,$0f,$07,$05,$19,$70,$f0
 .byte   $f0,$f0,$f0,$f0,$e4,$e8,$b0,$30
 .byte   $60,$40,$c0,$80,$00,$00,$00,$00
 .byte   $00,$00


                ORG     $FF00
; first column

 .byte   $00,$00,$00,$00,$00,$00,$00,$00
 .byte   $00,$01,$01,$03,$07,$04,$01,$31
 .byte   $31,$31,$21,$21,$61,$60,$4e,$fe
 .byte   $3e,$1e,$1e,$1e,$1f,$1f,$1f,$1e
 .byte   $18,$10,$30,$30,$18,$18,$1c,$0c
 .byte   $0e,$0f,$00,$00,$00,$00,$00,$00
 .byte   $00,$00

; second column

 .byte   $00,$00,$03,$03,$07,$07,$87,$cf
 .byte   $8f,$8f,$88,$10,$70,$f0,$f0,$f8
 .byte   $f8,$f8,$fb,$f7,$87,$07,$07,$07
 .byte   $03,$03,$03,$03,$02,$1e,$3e,$fe
 .byte   $fe,$7f,$7f,$7f,$3f,$3f,$38,$00
 .byte   $60,$f0,$f8,$7c,$3c,$1e,$0c,$0c
 .byte   $00,$00

; third column

 .byte   $00,$20,$fe,$8f,$07,$07,$02,$8e
 .byte   $be,$7f,$3f,$3f,$3f,$1f,$1e,$18
 .byte   $00,$70,$f0,$f0,$f0,$f8,$f8,$f8
 .byte   $fd,$fb,$e3,$03,$03,$03,$01,$01
 .byte   $01,$01,$00,$07,$bf,$7f,$7f,$3f
 .byte   $3f,$1f,$1f,$08,$10,$78,$7c,$1e
 .byte   $0f,$00

; fourth column

 .byte   $00,$00,$00,$00,$dc,$be,$0f,$07
 .byte   $03,$01,$81,$83,$cf,$bf,$3f,$1f
 .byte   $1f,$1f,$0f,$0f,$0f,$04,$18,$78
 .byte   $f8,$fc,$fc,$fc,$fc,$fc,$fc,$fd
 .byte   $f3,$c3,$03,$03,$03,$03,$83,$82
 .byte   $8c,$bc,$7c,$38,$39,$38,$38,$60
 .byte   $40,$00

; fifth column

 .byte   $00,$00,$00,$00,$00,$00,$00,$80
 .byte   $c0,$f0,$98,$08,$08,$84,$84,$86
 .byte   $c2,$c2,$c4,$fe,$1e,$1e,$1e,$1e
 .byte   $0e,$0e,$0e,$0e,$0d,$11,$70,$f2
 .byte   $e2,$e2,$e0,$e0,$e0,$c8,$b0,$30
 .byte   $60,$40,$c0,$80,$00,$00,$00,$00
 .byte   $00,$00







;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Set up the 6502 interrupt vector table
;
		ORG	IntVectors
NMI             .word   Cart_Init
Reset           .word   Cart_Init
IRQ             .word   Cart_Init
        
;		END
