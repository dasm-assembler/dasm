; Sean Riddle's lights.asm, the first reallife program ever assembled
; by dasm2. The usual modifications to f8tool source were needed:
; - Needed to add the processor directive at the top of the source.
; - Needed to indent the org directives because dasm2 would parse
;   them as labels.
; - The first org directive sets the org fill value to 0, since this
;   is what f8tool uses and the reference binary was assembled with
;   f8tool.
;
;lights out
;by Sean Riddle
;6/2004
;members.cox.net/seanriddle
;
; Copyright (C) 2004 Sean Riddle (members.cox.net/seanriddle)
;
; lights is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; lights is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with lights; if not, write to the Free Software
; Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

;for the Channel F multigame
;it will play in MESS
;it requires MG RAM at $2800 (could be rewritten to use regs only)

;instructions:
;make all lights green
;l/r/u/d moves cursor
;push down toggles lights
;pull up replays current pattern
;when complete, the count of moves is shown
;then up replays pattern, down goes to next random
;after all 64 patterns are played randomly, the complements (red<->green) are played

;game 1 is normal - all included patterns are solvable
;game 2 is on a torus (wrap-around) !!! not all included patterns are solvable on a torus
;game 3 is lit-only (only lit buttons can be pressed)

;mode 1 toggles self and u/d/l/r neighbors - + pattern
;mode 2 only toggles u/d/l/r neighbors !!! not all included patterns are solvable in this mode
;mode 3 toggles self and nw/ne/sw/se neighbors - X pattern
;mode 4 toggles nw/ne/sw/ne - X not self 

;todo:
;build game 2 or mode 2 patterns from random legal moves
;add random legal moves to patterns to create more
;rotate, invert patterns randomly for more variation
;three-state red, green, gray
;L modes
;more symbols-greek, smilies, arcade, 
;currently 12 bytes short of 2K, but there's lots more room

;egg - certain game/mode combo, solve certain pattern in minimum moves

		processor f8

;BIOS calls
clrscrn: equ $00d0							;uses r31
delay: equ $008f
pushk: equ $0107							;used to allow more subroutine stack space
popk: equ $011e
drawchar: equ $0679

array: 			equ $2800					;use Schach RAM to hold current state of 25 lights
history: 		equ $2900					;patterns we have already used, so we don't repeat
temp4: 			equ $2a00					;temp storage of 4 bytes

		org	$800,0
CartridgeStart: db		$55	                ; valid cart indicator
                NOP                      	; unused byte

CartridgeEntry:
				LIS		$0                  ; init the h/w
                OUTS	1
                OUTS	4
                OUTS	5
                OUTS	0
                
                lisu	4					;r32=complement flag
                lisl	0
                lr		(IS),a
                
		        LI		$d6                 ; 
                LR		$3,A                ; clear screen to gray
                PI		clrscrn             ; 

				li $11
				lisu 3						;init R30 game 1, mode 1
                lisl 6
                lr (IS),a

				li $4a						;display G? prompt
				lr 0,a
				pi prompt
				lr a,4
				sl 4
				lisu 3						;r30 low nibble = game
				lisl 6
				lr (IS),a					;game 1 - normal, game 2 - wraparound; game 3 - lit only
				
				pi prompts					;display S?/M? ; mode 1 normal, mode 2 no center toggle
						
clrhist:									;clear history of patterns presented
				dci history
				li $41						; # of patterns +1
				lr 0,a
clrhist2:
				lis 0
				st
				ds 0
				bf 4,clrhist2
				
nextpat:
				lisu 2						;random #
				lisl 5
				lr a,(IS)
				
				lisu 3
				lisl 5
				as (IS)						;add to current pattern...
				ni $3f						;...mod 64...
				lr (IS),a					;...to get next pattern
				
				dci history					;check to see if we've already done this one
				adc
				lr q,dc
				lm
				ni $ff
				bt 4,unusedpat				;not yet used, display it
				
				;pattern already used, check for unused from top
				;if not found, clear array and start over
				
				dci history
