;*------------------------------------------------------*
;|文件名:	ieditor.asm									|
;|作者:		Ihsoh										|
;|创建时间:	2013-4-1									|
;|														|
;|描述:													|
;|ISystem自带的编辑器. 提供简单的编辑功能				|
;|														|
;|功能:													|
;|F1/ESC: 		退出编辑器.								|
;|F2:			保存文件.								|
;|F3: 			打开文件.								|
;|Up:			光标向上移动一行.						|
;|Down:			光标向下移动一行.						|
;|Left;			光标向左移动一列.						|
;|Right:		光标向右移动一列.						|
;|Backspace:	向左移动一个并用空格填充该位置.			|
;|Home:			光标移至第一列.							|
;|End:			光标移至最后一列.						|
;|Page Up:		向上一页.								|
;|Page Down:	向下一页.								|
;|CTRL + N:		在当前光标位置的新建一行.				|
;|CTRL + S:		保存.									|
;|CTRL + L:		清空整行.								|
;|CTRL + O:		打开文件.								|
;*------------------------------------------------------*

ORG		0
BITS	16
CPU		8086

%INCLUDE	'..\..\common\common.mac'

SECTION .data
	ScreenX		DB 0	;光标在屏幕的X坐标
	ScreenY		DB 0	;光标在屏幕的Y坐标
	
	BufferX		DW 0	;光标在文本缓冲区的X坐标
	BufferY		DW 0	;光标在文本缓冲区的Y坐标
	
	ScreenTopX	DW 0	;屏幕左上角相对于文本缓冲区的X坐标
	ScreenTopY	DW 0	;屏幕左上角相对于文本缓冲区的Y坐标

;屏幕和缓冲区的关系:
;	
;	+---------------------------+
;	|							|
;	|		    Buffer			|
;	|							|
;	T---------------------------+---+	+---+---------------------------+
;	|(ScreenTopX, ScrrenTopY)	|	|	|   |							|
;	|							|	|	|   |							|
;	|							|	|	|   |							|
;	|			Buffer			|	+---+   |			Screen			|
;	|							|	|	|   |							|
;	|	P(BufferX, BufferY)		|	|	|   |	C(ScreenX, ScreenY)		|
;	|							|	|	|	|							|
;	+---------------------------+---+	+---+---------------------------+
;	|							|
;	|			Buffer			|
;	|							|
;	+---------------------------+
;
;	Screen的左上角在缓冲区的位置由点T(ScreenTopX, ScrrenTopY)给出.
;	
;	Screen上的光标点C(ScreenX, ScreenY)相对于缓冲区的位置的算法为:
;	BufferX = ScreenTopX + ScreenX
;	BufferY = ScreenTopY + ScreenY
;
;	P(BufferX, BufferY)转换为线性位置的算法为:
;	BufferIndex = BufferY * ScreenWidth + BufferX
;
	
	FileName		DB 'noname'	;文件名
					TIMES 20 - ($ - FileName) DB 0
	OpenFileInfo	DB 'Filename: ', 0
	OpenFileErrMsg	DB 13, 10, 'Error: Cannot open file! Create a new file?(0:yes, 1:no): ', 0
	OpenFileName:	TIMES 21 DB 0
	AppInfo			DB 'IEditor [Version ', MainVer, '.', Ver , ']', 13, 10
					DB 'Ihsoh Software, 2013-4-3', 13, 10, 13, 10, 0
	
	;Tab键的位置
	TabPos			DB 	4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, \
						68, 72, 76, 0
	
SECTION .text

	%INCLUDE	'ieditor.inc'
	%INCLUDE	'ieditor.mac'

	JMP		Start
	
;过程名:	ClearBuffer
;描述:		清空缓冲区.
;参数:		无
;返回值:	无
Procedure	ClearBuffer
	PUSHF
	PUSH	AX
	PUSH	CX
	PUSH	ES
	PUSH	DI
	
	CLD
	MOV		AX, BufferSegAddr
	MOV		ES, AX
	MOV		DI, BufferOffAddr
	MOV		CX, MaxRow
ClearBuffer_Loop:
	PUSH	CX
	MOV		CX, MaxCol
	MOV		AL, ' '
	REP STOSB
	MOV		AL, 13
	STOSB
	MOV		AL, 10
	STOSB
	POP		CX
	LOOP	ClearBuffer_Loop
	
	POP		DI
	POP		ES
	POP		CX
	POP		AX
	POPF
	RET
