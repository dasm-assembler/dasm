/*
 * TODO: Don't use this yet! I still need to adapt it further! [phf]
 *
 * $Id: .indent.pro 272 2008-11-02 02:47:46Z phf $
 *
 * Indentation for the dasm project using GNU indent.
 * Based on example by Daniel Mittler <dmittler at jhu dot edu>.
 */
--dont-format-comments
--blank-lines-after-declarations
--blank-lines-after-procedures
/*--blank-lines-before-block-comments*/
--break-before-boolean-operator
--braces-on-if-line
--braces-on-struct-decl-line
--cuddle-do-while
--cuddle-else
--space-after-cast
--no-blank-lines-after-commas
--dont-break-function-decl-args
--no-space-after-function-call-names
--no-space-after-parentheses
--dont-break-procedure-type
--dont-space-special-semicolon
--space-after-for
--space-after-if
--space-after-while
--braces-on-func-def-line /* undocumented in GNU indent 2.2.9 */
