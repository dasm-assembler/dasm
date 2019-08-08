; MACRO.H for Channel F
; Version 1.01, 2/NOVEMBER/2004

VERSION_MACRO	= 101

;
; THIS FILE IS EXPLICITLY SUPPORTED AS A DASM-PREFERRED COMPANION FILE
; PLEASE DO *NOT* REDISTRIBUTE MODIFIED VERSIONS OF THIS FILE!
;
; This file defines DASM macros useful for development for the Channel F.
; It is distributed as a companion machine-specific support package
; for the DASM compiler. Updates to this file, DASM, and associated tools are
; available at at http://www.atari2600.org/dasm
;
; Many thanks to the people who have contributed.  If you take issue with the
; contents, or would like to add something, please write to me
; (atari2600@taswegian.com) with your contribution.
;
; Latest Revisions...
;
; 1.01   2/NOV/2004     Contribution from Kevin Lipe
; 1.00  31/OCT/2004     - initial version



;===================================================================
; M A C R O S
;===================================================================

;-------------------------
; CARTRIDGE_START
; Original Author: Sean Riddle
; Inserts the $55 that signals a valid Channel F cartridge and
; then inserts the NOP that takes up the next byte, which places
; the Channel F at the cartridge entry point, $802.

MAC CARTRIDGE_START
CartridgeStart: db $55                 ; valid cart indicator
                NOP                      ; unused byte
ENDM

;-------------------------
; PROMPTS_NO_T
; Original Author: Sean Riddle
; This code functions the same as the "prompts" section of the BIOS,
; but this code doesn't have a "T?" prompt, so it's useful in games that
; don't have time limits or settings.

MAC PROMPTS_NOT
prompts SUBROUTINE
        LR   K,P                 ; 
                PI   pushk               ; 
.prompts2:      LI   $85                 ; red 5 (S)
                LR   $0,A                ; 
                PI   prompt              ; 
                LR   A,$4                ; 
                CI   $08                 ; is it button 4, Start?
                BF   $4,.notbut4         ; no, check others
.notbut2:
                PI   popk                ; yes, return
                PK                       ; 
                
.notbut4:       CI   $02                 ; is it button 2, Mode?
                BF   $4,.notbut2         ; 
                LI   $8e                 ; red M
                LR   $0,A                ; 
                PI   prompt              ; 
                LISU 3                   ; 
                LISL 6                   ; 
                LR   A,(IS)              ; 
                as 4 ;add the mode to the game #
                LR   (IS),A              ; 
                BF   $0,.prompts2        ; 
ENDM



; EOF
