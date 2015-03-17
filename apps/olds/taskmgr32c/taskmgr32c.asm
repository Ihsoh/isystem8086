;*------------------------------------------------------*
;|文件名:	taskmgr32c.asm								|
;|作者:		Ihsoh										|
;|创建时间:	2013-5-11									|
;|														|
;|概述:													|
;|	任务管理器核心主文件								|
;*------------------------------------------------------*

ORG		0H
BITS	16
CPU		8086

SECTION	.bss

SECTION	.data
	
SECTION	.text
	
Header:

	DB	'MTA16'
	DW	Start
	TIMES 256 - ($ - Header) DB 0

	Buffer1	DB 'wahaha~~'
	Buffer2	DB 0, 0, 0, 0, 0, 0, 0, 0, 0
	
Start:
	
	MOV		AX, CS
	MOV		DS, AX
	MOV		ES, AX
	MOV		SS, AX
	MOV		SP, 0FFFEH
	
	MOV		CX, 1
	MOV		AH, 0
	INT		25H
	
	MOV		DX, BX
	MOV		AH, 6
	INT		21H
	MOV		AH, 2
	INT		21H
	MOV		AH, 3
	INt		21H
	
	XOR		DH, DH
	MOV		DL, AL
	MOV		AH, 6
	INT		21H
	MOV		AH, 2
	INT		21H
	MOV		AH, 3
	INt		21H
	
	MOV		SI, 0
	MOV		DI, Buffer1
	MOV		CX, 8
	MOV		AH, 4
	INT		25H
	
	XOR		DH, DH
	MOV		DL, AL
	MOV		AH, 6
	INT		21H
	MOV		AH, 2
	INT		21H
	MOV		AH, 3
	INt		21H
	
	MOV		SI, 0
	MOV		DI, Buffer2
	MOV		CX, 8
	MOV		AH, 5
	INT		25H
	
	XOR		DH, DH
	MOV		DL, AL
	MOV		AH, 6
	INT		21H
	MOV		AH, 2
	INT		21H
	MOV		AH, 3
	INT		21H
	
	MOV		SI, Buffer2
	MOV		AH, 4
	INT		21H
	
	MOV		AH, 0
	INT		21H