checknext:
				lm
				ni $ff
				bf 4,checknext				;will always complete because we zeroed 1 extra byte in clrhist
				lr q,dc
				lr a,ql
				ai $ff						;lm moved us 1 past
				lr ql,a
				ci $40						;all done?
				bf 4,foundone				
				li $ff
				lisu 4						; set flag to start complements (random/rotations later)
				lisl 0
				lr (IS),a
				bf 0,clrhist				;no empty slots, start over
foundone:
				lr (IS),a					;update the current pattern
				
unusedpat:		
				lr dc,q						;mark it used
				li $ff
				st
				
replay:
				lisu 3						;current pattern # in r29
				lisl 5
				lr a,(IS)
				
				dci pats
				adc							; add cur pat to data pointer 5 times to get to the pat data
				adc
				adc
				adc
				adc
				
getpat:			lisu 2						;read bit pattern into regs 16-20
				lisl 0
				lis 5
				lr 1,a
getpatloop:
				lm
				lr (is)+,a
				ds 1
				bf 4,getpatloop
				
				dci array					;put byte pattern into RAM
				lis 5
				lr 2,a

				lisu 4						;first time through 0, next time FF
				lisl 0
				lr a,(IS)
				lr 3,a						;temp

				lisu 2
				lisl 0
storepatloop2:
				lis 5
				lr 1,a
				lr a,(is)+
storepatloop:
				lr 0,a
				ni $ff
				
				lr a,3
				
				bf 1,onebit
				bf 0,storepat
onebit:
				com
storepat:
				st
				lr a,0
				sl 1
				lr 0,a
				ds 1
				bf 4,storepatloop
				ds 2
				bf 4,storepatloop2

				lis 0						;init variables to 0
                lisu 3
                lisl 0
                lr (IS)+,a					;r24=row (3:0)
                lr (IS)+,a					;r25=col (3:1)
                lr (IS)+,a             		;r26=cursor (3:2)
                lr (IS)+,a					;r27=array index (3:3)
                lr (IS)+,a					;r28=parameter to toggle sub (3:4)
                lisu 2
                lisl 6
                lr (IS)+,a					;r22=press counter lo (2:6)
                lr (IS)+,a					;r23=press counter hi (2:7)

				dci grid					;draw the grid
				pi	drawlines
			
;draw lights
				li 24						;array index
				lisu 3
				lisl 3
				lr (IS),a
sqloop:
				pi drawlight
				lisu 3
				lisl 3
				lr a,(IS)
				ai $ff
				lr (IS),a
				bt 2,sqloop					;branch if >= 0

				dci currows
				lm
				lr	5,a						;upper row
				dci curcols
				lm
				lr	4,a						;left col
				dci	cursor					;start with the cursor in light 0
				pi square
				
mainloop:
				pi readhc
				lr 0,a						;save controller input
				lisu 2                		;inc random seed
				lisl 5
				lr a,(IS)
				inc
				lr (IS),a

; redraw current square without cursor
				lisu 3
				lisl 2
				lr a,(IS)+
				lr (IS),a
				pi drawlight

				lr a,0
				ni $0c
				bt 4,notupdn
				ci $08
				bf 4,down
				lisu 3						;up
				lisl 0						;r24 is row
				ds (IS)						;prev row
				bf 2,minrow					;>=0?
updcurx:
				jmp updcur
minrow:
				lis 4
				lr (IS),a	
				bf 0,updcurx
down:
				lisu 3
				lisl 0
				lr a,(IS)
				inc							;next row
				lr (IS),a
				ci 5						;too far?
				bt 4,maxrow
				bf 0,updcurx
maxrow:
				lis 0
				lr (IS),a
				bf 0,updcurx
notupdn:
				lr a,0
				ni $03
				bt 4,notlfrt
				ci $02
				bf 4,right
				lisu 3						;left
				lisl 1						;r25 is col
				ds (IS)						;prev col
				bf 2,prevcol				;>=0?
				bf 0,updcurx
