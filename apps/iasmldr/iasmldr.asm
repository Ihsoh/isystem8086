;*------------------------------------------------------*
;|文件名:	iasmldr.asm									|
;|作者:		Ihsoh										|
;|创建时间:	2013-12-26									|
;|														|
;|描述:													|
;|	IASM的加载器										|
;*------------------------------------------------------*	

Header:

	DB	'MTA16'
	DW	Start
	TIMES 20 DB 0

	IASM_A_PAK	DB 'iasm.a.pak'
				TIMES 20 - ($ - IASM_A_PAK) DB 0
	IASM_B_PAK	DB 'iasm.b.pak'
				TIMES 20 - ($ - IASM_B_PAK) DB 0
	
Start:
	
	MOV		AX, CS
	MOV		DS, AX
	MOV		ES, AX
	MOV		SS, AX
	MOV		SP, 0FFFEH
	
	;加载iasm.a.pak
	MOV		DI, 256
	MOV		SI, IASM_A_PAK
	XOR		BX, BX
	MOV		CX, 54
	MOV		AH, 43
	CLD
	
Read1:
	
	INT		21H
	ADD		DI, 512
	INC		BX
	LOOP	Read1
	
	;加载iasm.b.pak
	MOV		SI, IASM_B_PAK
	XOR		BX, BX
	MOV		CX, 54
	
Read2:

	INT		21H
	ADD		DI, 512
	INC		BX
	LOOP	Read2
	
	MOV		SI, 256 + 256
	MOV		DI, 256
	MOV		CX, 2 * 27648
	CLD
	REP MOVSB
	
	JMP		RealStart
	
	TIMES 256 - ($ - Header) DB 0
	
RealStart:
