; MACRO.H

; Based on the 2600 macro.h file.
; Macros irrelevant to the 7800 have been removed, and the sleep macro 
; has been adapted to give accurate results on the 7800.

; Version 1.0 2019/12/11 (based on the 2600 Version 1.05, 13/NOVEMBER/2003)

; Available macros...
;   SLEEP n             - sleep for n cycles
;   SET_POINTER         - load a 16-bit absolute to a 16-bit variable

;-------------------------------------------------------------------------------
; SLEEP duration
; Original author: Thomas Jentzsch
; Inserts code which takes the specified number of cycles to execute.  This is
; useful for code where precise timing is required.
; ILLEGAL-OPCODE VERSION DOES NOT AFFECT FLAGS OR REGISTERS.
; LEGAL OPCODE VERSION MAY AFFECT FLAGS
; Uses illegal opcode (DASM 2.20.01 onwards).

            MAC SLEEP            ;usage: SLEEP n (n>1)
.CYCLES     SET {1}

                IF .CYCLES < 2
                    ECHO "MACRO ERROR: 'SLEEP': Duration must be > 1"
                    ERR
                ENDIF

                IF .CYCLES & 1
                    IFNCONST NO_ILLEGAL_OPCODES
                        nop $80
                    ELSE
                        bit $80
                    ENDIF
.CYCLES             SET .CYCLES - 3
                ENDIF
            
                REPEAT .CYCLES / 2
                    nop
                REPEND
            ENDM

;-------------------------------------------------------
; SET_POINTER
; Original author: Manuel Rotschkar
;
; Sets a 2 byte RAM pointer to an absolute address.
;
; Usage: SET_POINTER pointer, address
; Example: SET_POINTER SpritePTR, SpriteData
;
; Note: Alters the accumulator, NZ flags
; IN 1: 2 byte RAM location reserved for pointer
; IN 2: absolute address

            MAC SET_POINTER
.POINTER    SET {1}
.ADDRESS    SET {2}

                LDA #<.ADDRESS  ; Get Lowbyte of Address
                STA .POINTER    ; Store in pointer
                LDA #>.ADDRESS  ; Get Hibyte of Address
                STA .POINTER+1  ; Store in pointer+1

            ENDM

; EOF