prevcol:
				lis 4
				lr (IS),a
				bf 0,updcurx
right:
				lisu 3
				lisl 1
				lr a,(IS)
				inc							;next col
				lr (IS),a
				ci 5						;too far?
				bf 4,updcurx
				lis 0
				lr (IS),a
				bf 0,updcurx
notlfrt:
				lr a,0
				ni $c0
				bt 4,mainloop
				ci $80
				bt 4,pushdown
				jmp replay					;pull up replays current pattern

pushdown:
				lisu 3
				lisl 6
				lr a,(IS)
				ni $f0
				ci $40						;if game=3, can only press lit buttons
				bf 4,allowpress

				lisu 3
				lisl 2						; get cursor loc
				lr a,(IS)
				
				dci array
				adc
				lm
				ni $ff
				bf 4,allowpress
				
				;make a bad sound
				li		$3f
				lr		1,a
sndloop:
				li		$80
				outs	5
				lis		1
				lr		5,a
				pi		delay
				lis		0
				outs	5
				ds		1
				bf		4,sndloop
				
				jmp		updcur
				
allowpress:
				lisu 2						;inc press counter
				lisl 6		
				lis 1
				ai $66
				asd (IS)					;thanks Fredric!
				lr (IS)+,a
				bf 2,nocarry
				lis 1
				ai $66
				asd (IS)
				lr (IS),a
nocarry:
				lisu 3
                lisl 6						;if mode=2 or 4, no center toggle
                lr a,(IS)
                ni $f						;mode in low nibble
                ci $2
                bt 4,nocenter
                ci $8
                bt 4,nocenter
				
				lisu 3
				lisl 2						; get cursor loc
				lr a,(IS)
				
				lisl 4
				lr (IS),a
				pi toggle					;toggle center light

nocenter:
				dci plustable
				lisu 3
                lisl 6						;if mode=3 or 4, X toggle
                lr a,(IS)
                ni $f						;mode in low nibble
                ci $4
                bt 4,xtoggle				;mode 3, X toggle
                ci $8
                bt 4,xtoggle				;mode 4, X toggle
                bf 0,dotoggle
                
xtoggle:
				dci xtable
dotoggle:
                lisl 2
                lr a,(IS)					;get cursor loc
                as (IS)
                as (IS)
                as (IS)						;*4 for table offset
                adc
                lm							;get 4 neighbors
                lr 0,a
                lm
                lr 1,a
                lm
                lr 2,a
                lm
                
                dci temp4					;store them temporarily
                st			;t4
                lr a,2
                st			;t3
                lr a,1
                st			;t2
                lr a,0
                st			;t1

				dci temp4                	;toggle each neighbor
				li 3
				adc
                pi togglem	;t1
                
                dci temp4
                lm
                lm
                pi togglem	;t2
                
                dci temp4
                lm
                pi togglem	;t3
                
                dci temp4
                pi togglem	;t4
                
updcur:
				lisu 3						;calc index from row and col
				lisl 0
				lr a,(IS)
				as (IS)
				as (IS)
				as (IS)
				as (IS)						;row * 5

				lisu 3
				lisl 1
				as (IS)+					;add in col
				lr (IS),a

				dci currows					;get row and column to draw cursor
				lisu 3
				lisl 0
				lr a,(IS)+
				adc
				lm
				lr 5,a
				
				dci curcols
				lr a,(IS)
				adc
				lm
				lr 4,a
				
				dci	cursor					;draw the cursor
				pi square
				
				LI $3F						;debounce by pausing a while
				LR 5,A
                pi delay
				
				;check for all green (win)
				dci array
				li 24						;array index
				lisu 3
				lisl 3
				lr (IS),a
chkloop:
				lm
				ni $ff
				bt 4,lighton
				jmp mainloop				;not a win
lighton:
				lisu 3
				lisl 3
				lr a,(IS)
				ai $ff
				lr (IS),a
				bt 2,chkloop

				;made it here, all off
				;make a winning sound
				li		$3f
				lr		1,a
