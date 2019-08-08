	processor f8
	org	$800

player1:	equ $2800
p1next:	equ	$28e4
p1fig:	equ $28ec
p1activ: equ $28fd
p1bonus: equ $28f5
p1penalty: equ $28f6
p1score: equ $28f7 ; 8 9
p1delay: equ $28fb
p1delay_counter: equ $28fc
p1randomnext: equ $28fe
p1random: equ $28ff
p1borderc1: equ $2a80
p1borderc2: equ $2a81
p1fieldposy: equ $2a82
p1fieldposx: equ $2a83
p1blockc1: equ $2a84
p1blockc2: equ $2a85
p1scoreparam: equ $2a86
p1nextparam: equ $2a8a
; equ $2a8e

player2:	equ $2900
p2next:	equ	$29e4
p2fig:	equ $29ec
p2activ: equ $29fd
p2bonus: equ $29f5
p2penalty: equ $29f6
p2score: equ $29f7
p2delay: equ $29fb
p2delay_counter: equ $29fc
p2randomnext: equ $29fe
p2random: equ $29ff
p2borderc1: equ $2a90
p2borderc2: equ $2a91
p2fieldposy: equ $2a92
p2fieldposx: equ $2a93
p2blockc1: equ $2a94
p2blockc2: equ $2a95
p2scoreparam: equ $2a96
p2nextparam: equ $2a9a
; equ $2a9e

; player structure layout
; 19 lines a 12 columns
; ec next figure
; ed figure (0..6, $ff for none)
; ee rotation
; ef x
; f0 y
; f5 bonus
; f6 penalty
; f7 score changed
; f8/f9	score
; fa	counter until speed change
; fb	speed/delay
; fc	speed/delay counter
; fd activ
; fe next randomnumber
; ff actual randomnumber

randomnumber:	equ	$2a00
gamemode:	equ	$2a01
palvideomode:	equ	$2a02
;shownextfigure: equ	$2a03

header:
		db	$55	; cartridge id
		db	$2b	; unknown
entry:
		pi 	clearscreen
		dci	gamemode
		lis	0
		st
		dci	palvideomode
		lis	1
		st
;		dci	shownextfigure
;		lis	0
;		st

; load videmo mode constants
		dci	palvideomode
		lm
		dci	pal
		ci	0
		bt	4,vmodepal
		dci	ntsc
vmodepal:	li	$10
		lr	is,a
		li	28
		lr	8,a
vmode1:		lm
		lr	(is),a
		lr	a,is
		ai	1
		lr	is,a
		ds	8
		bf	4,vmode1
		li	$10
		lr	is,a
		lis	14
		lr	8,a
		dci	p1borderc1
vmode2:		lr	a,(is)
		st
		lr	a,is
		ai	1
		lr	is,a
		ds	8
		bf	4,vmode2
		lis	14
		lr	8,a
		dci	p2borderc1
vmode3:		lr	a,(is)
		st
		lr	a,is
		ai	1
		lr	is,a
		ds	8
		bf	4,vmode3

		dci	p1fig
		lis	0
		st
		dci	p2fig
		st
		dci	p1activ
		st
		dci	p2activ
		st

		dci	player1
		pi	clearfield
		dci	player2
		pi	clearfield

		dci	p1borderc1
		lm
		lr	4,a
		lm
		lr	5,a
		lm
		lr	0,a
		lm
		lr	1,a
		dci	player1
		pi	drawfield

		dci	p2borderc1
		lm
		lr	4,a
		lm
		lr	5,a
		lm
		lr	0,a
		lm
		lr	1,a
		dci	player2
		pi	drawfield

		dci	copyright
		pi	drawansistringv

		dci	thanx
		pi	drawansistring

		li	$13
		lr	is,a
		li	$30
		lr	(is)-,a
		lr	(is)-,a
		lr	(is)-,a
		lr	(is),a
		dci	p1scoreparam
		lm
		lr	1,a
		lm
		lr	2,a
		lm	
		lr	3,a
		lm
		lr	4,a
		lis	4
		lr	9,a
		pi	drawstring

		li	$10
		lr	is,a
		dci	p2scoreparam
		lm
		lr	1,a
		lm
		lr	2,a
		lm	
		lr	3,a
		lm
		lr	4,a
		lis	4
		lr	9,a
		pi	drawstring

		dci	p1fig
		lm	
		lr	0,a
		lis	0
		lr	1,a
		pi	getfigure

		li	$31
		lr	is,a
		dci	p1next
		lr	a,(is)+
		st
		lr	a,(is)+
		st
		li	$35
		lr	is,a
		lr	a,(is)+
		st
		lr	a,(is)+
		st		
		li	$39
		lr	is,a
		lr	a,(is)+
		st
		lr	a,(is)+
		st
		li	$3d
		lr	is,a
		lr	a,(is)+
		st
		lr	a,(is)+
		st
		
		dci	p1nextparam
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	p1next
		pi	drawnext

		dci	p1fig
		lm	
		lr	0,a
		lis	0
		lr	1,a
		pi	getfigure

		li	$31
		lr	is,a
		dci	p2next
		lr	a,(is)+
		st
		lr	a,(is)+
		st
		li	$35
		lr	is,a
		lr	a,(is)+
		st
		lr	a,(is)+
		st		
		li	$39
		lr	is,a
		lr	a,(is)+
		st
		lr	a,(is)+
		st
		li	$3d
		lr	is,a
		lr	a,(is)+
		st
		lr	a,(is)+
		st

		dci	p2nextparam
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	p2next
		pi	drawnext		

