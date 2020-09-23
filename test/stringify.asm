
	    .PROCESSOR		6502

	.ORG	1024
	.word   1968
	.word   endprg
	.byte	$FF			; make odd alignment
	.byte   [MainStart]d		; ASCII representation of MainStart
	.byte  " TEST ",0
endprg:
	.byte	0,0,0

	.ORG		1234
MainStart:
	rts