snd2loop:
				li		$40
				outs	5
				
				lr		a,1
snd2loopa:
				inc
				bf		4,snd2loopa
				
				lis		0
				outs	5
				ds		1
				bf		4,snd2loop
						
                li 32				;draw 2 concentric squares
                lr 4,a
                li 18
                lr 5,a
                dci winsq1
                pi square
                li 34
                lr 4,a
                li 20
                lr 5,a
                dci winsq2
                pi square
                
                ;print # of moves from r23/22 (2:7/6)
                li $1e				;row
                lr 2,a
                lisu 2				;1000s digit
                lisl 7
                lr a,(IS)
                sr 4
                bt 4,lt1000
                oi $40
                lr 0,a
                li $2a
                lr 1,a
                pi drawchar
                bf 0,d100

lt1000:
				li $2c
				lr 1,a
d100:				
				lr a,1
				inc
				lr 1,a
                lisu 2				;100s digit
                lisl 7
                lr a,(IS)
                ni $f
                bf 4,gt100
                lr a,(IS)			;print 0 if x0yz where x > 0
                ni $ff
                bt 4,lt100
                lis 0
                
gt100:
                oi $40
                lr 0,a
                pi drawchar
                bf 0,d10

lt100:
				li $30
				lr 1,a
d10:
				lr a,1
				inc
				lr 1,a
                lisu 2				;10s digit
                lisl 6
                lr a,(IS)
                sr 4
                bf 4,gt10
                lisl 7				;print 0 if xy0z where x or y > 0
                lr a,(IS)
                ni $ff
                bt 4,lt10
                lis 0
gt10:
                oi $40
                lr 0,a
                pi drawchar
                bf 0,d1

lt10:
				li $34
				lr 1,a
d1:
				lr a,1
				inc
				lr 1,a
                lisu 2				;1s digit
                lisl 6
                lr a,(IS)
                ni $f
                oi $40
                lr 0,a
                pi drawchar
                
waitupdn:
                pi readhc			;wait for hc up/down
                ni $c0
                bt 4,waitupdn
                ni $80
                bt 4,redo
                jmp nextpat			;down - get next random pattern
                
redo:
				lisu 3
                lisl 6
                lr a,(IS)
                ci $41
                bf 4,replay2
                
                lisl 5
                lr a,(IS)
                ci 51
                bf 4,replay2
                
				lisu 2
				lisl 7
				lr a,(IS)-
				ci 0
				bf 4,replay2
				lr a,(IS)
				ci $18
				bf 4,replay2

		        LI		$d6
                LR		$3,A
                PI		clrscrn
                		              
                dci blitparm
                pi blit
                
				li $3f
				lr 1,a
snd3loop:
				li $40
				outs 5
				li $1
				lr 5,a
				pi delay
				lis 0
				outs 5
				ds 1
				bf 4,snd3loop

				pi readhc
		        LI		$d6
                LR		$3,A
                PI		clrscrn
				
				jmp nextpat			
replay2:
				jmp replay			;up - replay current pattern


readhc:								;wait until one of the hand controllers is moved
				lis 0
				outs 0
				lis 0
				outs 4
				ins 4
				com					;un-invert port data
				ni $cf				;mask off twists, since we don't use them
				bf 4,gothc
				outs 1
				ins 1
				com
				ni $cf
				bt 4,readhc
gothc:			pop


togglem:							;change a square (index in memory) from red to green or vice versa
				lr k,p
				pi pushk
				
				lm
				lr 0,a
				ni $80				;if high bit set, then this was a wrap
				bt 4,notwrapped
				
				lisu 3
                lisl 6				;if game=$2x, allow wrap-around
                lr a,(IS)
                ni $f0				;mode in low nibble
                ci $20
				bf 4,toggleexit2	;no wrap allowed
				