EndProc		ClearBuffer
	
;过程名:	Init
;描述:		初始化.
;参数:		无
;返回值:	无
Procedure	Init
	PUSHF
	PUSH	AX
	PUSH	CX
	PUSH	ES
	PUSH	DI
	;初始化缓冲区
	CALL	ClearBuffer
	
	;清屏
	; 不知道为什么不能用!!!
	; MOV		AH, 5
	; INT		21H
	
	; 不知道为什么不能用!!!
	; XOR		BH, BH
	; XOR		DX, DX
	; MOV		AH, 2
	; INT		16H
	; MOV		CX, 80 * 25
	; MOV		AL, ' '
	; MOV		AH, 1
	; REP INT 21H
	MOV		DI, 0B800H
	MOV		ES, DI
	MOV		DI, 0
	MOV		CX, 80 * 25
	MOV		AH, 7
	MOV		AL, ' '
	CLD
	REP STOSW
	
	;重置光标位置
	CALL	UpdateCaretPos
	
	POP		DI
	POP		ES
	POP		CX
	POP		AX
	POPF
	RET
EndProc		Init
	
;过程名:	PrintBuffer
;描述:		显示缓冲区.
;参数:		ScreenTopX=屏幕左上角在缓冲区的X坐标
;			ScreenTopY=屏幕左上角在缓冲区的Y坐标
;返回值:	无
Procedure	PrintBuffer
	PUSHF
	PUSH	DS
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	AX
	PUSH	CX
	PUSH	DX
	
	;填充显存
	MOV		AX, BufferSegAddr
	MOV		DS, AX
	MOV		AX, [CS:ScreenTopY]
	MOV		DX, ScreenWidth
	MUL		DX
	ADD		AX, [CS:ScreenTopX]
	MOV		SI, AX
	MOV		AX, 0B800H
	MOV		ES, AX
	MOV		DI, 0
	MOV		CX, ScreenWidth * ScreenHeight
	MOV		AH, 7
	CLD
PrintBuffer_Fill:
	LODSB
	CMP		AL, 13
	JNE		PrintBuffer_NotCR
	MOV		AL, ' '
	
PrintBuffer_NotCR:
	
	CMP		AL, 10
	JNE		PrintBuffer_NotLF
	MOV		AL, ' '
	
PrintBuffer_NotLF:
	
	STOSW
	LOOP	PrintBuffer_Fill
	
	POP		DX
	POP		CX
	POP		AX
	POP		DI
	POP		SI
	POP		ES
	POP		DS
	POPF
	RET
EndProc		PrintBuffer
	
;过程名:	OpenFile
;描述:		打开文件.
;参数:		DS:SI=文件名段地址:偏移地址
;返回值:	如果CF=0则成功, 否则失败.
Procedure	OpenFile
	PUSH	AX
	PUSH	ES
	PUSH	DI
	PUSH	CX
	;初始化缓冲区
	CALL	ClearBuffer
	
	;读取文件到缓冲区
	MOV		AX, BufferSegAddr
	MOV		ES, AX
	MOV		DI, BufferOffAddr
	MOV		AH, 13
	INT		21H
	
	;!!!OR指令使CF=0!!!
	OR		AL, AL
	JZ		OpenFile_Err
	STC
	
OpenFile_Err:
	
	POP		CX
	POP		DI
	POP		ES
	POP		AX
	RET
EndProc		OpenFile

;过程名:	SaveFile
;描述:		保存文件.
;参数:		DS:SI=文件名段地址:偏移地址
;返回值:	无
Procedure	SaveFile
	PUSHF
	PUSH	AX
	PUSH	ES
	PUSH	DI
	PUSH	CX
	
	;尝试删除文件
	MOV		AH, 11
	INT		21H
	
	;新建文件
	MOV		AH, 10
	INT		21H
	
	;保存文件
	MOV		AX, BufferSegAddr
	MOV		ES, AX
	MOV		DI, BufferOffAddr
	MOV		CX, BufferLen
	MOV		AH, 12
	INT		21H
	
	POP		CX
	POP		DI
	POP		ES
	POP		AX
	POPF
	RET
EndProc		SaveFile

