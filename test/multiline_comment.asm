 processor 6502
 ORG $1000

 /* comment 1 */ lda #0

 /* IF 0
*/

  lda #1/*test*/
 /*

*/  lda #2

/*
hello */label lda #3

/*foo*/ lda #4/*bar*/

/* more 
   comments
     */mylabel lda #5 /* too many comments*/ /*but we can deal with it*/

