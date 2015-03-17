;*------------------------------------------------------*
;|文件名:	boot.asm									|
;|作者:		Ihsoh										|
;|创建时间:	2012-12-30									|
;|														|
;|概述:													|
;|该程序会被BIOS加载到0000:7C00H的位置并被执行. 该程	|
;|序会加载内核到1000:0000H并跳转到那里去执行.			|
;*------------------------------------------------------*

ORG 	7C00H
BITS	16
CPU		8086

SectorCount 	EQU 64		;Kernel size is 32768 bytes
Cly 			EQU 0
Sector 			EQU 2
Head 			EQU 0
Driver 			EQU 0H		;Floppy
SegAddr 		EQU 1000H	;System segment address
OffAddr 		EQU 0000H	;System offest address

%MACRO		Die 1
	MOV		AX, 0
	MOV		DS, AX
	MOV		SI, %1
	MOV		AX, 0B800H
	MOV		ES, AX
	XOR		DI, DI
	CLD
	MOV		AH, 7
%%Loop:
	LODSB
	OR		AL, AL
	JE		$
	STOSW
	JMP		%%Loop
%ENDMACRO

JMP 	Start
KernelAddr 		DW OffAddr, SegAddr
ErrorMsg1		DB 'Error: Unknow error', 0
ErrorMsg2		DB 'Error: Must start with 1.44M floppy', 0

Start:

;Check disk
MOV		DL, 0
MOV		AH, 8
INT		13H
JNC		Success
Die		ErrorMsg1

Success:

CMP		BL, 4
JE		Is_1_44_Floppy
Die		ErrorMsg2

Is_1_44_Floppy:

;Init disk
MOV 	AL, Driver
MOV 	AH, 0
INT 	13H

;Load kernel
MOV 	AX, SegAddr
MOV 	ES, AX
MOV 	AL, SectorCount
MOV 	CH, Cly
MOV 	CL, Sector
MOV 	DH, Head
MOV 	DL, Driver
MOV 	BX, OffAddr
MOV 	AH, 02H
INT 	13H

;Jump to kernel address
JMP 	FAR [CS:KernelAddr]

;Fill 0
TIMES 	510 - ($ - $$) DB 0

;End flag
DW 		0AA55H