;过程名:	ScreenToBuffer
;描述:		屏幕坐标到缓冲区坐标.
;参数:		ScreenTopX=屏幕左上角相对于缓冲区的X坐标
;			ScreenTopY=屏幕左上角相对于缓冲区的Y坐标
;			ScreenX=屏幕X坐标
;			ScreenY=屏幕Y坐标
;返回值:	BufferX=缓冲区X坐标
;			BufferY=缓冲区Y坐标
Procedure	ScreenToBuffer
	PUSHF
	PUSH	AX
	MOV		AX, [CS:ScreenTopX]
	ADD		AL, [CS:ScreenX]
	MOV		[CS:BufferX], AX
	MOV		AX, [CS:ScreenTopY]
	ADD		AL, [CS:ScreenY]
	MOV		[CS:BufferY], AX
	POP		AX
	POPF
	RET
EndProc		ScreenToBuffer

;过程名:	UpdateCaretPos
;描述:		更新光标位置.
;参数:		ScreenX=光标X坐标
;			ScreenY=光标Y坐标
;返回值:	无
Procedure	UpdateCaretPos
	PUSHF
	PUSH	AX
	PUSH	BX
	PUSH	DX
	;设置光标位置
	XOR		BH, BH				;0页
	MOV		DL, [CS:ScreenX]	;X
	MOV		DH, [CS:ScreenY]	;Y
	MOV		AH, 2
	INT		10H
	
	POP		DX
	POP		BX
	POP		AX
	POPF
	RET
EndProc		UpdateCaretPos

;过程名:	PrintStatusBar
;描述:		显示状态栏.
;参数:		无
;返回值:	无
Procedure	PrintStatusBar
	PUSHF
	PUSH	AX
	PUSH	CX
	PUSH	DS
	PUSH	SI
	PUSH	ES
	PUSH	DI
	
	MOV		DI, 0B800H
	MOV		ES, DI
	MOV		DI, 80 * 24	* 2	;80 * 24 * 2 = 第25行第一列的位置
	MOV		SI, FileName
	MOV		CX, 20
	MOV		AH, StatusBarProp
	CLD
	;0-19列
PrintStatusBar_PrintFN:
	LODSB
	STOSW
	LOOP	PrintStatusBar_PrintFN
	
	;20列
	MOV		AL, '|'
	STOSW
	
	;21-22列
	XOR		AH, AH
	MOV		AL, [CS:ScreenX]
	MOV		CL, 10
	DIV		CL
	ADD		AL, '0'
	PUSH	AX	;保存余数
	MOV		AH, StatusBarProp
	STOSW
	POP		AX
	MOV		AL, AH
	ADD		AL, '0'
	MOV		AH, StatusBarProp
	STOSW
	
	;22-23列
	MOV		AL, ','
	STOSW
	MOV		AL, ' '
	STOSW
	
	;24-26列
	MOV		AX, [CS:ScreenTopY]
	ADD		AL, [CS:ScreenY]
	ADC		AH, 0
	MOV		CL, 100
	DIV		CL
	PUSH	AX
	ADD		AL, '0'
	MOV		AH, StatusBarProp
	STOSW
	POP		AX
	MOV		AL, AH
	XOR		AH, AH
	MOV		CL, 10
	DIV		CL
	PUSH	AX
	ADD		AL, '0'
	MOV		AH, StatusBarProp
	STOSW
	POP		AX
	MOV		AL, AH
	ADD		AL, '0'
	MOV		AH, StatusBarProp
	STOSW
	
	;27列
	MOV		AL, '|'
	STOSW
	
	;28-79列
	MOV		CX, 79 - 28 + 1
	MOV		AL, ' '
	REP STOSW
	
	POP		DI
	POP		ES
	POP		SI
	POP		DS
	POP		CX
	POP		AX
	POPF
	RET
EndProc		PrintStatusBar

