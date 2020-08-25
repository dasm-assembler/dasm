;
; the purpose of this macro is to declare a global variable without need to know its exact location
; the location is automatically derived from its predecessor 'next_var' which gets redefined at every 
; occurence. This way one can copy-paste, comment-out or conditional-define variable declarations
; without worrying. Memory usage will be always as compact as possible
;
	.PROCESSOR	6803

	#ifnconst	RAM_START
RAM_START	.EQU		0x80
	#endif

        .mac declare_var ; {name},{size}
{1}             .EQU            next_var
next_var        .set            ({1}+{2})
        .endm

next_var	.set		RAM_START

	declare_var	first, 1		; declare variable name "first" 8bit 
	declare_var	second, 2		; declare variable name "second" 16bit
    #if (RAM_START == 0x60)
	declare_var	extra, 3		; in case of special define, include variable extra 24bit
    #endif
	declare_var	third, 4		; declare variable name "third" 32bit

	.ORG		0x8000

example_code:
	ldx	#third
	txs
	ldaa	first
	staa	,X
    #if (RAM_START == 0x60)
	staa.w	extra
    #endif

