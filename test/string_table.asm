
	.ORG	$e295

_Str0Desc:
	DC.b	_Str1Desc - _Str0Desc,$03
	DC.b	$09,$04
_Str1Desc:
	DC.b	_Str2Desc - _Str1Desc,$03	; $28,$03
	DC.w	"MCT Electronikladen"
_Str2Desc:
	DC.b	_StrTable - _Str2Desc,$03	; $2e,$03
	DC.w	"USB08 Evaluation Board"
_StrTable:
_Str0Vec	DC.w	_Str0Desc	; ..    $e2ef   e2 95
_Str1Vec	DC.w	_Str1Desc	; ..    $e2f1   e2 99
_Str2Vec	DC.w	_Str2Desc	; ..    $e2f3   e2 c1