;过程名:	WriteBuffer
;描述:		写缓冲区.
;参数:		BufferX=写缓冲区的X坐标
;			BufferY=写缓冲区的Y坐标
;			AL=字符
;返回值:	无
Procedure	WriteBuffer
	PUSHF
	PUSH	ES
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	DS
	PUSH	SI
	PUSH	DI
	;BX = BufferY * ScreenWidth + BufferX
	PUSH	AX
	MOV		AX, [CS:BufferY]
	MOV		BX, ScreenWidth
	MUL		BX
	ADD		AX, [CS:BufferX]
	MOV		BX, AX
	POP		AX
	
	CMP		BYTE [CS:ScreenX], MaxCol - 1
	JE		WriteBuffer_LineEnd
	
	;DX = BufferY * ScreenWidth + MaxCol - 1
	PUSH	AX
	PUSH	BX
	MOV		AX, [CS:BufferY]
	MOV		BX, ScreenWidth
	MUL		BX
	ADD		AX, MaxCol
	DEC		AX
	MOV		DX, AX
	POP		BX
	POP		AX
	
	;从插入点至到MaxCol向右移动一个字符, 
	;MaxCol - 1位置的字符被抛弃
	PUSH	AX
	MOV		SI, BufferSegAddr
	MOV		DS, SI
	MOV		SI, DX
	DEC		SI		;SI=当前行的倒数第二个字符
	MOV		DI, DX	;DI=当前行的最后一个字符

WriteBuffer_ShiftRight:	;右移
	
	MOV		AL, [SI]
	MOV		[DI], AL
	CMP		BX, SI
	JE		WriteBuffer_ExitShiftRight
	DEC		SI
	DEC		DI
	JMP		WriteBuffer_ShiftRight
	
WriteBuffer_ExitShiftRight:	;退出左移
	
	POP		AX
	
	;[ES:BX] = Char
	MOV		DX, BufferSegAddr
	MOV		ES, DX
	MOV		[ES:BX], AL
	
	INC		BYTE [CS:ScreenX]
	CALL	ScreenToBuffer
	
	JMP		WriteBuffer_NotLineEnd
	
WriteBuffer_LineEnd:	;已到行尾

	;[ES:BX] = Char
	MOV		DX, BufferSegAddr
	MOV		ES, DX
	MOV		[ES:BX], AL

WriteBuffer_NotLineEnd:	;未到行尾
	
	CALL	PrintBuffer
	
	POP		DI
	POP		SI
	POP		DS
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	POP		ES
	POPF
	RET
EndProc		WriteBuffer

;过程名:	ScreenDown
;描述:		屏幕相对于缓冲区向下滚.
;参数:		ScreenTopY=屏幕左上角相对于缓冲区的Y坐标
;返回值:	ScreenTopY=屏幕左上角相对于缓冲区的Y坐标加1或者不变
Procedure	ScreenDown
	PUSHF
	PUSH	AX
	CMP		WORD [CS:ScreenTopY], MaxRow - ScreenHeight
	JE		ScreenDown_Bottom
	INC		WORD [CS:ScreenTopY]
	CALL	ScreenToBuffer
	CALL	PrintBuffer
	
ScreenDown_Bottom:	;已经到底

	POP		AX
	POPF
	RET
EndProc		ScreenDown

;过程名:	ScreenUp
;描述:		屏幕相对于缓冲区向上滚.
;参数:		ScreenTopY=屏幕左上角相对于缓冲区的Y坐标
;返回值:	ScreenTopY=屏幕左上角相对于缓冲区的Y坐标减1或者不变
Procedure	ScreenUp
	PUSHF
	PUSH	AX
	CMP		WORD [CS:ScreenTopY], 0
	JE		ScreemUp_Top
	DEC		WORD [CS:ScreenTopY]
	CALL	ScreenToBuffer
	CALL	PrintBuffer
	
ScreemUp_Top:	;已经到顶
	
	POP		AX
	POPF
	RET
EndProc		ScreenUp

;过程名:	PageUp
;描述:		向上滚一页.
;参数:		ScreenTopY=屏幕左上角相对于缓冲区的Y坐标
;返回值:	ScreenTopY=屏幕左上角相对于缓冲区的Y坐标减去ScreenHeight或者为0
Procedure	PageUp
	PUSHF
	CMP		WORD [CS:ScreenTopY], ScreenHeight
	JA		PageUp_Label1		
	MOV		WORD [CS:ScreenTopY], 0
	JMP		PageUp_Label2
	
PageUp_Label1:
	
	SUB		WORD [CS:ScreenTopY], ScreenHeight
	
PageUp_Label2:
	CALL	ScreenToBuffer
	CALL	PrintBuffer
	
	POPF
	RET
EndProc		PageUp

