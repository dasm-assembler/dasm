
MoniRomDelayLoop:
   deca                       ; J     $fd21   4a
_Lfd22:
   psha                       ; .     $fd22   87
   deca                       ; J     $fd23   4a
   deca                       ; J     $fd24   4a
_Lfd25:
   dbnza    _Lfd25            ; K.    $fd25   4b fe
   pula                       ; .     $fd27   86
   dbnzx    _Lfd22            ; ..    $fd28   5b f8
   rts                        ; .     $fd2a   81


