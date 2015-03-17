;*------------------------------------------------------*
;|文件名:	int21h.inc									|
;|作者:		Ihsoh										|
;|创建时间:	2013-5-15									|
;|														|
;|概述:													|
;|	INT 21H中断程序										|
;*------------------------------------------------------*
	
SECTION	.data
	INT21H_SS	DW 0
	INT21H_ESP	DD 0

SECTION	.text
	
;过程名:	TempINT21H
;描述:		临时21号中断程序
;参数:		因AH不同而不同
;返回值:	因AH不同而不同
Procedure	TempINT21H
	BITS	32
	
	CALL	Resume8259A
	
	;AH=1
	CMP		AH, 1
	JNE		TempINT21H_Label1
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_EAX], AL
	POP		BX
	POP		DS
	
TempINT21H_Label1:
	;AH=4
	CMP		AH, 4
	JNE		TempINT21H_Label2
	PUSH	ES
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		ES, BX
	MOV		[ES:INT21HTSS + TSS_ESI], ESI
	MOV		[ES:INT21HTSS + TSS_DS], DS
	POP		BX
	POP		ES
	
TempINT21H_Label2:	

	;AH=6
	CMP		AH, 6
	JNE		TempINT21H_Label3
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_EDX], DX
	POP		BX
	POP		DS
	
TempINT21H_Label3:

	;AH=8 || AH=31
	CMP		AH, 31
	JE		TempINT21H_Label14
	CMP		AH, 8
	JNE		TempINT21H_Label4
	
TempINT21H_Label14:
	
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_ES], ES
	MOV		[INT21HTSS + TSS_EDI], EDI
	POP		BX
	POP		DS
	
TempINT21H_Label4:

	;AH=10 || AH=11
	CMP		AH, 10
	JE		TempINT21H_Label5
	CMP		AH, 11
	JNE		TempINT21H_Label6
	
TempINT21H_Label5:
	
	PUSH	ES
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		ES, BX
	MOV		[ES:INT21HTSS + TSS_DS], DS
	MOV		[ES:INT21HTSS + TSS_ESI], ESI
	POP		BX
	POP		ES
	
TempINT21H_Label6:

	;AH=12
	CMP		AH, 12
	JNE		TempINT21H_Label7
	PUSH	FS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		FS, BX
	MOV		[FS:INT21HTSS + TSS_DS], DS
	MOV		[FS:INT21HTSS + TSS_ESI], ESI
	MOV		[FS:INT21HTSS + TSS_ES], ES
	MOV		[FS:INT21HTSS + TSS_EDI], EDI
	MOV		[FS:INT21HTSS + TSS_ECX], ECX
	POP		BX
	POP		FS
	
TempINT21H_Label7:

	;AH=13
	CMP		AH, 13
	JNE		TempINT21H_Label8
	PUSH	FS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		FS, BX
	MOV		[FS:INT21HTSS + TSS_DS], DS
	MOV		[FS:INT21HTSS + TSS_ESI], ESI
	MOV		[FS:INT21HTSS + TSS_ES], ES
	MOV		[FS:INT21HTSS + TSS_EDI], EDI
	POP		BX
	POP		FS
	
TempINT21H_Label8:

	;AH=14 || AH=20 || AH=23
	CMP		AH, 14
	JE		TempINT21H_Label9
	CMP		AH, 20
	JE		TempINT21H_Label9
	CMP		AH, 23
	JNE		TempINT21H_Label12
	
TempINT21H_Label9:

	PUSH	ES
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		ES, BX
	MOV		[ES:INT21HTSS + TSS_DS], DS
	MOV		[ES:INT21HTSS + TSS_ESI], ESI
	POP		BX
	POP		ES
	
TempINT21H_Label12:

	;AH=16
	CMP		AH, 16
	JNE		TempINT21H_Label10
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_EAX], AL
	MOV		[INT21HTSS + TSS_EBX], BL
	POP		BX
	POP		DS
	
TempINT21H_Label10:

	;AH=19 || AH=24 || AH=25
	CMP		AH, 24
	JE		TempINT21H_Label13
	CMP		AH, 25
	JE		TempINT21H_Label13
	CMP		AH, 19
	JNE		TempINT21H_Label11
	