;过程名:	PageDown
;描述:		向下滚一页.
;参数:		ScreenTopY=屏幕左上角相对于缓冲区的Y坐标
;返回值:	ScreenTopY=屏幕左上角相对于缓冲区的Y坐标加上ScreenHeight或者为MaxRow - ScreenHeight
Procedure	PageDown
	PUSHF
	PUSH	AX
	MOV		AX, [CS:ScreenTopY]
	ADD		AX, ScreenHeight
	CMP		AX, MaxRow - ScreenHeight
	JAE		PageDown_Label1
	ADD		WORD [CS:ScreenTopY], ScreenHeight
	JMP		PageDown_Label2
	
PageDown_Label1:
	
	MOV		WORD [CS:ScreenTopY], MaxRow - ScreenHeight

PageDown_Label2:
	
	CALL	ScreenToBuffer
	CALL	PrintBuffer
	POP		AX
	POPF
	RET
EndProc		PageDown

;过程名:	Tab
;描述:		Tab键处理程序.
;参数:		ScreenX=屏幕左上角相对于缓冲区的X坐标
;返回值:	ScreenX=TabPos表里的其中一个值
Procedure	Tab
	PUSHF
	PUSH	AX
	MOV		SI, TabPos
Tab_Loop:
	LODSB
	OR		AL, AL
	JZ		Tab_Exit
	CMP		[CS:ScreenX], AL
	JAE		Tab_Loop
	MOV		[CS:ScreenX], AL
	CALL	ScreenToBuffer
	
Tab_Exit:
	
	POP		AX
	POPF
	RET
EndProc		Tab

;过程名:	EnterK
;描述:		Enter键处理程序.
;参数:		ScreenTopY=屏幕左上角相对于缓冲区的Y坐标
;返回值:	ScreenTopY
;			ScreenY
;			ScreenX=0
Procedure	EnterK
	PUSHF
	PUSH	AX
	MOV		AX, [CS:ScreenTopY]
	ADD		AX, [CS:ScreenY]
	MOV		BYTE [CS:ScreenX], 0
	CMP		AX, MaxRow - 1
	JE		EnterK_BufferBottom
	CMP		BYTE [CS:ScreenY], ScreenHeight - 1
	JL		EnterK_Bottom
	CALL	ScreenDown
	JMP		EnterK_Exit
	
EnterK_Bottom:
	INC		BYTE [CS:ScreenY]
	CALL	ScreenToBuffer
	
EnterK_Exit:
EnterK_BufferBottom:

	POP		AX
	POPF
	RET
EndProc		EnterK

;过程名:	NewLineC
;描述:		在光标当前位置新建一行, 如果是最末行则不新建
;参数:		ScreenTopY
;			ScreenY
;返回值:	BufferSegAddr:BufferOffAddr ~ BufferSegAddr:(BufferOffAddr + 1)
Procedure	NewLineC
	PUSHF
	PUSH	AX
	PUSH	BX
	PUSH	DX
	PUSH	DS
	PUSH	SI
	PUSH	ES
	PUSH	DI
	
	MOV		AX, BufferSegAddr
	MOV		DS, AX
	MOV		ES, AX
	MOV		SI, BufferOffAddr + BufferLen - ScreenWidth - 1	;倒数第二行在缓冲区的最后一列的位置
	MOV		DI, BufferOffAddr + BufferLen - 1				;最后一行在缓冲区的最后一列的位置
	
	;CX=当前行第一列在缓冲区的位置(包括)至最后一行最后一列(包括)直接的字节数
	MOV		AX, [CS:ScreenTopY]
	ADD		AL, [CS:ScreenY]
	ADC		AH, 0
	MOV		BX, ScreenWidth
	MUL		BX
	MOV		CX, BufferOffAddr
	ADD		CX, BufferLen
	SUB		CX, AX	;CX = CX - 1 - AX + 1 ==> CX = CX - AX
	
	;倒数第二行->倒数第一行(抛弃最后一行)
	;倒数第三行->倒数第二行
	;...
	;第二行->第三行
	;第一行->第二行
	PUSH	AX
	STD
NewLineC_Loop:
	LODSB
	STOSB
	LOOP	NewLineC_Loop
	POP		AX
	
	;清空当前行
	MOV		DI, AX
	
	MOV		AL, ' '
	MOV		CX, MaxCol
	CLD
	REP STOSB
	
	CALL	PrintBuffer
	
	POP		DI
	POP		ES
	POP		SI
	POP		DS
	POP		DX
	POP		BX
	POP		AX
	POPF
	RET
