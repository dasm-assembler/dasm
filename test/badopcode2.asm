; Test case for bug in error handling.

	processor 6502

	ORG $f800
	jsr ($1000)

