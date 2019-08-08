;;;
;
;   Some useful macros and techniques
;
	processor 6502

;;;
	mac	hexdig

	if [{1}] <= 9
	    dc.b	'0 + [{1}]
	else
	    dc.b	'a + [{1}] - 10
	endif

	endm
;;;
	mac	hexnum

	if [{1}] >= $10
	    hexnum [{1}] >> 4
	endif
	hexdig [{1}] & $0F

	endm
;;;

	org 0
	hexnum $123ab

;;;;
;
;	Using an uninitialised segment to create C-like 'struct's.
;	(idea taken from IBM-370 assembler style "dummy segments")

	seg.u	node
	org 0

s_next	ds.b	2	; a node has a pointer to the next one
s_data	ds.b	2	; and 2 bytes of data
s_more	ds.b	3	; and 3 bytes of something more

	seg	code

ptr	equ	42	; dummy value
	ldy #s_data
	lda (ptr),y	; get first byte of data