gameover:	pi	random

		li	$10
		outs	0
		ins	0
		ni	1
		bf	4,gameover1
		dci	gamemode
		lis	1
		st

gameover1:	lis	0
		outs	4
		ins	4
		ni	$80
		bf	4,gameover2
		dci	gamemode
		lis	2
		st

gameover2:	lis	0
		outs	1
		ins	1
		ni	$80
		bf	4,gameover3
		dci	gamemode
		lis	3
		st

gameover3:	dci	gamemode	
		lm
		ci	0
		bt	4,gameover

		dci	gamemode
		lm
		ci	1
		bf	4,start1
		dci	player1
		pi	clearfield
		dci	player2
		pi	clearfield
		lis	1
		dci	p1activ
		st
		dci	p2activ
		st
		jmp	playing1
start1:		ci	2
		bf	4,start2
		dci	player1
		pi	clearfield
		lis	1
		dci	p1activ
		st
		jmp	playing1
start2:		dci	player2
		pi	clearfield
		lis	1
		dci	p2activ
		st
playing1:	lis	0
		dci	gamemode
		st

		dci	p1activ
		lm
		ci	0
		bt	4,playingx
		dci	p1borderc1
		lm
		lr	4,a
		lm
		lr	5,a
		lm
		lr	0,a
		lm
		lr	1,a
		dci	player1
		pi	drawfield

		dci	p1nextparam
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	p1next
		pi	drawnext

playingx:	dci	p2activ
		lm
		ci	0
		bt	4,playingy
		dci	p2borderc1
		lm
		lr	4,a
		lm
		lr	5,a
		lm
		lr	0,a
		lm
		lr	1,a
		dci	player2
		pi	drawfield

		dci	p2nextparam
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	p2next
		pi	drawnext

playingy:
playing:	li	$10
		outs	0

		dci	p1activ
		lm
		ci	0
		bf	4,playing1b
waste1a:	jmp	waste1
playing1b:
		lis	0
		outs	4
		ins	4
		ni	4
		bt	4,playing1a

		dci	p1delay_counter
		lm	
		ai	$ff
		dci	p1delay_counter
		st
		ci	0
		bf	4,waste1a
playing1a:	dci	p1delay
		lm
		st

		lis	0
		outs	4
		ins	4
		lr	9,a
		dci	p1fig
		pi	move

		dci	p1bonus
		lm
		ci	0
		bt	4,playing1x
		ai	$ff
		dci	p2penalty
		am
		dci	p2penalty
		st
		lis	0
		dci	p1bonus
		st

playing1x:	dci	p1fieldposy
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	player1
		pi	drawfielddiff

		dci	p1nextparam
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	p1next
		pi	drawnextdiff


		dci	p1score
		lm
		ci	1
		bf	4,playing1c
		li	$13
		lr	is,a
		lm
		lr	0,a
		ni	$f
		oi	$30
		lr	(is)-,a
		lr	a,0
		sr	4
		oi	$30
		lr	(is)-,a
		lm
		lr	0,a
		ni	$0f
		oi	$30
		lr	(is)-,a
		lr	a,0
		sr	4
		oi	$30
		lr	(is),a
		dci	p1scoreparam
		lm
		lr	1,a
		lm
		lr	2,a
		lm	
		lr	3,a
		lm
		lr	4,a
		lis	4
		lr	9,a
		pi	drawstring

		dci	p1score
		lis	0
		st

playing1c:	dci	p1activ
		lm
		ci	0
		bf	4,playing2
		dci	p2activ
		lm
		ci	0
		bt	4,playing1d
		dci	p2activ
		lis	0
		st
		dci	player2won
		bf	0,playing1e
playing1d:	dci	strgameover1
playing1e:	pi	drawansistring
		bf	0,playing2
waste1:		pi	random

playing2:	li	$10
		outs	0

		dci	p2activ
		lm
		ci	0
		bf	4,playing2b
waste2a:	jmp	waste2
playing2b:	lis	0
		outs	1
		ins	1
		ni	4
		bt	4,playing2a

		dci	p2delay_counter
		lm	
		ai	$ff
		dci	p2delay_counter
		st
		ci	0
		bf	4,waste2a
playing2a:	dci	p2delay
		lm
		st

		lis	0
		outs	1
		ins	1
		lr	9,a
		dci	p2fig
		pi	move

		dci	p2bonus
		lm
		ci	0
		bt	4,playing2x
		ai	$ff
		dci	p1penalty
		am
		dci	p1penalty
		st
		lis	0
		dci	p2bonus
		st