notwrapped:
				lr a,0
				ni $7f				;strip off "wrapped" bit
				lisu 3
				lisl 4
				lr (IS),a
				bf 0,toggle2
				
toggle:								;change a square (index in r28) from red to green or vice versa
				lr k,p
				pi pushk

				lisu 3
				lisl 4
				lr a,(IS)		
toggle2:
				dci array
				adc
				lm
				ni $ff
				lr a,(IS)
				dci array
				adc
				bt 4,notset
				lis 0
				bf 0,toggleexit
notset:
				li $ff

toggleexit:
				st
				lisu 3
				lisl 4
				lr a,(IS)-
				lr (IS),a
				pi drawlight
toggleexit2:
				pi popk
				pk


square:							;draw a square
		; dci to color,width,height
		;r5=upper row
		;r4=left col
		; uses r2,r3,r6
				lr k,p
				pi pushk
				lm
				outs	1		;set the color
				
				lr a,5
				com
				outs 5			;set the row
				
				lm				;
				lr 5,a			;save the width
				
				lm
				lr 3,a			;store the height

squarev:
				lr a,4
				lr 2,a			;save col
				lr a,5			;get width
squareh:		lr 6,a			;save temp
				
				lr a,2
				com
				outs	4		;set the col
				lr a,2
				inc				;next col
				lr 2,a
				
				pi plot
							
				lr a,6
				ai $ff
				bf 4,squareh	;all done w/row
				
				ins 5			;next row
				ai $ff			;complement, so subtract 1
				outs 5
				
				lr a,3			
				ai $ff
				lr 3,a
				bf 4,squarev	;done full height?
				pi popk
				pk
				
				
drawlines:		; dci to table of 	color,len,row,start col (horiz if len<$80)
				;					color,len,col,start row (vert if len>$80)
				;					repeat until $FF
				;uses r5
				lr	k,p
line2:
				lm
				ci $ff
				bt 4,lineexit	;ff means done
				outs	1		;set the color

				lm				;get len
				ni $ff			;lm doesn't set status flags
				bf 1,vline		;vertical lines are negative (and sign bit backwards)
				lr 5,a			;horiz-save len
				lm
				com
				outs 5			;set the row
				lm				;get starting column
hline2:
				com
				outs 4			;set the column
				pi plot
				ds 5								
				bt 4,line2
				ins 4			; 
				com
				inc
				bf  0,hline2
lineexit:
				pk

vline:
				ni $3f			;mask off sign bit, limit len to 63 (height of screen)
				lr 5,a			;save it
				lm
				com
				outs 4			;set the column
				lm				;get starting row
vline2:
				com
				outs 5			;set the row
				pi plot		
				ds 5								
				bt 4,line2
				ins 5
				com
				inc
				bf  0,vline2
				
				
plot:							;this is what you have to do to transfer data to the screen memory
								;after setting row, column and color
				li	$60
				outs	0
				li	$50
				outs	0
				lis	6	; little delay
pdelay:			ai $ff
				bf	4,pdelay
				pop


drawlight:						;draws a red or green square based off value passed in r27
				lr k,p
				pi pushk

				lisu 3
				lisl 3
				lr a,(IS)
				dci sqrows
				adc
				lm		
				lr 5,a
				
				lr a,(IS)
				dci sqcols
				adc
				lm
				lr 4,a
				
				lr a,(IS)					; read color from array
				dci array
				adc
				lm
				ni $ff
				bt 4,dogreen
				dci redsq
				bf 0,dosq
dogreen:
				dci greensq
dosq:
				pi square
				pi popk
				pk

;blit code from cart 26
blit:
         		LR      K,P 
         		pi pushk
         		LISU    0
blitloop:
				LISL    4
getparms:
				LM          
				LR      (IS)-,A
				BR7     getparms
				LM          
				LR      H,DC
				DCI     blitparm
				ADC         
				
				LIS     1
				LR      $7,A
				INS     5  
				COM         
				NI      $c0
				AS      $1
				COM         
