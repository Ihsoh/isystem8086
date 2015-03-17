;entry16.asm
;Ihsoh
;2013-7-4

.MODEL TINY

EXTRN			_main:NEAR

.CODE

	ORG				256

Start:

	MOV				AX, CS
	MOV				DS, AX
	MOV				SS, AX
	MOV				ES, AX
	MOV				SP, 0FFFEH
	
	CALL			_main
	
	MOV				AH, 0
	INT				21H
	
END Start