playing2x:	dci	p2fieldposy
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	player2
		pi	drawfielddiff

		dci	p2nextparam
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	4,a
		lm
		lr	5,a
		dci	p2next
		pi	drawnextdiff

		dci	p2score
		lm
		ci	1
		bf	4,playing2c
		li	$13
		lr	is,a
		lm
		lr	0,a
		ni	$f
		oi	$30
		lr	(is)-,a
		lr	a,0
		sr	4
		oi	$30
		lr	(is)-,a
		lm
		lr	0,a
		ni	$0f
		oi	$30
		lr	(is)-,a
		lr	a,0
		sr	4
		oi	$30
		lr	(is),a
		dci	p2scoreparam
		lm
		lr	1,a
		lm
		lr	2,a
		lm	
		lr	3,a
		lm
		lr	4,a
		lis	4
		lr	9,a
		pi	drawstring

		dci	p2score
		lis	0
		st

playing2c:	dci	p2activ
		lm
		ci	0
		bf	4,game
		dci	p1activ
		lm
		ci	0
		bt	4,playing2d
		dci	p1activ
		lis	0
		st
		dci	player1won
		bf	0,playing2e
playing2d:	dci	strgameover2
playing2e:	pi	drawansistring
		bf	0,game
waste2:		pi	random
game:
; avoid standing random number if no key pressed
		dci	randomnumber
		lm
		dci	randomnumber
		ai	1
		st
		dci	p1random
		lm
		dci	p1random
		ai	1
		st
		dci	p2random
		lm
		dci	p2random
		ai	1
		st

		pi	random
		dci	p1activ
		lm
		dci	p2activ
		om
		bt	4,playing3
		jmp	playing
playing3:	jmp	gameover

; dci	address
; r0 video row
; r1 video col
; r4 color
; r5 color 2
drawfield:	lr	k,p
		li	19
		lr	6,a
		lr	a,1
		lr	8,a
dfloopl:	li	12
		lr	7,a
		
dfloopc:	lm
		ci	0
		bf	4,dfset
		li	$c0
		lr	2,a
		lr	3,a
		jmp	dfcont
dfset:		lr	a,4
		lr	2,a
		lr	a,5
		lr	3,a
dfcont:		pi	drawblock
		lr	a,1
		ai	$fd
		lr	1,a
		ds	7
		bf	4,dfloopc
		lr	a,8
		lr	1,a
		lr	a,0
		ai	$fd
		lr	0,a
		ds	6
		bf	4,dfloopl
		lr	p,k
		pop

; dci	address
; r0 video row
; r1 video col
; r4 color
; r5 color 2
drawfielddiff:	lr	k,p
		li	19
		lr	6,a
		lr	a,1
		lr	8,a
		lr	q,dc
dfdloopl:	li	12
		lr	7,a
		
dfdloopc:	lm
		ni	$ff
		bt	1,dfdnot
		ni	$7f
		bf	4,dfdset
		lr	dc,q
		st
		li	$c0
		lr	2,a
		lr	3,a
		jmp	dfdcont
dfdset:		lr	dc,q
		st
		lr	a,4
		lr	2,a
		lr	a,5
		lr	3,a
dfdcont:	pi	drawblock
dfdnot:		lr	q,dc
		lr	a,1
		ai	$fd
		lr	1,a
		ds	7
		bf	4,dfdloopc
		lr	a,8
		lr	1,a
		lr	a,0
		ai	$fd
		lr	0,a
		ds	6
		bf	4,dfdloopl
		lr	p,k
		pop

; dci	address
; r0 video row
; r1 video col
; r4 color
; r5 color 2
drawnext:	lr	k,p
		lis	4
		lr	6,a
		lr	a,1
		lr	8,a
dfnloopl:	lis	2
		lr	7,a
		
dfnloopc:	lm
		ci	0
		bf	4,dfnset
		lis	0
		lr	2,a
		lr	3,a
		jmp	dfncont
dfnset:		lr	a,4
		lr	2,a
		lr	a,5
		lr	3,a
dfncont:	pi	drawblock
		lr	a,1
		ai	$fd
		lr	1,a
		ds	7
		bf	4,dfnloopc
		lr	a,8
		lr	1,a
		lr	a,0
		ai	$fd
		lr	0,a
		ds	6
		bf	4,dfnloopl
		lr	p,k
		pop


; dci	address
; r0 video row
; r1 video col
; r4 color
; r5 color 2
drawnextdiff:	lr	k,p
		lis	4
		lr	6,a
		lr	a,1
		lr	8,a
		lr	q,dc
dndloopl:	lis	2
		lr	7,a
		
dndloopc:	lm
		ni	$ff
		bt	1,dndnot
		ni	$7f
		bf	4,dndset
		lr	dc,q
		st
		lis	0
		lr	2,a
		lr	3,a
		jmp	dndcont
dndset:		lr	dc,q
		st
		lr	a,4
		lr	2,a
		lr	a,5
		lr	3,a
dndcont:	pi	drawblock
dndnot:		lr	q,dc
		lr	a,1
		ai	$fd
		lr	1,a
		ds	7
		bf	4,dndloopc
		lr	a,8
		lr	1,a
		lr	a,0
		ai	$fd
		lr	0,a
		ds	6
		bf	4,dndloopl
		lr	p,k
		pop

; dci address		
clearfield:	lr	h,dc
		li	228
		lr	0,a
		li	0
cfloop:		st
		ds	0
		bf	4,cfloop
		lr	dc,h
		li	18
		lr	0,a
		li	1