TempINT21H_Label13:
	
	PUSH	FS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		FS, BX
	MOV		[FS:INT21HTSS + TSS_DS], DS
	MOV		[FS:INT21HTSS + TSS_ESI], ESI
	MOV		[FS:INT21HTSS + TSS_ES], ES
	MOV		[FS:INT21HTSS + TSS_EDI], EDI
	POP		BX
	POP		FS
	
TempINT21H_Label11:

	;AH=32
	CMP		AH, 32
	JNE		TempINT21H_Label15
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_EDX], DL
	POP		BX
	POP		DS
	
TempINT21H_Label15:

	;AH=33
	CMP		AH, 33
	JNE		TempINT21H_Label16
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_EDX], DX
	POP		BX
	POP		DS
	
TempINT21H_Label16:
	
	PUSH	DS
	PUSH	BX
	MOV		BX, Taskmgr32DSel
	MOV		DS, BX
	MOV		[INT21HTSS + TSS_EAX + 1], AH
	POP		BX
	POP		DS
	
	Call32	INT21HTSSSel, 0
	
	CALL	Set8259A
	
	IRETD
	
	BITS	16
EndProc		TempINT21H
	
	
;过程名:	INT21H
;描述:		21号中断程序
;参数:		因AH不同而不同
;返回值:	因AH不同而不同
Procedure	INT21H
	BITS	32
	
INT21H_Begin:

	CMP		AH, 0
	JNE		INT21H_Label1
	;*------------------------------------------------------*
	;|功能号:	0											|
	;|功能:		退出任务管理器								|
	;|参数:		无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	;--------------------0号功能结束--------------------
	
	BITS	32
	
INT21H_Label1:
	
	CMP		AH, 1
	JNE		INT21H_Label2
	
	;*------------------------------------------------------*
	;|功能号:	1											|
	;|功能:		打印一个字符								|
	;|操作数:	AL=字符										|
	;|返回值:	无											|
	;*------------------------------------------------------*
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]

	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------1号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label2:

	CMP		AH, 2
	JNE		INT21H_Label3
	;*------------------------------------------------------*
	;|功能号:	2											|
	;|功能:		回车										|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------2号功能结束--------------------

	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label3:
	
	CMP		AH, 3
	JNE		INT21H_Label4
	;*------------------------------------------------------*
	;|功能号:	3											|
	;|功能:		换行										|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------3号功能结束--------------------

	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label4:

	CMP		AH, 4
	JNE		INT21H_Label5
	;*------------------------------------------------------*
	;|功能号:	4											|
	;|功能:		打印以0结束的字符串.						|
	;|操作数:	DS=字符串所在的段的段描述符选择子			|
	;|			ESI=偏移									|
	;|返回值:	无											|
	;*------------------------------------------------------*
	CLD
	MOV		BX, BufferSel
	MOV		ES, BX
	XOR		EDI, EDI
	
INT21H_Label4_Loop:

	LODSB
	STOSB	
	OR		AL, AL
	JZ		INT21H_Label4_Exit
	JMP		INT21H_Label4_Loop
	
INT21H_Label4_Exit:

	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		AX, BufferSeg
	MOV		DS, AX
	MOV		SI, BufferOff
	MOV		AH, 4
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------4号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label5:
	
	CMP		AH, 5
	JNE		INT21H_Label6
	;*------------------------------------------------------*
	;|功能号:	5											|
	;|功能:		清屏.										|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------5号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label6:

	CMP		AH, 6
	JNE		INT21H_Label7
	;*------------------------------------------------------*
	;|功能号:	6											|
	;|功能:		打印一个整数.								|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------6号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label7:

	CMP		AH, 7
	JNE		INT21H_Label8
	;*------------------------------------------------------*
	;|功能号:	7											|
	;|功能:		输入一个字符.								|
	;|操作数:	无											|
	;|返回值:	AL=字符										|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------7号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
	
