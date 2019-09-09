
This document describes some properties of DASM's
Fairchild F8 backend It does *not* describe how to
program the F8.


Processor selection
-------------------

With DASM, the target CPU is selected with the PROCESSOR
directive inside the source file that should be assembled.
The F8 CPU is selected like this:

	processor f8
	processor F8	; case insensitive




Expressions with parentheses
----------------------------

Some of DASM's backends, for instance the one for the 6502,
don't allow parentheses in expressions that are part of a
mnemonic's operand, because parentheses are used in the 
6502's assembly language to denote indirect addressing.
Instead, you have to use brackets.

This is not the case with the F8 backend. Both parentheses
and brackets can be used everywhere, so the following lines
are parsed and assembled correctly:

        as      (2+2)*2 ; Assembles to $c8
        as      [2+2]*2 ; Assembles to $c8




Data definition directives
--------------------------

Since DS is an F8 instruction (decrement scratchpad register),
the DS directive isn't available anymore if DASM assembles
F8 code. Instead, use the RES directive, which works just like
the DS directive:

        ds.b    4,$33   ; Would assemble to $33 $33 $33 $33,
                        ; but isn't available in F8 mode
        res.b   4,$33   ; Assembles to $33 $33 $33 $33
        
Of course RES.W and RES.L do exist aswell.


For source code compatibility with f8tool (another F8 assembler), some
additional data definition directives are available : DB, DW and DD.
These work just like DC.B, DC.W and DC.L:

        dc.b    $f8     ; Assembles to $f8
        db      $f8     ; Assembles to $f8




Special register names
----------------------

For some of the special registers, multiple names are accepted:

        Register        Accepted names
        ------------------------------
        DC0             DC, DC0
        PC0             P0, PC0
        PC1             P, PC1
        J               J, Any expression that evaluates to 9
                        (This may seem strange, but J is really
                        just an alias for scratchpad register 9)
                        
The names DC, P0, P and J are standard syntax, the other forms
have been introduced for compatibility with other assemblers.

Thus, the following lines assemble all correctly:

        lr      h,dc    ; Assembles to $11
        lr      h,dc0   ; Assembles to $11
        lr      p0,q    ; Assembles to $0d
        lr      pc0,q   ; Assembles to $0d
        lr      p,q     ; Assembles to $09
        lr      pc1,q   ; Assembles to $09
        lr      w,j     ; Assembles to $1d
        lr      w,3*3   ; Assembles to $1d
        
        


Scratchpad register access
--------------------------

There are several ways to access scratchpad registers:

        Access mode                             Accepted syntax
        ---------------------------------------------------------------
        Direct access to registers 0..11        Any expression that
                                                evaluates to 0..11
        
        Access via ISAR                         S, (IS), any expression
                                                that evaluates to 12
                                                
        Access via ISAR, ISAR incremented       I, (IS)+, any expression
                                                that evaluates to 13
                                                
        Access via ISAR, ISAR decremented       D, (IS)-, any expression
                                                that evaluates to 14

The (IS), (IS)+ and (IS)- forms are not standard syntax and have
been mainly introduced for compatibility with f8tool.


For some of the directly accessible scratchpad registers aliases exist:

        Register        Alias name
        --------------------------
        9               J
        10              HU
        11              HL
        
Originally, J was only used with the LR instruction when accessing the
flags, but since J is just an alias for register 9, J can also be used
in normal scratchpad register operations.

The following lines assemble all correctly

        xs      2+2             ; Assembles to $e4
        xs      s               ; Assembles to $ec
        xs      (is)            ; Assembles to $ec
        xs      12              ; Assembles to $ec
        xs      i               ; Assembles to $ed
        xs      (is)+           ; Assembles to $ed
        xs      13              ; Assembles to $ed
        xs      d               ; Assembles to $ee
        xs      (is)-           ; Assembles to $ee
        xs      14              ; Assembles to $ee
        xs      9               ; Assembles to $e9
        xs      j               ; Assembles to $e9
        xs      hu              ; Assembles to $ea
        xs      hl              ; Assembles to $eb




No instruction optimizations are done
-------------------------------------

The assembler doesn't optimize instructions where a smaller
instruction could be used. It won't optimize between
OUT/OUTS, IN/INS and LI/LIS.

For instance, the following line assembles to $20 $00, even
though the LIS instruction could be used, which would need
only one byte.

        li      0       ; Assembles to $20 $00
