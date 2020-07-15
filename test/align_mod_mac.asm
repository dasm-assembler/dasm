

	.PROCESSOR	6502

	.TRACE		1

	.ifnconst	FLASH_START
FLASH_START	.EQU	$F800
	.endif

	.ifnconst	segmentModding
segmentModding	.EQU	1
	.endif

;
; the macro alignspace let you fill up memory until the next {block_size} boundary
; one must give a unique {name}
;
        .mac alignspace	; {name},{block_size}
_alMod{1}       .EQU    ({2}-(. % {2}))
_alSpace{1}     .DS     _alMod{1},$ac
        .endm

;
; the macro modspacer fills up memory with {char} until the next {block_size}
; one must give a unique {name}
;
        .mac  modspacer ; {name},{block_size},{char}
_mark{1}:
_delta{1}       .EQU    (_mark{1} - FLASH_START)
        .IF     (_delta{1} > 0)
_modulo{1}      .EQU    ({2}-(_delta{1} % {2}))

                        .IF ({3} > 0)
_modpos{1}              .EQU    (32-(. % 32))
                                .IF (_modpos{1} < 16)
                                        .DS             32,{3}
                                .ENDIF
                        .ENDIF
_spacer{1}      .DS             _modulo{1},{3}
        .ENDIF
        .ENDM

;
; the macro codesegmarker fills up memory
; the string {name} is padded with ' ' to length 16 and put at the beginning, the remaining is filled with 0xFF
; one must give a unique {ID}
;
; this is useful if you want to speed-up flash programming with a larger code base and the need to incorporate small changes
; because of the aligned segments a small code change may not introduce a complete shift of the following code and thus
; avoid to re-program the following pages
;
; if the symbol 'segmentModding' == 0 the segment starts at address (FLASH_START + block_size)
; if 'segmentModding' == 1 the segment starts at next 32 byte boundary  address
;
        .mac  codesegmarker  ; {name},{uniqueID},{block_size}
	    .IF (segmentModding > 0)
    		modspacer       {2}a,32,$ff
    	    .ELSE
    		.ORG ({3} + FLASH_START)
    	    .ENDIF
    	    .DC             {1}
    	    modspacer       {2}b,16,32
        .ENDM


	.ORG 	FLASH_START

	alignspace Registers,$40

	codesegmarker "serialFunc",5,$2a0

;	.INCLUDE<------>../common/serialFunc.asm
	nop

	codesegmarker "serialISR",6,$2e0

;	.INCLUDE<------>../common/serialISR.asm
	nop
	nop

	codesegmarker "timerISR",7,$400

	nop
;	.INCLUDE<------>../common/timerISR.asm








<-	