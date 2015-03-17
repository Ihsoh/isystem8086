;*------------------------------------------------------*
;|文件名:	mouse.inc									|
;|作者:		Ihsoh										|
;|创建时间:	2013-9-1									|
;|														|
;|描述:													|
;|	鼠标操作.											|
;*------------------------------------------------------*

MouseX		DW	0
MouseY		DW	0
MouseXSign	DW	0FF00H
MouseYSign	DW	0FF00H
MouseButton	DB	0
MouseState	DB	0

;过程名:	InitMouse
;功能:		初始化鼠标.
;参数:		无
;返回值:	无
Procedure	InitMouse
	PUSH	DS
	PUSH	AX
	PUSH	BX
	
	CALL	Init8259A
	
	CALL	EnableMouse
	MOV		BX, 0
	MOV		DS, BX
	MOV		BX, 74H * 4
	MOV		WORD [BX + 0], MouseHandle
	MOV		[BX + 2], CS
	
	;设置中断屏蔽字???
	IN		AL, 0A1H
	AND		AL, 11101111B
	OUT		0A1H, AL
	
	IN		AL, 60H
	
	POP		BX
	POP		AX
	POP		DS
	RET
EndProc		InitMouse

MouseHandle_Counter		DB	0

;过程名:	MouseHandle
;功能:		鼠标处理程序.
;参数:		无
;返回值:	无
Procedure	MouseHandle
	CMP		BYTE [CS:MouseState], 0
	JE		MouseHandle_DisableMouse
	PUSH	AX
	PUSH	BX
	
	IN		AL, 60H
	
	CMP		BYTE [CS:MouseHandle_Counter], 0
	JNE		MouseHandle_Byte2
	MOV		BYTE [CS:MouseButton], 0
	TEST	AL, 1H
	JZ		MouseHandle_LBNoDown
	OR		BYTE [CS:MouseButton], 1H
	
MouseHandle_LBNoDown:

	TEST	AL, 2H
	JZ		MouseHandle_RBNoDown
	OR		BYTE [CS:MouseButton], 2H

MouseHandle_RBNoDown:
	
	TEST	AL, 10H
	JNZ		MouseHandle_LeftD		
	MOV		WORD [CS:MouseXSign], 0
	
MouseHandle_LeftD:

	TEST	AL, 20H
	JNZ		MouseHandle_UpD
	MOV		WORD [CS:MouseYSign], 0
	
MouseHandle_UpD:
	
	JMP		MouseHandle_End
	
MouseHandle_Byte2:

	CMP		BYTE [CS:MouseHandle_Counter], 1
	JNE		MouseHandle_Byte3
	MOV		BX, [CS:MouseXSign]
	MOV		BL, AL
	ADD		[CS:MouseX], BX
	JMP		MouseHandle_End
		
MouseHandle_Byte3:

	MOV		BX, [CS:MouseYSign]
	MOV		BL, AL
	XOR		AX, AX
	SUB		AX, BX
	ADD		[CS:MouseY], AX
	
MouseHandle_End:
	
	INC		BYTE [CS:MouseHandle_Counter]
	CMP		BYTE [CS:MouseHandle_Counter], 3
	JNE		MouseHandle_NoResetCounter
	MOV		WORD [CS:MouseXSign], 0FF00H
	MOV		WORD [CS:MouseYSign], 0FF00H
	MOV		BYTE [CS:MouseHandle_Counter], 0
	
MouseHandle_NoResetCounter:
	
	MOV		AL, 20H
	OUT		20H, AL
	OUT		0A0H, AL
	POP		BX
	POP		AX
	
	JMP		MouseHandle_EnableMouse
	
MouseHandle_DisableMouse:

	PUSH	AX
	IN		AL, 60H
	MOV		AL, 20H
	OUT		20H, AL
	OUT		0A0H, AL
	POP		AX
	
MouseHandle_EnableMouse:
	
	IRET
EndProc		MouseHandle

;过程名:	Init8259A
;功能:		初始化8259A.
;参数:		无
;返回值:	无
Procedure	Init8259A
	PUSHF
	PUSH	AX
	MOV		AL, 11H
	OUT		0A0H, AL
	
	MOV		AL, 70H
	OUT		0A1H, AL
	
	MOV		AL, 2H
	OUT		0A1H, AL
	
	MOV		AL, 1H
	OUT		0A1H, AL
	
	MOV		AL, 0FFH
	OUT		0A1H, AL
	POP		AX
	POPF
	RET
EndProc		Init8259A

;过程名:	EnableMouse
;功能:		许可鼠标.
;参数:		无
;返回值:	无
Procedure	EnableMouse
	PUSHF
	PUSH	AX
	MOV		AL, 0A8H
	OUT		64H, AL
	MOV		AL, 0D4H
	OUT		64H, AL
	MOV		AL, 0F4H
	OUT		60H, AL
	MOV		AL, 60H
	OUT		64H, AL
	MOV		AL, 47H
	OUT		60H, AL
	POP		AX
	POPF
	RET
EndProc		EnableMouse