cfloop1:	st
		li	10
		adc
		li	1
		st
		ds	0
		bf	4,cfloop1
		st
		st
		st
		st
		st
		st
		st
		st
		st
		st
		st
		st
		lis	0
		cm		; playing $2[89]e4
		cm
		cm
		cm
		cm
		cm
		cm
		cm
		cm		; $2[89]ec
		li	$ff
		st		; no figure
		lr	h,dc
		li	$f5
		lr	11,a
		lr	dc,h
		lis	0
		st
		st
		st
		st
		st
		lis	1
		st
		lis	10
		st
		lis	1
		st
		pop

; dci field
; r9 keys
move:		lr	k,p
		lr	q,dc
		lm
;		ci	0
;		bf	4,moveend1
;		jmp	moveend
moveend1:	lm
		ci	$ff
		bf	4,next1
		jmp	next
next1:		lr	0,a
		lm
		lr	1,a
		lm	
		lr	2,a
		lm	
		lr	3,a
		pi	getfigure
		pi	fieldpos
		pi	clearfigure
		lr	a,9
		ni	$1
		bf	4,nmoveright
		lr	a,2
		ai	1
		lr	2,a
nmoveright:	lr	a,9
		ni	$2
		bf	4,nmoveleft
		lr	a,2
		ai	$ff
		lr	2,a
nmoveleft:	lr	a,9
		ni	$10
		bf	4,nmovecc
		lr	a,1
		ai	1
		ni	3
		lr	1,a
nmovecc:	lr	a,9
		ni	$20
		bf	4,nmovec
		lr	a,1
		ai	$ff
		ni	3
		lr	1,a
nmovec:		pi	getfigure
		pi	fieldpos
		pi	testfigure
		ci	0
		bt	4,nreload
		lr	dc,q
		lm
		lm
		lr	0,a
		lm
		lr	1,a
		lm
		lr	2,a
		lm
		lr	3,a
		pi	getfigure
nreload:	lr	dc,q
		lm
		lr	a,0
		st
		lr	a,1
		st
		lr	a,2
		st
		lr	a,3
		st
		lr	a,3
		ai	1
		lr	3,a
		pi	fieldpos
		pi	testfigure
		lr	9,a
		ci	0
		bf	4,nmovedown
		lr	dc,q
		lm
		lm
		lm
		lm
		lr	a,3
		st
nmovedown:	lr	dc,q
		lm
		lm
		lm
		lm
		lm
		lr	3,a
		pi	fieldpos
		pi	setfigure
		lr	a,9
		ci	0
		bf	4,nmovedown1
		jmp	moveend
nmovedown1:	pi	findlines
		lr	dc,q
		lr	h,dc
		li	$f6
		lr	11,a
		lr	dc,h
		lm
		ci	0
		bt	4,next
		lr	4,a
		lr	dc,h
		lis	0
		st
mpen:		pi	penalty
		ds	4
		bf	4,mpen
next:		lr	dc,q
		lr	h,dc
		li	$fa
		lr	11,a
		lr	dc,h
		lm	
		ai	1
		lr	dc,h
		st
		ci	$50
		bf	4,next2
		lr	dc,h
		lis	0
		st
		lm
		ci	1
		bt	4,next2
		ai	$ff
		lr	dc,h
		cm
		st
next2:		lr	dc,q
		lm
		st
		lr	dc,q
		lr	h,dc
		li	$fe
		lr	11,a
		lr	dc,h
		lm
		lm
		ni	7
		ci	7
		bf	4,nextloop
		lis	0
nextloop:	lr	0,a
		lr	dc,h
		lm
		st
		lr	dc,q
		lm
		st
		lr	dc,q
		lr	a,0
		st
		cm
		li	0	; rotation
		lr	1,a
		st
		li	$4	; x
		lr	2,a
		st
		li	$0	; y
		lr	3,a
		st
		lr	dc,q
		cm
		lm	
		lr	0,a
		lm	
		lr	1,a
		pi	getfigure
		lr	dc,q
		pi	fieldpos
		pi	testfigure
		ci	0
		bt	4, nextend
		; end of game
		lr	dc,q
		lr	h,dc
		li	$fd
		lr	11,a
		lr	dc,h
		li	$0
		st
		lr	dc,q
nextend:	lr	h,dc
		li	$f7
		lr	11,a
		lr	dc,h
		lis	1
		st
		li	$67
		amd	
		lr	4,a
		li	$66
		lnk
		amd
		lr	5,a
		lr	dc,h
		cm
		lr	a,4
		st
		lr	a,5
		st		
		pi	fieldpos
		pi	setfigure


		lr	dc,q
		lm	
		lr	0,a
		lis	0
		lr	1,a
		pi	getfigure

		li	$31
		lr	is,a		
		lr	h,dc
		li	$e4
		lr	11,a
		lr	dc,h
		lr	a,(is)+
		oi	$80
		st
		lr	a,(is)+
		oi	$80
		st
		li	$35
		lr	is,a
		lr	a,(is)+
		oi	$80
		st
		lr	a,(is)+
		oi	$80
		st		
		li	$39
		lr	is,a
		lr	a,(is)+
		oi	$80
		st
		lr	a,(is)+
		oi	$80
		st
		li	$3d
		lr	is,a
		lr	a,(is)+
		oi	$80
		st
		lr	a,(is)+
		oi	$80
		st

