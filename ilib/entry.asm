;entry.asm
;Ihsoh
;2013-2-17

.MODEL TINY
OPTION CASEMAP:NONE
.CODE
	ORG				0H
Start:
	EXTERN			_main:NEAR
	
	MOV				AX, CS
	MOV				DS, AX
	MOV				SS, AX
	MOV				ES, AX
	MOV				SP, 0FFFEH
	
	CALL			_main
	
	MOV				AH, 0
	INT				21H

END Start