blit2z:
				OUTS    5  
				DS      $3
				BF      2,blitexit
				LR      A,$2
				LR      $5,A
				LR      A,$0
				COM         
blit2a:
				OUTS    4  
				DS      $7
				BF      4,blit2c
				LIS     8
				LR      $7,A
				LM          
				LR      $6,A
				LIS     1
				NS      $4
				BT      4,blit2b
				LIS     0
				COM         
blit2b:
				ADC         
blit2c:
				LR      A,$6
				AS      $6
				LR      $6,A
				LR      A,$4
				BT      2,blit2d
				LIS     0
blit2d:
				COM         
				OUTS    1  
				LIS     6
				SL      4   
				OUTS    0  
				SL      1   
				OUTS    0  
				BF      0,blit2e
blit2e:
				AI      $60
				BF      4,blit2e
				DS      $5
				BT      4,blit2f
				INS     4  
				AI      $ff
				BF      0,blit2a
blit2f:
				INS     5  
				AI      $ff
				BF      0,blit2z
blitexit:

				LR      DC,H
				LIS     2
				NS      $4
				BT      4,blitloop
				pi 		popk
				PK          


;from the BIOS, but I removed T? (time) prompt
prompts:        LR   K,P                 ; 
                PI   pushk               ; 
prompts2:       LI   $85                 ; red 5 (S)
                LR   $0,A                ; 
                PI   prompt              ; 
                LR   A,$4                ; 
                CI   $08                 ; is it button 4, Start?
                BF   $4,notbut4          ; no, check others
notbut2:
                PI   popk                ; yes, return
                PK                       ; 
                
notbut4:        CI   $02                 ; is it button 2, Mode?
                BF   $4,notbut2          ; 
                LI   $8e                 ; red M
                LR   $0,A                ; 
                PI   prompt              ; 
                LISU 3                   ; 
                LISL 6                   ; 
                LR   A,(IS)                ; 
                as	4					;add the mode to the game #
                LR   (IS),A              ; 
                BF   $0,prompts2         ; 
                
                
;from the BIOS, but I added random # seed to readbuts
prompt:         LR   K,P                 ; $0099 - prints char r0 at row r1, col r2, waits for button
                PI   pushk               ; 
                LI   $33                 ; r1=$33
                LR   $1,A                ; 
                LI   $13                 ; r2=$13
                LR   $2,A                ; 
                PI   drawchar            ; 
                
                LI   $8b                 ; r0=8b	red ?
                LR   $0,A                ; 
                PI   drawchar            ; 
                
                PI   readbuts            ; 
                LI   $33                 ; r1=$33
                LR   $1,A                ; 
                LI   $13                 ; r2=$13
                LR   $2,A                ; 
                LIS  $d                  ; r0=$d
                LR   $0,A                ; 
                PI   drawchar            ; 
                PI   drawchar            ; 
                PI   popk                ; 
                PK                       ; 
                
readbuts:
				lisu 2                	;inc random seed
				lisl 5
				lr a,(IS)
				inc
				lr (IS),a
				
		        INS  0                   ; $00c1 - read buttons
                COM                      ; 
                NI   $0f                 ; 
                BT   4,readbuts          ; 
                LR   $4,A                ; 
                LI   $ff                 ; 
                LR   $5,A                ; 
dbounce:        DS   $5                  ; 
                BF   $4,dbounce         ; 
                jmp delay            ; 


;colors: $40-red, $80=blue, $0=green, $c0=bkg

;data to draw lines for grid
grid:				;horiz-color,len,row,start col
				db $80,66,7,18
				db $80,66,17,18
				db $80,66,27,18
				db $80,66,37,18
				db $80,66,47,18
				db $80,66,57,18
					;vert-color,$80+len,col,start row
				db $80,179,18,7	;179=128+51
				db $80,179,31,7
				db $80,179,44,7
				db $80,179,57,7
				db $80,179,70,7
				db $80,179,83,7
				db $ff

;data to draw squares
redsq:				;color,width,height
				db $40,12,9