moveend:	lr	p,k
		pop

; r0 figure
; r1 rotation
getfigure:	lr	h,dc
		dci	figures
		lr	a,0
		ci	0
		bt	4,gfn
		lr	4,a
		li	$40
gfn1:		adc
		ds	4
		bf	4,gfn1
gfn:		lr	a,1
		ci	0
		bt	4,gff
		lr	4,a
		li	$10
gff1:		adc
		ds	4
		bf	4,gff1
gff:		li	$30
		lr	is,a
		li	$10
		lr	4,a
gfloop:		lm
		lr	(is),a
		lr	a,is
		ai	1
		lr	is,a
		ds	4
		bf	4,gfloop
		lr	dc,h
		pop


findlines:
		lr	h,dc
		li	205		; 17*(1+10+1)+1
		lr	11,a		; hl
fl2:		lr	dc,h
		li	9
		lr	8,a
		lm
fl1:		nm
		ds	8
		bf	4,fl1
		ni	1
		bf	4,fl3
fl1c:		lr	a,11
		ai	244		; -12
		lr	11,a
		ci	1
		bf	4,fl2		
		pop
	; zeile gefunden
fl3:		lr	a,11
		lr	9,a
		li	$f5
		lr	11,a
		lr	dc,h
		lm
		ai	1
		lr	dc,h
		st
		lr	a,9
		lr	11,a
fl1d:		lr	a,11
		ai	244
		lr	11,a
		ci	1
		bf	4,fl1e
		lr	a,9
		lr	11,a
		jmp	fl2
fl1e:		lr	dc,h		
		li	$30
		lr	is,a
		li	10
		lr	8,a
fl1a:		lm
		lr	(is),a
		lr	a,is
		ai	1
		lr	is,a
		ds	8
		bf	4,fl1a
		lm
		lm
		li	$30
		lr	is,a
		li	10
		lr	8,a
fl1b:		lr	a,(is)
		; probably doing check to only redraw changed blocks
		oi	$80
		st
		lr	a,is
		ai	1
		lr	is,a
		ds	8
		bf	4,fl1b
		jmp	fl1d

penalty:	li	17
		lr	6,a
		lis	13
		lr	dc,q
		lr	h,dc
		lr	11,a
penalty3:	lr	dc,h
		lis	10
		lr	5,a
		li	$30
		lr	is,a
penalty1:	lm
		lr	(is),a
		lr	a,is
		ai	1
		lr	is,a
		ds	5
		bf	4,penalty1
		lr	a,11
		ai	$f4		; -12
		lr	11,a
		lr	dc,h
		li	$30
		lr	is,a
		lis	10
		lr	5,a
penalty2:	lr	a,(is)
		oi	$80
		st
		lr	a,is
		ai	1
		lr	is,a
		ds	5
		bf	4,penalty2
		lr	a,11
		ai	24
		lr	11,a
		ds	6
		bf	4,penalty3
		lr	a,11
		ai	$f4
		lr	11,a
		lr	dc,h
		lis	10
		lr	5,a
		li	$81
penalty5:	st
		ds	5
		bf	4,penalty5
		li	$ff
		lr	11,a
		lr	dc,h
		lm
		ni	7
		ai	206	;12*17+1+1
		lr	11,a
		lr	dc,h
		li	$80
		st
		pop
; r2 x
; r3 y
; dci
fieldpos:	lr	h,dc
		lr	a,2
		lr	$b,a
		ni	$ff
		bt	1,fieldpos3
		ds	$a
fieldpos3:	lr	dc,h
		lr	a,3
		ci	0
		bt	4, fieldpos1
		lr	4,a
		li	12
fieldpos2:	adc
		ds	4
		bf	4, fieldpos2
fieldpos1:	
		pop

; r0 figure
; r1 rotation
; r2 x
; r3 y
; dci field
testfigure:	lr	h,dc
		li	4
		lr	4,a
		li	$30
		lr	is,a
tpy:		li	4
		lr	5,a
tpx:		lr	a,(is)
		ci	0
		bt	4,tpnothing
		lm
		ni	$7f
		bt	4,tpcont
		lr	dc,h
		li	1	; something in it
		pop
tpnothing:	cm	
tpcont:		lr	a,is
		ai	1
		lr	is,a
		ds	5
		bf	4,tpx
		li	8
		adc
		ds	4
		bf	4,tpy
		lr	dc,h
		li	0
		pop

; r2 x
; r2 y
; dci field
clearfigure:	lr	h,dc
		li	4
		lr	4,a
		li	$30
		lr	is,a
		
cfy:		li	4
		lr	5,a
cfx:		lr	a,(is)
		ci	0
		bt	4,cfnothing
		li	$80
		st
		jmp	cfc
cfnothing:	cm
cfc:		lr	a,is
		ai	1
		lr	is,a
		ds	5
		bf	4,cfx
		li	8
		adc
		ds	4
		bf	4,cfy
		lr	dc,h
		pop

; dci field
setfigure:	lr	h,dc
		li	4
		lr	4,a
		li	$30
		lr	is,a
		
sfy:		li	4
		lr	5,a
sfx:		lr	a,(is)
		ci	0
		bt	4,sfnothing
		li	$81
		st
		jmp	sfc
