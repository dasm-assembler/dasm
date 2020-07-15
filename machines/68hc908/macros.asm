
;
;	MACRO declaration keywords _must_ be lowercase
;	MACRO names _must_ be all lowercase
;	MACROS may be nested


	.macro  declare_vector
		.ORG  	{1}
		.word	{2}
	.endm

	.macro declare_var ; {name},{size}
{1}		.EQU	next_var
next_var	.set	({1}+{2})
	.endm

;;;;;	take care to code
;next_var	.set	RAM_START
;;;;;	before using 'declare_var' for the first time

	.macro	store_reg	; {reg_addr},{value}
		#if ({1} > 255)
			ldA     {2}		;[2]
			stA		{1}		;[3]
		#else
			mov 	{2},{1}
		#endif
	.endm

	.macro	store_reg16	; {reg_addr},{value}
		ldHX     {2}		;[2]
		stHX     {1}		;[3]
	.endm

	.macro	set_bits	; {bitmask},{reg_addr}
		ldHX    #{1}
		ldA	,X
		orA	{2}
		stA	,X
	.endm

	.macro	clear_bits	; {bitmask},{reg_addr}
		ldHX	#{1}
		ldA	,x
		and	#[[~{2}] & $FF]
		stA	,x
	.endm


	.macro	shake_pin	; {pin}, {port}
		brset	{1},{2},_off$
		bset	{1},{2}
		bra	_after$
_off$:
		bclr	{1},{2}
_after$:
	.endm


	.macro busy_loop_delay	; {count}
		ldA	{1}				; [2]
_bLoop$:
		dbnzA	_bLoop$		; [3] 3*(arg-1) + 2 T
	.endm

	.macro and_bits		; {reg_addr},{bitmask}
		ldA.8  {1}
		and    {2}
		stA.8  {1}
	.macro


	.macro or_bits		; {reg_addr},{bitmask}
		ldA.8  {1}
		orA    {2}
		stA.8  {1}
	.endm