greensq:			;color,width,height
				db $0,12,9
cursor:				;color,width,height
				db $c0,4,3
winsq1:
				db $40,38,29	;red
winsq2:
				db $c0,34,25	;bkg

;locations to draw each light, based on index (0-24)
sqrows:			
				db   8, 8, 8, 8, 8,18,18,18,18,18,28,28,28,28,28,38,38,38,38,38,48,48,48,48,48
sqcols:			
				db  19,32,45,58,71,19,32,45,58,71,19,32,45,58,71,19,32,45,58,71,19,32,45,58,71
				
;locations to draw cursor, based on row/column
currows:
				db	11,21,31,41,51
curcols:
				db	23,36,49,62,75

plustable:		;squares to toggle based on cursor N E W S +128 means wrapped
				db 148,  1,132,  5
				db 149,  2,  0,  6
				db 150,  3,  1,  7
				db 151,  4,  2,  8  
				db 152,128,  3,  9
				
				db   0,  6,137, 10
				db   1,  7,  5, 11
				db   2,  8,  6, 12
				db   3,  9,  7, 13
				db   4,133,  8, 14
				
				db   5, 11,142, 15
				db   6, 12, 10, 16
				db   7, 13, 11, 17
				db   8, 14, 12, 18
				db   9,138, 13, 19
				
				db  10, 16,147, 20
				db  11, 17, 15, 21
				db  12, 18, 16, 22
				db  13, 19, 17, 23
				db  14,143, 18, 24
				
				db  15, 21,152,128
				db  16, 22, 20,129
				db  17, 23, 21,130
				db  18, 24, 22,131
				db  19,148, 23,132
				
xtable:		;squares to toggle based on cursor NW NE SW SE +128 means wrapped
				db 152,149,137,  6
				db 148,150,  5,  7
				db 149,151,  6,  8
				db 150,152,  7,  9
				db 151,148,  8,133
				
				db 132,  1,142, 11
				db   0,  2, 10, 12
				db   1,  3, 11, 13
				db   2,  4, 12, 14
				db   3,128, 13,138
				
				db 137,  6,147, 16
				db   5,  7, 15, 17
				db   6,  8, 16, 18
				db   7,  9, 17, 19
				db   8,133, 18,143
				
				db 142, 11,152, 21
				db  10, 12, 20, 22
				db  11, 13, 21, 23
				db  12, 14, 22, 24
				db  13,138, 23,148
				
				db 147, 16,132,129
				db  15, 17,128,130
				db  16, 18,129,131
				db  17, 19,130,132
				db  18,143,132,128 