sfnothing:	cm
sfc:		lr	a,is
		ai	1
		lr	is,a
		ds	5
		bf	4,sfx
		li	8
		adc
		ds	4
		bf	4,sfy
		lr	dc,h
		pop

; pseudo random number generation
; (f3853 timer not used; irqs generally difficult to use with subroutines)
; so if user presses a key in while loop simply its counter is taken
; (random number from 0 to 255)
; I expect a 50% change him to press
; 0 has several clocks more, so its more likely to come
random:		
		li	$10
		outs	0
		ins	0
		lr	2,a
		lis	0
		outs	4
		ins	4
		lr	3,a
		lis	0
		outs	1
		ins	1
		lr	4,a
		lis	0
		lr	1,a
random1:	
; currently not needed
;		ins	0
;		xs	2
;		bt	4,random1a
;		xs	2
;		lr	2,a
;		dci	randomnumber
;		lr	a,1
;		st
random1a:	ins	4
		xs	3	
		bt	4,random1b
		xs	3
		lr	3,a
		dci	p1randomnext
		lr	a,1
		st
random1b:	ins	1
		xs	4	
		bt	4,random1c
		xs	4
		lr	4,a
		dci	p2randomnext
		lr	a,1
		st
random1c:	ds	1
		bf	4,random1
		pop

; r0 video row
; r1 video col
; r2 color
; r3 color 2
drawblock:	li	$50
		outs	0
		ins	5
		ni	$c0
		xs	0
		outs	5
		ins	4
		ni	$80
		xs	1
		outs	4
		lr	a,2
		outs	1
		li	$60	; p 0/0
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay1:	ds	10
		bf	4,dbdelay1
		ins	4
		ai	$ff
		outs	4
		li	$60	; p 1/0
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay2:	ds	10
		bf	4,dbdelay2
		ins	4
		ai	$ff
		outs	4
		li	$60	; p 2/0
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay3:	ds	10
		bf	4,dbdelay3
		ins	5
		ai	$ff
		outs	5
		li	$60	; p 2/1
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay4:	ds	10
		bf	4,dbdelay4
		ins	4
		inc
		outs	4
		lr	a,3
		outs	1
		li	$60	; p 1/1
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay5:	ds	10
		bf	4,dbdelay5
		ins	4
		inc
		outs	4
		lr	a,2
		outs	1
		li	$60	; p 0/1
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay6:	ds	10
		bf	4,dbdelay6
		ins	5
		ai	$ff
		outs	5
		li	$60	; p 0/2
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay7:	ds	10
		bf	4,dbdelay7
		ins	4
		ai	$ff
		outs	4
		li	$60	; p 1/2
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay8:	ds	10
		bf	4,dbdelay8
		ins	4
		ai	$ff
		outs	4
		li	$60	; p 2/2
		outs	0
		li	$50
		outs	0
		lis	6
		lr	10,a
dbdelay9:	ds	10
		bf	4,dbdelay9
		pop

clearscreen:	lis	0
		outs	1
		li	$3f
		lr	0,a
csloopl:	lr	a,0
		outs	5
		li	$7f
		lr	1,a
csloopc:	lr	a,1
		outs	4
		li	$60
		outs	0
		li	$50
		outs	0
		lis	6	; in bowling 4
		lr	4,a
csdelay:	ds	4
		bf	4,csdelay
		ds	1
		bt	2,csloopc
		ds	0
		bt	2,csloopl	
		pop

; 7 figures
; in 4 rotations
; 4x4 size
figures:	
		db	0,1,0,0
		db	0,1,0,0
		db	0,1,0,0
		db	0,1,0,0

		db	0,0,0,0
		db	1,1,1,1
		db	0,0,0,0
		db	0,0,0,0

		db	0,1,0,0
		db	0,1,0,0
		db	0,1,0,0
		db	0,1,0,0

		db	0,0,0,0
		db	1,1,1,1
		db	0,0,0,0
		db	0,0,0,0
;
		db	0,1,0,0
		db	0,1,1,0
		db	0,0,1,0
		db	0,0,0,0
		
		db	0,1,1,0
		db	1,1,0,0
		db	0,0,0,0
		db	0,0,0,0

		db	0,1,0,0
		db	0,1,1,0
		db	0,0,1,0
		db	0,0,0,0
		
		db	0,1,1,0
		db	1,1,0,0
		db	0,0,0,0
		db	0,0,0,0
;
		db	0,0,1,0
		db	0,1,1,0
		db	0,1,0,0
		db	0,0,0,0

		db	1,1,0,0
		db	0,1,1,0
		db	0,0,0,0
		db	0,0,0,0

		db	0,0,1,0
		db	0,1,1,0
		db	0,1,0,0
		db	0,0,0,0

		db	1,1,0,0
		db	0,1,1,0
		db	0,0,0,0
		db	0,0,0,0
;
		db	0,1,0,0
		db	0,1,1,0
		db	0,1,0,0
		db	0,0,0,0

		db	0,1,0,0
		db	1,1,1,0
		db	0,0,0,0
		db	0,0,0,0
		
		db	0,1,0,0
		db	1,1,0,0
		db	0,1,0,0
		db	0,0,0,0

		db	0,0,0,0
		db	1,1,1,0
		db	0,1,0,0
		db	0,0,0,0
