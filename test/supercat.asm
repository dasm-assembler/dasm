; Pretty horrible bug... :-/ Switching to signed instead
; of unsigned arithmetic made this easier to handle but
; bad use of .ds can still lead to huge files since DASM
; doesn't enforce an upper bound yet. And what should it
; be? Hmm...
;
; From "supercat" on the atariage.com forums, forwarded by
; Andrew Davie 2008/04/12.
;
; Peter H. Froehlich
; phf at acm dot org

	.processor 6502

	.org	$7FF8
	.byte	1,2,3,4,5

; original line by "supercat", would produce huge files
; since the "-1" result would be interpreted as a huge
; positive number
	.ds	$7FFC-*

; Peter's line made the problem tractable and revealed
; the actual bug, checking this with xxd clearly shows
; the -1 bit pattern 0xffffffff which unsigned is huge.
	.long	$7FFC-*

	.byte	1,2,3,4,5

	.end