EndProc		NewLineC

;过程名:	ClearLine
;描述:		清除当前行.
;参数:
;返回值:
Procedure	ClearLine
	PUSHF
	PUSH	AX
	PUSH	CX
	PUSH	DX
	PUSH	ES
	PUSH	DI
	MOV		AX, [CS:ScreenTopY]
	ADD		AL, [CS:ScreenY]
	ADC		AH, 0
	MOV		DX, ScreenWidth
	MUL		DX
	MOV		DI, AX
	MOV		AX, BufferSegAddr
	MOV		ES, AX
	MOV		CX, MaxCol
	MOV		AL, ' '
	REP STOSB
	
	CALL	PrintBuffer
	
	POP		DI
	POP		ES
	POP		DX
	POP		CX
	POP		AX
	POPF
	RET
EndProc		ClearLine

;过程名:	Backspace
;描述:		处理退格.
;参数:
;返回值:
Procedure	Backspace
	PUSHF
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SI
	PUSH	DI
	PUSH	DS
	
	CMP		BYTE [CS:ScreenX], 0
	JE		Backspace_LeftBound
	;BX = BufferY * ScreenWidth + BufferX - 1
	PUSH	AX
	MOV		AX, [CS:BufferY]
	MOV		BX, ScreenWidth
	MUL		BX
	ADD		AX, [CS:BufferX]
	MOV		BX, AX
	DEC		BX
	POP		AX
	
	;DX = BufferY * ScreenWidth + MaxCol - 1
	PUSH	AX
	PUSH	BX
	MOV		AX, [CS:BufferY]
	MOV		BX, ScreenWidth
	MUL		BX
	ADD		AX, MaxCol
	DEC		AX
	MOV		DX, AX
	POP		BX
	POP		AX
	
	;左移
	MOV		AX, BufferSegAddr
	MOV		DS, AX
	MOV		DI, BX	;DI=当前光标位置
	MOV		SI, BX
	INC		SI		;SI=当前光标位置的下一个位置

Backspace_ShiftLeft:

	MOV		AL, [SI]
	MOV		[DI], AL
	CMP		DI, DX
	JE		Backspace_ShiftLeftExit
	INC		SI
	INC		DI
	JMP		Backspace_ShiftLeft
	
Backspace_ShiftLeftExit:
	
	;用空白字符填充当前行的最后一列
	PUSH	AX
	PUSH	BX
	PUSH	DS
	MOV		AX, BufferSegAddr
	MOV		DS, AX
	MOV		AX, [CS:BufferY]
	MOV		BX, ScreenWidth
	MUL		BX
	ADD		AX, MaxCol
	DEC		AX
	MOV		BX, AX
	MOV		BYTE [BX], ' '
	POP		DS
	POP		BX
	POP		AX 
	
	DEC		BYTE [CS:ScreenX]
	CALL	ScreenToBuffer
	CALL	PrintBuffer
	
Backspace_LeftBound:	;已是左界
	
	POP		DS
	POP		DI
	POP		SI
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	POPF
	RET
EndProc		Backspace

;过程名:	Convert
;描述:		转换缓冲区数据格式. 把0~77列的CR和LF转换为
;			SPACE. 把78~79列分别设置为CR和LF.
;参数:		缓冲区数据
;返回值:	缓冲区数据
; Procedure	Convert
	; PUSHF
	; PUSH	AX
	; PUSH	CX
	; PUSH	DX
	; PUSH	DS
	; PUSH	SI
	; PUSH	ES
	; PUSH	DI
	
	; MOV		AX, BufferSegAddr
	; MOV		DS, AX
	; MOV		ES, AX
	; MOV		SI, BufferOffAddr
	; MOV		DI, BufferOffAddr
	; MOV		CX, ScreenWidth * MaxRow
	; XOR		DL, DL	;列数计数器
	; CLD
; Convert_Label1:
	; CMP		DL, MaxCol
	; JE		Convert_SetCRLF
	
	; LODSB
	; CMP		AL, 13
	; JE		Convert_CR
	; CMP		AL, 10
	; JE		Convert_LF
	; JMP		Convert_NotCRLF
	
; Convert_CR:
; Convert_LF:	

	; MOV		AL, ' '