;
		db	0,1,0,0
		db	0,1,0,0
		db	0,1,1,0
		db	0,0,0,0

		db	0,0,1,0
		db	1,1,1,0
		db	0,0,0,0
		db	0,0,0,0

		db	1,1,0,0
		db	0,1,0,0
		db	0,1,0,0
		db	0,0,0,0

		db	0,0,0,0
		db	1,1,1,0
		db	1,0,0,0
		db	0,0,0,0
;
		db	0,1,1,0
		db	0,1,0,0
		db	0,1,0,0
		db	0,0,0,0

		db	1,0,0,0
		db	1,1,1,0
		db	0,0,0,0
		db	0,0,0,0

		db	0,1,0,0
		db	0,1,0,0
		db	1,1,0,0
		db	0,0,0,0

		db	0,0,0,0
		db	1,1,1,0
		db	0,0,1,0
		db	0,0,0,0
;
		db	0,1,1,0
		db	0,1,1,0
		db	0,0,0,0
		db	0,0,0,0

		db	0,1,1,0
		db	0,1,1,0
		db	0,0,0,0
		db	0,0,0,0

		db	0,1,1,0
		db	0,1,1,0
		db	0,0,0,0
		db	0,0,0,0

		db	0,1,1,0
		db	0,1,1,0
		db	0,0,0,0
		db	0,0,0,0

pal:	
	; player 1
	db	$40, $80	; block border color border, mid
	db	$3a, $70	; fieldpos y, x
	db	$40, 0		; block color border, mid
	db	$3a, $74, $40, 0	; score pos x,y,foreground, backgroundcolor
	db	$0d, $77, $40,$80	; x,y, color border, mid next figure

	; player 2
	db	$80, $40	; block border color border, mid
	db	$3a, $40	; fieldpos y, x
	db	$80, 0		; block color border, mid
	db	$3a, $1b, $80, 0	; score pos x,y,foreground, backgroundcolor
	db	$0d, $1b, $80,$40

ntsc:	
	; player 1
	db	$40, $80	; block border color border, mid
	db	$3a, $72	; fieldpos y, x
	db	$40, 0		; block color border, mid
	db	$3a, $4d, $40, 0	; score pos x,y,foreground, backgroundcolor
	db	$0d, $4d, $40,$80	; x,y, color border, mid next figure

	; player 2
	db	$80, $40	; block border color border, mid
	db	$3a, $3e	; fieldpos y, x
	db	$80, 0		; block color border, mid
	db	$3a, $42, $80, 0	; score pos x,y,foreground, backgroundcolor
	db	$0d, $45, $80,$40

ascii:	db	0,0,0,0,0,0,0,0 
	db	0,0,0,0,0,0,0,0 
	db	0,0,0,0,0,0,0,0 
	db	0,0,0,0,0,0,0,0 
	db	36,0,0,0,0,0,0,0 
	db	0,0,0,0,0,0,0,0 
	db	0,1,2,3,4,5,6,7
	db	8,9,0,0,0,0,0,0
	db	37,10,11,12,13,14,15,16
	db	17,18,19,20,21,22,23,24
	db	25,26,27,28,29,30,31,32
	db	33,34,35

font:	db	$40,$a0,$a0,$a0,$40	; 0
	db	$40,$40,$40,$40,$40
	db	$c0,$20,$40,$80,$e0
	db	$e0,$20,$e0,$20,$e0
	db	$a0,$a0,$e0,$20,$20
	db	$e0,$80,$c0,$20,$c0
	db	$40,$80,$c0,$a0,$40
	db	$e0,$20,$20,$40,$40
	db	$40,$a0,$40,$a0,$40
	db	$40,$a0,$60,$20,$40
	db	$40,$a0,$e0,$a0,$a0	; a
	db	$c0,$a0,$c0,$a0,$c0
	db	$60,$80,$80,$80,$60
	db	$c0,$a0,$a0,$a0,$c0
	db	$e0,$80,$e0,$80,$e0
	db	$e0,$80,$e0,$80,$80
	db	$60,$80,$a0,$a0,$60
	db	$a0,$a0,$e0,$a0,$a0
	db	$40,$40,$40,$40,$40
	db	$40,$40,$40,$40,$80
	db	$a0,$c0,$80,$c0,$a0
	db	$80,$80,$80,$80,$e0
	db	$a0,$e0,$a0,$a0,$a0
	db	$c0,$a0,$a0,$a0,$a0
	db	$40,$a0,$a0,$a0,$40	; o
	db	$c0,$a0,$a0,$c0,$80
	db	$40,$c0,$c0,$c0,$60
	db	$c0,$a0,$a0,$c0,$a0
	db	$60,$80,$40,$20,$c0
	db	$e0,$40,$40,$40,$40
	db	$a0,$a0,$a0,$a0,$60
	db	$a0,$a0,$a0,$a0,$40
	db	$a0,$a0,$a0,$e0,$a0
	db	$a0,$a0,$40,$a0,$a0
	db	$a0,$a0,$40,$40,$40
	db	$e0,$20,$40,$80,$e0
	db	0,0,0,0,0
	db	$60,$a0,$a0,$60,$a0	; tetris r

	