INT21H_Label8:

	CMP		AH, 8
	JNE		INT21H_Label9
	;*------------------------------------------------------*
	;|功能号:	8											|
	;|功能:		输入一个字符串.								|
	;|操作数:	无											|
	;|返回值:	ES:EDI=字符串选择字:偏移地址				|
	;*------------------------------------------------------*

	;保存ES和DI
	MOV		CX, ES
	MOV		DX, DI
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		ES, BX
	MOV		DI, BufferOff
	
	MOV		AH, 8
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;恢复ES和DI
	MOV		ES, CX
	MOV		DI, DX
	
	MOV		BX, BufferSel
	MOV		DS, BX
	MOV		ESI, BufferOff
	CLD
	
INT21H_Label8_Loop:
	
	LODSB
	STOSB
	OR		AL, AL
	JNZ		INT21H_Label8_Loop
	
	;--------------------8号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label9:
	
	CMP		AH, 9
	JNE		INT21H_Label10
	;*------------------------------------------------------*
	;|功能号:	9											|
	;|功能:		提示信息并等待用户按下一个键.				|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------9号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label10:
	
	CMP		AH, 10
	JNE		INT21H_Label11
	;*------------------------------------------------------*
	;|功能号:	10											|
	;|功能:		新建一个文件.								|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	如果AL=0则成功, 否则失败					|
	;*------------------------------------------------------*
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	CLD
	
	MOV		ECX, MaxFileNameLen
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		SI, BufferOff
	
	INT		21H
	
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------10号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label11:

	CMP		AH, 11
	JNE		INT21H_Label12
	;*------------------------------------------------------*
	;|功能号:	11											|
	;|功能:		删除一个文件.								|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	如果AL=0则成功, 否则失败					|
	;*------------------------------------------------------*
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	CLD
	
	MOV		ECX, MaxFileNameLen
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		SI, BufferOff
	
	INT		21H
	
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------11号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label12:
	
	CMP		AH, 12
	JNE		INT21H_Label13
	;*------------------------------------------------------*
	;|功能号:	12											|
	;|功能:		写数据到一个文件内一个文件.					|
	;|操作数:	DS:ESI=文件名缓冲区段描述符选择子:偏移地址	|
	;|			ES:EDI=数据缓冲区段描述符选择子:偏移地址	|
	;|			ECX=数据长度								|
	;|返回值:	如果AL=0则成功, 否则失败					|
	;*------------------------------------------------------*
	
	PUSH	ES
	POP		FS
	MOV		EBX, EDI
	MOV		EDX, ECX
	
	;复制文件名到: BufferSeg:BufferOff
	MOV		AX, BufferSel
	MOV		ES, AX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	;复制要写入的数据到: BufferSeg:(BufferOff + MaxFileNameLen)
	PUSH	FS
	POP		DS
	MOV		ESI, EBX
	MOV		ECX, EDX
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		ES, BX
	MOV		SI, BufferOff
	MOV		DI, BufferOff + MaxFileNameLen
	MOV		CX, DX
	MOV		AH, 12
	INT		21H
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------12号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label13:

	CMP		AH, 13
	JNE		INT21H_Label14
	;*------------------------------------------------------*
	;|功能号:	13											|
	;|功能:		从一个文件里读数据.							|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	如果AL=0则成功, 否则失败					|
	;|			ES:EDI=数据缓冲区段地址:偏移地址			|
	;|			ECX=数据长度								|
	;*------------------------------------------------------*
	
	PUSH	ES
	POP		FS
	MOV		EBX, EDI
	
	;复制文件名到: BufferSeg:BufferOff
	MOV		AX, BufferSel
	MOV		ES, AX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	AX, EAX, FS
	
	LIDT	[RealVIDTR]
	
	MOV		AX, BufferSeg
	MOV		DS, AX
	MOV		ES, AX
	MOV		SI, BufferOff
	MOV		DI, BufferOff
	MOV		AH, 13
	INT		21H
	MOV		[CS:Taskmgr32TSS + TSS_ECX], CX
	MOV		WORD [CS:Taskmgr32TSS + TSS_ECX + 2], 0
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	AX, EAX, FS
	
	MOV		AX, BufferSel
	MOV		DS, AX
	MOV		ESI, BufferOff
	PUSH	FS
	POP		ES
	MOV		EDI, EBX
	AND		ECX, 0FFFFH
	CLD
	REP MOVSB
	
	;--------------------13号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label14:

	CMP		AH, 14
	JNE		INT21H_Label15
	;*------------------------------------------------------*
	;|功能号:	14											|
	;|功能:		确认文件是否存在.							|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	如果AL=0则文件不存在, 否则存在.				|								
	;*------------------------------------------------------*
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		SI, BufferOff
	INT		21H
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------14号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label15:
	
	CMP		AH, 15
	JNE		INT21H_Label16
	;*------------------------------------------------------*
	;|功能号:	15											|
	;|功能:		格式化文件数据.								|
	;|操作数:	无											|
	;|返回值:	无											|								
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	INT		21H
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------15号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label16:

	CMP		AH, 16
	JNE		INT21H_Label17
	;*------------------------------------------------------*
	;|功能号:	16											|
	;|功能:		打印一个字符并且指定属性.					|
	;|操作数:	AL=字符										|
	;|			BL=属性										|
	;|返回值:	无											|								|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	CX, ECX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	CX, ECX
	
	;--------------------16号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label17:
	
	CMP		AH, 19
	JNE		INT21_Label20
	;*------------------------------------------------------*
	;|功能号:	19											|
	;|功能:		修改文件名.									|
	;|操作数:	DS:ESI=原文件名								|
	;|			ES:EDI=目标文件名							|
	;|返回值:	如果Al=0则成功,否则失败.					|
	;*------------------------------------------------------*

	PUSH	ES
	POP		FS
	MOV		EDX, EDI
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	PUSH	FS
	POP		DS
	MOV		ESI, EDX
	MOV		ECX, MaxFileNameLen
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		ES, BX
	MOV		SI, BufferOff
	MOV		DI, BufferOff + MaxFileNameLen
	INT		21H
	MOV		[CS:Taskmgr32TSS + TSS_EAX], AL
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------19号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21_Label20:
	
	CMP		AH, 20
	JNE		INT21H_Label21
	;*------------------------------------------------------*
	;|功能号:	20											|
	;|功能:		运行命令.									|
	;|操作数:	DS:ESI=命令字符串							|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	CLD
	
