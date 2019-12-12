; Test symbolic includes
;
; Ensure that quoted, unquoted, and symbol based includes work.
;
; --Mike Saarna

	.processor	6502
	.org		0

TESTINCA = "include-a.inc"
PASSFORCEA = PASSFORCEB

	lda #PASSFORCEA
	include "include-a.inc"
	include include-a.inc
	include TESTINCA
	include "11include.inc"

	; Non-symbol compliant filenames without quotes...
	include 11include.inc

PASSFORCEB = 1


	.end
