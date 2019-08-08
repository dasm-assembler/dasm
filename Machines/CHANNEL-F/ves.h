; VES.H
; Fairchild Channel F Header
; Version 1.01, 2/NOVEMBER/2004

VERSION_CHANNELF	= 101
VERSION_VES		= 101

; THIS IS A PRELIMINARY RELEASE OF *THE* "STANDARD" VES.H
; THIS FILE IS EXPLICITLY SUPPORTED AS A DASM-PREFERRED COMPANION FILE
; PLEASE DO *NOT* REDISTRIBUTE THIS FILE!
;
; This file defines hardware registers and memory mapping for the
; Fairchild Channel-F. It is distributed as a companion machine-specific support package
; for the DASM compiler. Updates to this file, DASM, and associated tools are
; available at at http://www.atari2600.org/dasm
;
; Many thanks to the original author(s) of this file, and to everyone who has
; contributed to understanding the Channel-F.  If you take issue with the
; contents, or naming of registers, please write to me (atari2600@taswegian.com)
; with your views.  Please contribute, if you think you can improve this
; file!
;
; Latest Revisions...
; 1.01   2/NOV/2004	Kevin Lipe's version (combined macro/header)
;			renamed to VES.H
;			alternates provided for deprecated equates
;			ALL hardware/BIOS equates now in uppercase and prefixed
; 1.00  31/OCT/2004	- initial release



; Please contribute Channel-F header code to atari2600@taswegian.com



INCLUDE_DEPRECATED                 ; remove to DISABLE deprecated equates


;-------------------------------------------------------------------------------
; BIOS Calls
;------------------------

BIOS_CLEAR_SCREEN   = $00d0        ; uses r31
BIOS_DELAY          = $008f
BIOS_PUSH_K         = $0107        ; used to allow more subroutine stack space
BIOS_POP_K          = $011e
BIOS_DRAW_CHARACTER = $0679

;-------------------------------------------------------------------------------
; Colors

COLOR_RED           = $40
COLOR_BLUE          = $80
COLOR_GREEN         = $00
COLOR_BACKGROUND    = $C0

; Alternate (European) spellings...

COLOUR_RES          = COLOR_RED
COLOUR_BLUE         = COLOR_BLUE
COLOUR_GREEN        = COLOR_GREEN
COLOUR_BACKGROUND   = COLOR_BACKGROUND

;-------------------------------------------------------------------------------
; DEPRECATED equates.
; These present to be compatible with existing equate usage.  
; DO NOT USE THESE IN NEW CODE -- WE WANT TO GET RID OF THEM!

	IFCONST INCLUDE_DEPRECATED

clrscrn             = BIOS_CLEAR_SCREEN         ; DEPRECATED!
delay               = BIOS_DELAY                ; DEPRECATED!
pushk               = BIOS_PUSH_K               ; DEPRECATED!
popk                = BIOS_POP_K                ; DEPRECATED!
drawchar            = BIOS_DRAW_CHARACTER       ; DEPRECATED!

red                 = COLOR_RED                 ; DEPRECATED!
blue                = COLOR_BLUE                ; DEPRECATED
green               = COLOR_GREEN               ; DEPRECATED
bkg                 = COLOR_BACKGROUND          ; DEPRECATED

	ENDIF

;-------------------------------------------------------------------------------
; The following required for back-compatibility with code which does not use
; segments.

            SEG

; EOF