INT21H_Label20_Loop:

	LODSB
	STOSB
	OR		AL, AL
	JNZ		INT21H_Label20_Loop
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		SI, BufferOff
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------20号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label21:

	CMP		AH, 21
	JNE		INT21H_Label22
	;*------------------------------------------------------*
	;|功能号:	21											|
	;|功能:		关闭回显.									|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------21号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label22:

	CMP		AH, 22
	JNE		INT21H_Label23
	;*------------------------------------------------------*
	;|功能号:	22											|
	;|功能:		开启回显.									|
	;|操作数:	无											|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------22号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label23:
	
	CMP		AH, 23
	JNE		INT21H_Label24
	;*------------------------------------------------------*
	;|功能号:	23											|
	;|功能:		获取一个文件的长度.							|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	ECX=文件长度								|
	;*------------------------------------------------------*
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		SI, BufferOff
	INT		21H
	AND		ECX, 0FFFFH
	MOV		[CS:Taskmgr32TSS + TSS_ECX], ECX
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------23号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label24:

	CMP		AH, 24
	JNE		INT21H_Label25
	;*------------------------------------------------------*
	;|功能号:	24											|
	;|功能:		获取文件的修改时间.							|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	ES:EDI=文件修改时间缓冲区地址				|
	;*------------------------------------------------------*

	PUSH	ES
	POP		FS
	MOV		EDX, EDI
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX, FS
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		ES, BX
	MOV		SI, BufferOff
	MOV		DI, BufferOff
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX, FS
	
	PUSH	FS
	POP		ES
	MOV		EDI, EDX
	MOV		BX, BufferSel
	MOV		DS, BX
	MOV		ESI, BufferOff
	MOV		ECX, 7
	CLD
	REP MOVSB
	
	;--------------------24号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label25:
	
	CMP		AH, 25
	JNE		INT21H_Label26
	;*------------------------------------------------------*
	;|功能号:	25											|
	;|功能:		获取文件的创建时间.							|
	;|操作数:	DS:ESI=文件名缓冲区段地址:偏移地址			|
	;|返回值:	ES:EDI=文件创建时间缓冲区地址				|
	;*------------------------------------------------------*
	
	PUSH	ES
	POP		FS
	MOV		EDX, EDI
	
	MOV		BX, BufferSel
	MOV		ES, BX
	MOV		EDI, BufferOff
	MOV		ECX, MaxFileNameLen
	CLD
	REP MOVSB
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX, FS
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		DS, BX
	MOV		ES, BX
	MOV		SI, BufferOff
	MOV		DI, BufferOff
	INT		21H
	
	LIDT	[CS:VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX, FS
	
	PUSH	FS
	POP		ES
	MOV		EDI, EDX
	MOV		BX, BufferSel
	MOV		DS, BX
	MOV		ESI, BufferOff
	MOV		ECX, 7
	CLD
	REP MOVSB
	
	;--------------------25号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label26:
	
	CMP		AH, 26
	JNE		INT21H_Label27
	;*------------------------------------------------------*
	;|功能号:	26											|
	;|功能:		获取ISystem版本.							|
	;|操作数:	无											|
	;|返回值:	AH=主版本									|
	;|			AL=副版本									|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	MOV		[Taskmgr32TSS + TSS_EAX], AX
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------26号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label27:

	CMP		AH, 29
	JNE		INT21H_Label30
	;*------------------------------------------------------*
	;|功能号:	29											|
	;|功能:		获取文件系统号.								|
	;|操作数:	无											|
	;|返回值:	AH=主文件系统号								|
	;|			AL=文件系统号								|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	MOV		[Taskmgr32TSS + TSS_EAX], AX
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------29号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label30:

	CMP		AH, 30
	JNE		INT21H_Label31
	;*------------------------------------------------------*
	;|功能号:	30											|
	;|功能:		获取文件数量.								|
	;|操作数:	无											|
	;|返回值:	ECX=文件数									|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	AND		ECX, 0FFFFH
	MOV		[Taskmgr32TSS + TSS_ECX], ECX
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------30号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label31:

	CMP		AH, 31
	JNE		INT21H_Label32
	;*------------------------------------------------------*
	;|功能号:	31											|
	;|功能:		获取所有文件的文件名.						|
	;|操作数:	无											|
	;|返回值:	ES:EDI=	文件名缓冲区(缓冲区的大小必须大于或	|
	;|					等于文件数*20)						|
	;*------------------------------------------------------*
	
	PUSH	ES
	POP		FS
	MOV		EDX, EDI
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX, FS
	
	LIDT	[RealVIDTR]
	
	MOV		BX, BufferSeg
	MOV		ES, BX
	MOV		DI, BufferOff
	MOV		AH, 31
	INT		21H
	MOV		AH, 30
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX, FS
	
	PUSH	FS
	POP		ES
	MOV		EDI, EDX
	MOV		BX, BufferSel
	MOV		DS, BX
	MOV		ESI, BufferOff
	MOV		AX, MaxFileNameLen
	MUL		CX
	MOV		CX, DX
	SHL		ECX, 16
	MOV		CX, AX
	CLD
	REP MOVSB
	
	;--------------------31号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label32:

	CMP		AH, 32
	JNE		INT21H_Label33
	;*------------------------------------------------------*
	;|功能号:	32											|
	;|功能:		打印一个8位的十六进制数.					|
	;|操作数:	DL=8位十六进制数							|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------32号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label33:

	CMP		AH, 33
	JNE		INT21H_Label34
	;*------------------------------------------------------*
	;|功能号:	33											|
	;|功能:		打印一个16位的十六进制数.					|
	;|操作数:	DL=16位十六进制数							|
	;|返回值:	无											|
	;*------------------------------------------------------*
	
	INT21H_EnterReal_SS_ESP_DS	BX, EBX
	
	LIDT	[RealVIDTR]
	
	INT		21H
	
	LIDT	[VIDTR]
	
	INT21H_EnterProd_SS_ESP	BX, EBX
	
	;--------------------33号功能结束--------------------
	
	BITS	32
	
	JMP		INT21H_End
	
INT21H_Label34:
	
INT21H_End:
	
	IRETD
	JMP		INT21H_Begin
	BITS	16
EndProc		INT21H