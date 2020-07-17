
	.PROCESSOR	68908

	.ORG	0

    .mac predef
	lda	#5
    .macro

Main:	
	predef
	rola
	write_usb
	rora
	rts

    .macro write_usb
	sta	6
    .endm

;
; premature end of file  or  '"endm" missing'
;