; r0 char as ascii
; r1 row
; r2 column
; r3 foreground color
; r4 background color
drawchar:
		dci	ascii
		lr	a,0
		adc	
		lm
		dci	font
		adc
		adc
		adc
		adc
		adc
		li	5
		lr	10,a
		ins	5
		ni	$c0
		xs	1
		outs	5
drawchar1:	lm
		lr	5,a
		lr	a,2
		ins	4
		ni	$80
		xs	2
		outs	4
		lis	3
		lr	8,a
dcl4:
		lr	a,5
		ni	$80
		bf	4,dcl1
		lr	a,4
		bf	0,dcl2
dcl1:		lr	a,3
dcl2:		outs	1
		li	$60
		outs	0
		li	$50
		outs	0
		lis	6
		lr	6,a
dcl3:		ds	6
		bf	4,dcl3
		lr	a,5
		sl	1
		lr	5,a
		ins	4
		ai	$ff
		outs	4
		ds	8
		bf	4,dcl4
		ins	5
		ai	$ff
		outs	5
		ds	10
		bf	4,drawchar1

		pop

; is string
; r1 row
; r2 column
; r3 foreground color
; r4 background color
; r9 count
drawstring:
		lr	k,p
drawstring1:	lr	a,(is)
		lr	0,a
		pi	drawchar
		lr	a,is
		ai	1
		lr	is,a
		lr	a,1
		ai	$fa
		lr	1,a
		ds	9
		bf	4,drawstring1
		lr	p,k
		pop

; dc string
drawansistringv:
		lr	k,p
drawstring3v:	lm
		lr	q,dc
		lr	0,a
		ci	0
		bt	4,drawstring4v
		ci	1
		bf	4,drawstring5v
		lm
		lr	1,a
		lm
		lr	2,a
		bf	0,drawstring3v
drawstring5v:	ci	2
		bf	4,drawstring6v
		lm
		lr	3,a
		lm
		lr	4,a
		bf	0,drawstring3v
drawstring6v:	pi	drawchar
		lr	a,is
		ai	1
		lr	is,a
		lr	a,1
		ai	$fa
		lr	1,a
		lr	dc,q
		jmp	drawstring3v

drawstring4v:	lr	p,k
		pop

; dc string
drawansistring:
		lr	k,p
drawstring3:	lm
		lr	q,dc
		lr	0,a
		ci	0
		bt	4,drawstring4
		ci	1
		bf	4,drawstring5
		lm
		lr	1,a
		lm
		lr	2,a
		bf	0,drawstring3
drawstring5:	ci	2
		bf	4,drawstring6
		lm
		lr	3,a
		lm
		lr	4,a
		bf	0,drawstring3
drawstring6:	pi	drawchar
		lr	a,is
		ai	1
		lr	is,a
		lr	a,2
		ai	$fc
		lr	2,a
		lr	dc,q
		jmp	drawstring3

drawstring4:	lr	p,k
		pop

thanx:
		db	1, $3a, $3a
		db	1, $34, $3a
		db	2, $40, $c0
		db	$20,$48,$41,$4c,$4c,$4f
		db	1, $2e, $3a
		db	1, $28, $3a
		db	2, $80, $c0
		db	$20,$46,$52,$41,$4e,$4b
		db	1, $22, $3a
		db	1, $1c, $3a
		db	2, $00, $c0
		db	$46,$52,$45,$44,$52,$49,$43
		db	1, $16, $3a
		db	1, $10, $38
		db	2, $40, $c0
		db	$20,$53,$45,$41,$4e
		db	1, $0a, $3a
		db	0

strgameover1:	
		db	2, $80, $c0
		db	1, $23, $6f
		db	$20,$47,$41,$4d,$45,$20
		db	1, $1d, $68
		db	$20,$4f,$56,$45,$52,$20
		db	0
strgameover2:	
		db	2, $40, $c0
		db	1, $23, $3b
		db	$20,$47,$41,$4d,$45,$20
		db	1, $1d, $34
		db	$20,$4f,$56,$45,$52,$20
		db	0

player1won:	
		db	2, $80, $c0
		db	1, $20, $66
		db	$57,$4f,$4e
		db	2, $40, $c0
		db	1, $20, $34
		db	$4c,$4f,$53,$54
		db	0

player2won:
		db	2, $80, $c0
		db	1, $20, $68
		db	$4c,$4f,$53,$54
		db	2, $40, $c0
		db	1, $20, $32
		db	$57,$4f,$4e
		db	0

copyright:
		db	2, $80, $0
		db	1, $37, $6d
		db	$20,$54,$45,$54,$40,$49,$53,$20
		db	2, $40, $c0
		db	1, $3a, $68
		db	$43,$4f,$50,$59,$52,$49,$47,$48,$54
		db	2, $40, $0
		db	1, $2b, $64
		db	$32, $30, $30, $34
		db	1, $25, $5f
		db	2, $40, $c0
		db	$42,$59
		db	1, $3a, $5a
		db	2, $00, $c0
		db	$20,$20,$50,$45,$54,$45,$52
		db	1, $3a, $56
		db	2, $80, $c0
		db	$20,$54,$52,$41,$55,$4e,$45,$52
		db	0

	align	4096-header