; Convert_NotCRLF:
	
	; STOSB
	; INC		DL
	; JMP		Convert_SkipSet
	
; Convert_SetCRLF:
	
	; LODSB	;抛弃
	; LODSB	;抛弃
	; MOV		AL, 13
	; STOSB
	; MOV		AL, 10
	; STOSB
	; DEC		CX
	; DEC		CX
	; XOR		DL, DL
	
; Convert_SkipSet:
	
	; LOOP	Convert_Label1
	
	; POP		DI
	; POP		ES
	; POP		SI
	; POP		DS
	; POP		DX
	; POP		CX
	; POP		AX
	; POPF
	; RET
; EndProc		Convert

;过程名:	Edit
;描述:		编辑.
;参数:		无
;返回值:	无
Procedure	Edit
	PushAR
Edit_Loop:
	MOV		AH, 1
	INT		16H
	JZ		Edit_NoKey
	MOV		AH, 0
	INT		16H
	
	PUSH	AX
	;检测CTRL键是否被按下
	MOV		AH, 2H
	INT		16H
	TEST	AL, 0000_0100B
	JZ		Edit_CTRLNoPress
	POP		AX
	;CTRL + N键
	CMP		AH, Key_N
	JNE		Edit_NoKeyCN
	CALL	NewLineC
	
Edit_NoKeyCN:
	;CTRL + S键
	CMP		AH, Key_S
	JNE		Edit_NoKeyCS
	MOV		SI, FileName
	CALL	SaveFile	;保存文件
	
Edit_NoKeyCS:
	;CTRL + L键
	CMP		AH, Key_L
	JNE		Edit_NoKeyCL
	CALL	ClearLine
	
Edit_NoKeyCL:
	;CTRL + O键
	CMP		AH, Key_O
	JNE		Edit_NoKeyCO
	JMP		OpenNewFile
	
Edit_NoKeyCO:
	
	JMP		Edit_NotCTRLKey
	
Edit_CTRLNoPress:	;CTRL键未被按下
	
	POP		AX

Edit_NotCTRLKey:	;不是CTRL键与其他键的组合键
	
	CMP		AH, Key_F1
	JE		Edit_Exit	;退出编辑
	CMP		AH, Key_Esc
	JE		Edit_Exit	;退出编辑
	CMP		AH, Key_F2
	JNE		Edit_NoSave
	MOV		SI, FileName
	CALL	SaveFile	;保存文件

Edit_NoSave:	;不是保存按键
	CMP		AH, Key_F3
	JNE		Edit_NoOpen
	MOV		SI, FileName
	CALL	OpenFile	;打开文件
	CALL	PrintBuffer
	
Edit_NoOpen:	;不是打开按键
	;上键
	CMP		AH, Key_Up
	JNE		Edit_NoKeyUp
	CMP		BYTE [CS:ScreenY], 0
	JE		Edit_UpBound
	DEC		BYTE [CS:ScreenY]
	CALL	ScreenToBuffer
	JMP		Edit_Next
	
Edit_UpBound:	;已是上界

	CALL	ScreenUp	;上滚一行

Edit_NoKeyUp:	;不是上键
	;下键
	CMP		AH, Key_Down
	JNE		Edit_NoKeyDown
	CMP		BYTE [CS:ScreenY], ScreenHeight - 1
	JE		Edit_DownBound
	INC		BYTE [CS:ScreenY]
	CALL	ScreenToBuffer
	JMP		Edit_Next

Edit_DownBound:	;已是下界	
	
	CALL	ScreenDown	;下滚一行
	
Edit_NoKeyDown:	;不是下键
	;左键
	CMP		AH, Key_Left
	JNE		Edit_NoKeyLeft
	CMP		BYTE [CS:ScreenX], 0
	JE		Edit_LeftBound
	DEC		BYTE [CS:ScreenX]
	CALL	ScreenToBuffer
	JMP		Edit_Next
	
Edit_LeftBound:	;不是左界
Edit_NoKeyLeft:	;不是左键
	;右键
	CMP		AH, Key_Right
	JNE		Edit_NoKeyRight
	CMP		BYTE [CS:ScreenX], MaxCol - 1
	JE		Edit_RightBound
	INC		BYTE [CS:ScreenX]
	CALL	ScreenToBuffer
	JMP		Edit_Next
	