;the patterns - 1 bit is red, 0 is green
;other code has to be adjusted to use more or less than 64 patterns
;not all of these are solvable on a torus or if only the neighbors toggle
pats:
;				db 		$00,$00,$00,$80,$c0		; easy one for test

                db      $20,$50,$88,$F8,$88
                db      $F0,$88,$88,$88,$F0
                db      $F8,$80,$F8,$80,$F8
                db      $F8,$80,$F0,$80,$80
                db      $88,$88,$F8,$88,$88
                db      $F8,$20,$20,$20,$F8
                db      $90,$A0,$C0,$A0,$90
                db      $80,$80,$80,$80,$F8
                db      $88,$D8,$A8,$88,$88
                db      $88,$C8,$A8,$98,$88
                db      $70,$88,$88,$88,$70
                db      $70,$88,$A8,$90,$68
                db      $70,$80,$70,$08,$70
                db      $F8,$20,$20,$20,$20
                db      $88,$88,$A8,$D8,$88
                db      $88,$50,$20,$50,$88
                db      $88,$50,$20,$20,$20
                db      $F8,$10,$20,$40,$F8
                db      $50,$50,$00,$70,$20
                db      $20,$70,$F8,$70,$20
                db      $D8,$20,$20,$20,$D8
                db      $00,$00,$20,$00,$00
                db      $00,$20,$50,$20,$00
                db      $20,$00,$88,$00,$20
                db      $50,$A8,$50,$A8,$50
                db      $A8,$70,$F8,$70,$A8
                db      $00,$D8,$50,$D8,$00
                db      $20,$88,$20,$88,$20
                db      $F8,$A8,$88,$A8,$F8
                db      $20,$60,$20,$20,$70
                db      $70,$08,$70,$80,$F8
                db      $70,$08,$70,$08,$70
                db      $F8,$00,$70,$00,$F8
                db      $70,$88,$70,$88,$70
                db      $F8,$88,$50,$70,$00
                db      $F8,$70,$20,$00,$00
                db      $20,$20,$F8,$20,$20
                db      $A8,$88,$F8,$88,$A8
                db      $C0,$A0,$A8,$28,$18
                db      $18,$08,$20,$80,$C0
                db      $D8,$88,$50,$88,$D8
                db      $18,$C0,$18,$C0,$18
                db      $A8,$A8,$A8,$A8,$A8
                db      $00,$F8,$00,$F8,$00
                db      $10,$78,$50,$F0,$40
                db      $C0,$E0,$70,$38,$18
                db      $60,$90,$70,$48,$30
                db      $A8,$A8,$50,$A8,$A8
                db      $98,$F0,$50,$78,$C8
                db      $70,$A8,$F8,$F8,$A8
                db      $A0,$E0,$B8,$10,$38
                db      $00,$00,$F8,$50,$50	; pi pattern
                db      $00,$40,$00,$10,$00
                db      $00,$50,$00,$50,$00
                db      $10,$48,$20,$90,$40
                db      $78,$E8,$F8,$B8,$F0
                db      $E0,$90,$E0,$90,$E0
                db      $30,$48,$C0,$48,$30
                db      $38,$48,$20,$90,$E0
                db      $00,$38,$00,$E0,$00
                db      $20,$20,$D8,$20,$20
                db      $20,$70,$A8,$20,$20
                db      $C8,$D0,$20,$58,$98
                db      $00,$70,$50,$70,$00
                
;8 bytes available here

		org	$f00
blitparm:				
		db	$82,19,96,$16,$8,$08,00,00
blitdata:
		db	$00,$45,$F0,$41,$E1,$01,$F7,$80,$44,$20,$40,$00
		db	$00,$45,$00,$41,$13,$81,$04,$40,$A4,$20,$A0,$00
		db	$00,$7D,$E0,$41,$E4,$41,$E7,$81,$14,$21,$10,$00
		db	$00,$45,$04,$41,$07,$C1,$04,$41,$F4,$21,$F0,$00
		db	$00,$45,$f3,$81,$04,$41,$F4,$41,$17,$BD,$10,$00
		db	$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	$F8,$8E,$F1,$C8,$BA,$1E,$04,$50,$72,$22,$3C,$F8
		db	$81,$44,$8A,$28,$92,$11,$03,$90,$82,$45,$22,$80
		db	$F2,$24,$F2,$0F,$92,$11,$74,$50,$73,$88,$BC,$F0
		db	$83,$E4,$8A,$28,$92,$11,$07,$D0,$0A,$4F,$A2,$80
		db	$82,$2E,$89,$C8,$BB,$DE,$04,$5E,$72,$28,$A2,$F8
		db	$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	$00,$00,$00,$79,$17,$81,$17,$DF,$20,$00,$00,$00
		db	$00,$00,$00,$44,$E4,$41,$11,$10,$20,$00,$00,$00
		db	$00,$00,$00,$45,$17,$81,$11,$1E,$20,$00,$00,$00
		db	$00,$00,$00,$45,$F4,$41,$11,$10,$00,$00,$00,$00
		db	$00,$00,$00,$79,$14,$40,$E1,$1F,$20,$00,$00,$00

;4 bytes available here

		org	$ff0
		db	$53,$65,$61,$6e,$20,$52,$69,$64,$64,$6c,$65,$20,$32,$30,$30,$34
