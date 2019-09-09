	; Test case for -D and -M options and echo pseudo-op.

SOURCE_SYMBOL	equ	255
	IMPROPER_SYMBOL	equ	127

	processor	6502
	echo		"Defined in source file:"
	echo		SOURCE_SYMBOL
	echo		"Defined improperly in source file:"
	echo		IMPROPER_SYMBOL
	echo		"Defined externally with -D and default:"
	echo		EXTERNAL_D_DEFAULT
	echo		"Defined externally with -D and 127:"
	echo		EXTERNAL_D_VALUE
	echo		"Defined externally with -M and default:"
	echo		EXTERNAL_M_DEFAULT
	echo		"Defined externally with -M and 127:"
	echo		EXTERNAL_M_VALUE
	end