Edit_RightBound:	;不是右界
Edit_NoKeyRight:	;不是右键
	;退格键
	CMP		AH, Key_Backspace
	JNE		Edit_NoKeyBackspace
	CALL	Backspace
	JMP		Edit_Next
	
Edit_NoKeyBackspace:	;不是退格键
	;Home键
	CMP		AH, Key_Home
	JNE		Edit_NoKeyHome
	MOV		BYTE [CS:ScreenX], 0
	CALL	ScreenToBuffer
	JMP		Edit_Next
	
Edit_NoKeyHome:	;不是Home键
	;End键
	CMP		AH, Key_End
	JNE		Edit_NoKeyEnd
	MOV		BYTE [CS:ScreenX], MaxCol - 1
	CALL	ScreenToBuffer
	JMP		Edit_Next
	
Edit_NoKeyEnd:	;不是End键
	;Page Up键
	CMP		AH, Key_PageUp
	JNE		Edit_NoKeyPageUp
	CALL	PageUp
	JMP		Edit_Next
	
Edit_NoKeyPageUp:	;不是Page Up键
	;Page Down键
	CMP		AH, Key_PageDown
	JNE		Edit_NoKeyPageDown
	CALL	PageDown
	JMP		Edit_Next
	
Edit_NoKeyPageDown:	;不是Page Down键
	;Tab键
	CMP		AH, Key_Tab
	JNE		Edit_NoKeyTab
	CALL	Tab
	JMP		Edit_Next
Edit_NoKeyTab:
	;Enter键
	CMP		AH, Key_Enter
	JNE		Edit_NoKeyEnter
	CALL	EnterK
	JMP		Edit_Next
	
Edit_NoKeyEnter:
	
	CMP		AL, PrintASCBeg
	JL		Edit_NotPrintASC
	CMP		AL, PrintASCEnd
	JA		Edit_NotPrintASC
	
	CALL	WriteBuffer	;写缓冲区
	
Edit_InputRightBound:	;输入一个字符后到右界, 不移动光标位置
Edit_NotPrintASC:		;非可打印字符
Edit_Next:				;按键已处理
	
	CALL	UpdateCaretPos	;更新光标位置
	
Edit_NoKey:	;无字符输入

	CALL	UpdateCaretPos	;更新光标位置
	
	CALL	PrintStatusBar
	
	JMP		Edit_Loop
	
Edit_Exit:	;退出编辑
	
	PopAR
	RET
EndProc		Edit
	
Start:
	
	MOV		AX, CS
	MOV		SS, AX
	MOV		SP, 0FFFEH
	PUSH	CS
	POP		ES
	PUSH	CS
	POP		DS
	
	CALL	Init	;初始化
	
	;打印程序信息
	MOV		SI, AppInfo
	MOV		AH, 4
	INT		21H
	
	;输入要打开的文件名
OpenNewFile:
	MOV		SI, OpenFileInfo
	MOV		AH, 4
	INT		21H
	MOV		DI, OpenFileName
	MOV		AH, 8
	INT		21H
	MOV		SI, OpenFileName
	CALL	OpenFile
	JNC		OpenFileNoErr
	MOV		SI, OpenFileErrMsg
	MOV		AH, 4
	INT		21H
	
CheckInput:	;检查输入

	MOV		AH, 0
	INT		16H
	CMP		AL, '0'
	JNE		NoCreateNewFile
	;创建新文件
	MOV		SI, OpenFileName
	MOV		AH, 10
	INT		21H
	CALL	OpenFile
	JMP		OpenFileNoErr
	MOV		AL, '0'
	MOV		AH, 1
	INT		21H
	
NoCreateNewFile:	;不创建新文件

	CMP		AL, '1'
	JE		Error_Exit
	JMP		CheckInput
	
OpenFileNoErr:	;打开文件无错误

	MOV		SI, OpenFileName
	MOV		DI, FileName
	CLD
	MOV		CX, 20
	
CopyFileName:	;复制文件名

	LODSB
	STOSB
	LOOP	CopyFileName
	
	CALL	PrintBuffer		;打印缓冲区
	CALL	Edit			;编辑
	
	;清屏
	MOV		AH, 5
	INT		21H
	JMP		Exit
	
Error_Exit:	;错误退出

	MOV		AL, '1'
	MOV		AH, 1
	INT		21H

Exit:	;正常退出
	
	MOV		AH, 0
	INT		21H
