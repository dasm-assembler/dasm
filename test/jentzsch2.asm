; Thomas Jentzsch <tjentzsch@web.de> test case for bug
; in error handling.

	processor 6502
	include vcs.h

	ORG $f800

Start
	bpl .error ; undefined label

	org $fffc
	.word Start
	.word Start
