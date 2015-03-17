;*------------------------------------------------------*
;|文件名:	kernel.asm									|
;|作者:		Ihsoh										|
;|创建时间:	2012-12-30									|
;|														|
;|概述:													|
;|ISystem内核. 内核被Boot程序加载到1000:0000H处执行.	|
;|内核向程序提供基本输入, 基本输出, 文件操作功能并包	|
;|含一个基本的命令行处理程序.							|
;*------------------------------------------------------*

ORG 	0
BITS	16
CPU		8086

%INCLUDE	'..\common\common.inc'
%INCLUDE	'..\common\common.mac'
%INCLUDE	'kernel.mac'

;警告:
;在调用AllocS之后禁止使用POPF, POP指令和修改堆栈指针!!! 除非
;使用了PUSH或调用了FreeS. 但是PUSH和POP必须匹配!!! 否则
;堆栈会被破坏!!!嵌套分配和释放必须匹配!!!

;宏名:		AllocS
;功能:		在内核堆栈上分配一块内存.
;参数:		参数1=要分配的长度.
;返回值:	BP=起始偏移位置, 段地址为SS.
%MACRO AllocS 1
	;检测内核堆栈是否足够大
	PUSH	AX
	PUSHF
	MOV		AX, SP
	CMP		AX, %1 - 4	;因为入栈了AX和标志寄存器, 所以-4
	JAE		%%AllocS_NoErr
	FatalError FEC_KernelStackOverflow
%%AllocS_NoErr:
	POPF
	POP		AX
	
	SUB		SP, %1
	MOV		BP, SP
%ENDMACRO

;宏名:		FreeS
;功能:		从内核堆栈释放一块内存. 
;参数:		参数1=分配的长度.
;返回值:	无.
%MACRO FreeS 1
	ADD		SP, %1
	;检测内核堆栈是否已损坏
	JNC		%%FreeS_NoErr
	FatalError	FEC_KernelStackBreakdown
%%FreeS_NoErr:
%ENDMACRO

;宏名:		FreeSKeepFlags
;功能:		从内核对战释放一块内存但不修改标志寄存器.
;参数:		参数1=分配的长度.
;返回值:	无.
%MACRO FreeSKeepFlags 1
	;储存Flags寄存器!!!FreeS操作会修改CF标志!!!
	PUSH	BP
	PUSHF
	POP		BP

	;释放空间
	FreeS	%1
	
	;恢复Flags寄存器!!!
	PUSH	BP
	POPF
	POP		BP
%ENDMACRO

MaxKernelSize 		EQU 32768	;内核大小. 32KB
MainVer 			EQU 0		;Kernel main version
Ver 				EQU 1		;Kernel version
KernelSegAddr		EQU 1000H	;Kernel SegAddr
KernelOffAddr		EQU 0		;Kernel OffAddr

kernelStartAddress:

JMP		Start

%INCLUDE	'fe.inc'	;致命错误处理
%INCLUDE	'disk.inc'	;硬盘操作
%INCLUDE	'mouse.inc'	;鼠标操作

;警告:
;由于系统通过中断(INT 21H, 20号功能)向程序提供运行命令的功能,
;由于要执行某些命令要调用某些过程, 而在调用时DS和ES可能不包含
;错误值(正常情况下在进入过程前和退出过程后DS和ES会等于CS), 所以
;在进入过程后为了保证DS值和ES的正常, 按照需求恢复DS或ES.

RegStackLen		EQU 512					;寄存器栈长度
RegStack:		TIMES RegStackLen DB 0	;寄存器栈
RegStackPointer	DW RegStackLen			;寄存器栈指针
RegStack_Flags	DW 0					;用于临时保存标志寄存器
RegStack_BX		DW 0					;用于临时保存BX

;宏名:		RegStack_Push
;功能:		把一个寄存器的值压入寄存器栈.
;参数:		参数1=寄存器
;返回值:	无
%MACRO RegStack_Push 1
	;临时储存Flags和BX
	PUSHF
	POP		WORD [CS:RegStack_Flags]
	MOV		[CS:RegStack_BX], BX
	
	SUB		WORD [CS:RegStackPointer], 2
	MOV		BX, [CS:RegStackPointer]
	MOV		[CS:RegStack + BX], %1
	
	;恢复Flags和BX
	PUSH	WORD [CS:RegStack_Flags]
	POPF
	MOV		BX, [CS:RegStack_BX]
%ENDMACRO

;宏名:		RegStack_Pop
;功能:		把一个值从寄存器栈中弹出并储存到
;			指定寄存器中.
;参数:		参数1=寄存器
;返回值:	无
%MACRO RegStack_Pop 1
	;临时储存Flags和BX
	PUSHF
	POP		WORD [CS:RegStack_Flags]
	MOV		[CS:RegStack_BX], BX
	
	MOV		BX, [CS:RegStackPointer]
	MOV		%1, [CS:RegStack + BX]
	ADD		WORD [CS:RegStackPointer], 2
	
	;恢复Flags和BX
	PUSH	WORD [CS:RegStack_Flags]
	POPF
	MOV		BX, [CS:RegStack_BX]
%ENDMACRO

;宏名:		SetDSES
;功能:		保存DS和ES, 恢复DS和ES为运行程序之前的值
;参数:		无
;返回值:	无
%MACRO SetDSES 0
	RegStack_Push	DS
	RegStack_Push	ES
	MOV 	DS, [CS:Exec_Register + ER_DS]
	MOV 	ES, [CS:Exec_Register + ER_ES]
%ENDMACRO

;宏名:		ResumeDSES
;功能:		恢复DS和ES为程序运行时的值
;参数:		无
;返回值:	无
%MACRO ResumeDSES 0
	RegStack_Pop	ES
	RegStack_Pop	DS
%ENDMACRO

;宏名:		SetDS
;功能:		保存DS, 恢复DS为程序运行之前的值
;参数:		无
;返回值:	无
%MACRO SetDS 0
	RegStack_Push	DS
	MOV 	DS, [CS:Exec_Register + ER_DS]
%ENDMACRO

;宏名:		ResumeDS
;功能:		恢复DS为程序运行时的值
;参数:		无
;返回值:	无
%MACRO ResumeDS 0
	RegStack_Pop	DS
%ENDMACRO ResumeDS

;宏名:		SetES
;功能:		保存ES, 恢复ES为程序运行之前的值
;参数:		无
;返回值:	无
%MACRO SetES 0
	RegStack_Push	ES
	MOV 	ES, [CS:Exec_Register + ER_ES]
%ENDMACRO

;宏名:		ResumeES
;功能:		恢复ES为程序运行时的值
;参数:		无
;返回值:	无
%MACRO ResumeES 0
	RegStack_Pop	ES
%ENDMACRO

True				EQU 1
False				EQU 0
AppRun				EQU 1	;程序正在运行
AppNoRun			EQU 0	;程序没有在运行

Exec_Flag			DB AppNoRun		;当程序运行时, 该标志位1否则为0
SystemCall			DB False		;是否正在系统调用? 如果一个程序使用
									;系统调用(通过中断INT 21H的20号功能执行
									;一个命令). 在调用中断时已经恢复为内核堆栈!!! 
									;所以如果为系统调用并且再调用中断时不恢复堆栈!!!

;宏名:		SetSSSP
;功能:		保存SS和SP, 恢复SS和SP为程序运行之前的值
;参数:		无
;返回值:	无
%MACRO SetSSSP 0
	;如果未启动程序, 则不设置SS和SP
	CMP		BYTE [CS:Exec_Flag], AppNoRun
	JE		%%SetSSSP_AppNoRun
	
	;如果已经处在系统调用状态, 则不设置SS和SP
	CMP		BYTE [CS:SystemCall], True
	JE		%%SetSSSP_SystemCall
	
	RegStack_Push	SS
	RegStack_Push	SP
	MOV		SS, [CS:Exec_Register + ER_SS]
	MOV		SP, [CS:Exec_Register + ER_SP]
	
%%SetSSSP_SystemCall:
%%SetSSSP_AppNoRun:

%ENDMACRO

;功能:		ResumeSSSP
;功能:		恢复SS和SP为程序运行时的值
;参数:		无
;返回值:	无
%MACRO ResumeSSSP 0
	;如果未启动程序, 则不恢复SS和SP
	CMP		BYTE [CS:Exec_Flag], AppNoRun
	JE		%%ResumeSSSP_AppNoRun
	
	;如果已经处于系统调用状态, 则不恢复SS和SP
	CMP		BYTE [CS:SystemCall], True
	JE		%%ResumeSSSP_SystemCall
	
	RegStack_Pop	SP
	RegStack_Pop	SS
	
%%ResumeSSSP_SystemCall:
%%ResumeSSSP_AppNoRun:

%ENDMACRO

;Output sub system(OSS)
ConsoleWidth 		EQU 80
ConsoleHeight 		EQU 25

;Function: Init OSS
Procedure	OSSInit
	PUSHF
	PUSH	AX
	MOV 	AL, 3
	MOV 	AH, 0
	INT 	10H
	MOV 	AL, 0
	MOV 	AH, 5
	INT 	10H
	POP 	AX
	POPF
	RET
EndProc		OSSInit

;Function: Clear screen
Procedure	OSSCls
	PUSHF
	PUSH	DI
	PUSH	ES
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	;Clear screen
	MOV		AX, 0B800H
	MOV		ES, AX
	MOV		DI, 0
	MOV		CX, 80 * 25
	MOV		AH, 7
	MOV		AL, ' '
	CLD
	REP STOSW
	
	;Set cursor to (0, 0)
	XOR 	BH, BH
	MOV 	DX, 0
	MOV 	AH, 2H
	INT 	10H
	
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POP		ES
	POP		DI
	POPF
	RET
EndProc		OSSCls

;Function:		OSSPrintChar
;Parameters:	AL=Char
Procedure	OSSPrintChar
	PUSHF
	PUSH 	BX
	MOV 	BL, 7
	CALL 	OSSPrintCharP
	POP 	BX
	POPF
	RET
EndProc		OSSPrintChar
	
;Function:		Print char with property
;Parameters:	AL=Char, BL=Property
Procedure	OSSPrintCharP
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	MOV 	BH, 0
	MOV 	CX, 1
	MOV 	AH, 9H
	INT 	10H
	MOV 	AH, 3H
	INT 	10H
	CMP 	DH, ConsoleHeight - 1
	JNE 	OSSPrintCharP_Label1
	CMP 	DL, ConsoleWidth - 1
	JNE 	OSSPrintCharP_Label3
	XOR 	CX, CX
	MOV 	DL, ConsoleWidth - 1
	MOV 	DH, ConsoleHeight - 1
	MOV 	AL, 1
	MOV 	AH, 6H
	INT 	10H
	MOV 	DL, 0
	MOV 	DH, ConsoleHeight - 1
	MOV 	AH, 2H
	INT 	10H
	JMP 	OSSPrintCharP_Label2
OSSPrintCharP_Label3:
	INC 	DL
	MOV 	AH, 2H
	INT 	10H
	JMP 	OSSPrintCharP_Label2
OSSPrintCharP_Label1:
	CMP 	DL, ConsoleWidth - 1
	JNE 	OSSPrintCharP_Label4
	MOV 	DL, 0
	INC 	DH
	MOV 	AH, 2H
	INT 	10H
	JMP 	OSSPrintCharP_Label2
OSSPrintCharP_Label4:
	INC 	DL
	MOV 	AH, 2H
	INT 	10H
OSSPrintCharP_Label2:
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	RET
EndProc		OSSPrintCharP
	
;Function:	Return
Procedure	OSSCR
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	
	;This action will be change cx!!!
	MOV 	BH, 0
	MOV 	AH, 3H
	INT 	10H
	
	MOV 	DL, 0
	MOV 	AH, 2H
	INT 	10H
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	RET
EndProc		OSSCR
	
;Function:	End line
Procedure	OSSLF
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	MOV 	BH, 0
	MOV 	AH, 3H
	INT 	10H
	CMP 	DH, ConsoleHeight - 1
	JNE 	OSSLF_Label1
	PUSH 	DX
	MOV		BH, 07H
	MOV 	AL, 1
	MOV 	CX, 0
	MOV 	DH, ConsoleHeight - 1
	MOV 	DL, ConsoleWidth - 1
	MOV 	AH, 6H
	INT 	10H
	POP 	DX
	MOV 	AH, 2H
	INT 	10H
	JMP 	OSSLF_Label2
OSSLF_Label1:
	INC 	DH
	MOV 	AH, 2H
	INT 	10H
OSSLF_Label2:
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	RET
EndProc		OSSLF
	
;Function:		PrintString
;Parameters:	DS:SI=String SegAddr:OffAddr
Procedure	OSSPrintString
	PUSHF
	PUSH 	SI
	PUSH 	AX
	CLD
OSSPrintString_Label1:
	LODSB
	;if char == 0 then return
	CMP 	AL, 0
	JE OSSPrintString_Label2
	
	CMP 	AL, 10
	JNE 	OSSPrintString_Label3
	CALL 	OSSLF
	JMP 	OSSPrintString_Label1
OSSPrintString_Label3:
	CMP 	AL, 13
	JNE 	OSSPrintString_Label4
	CALL 	OSSCR
	JMP 	OSSPrintString_Label1
OSSPrintString_Label4:
	CALL 	OSSPrintChar
	JMP 	OSSPrintString_Label1
OSSPrintString_Label2:
	POP 	AX
	POP 	SI
	POPF
	RET
EndProc		OSSPrintString
	
;Dividends
Dividend DW 10000, 1000, 100, 10, 1
	
;Function:		Print a integer(2 Bytes)
;Parameters:	AX=Integer
Procedure	OSSPrintInteger
	PUSHF
	PUSH 	AX
	CMP 	AX, 0
	JE 		OSSPrintInteger_Label4
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	PUSH 	SI
	PUSH 	DI
	PUSH 	BP
	XOR 	BP, BP	;If bp == 0 then don't print 0
	XOR 	DX, DX
	XOR 	SI, SI
	MOV 	BX, Dividend
	MOV 	CX, 5
OSSPrintInteger_Label1:
	MOV 	DI, [CS:BX + SI]
	DIV 	DI
	CMP 	AL, 0
	JNE 	OSSPrintInteger_Label2
	CMP 	BP, 0
	JE 		OSSPrintInteger_Label3
OSSPrintInteger_Label2:
	ADD 	AL, '0'
	CALL 	OSSPrintChar
	MOV 	BP, 1
OSSPrintInteger_Label3:
	MOV 	AX, DX
	XOR 	DX, DX
	ADD 	SI, 2
	LOOP 	OSSPrintInteger_Label1
	POP 	BP
	POP 	DI
	POP 	SI
	POP 	DX
	POP 	CX
	POP 	BX
	JMP 	OSSPrintInteger_Label5
OSSPrintInteger_Label4:
	MOV 	AL, '0'
	CALL 	OSSPrintChar
OSSPrintInteger_Label5:
	POP 	AX
	POPF
	RET
EndProc		OSSPrintInteger
	
;Function:		Print two BCD codes
;Parameters:	AL=BCD codes
Procedure	OSSPrintBCD
	PUSHF
	PUSH 	AX
	PUSH 	CX
	MOV 	AH, AL
	MOV 	CL, 4
	SHR 	AL, CL
	CMP 	AL, 0
	JE 		OSSPrintBCD_Label1
	ADD 	AL, '0'
	CALL 	OSSPrintChar
OSSPrintBCD_Label1:
	MOV 	AL, AH
	AND 	AL, 0FH
	ADD 	AL, '0'
	CALL 	OSSPrintChar
	POP 	CX
	POP 	AX
	POPF
	RET
EndProc		OSSPrintBCD
	
;过程名:	OSSPrintHex8Bit
;功能:		打印一个8位的十六进制数.
;参数:		AL=十六进制数
;返回值:	无
Procedure	OSSPrintHex8Bit
	PUSHF
	PUSH	AX
	PUSH	CX
	
	MOV		CL, 4
	ROR		AL, CL
	PUSH	AX
	AND		AL, 0FH
	CMP		AL, 10
	JAE		OSSPrintHex8Bit_Label1
	ADD		AL, '0'
	JMP		OSSPrintHex8Bit_Label2
	
OSSPrintHex8Bit_Label1:

	ADD		AL, 'A' - 10
	
OSSPrintHex8Bit_Label2:

	CALL	OSSPrintChar
	POP		AX
	MOV		CL, 4
	SHR		AL, CL
	AND		AL, 0FH
	CMP		AL, 10
	JAE		OSSPrintHex8Bit_Label3
	ADD		AL, '0'
	JMP		OSSPrintHex8Bit_Label4
		
OSSPrintHex8Bit_Label3:

	ADD		AL, 'A' - 10

OSSPrintHex8Bit_Label4:

	CALL	OSSPrintChar
	
	POP		CX
	POP		AX
	POPF
	RET
EndProc		OSSPrintHex8Bit
	
;过程名:	OSSPrintHex16Bit
;功能:		打印一个16位的十六进制数
;参数:		AX=十六进制数
;返回值:	无
Procedure	OSSPrintHex16Bit
	PUSH	AX
	XCHG	AH, AL
	CALL	OSSPrintHex8Bit
	XCHG	AH, AL
	CALL	OSSPrintHex8Bit
	POP		AX
	RET
EndProc		OSSPrintHex16Bit

;----------------------------------------
	
;Input sub system(ISS)

;Function:	Get a char from keyboard
;Return:	AL=Char, AH=Code
Procedure	ISSInputChar
	PUSHF
	
	MOV 	AH, 0H
	INT 	16H
	CMP 	AL, 0
	JE 		ISSInputChar_Label1
	CALL 	OSSPrintChar
ISSInputChar_Label1:
	
	POPF
	RET
EndProc		ISSInputChar

;Function:	Get a string from keyboard until get a return key(13)
;Return:	ES:DI=String SegAddr:OffAddr
Procedure	ISSInputString
	PUSHF
	PUSH 	BP	;Current caret position
	PUSH 	AX
	PUSH 	CX	;Char count
	PUSH 	DI
	PUSH 	SI
	XOR 	CX, CX
	CLD
ISSInputString_Label1:
	MOV 	AH, 0H
	INT 	16H
	CMP 	AL, 13
	JE 		ISSInputString_Label2
	CMP 	AL, 8
	JNE 	ISSInputString_Label3
	CMP 	CX, 0
	JE 		ISSInputString_Label1
	;Backspace
	DEC 	DI
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	XOR 	BH, BH
	MOV 	AH, 3H
	INT 	10H
	CMP 	DL, 0
	MOV 	SI, DX
	JE 		ISSInputString_Label4
	DEC 	DL
	MOV 	AH, 2H
	INT 	10H
	;Print ' '
	MOV 	AL, ' '
	MOV 	BH, 0
	MOV 	BL, 7
	MOV 	CX, 1
	MOV 	AH, 9H
	INT 	10H
	
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	
	DEC 	CX
	JMP 	ISSInputString_Label1
ISSInputString_Label4:
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX

	PUSH 	DX
	MOV 	DX, SI
	;Cursor in (0, y)
	CMP 	DH, 0
	
	POP 	DX
	;Cursor in (0, 0)
	JE 		ISSInputString_Label1
	
	PUSH 	DX
	MOV 	DX, SI
	
	;Set cursor to (ConsoleWidth - 1, y - 1) and print ' '
	MOV 	DL, ConsoleWidth - 1
	DEC 	DH
	
	MOV 	AH, 2H
	INT 	10H
	
	;Print ' '
	MOV 	AL, ' '
	MOV 	BH, 0
	MOV 	BL, 7
	PUSH 	CX
	MOV 	CX, 1
	MOV 	AH, 9H
	INT 	10H
	POP 	CX
	
	DEC 	CX
	POP 	DX
	JMP 	ISSInputString_Label1
ISSInputString_Label3:
	CMP 	AH, 77
	JNE 	ISSInputString_Label5
	;Right
	
	;No implement!!!
	
ISSInputString_Label5:
	CMP 	AH, 75
	JNE 	ISSInputString_Label6
	;Left
	
	;No implement!!!
	
ISSInputString_Label6:
	CALL 	OSSPrintChar
	
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	XOR 	BH, BH
	MOV 	AH, 3H
	INT 	10H
	MOV 	AH, 2H
	INT 	10H
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	
	STOSB
	INC 	CX
	JMP 	ISSInputString_Label1
ISSInputString_Label2:
	MOV 	AL, 0
	STOSB
	CALL 	OSSCR
	CALL 	OSSLF
	POP 	SI
	POP 	DI
	POP 	CX
	POP 	AX
	POP 	BP
	POPF
	RET
EndProc		ISSInputString
	
ISSPause_Msg DB 'Press any key to continue!', 13, 10, 0
	
;Function:	Pause
Procedure	ISSPause
	PUSHF
	PUSH 	SI
	PUSH 	AX
	MOV 	SI, ISSPause_Msg
	CALL 	OSSPrintString
	MOV 	AH, 0
	INT 	16H
	POP 	AX
	POP 	SI
	POPF
	RET
EndProc		ISSPause
	
;----------------------------------------

;Console sub system(CSS)

;Command name
TimeCommand 	DB 'time', 0		;Print current time
ClsCommand 		DB 'cls', 0			;Clear screen
VerCommand 		DB 'ver', 0			;Print kernel version
HelpCommand 	DB 'help', 0		;Print help
NewFileCommand 	DB 'newfile', 0		;Create a new file
DelFileCommand 	DB 'delfile', 0		;Delete a file
FilesCommand 	DB 'files', 0		;Print all files infomation
ExecCommand 	DB 'exec', 0		;Execute a application
RebootCommand 	DB 'reboot', 0		;Reboot computer
FormatCommand 	DB 'format', 0		;Format file data
SetDateCommand 	DB 'setdate', 0		;Set date
SetTimeCommand 	DB 'settime', 0		;Set time
RenameCommand	DB 'rename', 0		;Rename
EchoOffCommand	DB 'echooff', 0		;Echo off
EchoOnCommand	DB 'echoon', 0		;Echo on
PauseCommand	DB 'pause', 0		;Wait user press key
EchoCommand		DB 'echo', 0		;Print text
Help1Command	DB '?', 0			;Print help
CDCommand		DB 'cd', 0			;Change current disk
CopyCommand		DB 'copy', 0		;Copy file
CutCommand		DB 'cut', 0			;Cut file

%ifdef DEBUG

;Test command name
TestCommand 	DB 'test', 0			;Test!!!

%endif

;Store command name
MaxCommandLength 	EQU 10
Command: 			TIMES MaxCommandLength + 1 DB 0

;Store command parameters
MaxParametersLength EQU 100H
Parameters: 		TIMES MaxParametersLength + 1 DB 0

;Input buffer
InputBuffer 		TIMES MaxCommandLength + MaxParametersLength + 1 DB 0

NotValidCommand 	DB 'This command is not valid!', 13, 10, 0

ParameterErrorMsg 	DB 'Parameter error!', 13, 10, 0

Console_Echo		DB True		;执行命令是否回显

;过程:		EnterConsole
;功能:		进入控制台
Procedure	EnterConsole
	PUSH	AX
	PUSH	DI
	PUSH	SI
	
EnterConsole_Label1:
	;接受用户输入
	CMP		BYTE [CS:Console_Echo], False
	JE		EnterConsole_NoEcho
	MOV		AL, [CS:CurrentDisk]
	CALL	FSSGetDiskName
	XCHG	AH, AL
	CALL	OSSPrintChar
	MOV		AL, AH
	CALL	OSSPrintChar
	;Print '>'
	MOV 	AL, '>'
	CALL 	OSSPrintChar
	MOV		AL, ' '
	CALL	OSSPrintChar
	
EnterConsole_NoEcho:
	
	;Wait user input command
	MOV 	DI, InputBuffer
	CALL 	ISSInputString
	
	MOV 	SI, InputBuffer
	
	CALL	Console
	JMP		EnterConsole_Label1
	
	POP		SI
	POP		DI
	POP		AX
	RET
EndProc		EnterConsole

;过程:		Console
;功能:		执行命令
;参数:		DS:SI=命令字符串段地址:偏移地址
;返回值:	无
;备注:		原来该过程的工作是接受用户输入命令并执行, 但是
;			因为需要向应用程序提供执行命令的功能, 所以修改
;			该过程为只执行命令但不接受用户输入.
Procedure	Console
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	PUSH 	SI
	PUSH 	DI
	PUSH 	BP
	
	;Clear command
	MOV 	CX, MaxCommandLength + 1
	XOR 	BX, BX
Console_ClearCommandLoop:
	MOV 	BYTE [CS:Command + BX], 0
	INC 	BX
	LOOP 	Console_ClearCommandLoop
	
	;Clear parameters
	MOV 	CX, MaxParametersLength + 1
	XOR 	BX, BX
Console_ClearParametersLoop:
	MOV 	BYTE [CS:Parameters + BX], 0
	INC 	BX
	LOOP 	Console_ClearParametersLoop
	;Execute command
	
	;Check command is empty
	CALL 	StrLen
	CMP 	CX, 0
	JE 		Console_Label1
	
	;Get command name
	MOV 	AH, ' '
	MOV 	BX, 0
	CALL 	StrPosL
	JNC 	Console_Label2
	;No parameters command
	
	;Is time command?
	MOV 	DI, TimeCommand
	CALL 	StrCmp
	JC 		Console_NotTimeCommand
	CALL 	ShowCurrentTime
	JMP 	Console_Label1
Console_NotTimeCommand:
	
	;Is cls command?
	MOV 	DI, ClsCommand
	CALL 	StrCmp
	JC 		Console_NotClsCommand
	CALL 	OSSCls
	JMP 	Console_Label1
Console_NotClsCommand:

	;Is ver command?
	MOV 	DI, VerCommand
	CALL 	StrCmp
	JC 		Console_NotVerCommand
	CALL 	ShowVersion
	JMP 	Console_Label1
Console_NotVerCommand:

	;Is help command?
	MOV 	DI, HelpCommand
	CALL 	StrCmp
	JC 		Console_NotHelpCommand
	CALL 	ShowHelp
	JMP 	Console_Label1
Console_NotHelpCommand:

	;Is files command?
	MOV 	DI, FilesCommand
	CALL 	StrCmp
	JC 		Console_NotFilesCommand
	PUSHF
	CLI
	CALL 	Files
	POPF
	JMP 	Console_Label1
Console_NotFilesCommand:

	;Is reboot command?
	MOV 	DI, RebootCommand
	CALL 	StrCmp
	JC 		Console_NotRebootCommand
	CALL 	Reboot
	JMP 	Console_Label1
Console_NotRebootCommand:

	;Is format command?
	MOV 	DI, FormatCommand
	CALL 	StrCmp
	JC 		Console_NotFormatCommand
	CALL 	Format
	JMP 	Console_Label1
Console_NotFormatCommand:

	;Is setdate command?
	MOV 	DI, SetDateCommand
	CALL 	StrCmp
	JC 		Console_NotSetDateCommand
	CALL 	SetDate
	JMP 	Console_Label1
Console_NotSetDateCommand:

	;Is settime command?
	MOV 	DI, SetTimeCommand
	CALL 	StrCmp
	JC 		Console_NotSetTimeCommand
	CALL 	SetTime
	JMP 	Console_Label1
Console_NotSetTimeCommand:

	;Is echooff command?
	MOV		DI, EchoOffCommand
	CALL	StrCmp
	JC		Console_NotEchoOffCommand
	MOV		BYTE [CS:Console_Echo], False
	JMP 	Console_Label1
Console_NotEchoOffCommand:

	;Is echoon command?
	MOV		DI, EchoOnCommand
	CALL	StrCmp
	JC		Console_NotEchoOnCommand
	MOV		BYTE [CS:Console_Echo], True
	JMP 	Console_Label1
Console_NotEchoOnCommand:

	;Is pause command?
	MOV		DI, PauseCommand
	CALL	StrCmp
	JC		Console_NotPauseCommand
	MOV		AH, 9
	INT		21H
	JMP		Console_Label1
Console_NotPauseCommand:

	;Is '?' command?
	MOV		DI, Help1Command
	CALL	StrCmp
	JC		Console_NotHelp1Command
	CALL	ShowHelp
	JMP		Console_Label1
Console_NotHelp1Command:
	
	;Is newfile, delfile, exec, rename, echo, cd command?
	MOV 	DI, NewFileCommand
	CALL 	StrCmp
	JNC 	Console_Label4
	MOV 	DI, DelFileCommand
	CALL 	StrCmp
	JNC 	Console_Label4
	MOV 	DI, ExecCommand
	CALL 	StrCmp
	JNC 	Console_Label4
	MOV		DI, RenameCommand
	CALL	StrCmp
	JNC		Console_Label4
	MOV		DI, EchoCommand
	CALL	StrCmp
	JNC		Console_Label4
	MOV		DI, CDCommand
	CALL	StrCmp
	JNC		Console_Label4
	JMP 	Console_Label3
	
Console_Label4:
	MOV 	SI, ParameterErrorMsg
	CALL 	OSSPrintString
	JMP 	Console_Label1
Console_Label3:

%IFDEF DEBUG

	;Is test command?
	MOV 	DI, TestCommand
	CALL 	StrCmp
	JC 		Console_NotTestCommand
	CALL 	_Test
	JMP 	Console_Label1
Console_NotTestCommand:

%ENDIF

	;Invalid command
	MOV 	SI, NotValidCommand
	CALL 	OSSPrintString
	JMP 	Console_Label1
	
Console_Label2:	
	;Include parameters command
	
	;Get command name
	MOV 	DI, Command
	MOV 	CX, BX
	CALL 	StrLeft
	
	;Get parameters
	MOV 	DI, Parameters
	CALL 	StrLen	;Get input string length
	SUB 	CX, BX
	DEC 	CX
	CALL 	StrRight
	
	;Is newfile command?
	MOV 	SI, Command
	MOV 	DI, NewFileCommand
	CALL 	StrCmp
	JC 		Console_NotNewFileCommand
	CALL 	NewFile
	JMP 	Console_Label1
Console_NotNewFileCommand:

	;Is delfile command?
	MOV 	DI, DelFileCommand
	CALL 	StrCmp
	JC 		Console_NotDelFileCommand
	CALL 	DelFile
	JMP 	Console_Label1
Console_NotDelFileCommand:

	;Is exec command?
	MOV 	DI, ExecCommand
	CALL 	StrCmp
	JC 		Console_NotExecCommand
	CALL 	Exec
	JMP 	Console_Label1
Console_NotExecCommand:

	;Is rename command?
	MOV		DI, RenameCommand
	CALL	StrCmp
	JC		Console_NotRenameCommand
	CALL	Rename
	JMP		Console_Label1
Console_NotRenameCommand:

	;Is echo command?
	MOV		DI, EchoCommand
	CALL	StrCmp
	JC		Console_NotEchoCommand
	PUSH	SI
	MOV		SI, Parameters
	CALL	OSSPrintString
	POP		SI
	CALL	OSSCR
	CALL	OSSLF
	JMP		Console_Label1
	
Console_NotEchoCommand:
	
	;Is cd command?
	MOV		DI, CDCommand
	CALL	StrCmp
	JC		Console_NotCDCommand
	CALL	CD
	JMP		Console_Label1
	
Console_NotCDCommand:

	;Is copy command?
	MOV		DI, CopyCommand
	CALL	StrCmp
	JC		Console_NotCopyCommand
	CALL	Copy
	JMP		Console_Label1
	
Console_NotCopyCommand:

	;Is cut command?
	MOV		DI, CutCommand
	CALL	StrCmp
	JC		Console_NotCutCommand
	CALL	Cut
	JMP		Console_Label1
	
Console_NotCutCommand:
	
	;Invalid command
	MOV 	SI, NotValidCommand
	CALL 	OSSPrintString
	
	JMP 	Console_Label1
	
Console_Label1:
	
	POP 	BP
	POP 	DI
	POP 	SI
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	
	RET
EndProc		Console

HelpText DB	'Format:  Command [Parameters]', 13, 10, 13, 10,	\
			'Command  Function', 13, 10,						\
			'-------  --------------------------', 13, 10,		\
			'time     Show current time', 13, 10,				\
			'cls      Clear screen', 13, 10,					\
			'ver      Show ISystem version', 13, 10,			\
			'help     Show help', 13, 10,						\
			'newfile  Create a new file', 13, 10,				\
			'delfile  Delete a file', 13, 10, 					\
			'fils     Print all files infomation', 13, 10,		\
			'exec     Execute a application', 13, 10,			\
			'reboot   Reboot computer', 13, 10,					\
			'format   Delete all file', 13, 10,					\
			'setdate  Set new date', 13, 10,					\
			'settime  Set new time', 13, 10, 					\
			'echooff  Echo off', 13, 10,						\
			'echoon   Echo on', 13, 10, 						\
			'pause    Wait user press key', 13, 10,				\
			'echo     Print text', 13, 10,						\
			'?        Print help', 13, 10, 						\
			'cd       Change current disk', 13, 10, 			\
			'copy     Copy file', 13, 10, 						\
			'cut      Cut file', 13, 10, 0
	
;Function:	Print help
Procedure	ShowHelp
	PUSHF
	PUSH 	SI
	MOV 	SI, HelpText
	CALL 	OSSPrintString
	POP 	SI
	POPF
	RET
EndProc		ShowHelp
	
ShowVersion_Msg1 DB 'ISystem Test [Version ', 0
ShowVersion_Msg2 DB ']', 13, 10, 'Ihsoh Software, 2013-2-15', 0
	
;Function:	Print kernel version
Procedure	ShowVersion
	PUSHF
	PUSH 	SI
	MOV 	SI, ShowVersion_Msg1
	CALL 	OSSPrintString
	MOV 	AX, MainVer
	CALL 	OSSPrintInteger
	MOV 	AL, '.'
	CALL 	OSSPrintChar
	MOV 	AX, Ver
	CALL 	OSSPrintInteger
	MOV 	SI, ShowVersion_Msg2
	CALL 	OSSPrintString
	CALL 	OSSCR
	CALL 	OSSLF
	POP 	SI
	POPF
	RET
EndProc		ShowVersion
	
;Function:	Show current time
Procedure	ShowCurrentTime
	PUSHF
	PUSH 	AX
	;Print year
	MOV 	AL, 32H
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	MOV 	AL, 9
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Print month
	MOV 	AL, 8
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Print day
	MOV 	AL, 7
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	
	;Print ' '
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	
	;Print hour
	MOV 	AL, 4
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Print minute
	MOV 	AL, 2
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Print second
	MOV 	AL, 0
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	OSSPrintBCD
	
	CALL 	OSSCR
	CALL 	OSSLF
	POP 	AX
	POPF
	RET
EndProc		ShowCurrentTime

;Max file name length
MaxFileNameLength 			EQU 20

;File name
NewFile_FileName: 			TIMES MaxFileNameLength DB 0

;Newfile command messages
NewFile_FileNameEmptyMsg 	DB 'File name is empty!', 13, 10, 0
NewFile_FileNameTooLongMsg 	DB 'File name is too long!', 13, 10, 0
NewFile_SuccessedMsg 		DB 'Create a file successfully!', 13, 10, 0
NewFile_FailedMsg 			DB 'Failed to create file!', 13, 10, 0
	
;Function:	Create a file
Procedure	NewFile
	PUSHF
	PUSH 	AX
	PUSH 	CX
	PUSH 	SI
	PUSH 	DI
	
	MOV		DI, NewFile_FileName
	MOV		CX, MaxFileNameLength
	XOR		AL, AL
	CLD
	REP STOSB
	
	MOV 	SI, Parameters
	;Check file name length
	CALL 	StrLen
	CMP 	CX, MaxFileNameLength
	JA 		NewFile_Label5
	CMP 	CX, 0
	JNE 	NewFile_Label6
	;File name length is 0
	MOV 	SI, NewFile_FileNameEmptyMsg
	CALL 	OSSPrintString
	POP 	DI
	POP 	SI
	POP 	CX
	POP 	AX
	POPF
	RET
	
NewFile_Label6:
	
	CLD
	MOV 	DI, NewFile_FileName
NewFile_Label1:
	LODSB
	CMP 	AL, 0
	JE 		NewFile_Label2
	STOSB
	JMP 	NewFile_Label1
NewFile_Label2:
	MOV 	SI, NewFile_FileName
	MOV		AH, 10
	INT		21H
	OR		AL, AL
	JNZ		NewFile_Label3
	MOV 	SI, NewFile_SuccessedMsg
	JMP 	NewFile_Label4
NewFile_Label3:
	MOV 	SI, NewFile_FailedMsg
	JMP 	NewFile_Label4
NewFile_Label5:
	MOV 	SI, NewFile_FileNameTooLongMsg
NewFile_Label4:
	CALL 	OSSPrintString
	
	POP 	DI
	POP 	SI
	POP 	CX
	POP 	AX
	POPF
	RET
EndProc		NewFile
	
;File name
DelFile_FileName: 			TIMES MaxFileNameLength DB 0
	
;Delfile command messages
DelFile_FileNameEmptyMsg 	DB 'File name is empty!', 13, 10, 0
DelFile_FileNameTooLongMsg 	DB 'File name is too long!', 13, 10, 0
DelFile_SuccessedMsg 		DB 'Delete a file successfully!', 13, 10, 0
DelFile_FailedMsg 			DB 'Failed to delete file!', 13, 10, 0	

;Function:	Delete a file
Procedure	DelFile
	PUSHF
	PUSH 	AX
	PUSH 	CX
	PUSH 	SI
	PUSH 	DI
	
	MOV		DI, DelFile_FileName
	MOV		CX, MaxFileNameLength
	XOR		AL, AL
	CLD
	REP	STOSB
	
	MOV 	SI, Parameters
	;Check file name length
	CALL 	StrLen
	CMP 	CX, MaxFileNameLength
	JA 		DelFile_Label5
	CMP 	CX, 0
	JNE 	DelFile_Label6
	;File name length is 0
	MOV 	SI, DelFile_FileNameEmptyMsg
	CALL 	OSSPrintString
	POP 	DI
	POP 	SI
	POP 	CX
	POP 	AX
	POPF
	RET
	
DelFile_Label6:
	
	CLD
	MOV 	DI, DelFile_FileName
DelFile_Label1:
	LODSB
	CMP 	AL, 0
	JE 		DelFile_Label2
	STOSB
	JMP 	DelFile_Label1
DelFile_Label2:
	MOV 	SI, DelFile_FileName
	MOV		AH, 11
	INT		21H
	OR		AL, AL
	JNZ		DelFile_Label3
	MOV 	SI, DelFile_SuccessedMsg
	JMP 	DelFile_Label4
DelFile_Label3:
	MOV 	SI, DelFile_FailedMsg
	JMP 	DelFile_Label4
DelFile_Label5:
	MOV 	SI, DelFile_FileNameTooLongMsg
DelFile_Label4:
	CALL 	OSSPrintString
	
	POP 	DI
	POP 	SI
	POP 	CX
	POP 	AX
	POPF
	RET
EndProc		DelFile
	
;File parameter length
FileParameterLength 	EQU 512

Files_Message1 			DB ' File(s), ', 0
Files_Message2 			DB ' Slot(s)', 13, 10, 0
Files_Message3			DB 'Name'
						TIMES 21 - ($ - Files_Message3) DB ' '
Files_Message4			DB 'Created'
						TIMES 21 - ($ - Files_Message4) DB ' '
Files_Message5			DB 'Changed'
						TIMES 20 - ($ - Files_Message5) DB ' '
						DB 'Length', 13, 10
						TIMES 20 DB '-'
						DB ' '
						TIMES 19 DB '-'
						DB ' ', ' '
						TIMES 19 DB '-'
						DB ' '
						TIMES 6 DB '-'
						DB 13, 10, 0
Files_Message6			DB 'Disk: ', 0
						
	
;Function:	Print all files infomation
;Format: 
;
;Disk: [DiskName]
;
;Name       Created       Changed       Length
;---------- ------------- ------------- ------------
;[FileName] [CreatedTime] [ChangedTime] [FileLength]
;[FileName] [CreatedTime] [ChangedTime] [FileLength]
;...
;
;         [File count] file(s), [Slot count] slot(s)
; 
Procedure	Files
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	PUSH 	DI
	PUSH	BP
	PUSH	ES
	PUSH	SI
	
	CALL	OSSCR
	CALL	OSSLF
	MOV		SI, Files_Message6
	CALL	OSSPrintString
	MOV		AL, [CS:CurrentDisk]
	CALL	FSSGetDiskName
	XCHG	AH, AL
	CALL	OSSPrintChar
	XCHG	AH, AL
	CALL	OSSPrintChar
	CALL	OSSCR
	CALL	OSSLF
	CALL	OSSCR
	CALL	OSSLF
	
	AllocS	FileParameterLength
	
	MOV		SI, Files_Message3
	CALL	OSSPrintString
	
	MOV 	AX, MaxFileCount
	PUSH	SS
	POP		ES
	MOV 	BX, BP
	MOV 	CX, FileParameterStartSector
	XOR 	DX, DX
	XOR 	DI, DI
Files_Label1:
	;If DX=10 or DX=20 or DX=30 or DX=40 then pause
	CMP 	DX, 10
	JE 		Files_Label17
	JMP 	Files_Label18
	CMP 	DX, 20
	JE 		Files_Label17
	JMP 	Files_Label18
	CMP 	DX, 30
	JE 		Files_Label17
	JMP 	Files_Label18
	CMP 	DX, 40
	JE 		Files_Label17
	JMP 	Files_Label18
	
Files_Label17:

	CMP		DX, SI
	JE		Files_Label18
	MOV		SI, DX
	CALL 	ISSPause
	
Files_Label18:

	;Read file parameter
	CALL 	FSSReadSector
	
	;Used?
	CMP 	BYTE [BP + FP_Use], 0
	JE 		Files_Label3
	
	INC 	DI
	
	PUSH 	AX
	
	;Print file name
	PUSH 	AX
	PUSH 	CX
	PUSH	BP
	MOV 	CX, MaxFileNameLength
Files_Label4:
	MOV 	AL, [BP + FP_FileName]
	CALL 	OSSPrintChar
	INC 	BP
	LOOP 	Files_Label4
	POP		BP
	POP 	CX
	POP 	AX
	
	;Print ' '
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	
	;Print created year
	MOV 	AX, [BP + FP_CreatedYear]
	CALL 	OSSPrintInteger
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Print created month
	MOV 	AL, [BP + FP_CreatedMonth]
	CMP 	AL, 10
	JAE 	Files_Label6
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label6:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Print created day
	MOV 	AL, [BP + FP_CreatedDay]
	CMP 	AL, 10
	JAE 	Files_Label7
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label7:
	XOR 	AH, AH
	CALL 	OSSPrintInteger	
	
	;Print ' '
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	
	;Print created hour
	MOV 	AL, [BP + FP_CreatedHour]
	CMP 	AL, 10
	JAE 	Files_Label8
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label8:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Print created minute
	MOV 	AL, [BP + FP_CreatedMinute]
	CMP 	AL, 10
	JAE 	Files_Label9
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label9:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Print created second
	MOV 	AL, [BP + FP_CreatedSecond]
	CMP 	AL, 10
	JAE 	Files_Label10
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label10:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print '  '
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	CALL 	OSSPrintChar
	
	;Print changed year
	MOV 	AX, [BP + FP_ChangedYear]
	CALL 	OSSPrintInteger
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Print changed month
	MOV 	AL, [BP + FP_ChangedMonth]
	CMP 	AL, 10
	JAE 	Files_Label12
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label12:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Print changed day
	MOV 	AL, [BP + FP_ChangedDay]
	CMP 	AL, 10
	JAE 	Files_Label13
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label13:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print ' '
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	
	;Print changed hour
	MOV 	AL, [BP + FP_ChangedHour]
	CMP 	AL, 10
	JAE 	Files_Label14
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label14:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Print changed minute
	MOV 	AL, [BP + FP_ChangedMinute]
	CMP 	AL, 10
	JAE 	Files_Label15
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label15:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Print changed second
	MOV 	AL, [BP + FP_ChangedSecond]
	CMP 	AL, 10
	JAE 	Files_Label16
	PUSH 	AX
	MOV 	AL, '0'
	CALL 	OSSPrintChar
	POP 	AX
Files_Label16:
	XOR 	AH, AH
	CALL 	OSSPrintInteger
	
	;Print ' '
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	
	;Print file length
	MOV 	AX, [BP + FP_Length]
	CALL 	OSSPrintInteger
	
	CALL 	OSSCR
	CALL 	OSSLF
	
	POP 	AX
	
	INC 	DX
	
Files_Label3:	
	INC 	CX
	DEC 	AX
	CMP 	AX, 0
	JE 		Files_Label2
	JMP 	Files_Label1
Files_Label2:	

	CALL 	OSSCR
	CALL 	OSSLF

	MOV 	CX, 46
Files_Label19:
	MOV 	AL, ' '
	CALL 	OSSPrintChar
	LOOP 	Files_Label19
	
	MOV 	AX, DI
	CALL 	OSSPrintInteger
	
	PUSH 	SI
	
	MOV 	SI, Files_Message1
	CALL 	OSSPrintString
	
	MOV 	AX, MaxFileCount
	SUB 	AX, DI
	CALL 	OSSPrintInteger
	
	MOV 	SI, Files_Message2
	CALL 	OSSPrintString
	
	POP 	SI
	
	FreeSKeepFlags	FileParameterLength
	
	POP		SI
	POP		ES
	POP		BP
	POP 	DI
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	RET
EndProc		Files
	
;Registers
Exec_Register 		DW 0	;Return OffAddr	+0
					DW 0	;Return SegAddr	+2
					DW 0	;Flag Register	+4
					DW 0	;AX				+6
					DW 0	;BX				+8
					DW 0	;CX				+10
					DW 0	;DX				+12
					DW 0	;SI				+14
					DW 0	;DI				+16
					DW 0	;BP				+18
					DW 0	;SP				+20
					DW 0	;DS				+22
					DW 0	;SS				+24
					DW 0	;ES				+26
					
ER_ReturnOffAddr	EQU 0
ER_ReturnSegAddr	EQU 2
ER_FlagRegister		EQU 4
ER_AX				EQU 6
ER_BX				EQU 8
ER_CX				EQU 10
ER_DX				EQU 12
ER_SI				EQU 14
ER_DI				EQU 16
ER_BP				EQU 18
ER_SP				EQU 20
ER_DS				EQU 22
ER_SS				EQU 24
ER_ES				EQU 26

;Load application to 4000H:0000H
ApplicationSegAddr 	EQU 4000H
ApplicationOffAddr 	EQU 0000H
ApplicationAddr 	DW ApplicationOffAddr, ApplicationSegAddr

Exec_ErrorMsg 		DB 'Cannot open this application!', 13, 10, 0
Exec_ErrorMsg1		DB 'Cannot run more one application!', 13, 10, 0
	
;Function:	Execute a application
Procedure	Exec
	PUSH	SI
	PUSHF
	CMP		BYTE [CS:Exec_Flag], 1
	JNE		Exec_NoRun
	MOV		SI, Exec_ErrorMsg1
	CALL	OSSPrintString
	POPF
	POP		SI
	STC
	RET
	
Exec_NoRun:
	POPF
	POP		SI

	;Safe registers
	MOV 	WORD [CS:Exec_Register + ER_ReturnOffAddr], Exec_ReturnPos
	MOV 	WORD [CS:Exec_Register + ER_ReturnSegAddr], KernelSegAddr
	PUSH 	AX
	PUSHF
	POP 	AX
	MOV 	[CS:Exec_Register + ER_FlagRegister], AX
	POP 	AX
	MOV 	[CS:Exec_Register + ER_AX], AX
	MOV 	[CS:Exec_Register + ER_BX], BX
	MOV 	[CS:Exec_Register + ER_CX], DX
	MOV 	[CS:Exec_Register + ER_DX], CX
	MOV 	[CS:Exec_Register + ER_SI], SI
	MOV 	[CS:Exec_Register + ER_DI], DI
	MOV 	[CS:Exec_Register + ER_BP], BP
	MOV 	[CS:Exec_Register + ER_SP], SP
	MOV 	[CS:Exec_Register + ER_DS], DS
	MOV 	[CS:Exec_Register + ER_SS], SS
	MOV 	[CS:Exec_Register + ER_ES], ES
	
	;加载程序到内存
	MOV		SI, Parameters
	MOV		AX, ApplicationSegAddr
	MOV		ES, AX
	MOV		DI, ApplicationOffAddr
	MOV		AH, 13
	INT		21H
	
	;是否发生错误?
	OR		AL, AL
	JZ	 	Exec_Label1
	;加载时发生错误
	MOV 	SI, Exec_ErrorMsg
	CALL 	OSSPrintString
	MOV 	SI, [CS:Exec_Register + ER_SI]	;Resume SI
	JMP		Exec_ErrEnd
	
Exec_Label1:
	
	MOV		BYTE [CS:Exec_Flag], AppRun
	
	;Execute
	JMP 	FAR [CS:ApplicationAddr]
	
Exec_ReturnPos:

	MOV		BYTE [CS:Exec_Flag], AppNoRun

Exec_ErrEnd:
	
	;Resume registers
	MOV 	AX, [CS:Exec_Register + ER_AX]
	MOV 	BX, [CS:Exec_Register + ER_BX]
	MOV 	CX, [CS:Exec_Register + ER_CX]
	MOV 	DX, [CS:Exec_Register + ER_DX]
	MOV 	SI, [CS:Exec_Register + ER_SI]
	MOV 	DI, [CS:Exec_Register + ER_DI]
	MOV 	BP, [CS:Exec_Register + ER_BP]
	MOV 	SP, [CS:Exec_Register + ER_SP]
	MOV 	DS, [CS:Exec_Register + ER_DS]
	MOV 	SS, [CS:Exec_Register + ER_SS]
	MOV 	ES, [CS:Exec_Register + ER_ES]
	PUSH 	AX
	MOV 	AX, [CS:Exec_Register + ER_FlagRegister]
	PUSH 	AX
	POPF
	POP 	AX
	
	;CRLF
	CALL 	OSSCR
	CALL 	OSSLF
	CLC
	
	RET
EndProc		Exec

;过程名:	INT21H_Func0
;功能:		默认21H中断0号功能执行过程
;参数:		无
;返回值:	无
Procedure	INT21H_Func0
	;返回系统
	JMP 	FAR [CS:Exec_Register]
	;INT21H_Func0结束
EndProc		INT21H_Func0
	
	;默认的21H中断0号功能过程的段地址和偏移地址
	INT21H_Func0Addr	DW INT21H_Func0, KernelSegAddr	
	
;Function:		Interrupt 21H
;Parameters:	AH=Function number
Procedure	INT21H
	;CLI
	CMP		AH, 0
	JNE 	INT21H_Label1
	JMP		FAR [CS:INT21H_Func0Addr]
	
INT21H_Label1:

	;SetSSSP	;切换到内核堆栈
	
	CMP 	AH, 1
	JNE 	INT21H_Label2
	;Print a char
	SetDSES
	CALL 	OSSPrintChar
	ResumeDSES
	JMP		INT21_End

INT21H_Label2:
	CMP 	AH, 2
	JNE 	INT21H_Label3
	;Carriage return
	SetDSES
	CALL 	OSSCR
	ResumeDSES
	JMP		INT21_End
	
INT21H_Label3:
	CMP 	AH, 3
	JNE 	INT21H_Label4
	;Line feed
	SetDSES
	CALL 	OSSLF
	ResumeDSES
	JMP		INT21_End
	
INT21H_Label4:
	CMP 	AH, 4
	JNE 	INT21H_Label5
	;Print string
	SetES
	CALL 	OSSPrintString
	ResumeES
	JMP		INT21_End
	
INT21H_Label5:
	CMP 	AH, 5
	JNE 	INT21H_Label6
	;Clear screen
	SetDSES
	CALL 	OSSCls
	ResumeDSES
	JMP		INT21_End
	
INT21H_Label6:
	CMP 	AH, 6
	JNE 	INT21H_Label7
	PUSH 	AX
	MOV 	AX, DX
	;Print integer
	SetDSES
	CALL 	OSSPrintInteger
	ResumeDSES
	POP 	AX
	JMP		INT21_End
	
INT21H_Label7:
	CMP 	AH, 7
	JNE 	INT21H_Label8
	;Input char
	SetDSES
	CALL 	ISSInputChar
	ResumeDSES
	JMP		INT21_End
	
INT21H_Label8:
	CMP 	AH, 8
	JNE 	INT21H_Label9
	;Input string
	SetDS
	CALL 	ISSInputString
	ResumeDS
	JMP		INT21_End
	
INT21H_Label9:
	CMP 	AH, 9
	JNE 	INT21H_Label10
	;Pause
	SetDSES
	CALL 	ISSPause
	ResumeDSES
	JMP		INT21_End
	
INT21H_Label10:
	CMP 	AH, 10
	JNE 	INT21H_Label11
	;Create a file
	SetES
	XOR 	AL, AL
	CALL 	FSSNewFile
	JNC 	INT21H_Label10_Successed
	;Failed
	MOV 	AL, 1
INT21H_Label10_Successed:
	ResumeES
	JMP		INT21_End
	
INT21H_Label11:
	CMP 	AH, 11
	JNE 	INT21H_Label12
	;Delete a file
	SetES
	XOR 	AL, AL
	CALL 	FSSDeleteFile
	JNC 	INT21H_Label11_Successed
	;Failed
	MOV 	AL, 1
INT21H_Label11_Successed:
	ResumeES
	JMP		INT21_End
	
INT21H_Label12:
	CMP 	AH, 12
	JNE 	INT21H_Label13
	;Write data to file
	XOR 	AL, AL
	CALL 	FSSWriteData
	JNC 	INT21H_Label12_Successed
	;Failed
	MOV 	AL, 1
INT21H_Label12_Successed:
	JMP		INT21_End
	
INT21H_Label13:
	CMP 	AH, 13
	JNE 	INT21H_Label14
	;Read data from file
	XOR 	AL, AL
	CALL 	FSSReadFile
	JNC 	INT21H_Label13_Successed
	;Failed
	MOV 	AL, 1
INT21H_Label13_Successed:
	JMP		INT21_End
	
INT21H_Label14:
	CMP 	AH, 14
	JNE 	INT21H_Label15
	;Check file exists
	SetES
	XOR 	AL, AL
	CALL 	FSSFileExists
	JNC 	INT21H_Label14_Successed
	;Exists
	MOV 	AL, 1
INT21H_Label14_Successed:
	ResumeES
	JMP		INT21_End
	
INT21H_Label15:
	CMP 	AH, 15
	JNE 	INT21H_Label16
	;Format file data
	SetDSES
	CALL 	FSSFormat
	ResumeDSES
	JMP		INT21_End
	
INT21H_Label16:
	CMP 	AH, 16
	JNE 	INT21_Label17
	;Print char with property
	SetDSES
	CALL 	OSSPrintCharP
	ResumeDSES
	JMP		INT21_End
	
INT21_Label17:
	CMP		AH, 17
	JNE		INT21_Label18
	;设置中断向量
	PUSH	ES
	PUSH	DI
	PUSH	AX
	PUSH	CX
	
	XOR		CX, CX
	MOV		ES, CX
	MOV		CL, 4
	MUL		CL
	MOV		DI, AX
	
	CLI
	MOV		[ES:DI], BX
	MOV		[ES:DI + 2], DS
	STI
	
	POP		CX
	POP		AX
	POP		DI
	POP		ES
	JMP		INT21_End

INT21_Label18:
	CMP		AH, 18
	JNE		INT21_Label19
	;获取中断向量
	PUSH	ES
	PUSH	DI
	PUSH	AX
	PUSH	CX
	
	XOR		CX, CX
	MOV		ES, CX
	MOV		CL, 4
	MUL		CL
	MOV		DI, AX
	
	MOV		DS, [ES:DI + 2] 
	MOV		BX, [ES:DI]
	
	POP		CX
	POP		AX
	POP		DI
	POP		ES
	JMP		INT21_End
	
INT21_Label19:
	CMP		AH, 19
	JNE		INT21_Label20
	;修改文件名
	XOR		AL, AL
	CALL	FSSRename
	JNC		INT21_Label19_Successed
	;失败
	MOV		AL, 1
INT21_Label19_Successed:
	JMP		INT21_End
	
INT21_Label20:
	CMP		AH, 20
	JNE		INT21_Label21
	;运行命令
	MOV		BYTE [CS:SystemCall], True
	PUSH	SI
	PUSH	DI
	PUSH	CX
	PUSH	AX
	SetES
	MOV		DI, InputBuffer
INT21_Label20_GetCmd:
	LODSB
	STOSB
	OR		AL, AL
	JZ		INT21_Label20_ExitLoop
	JMP		INT21_Label20_GetCmd
INT21_Label20_ExitLoop:
	MOV		SI, InputBuffer
	SetDS
	CMP		BYTE [CS:Console_Echo], False
	JE		INT21_NoEcho
	MOV		AL, '>'
	CALL	OSSPrintChar
	MOV		AL, ' '
	CALL	OSSPrintChar
	CALL	OSSPrintString
	CALL	OSSCR
	CALL	OSSLF
INT21_NoEcho:
	CALL	Console
	ResumeDS
	ResumeES
	POP		AX
	POP		CX
	POP		DI
	POP		SI
	MOV		BYTE [CS:SystemCall], False
	JMP		INT21_End
	
INT21_Label21:
	CMP		AH, 21
	JNE		INT21_Label22
	MOV		BYTE [CS:Console_Echo], False
	JMP		INT21_End
	
INT21_Label22:
	CMP		AH, 22
	JNE		INT21_Label23
	MOV		BYTE [CS:Console_Echo], True
	JMP		INT21_End
	
INT21_Label23:
	CMP		AH, 23
	JNE		INT21_Label24
	CALL	FSSGetFileLen
	JMP		INT21_End
	
INT21_Label24:
	CMP		AH, 24
	JNE		INT21_Label25
	XOR		AX, AX
	CALL	FSSGetFileChangedDate
	JNC		INT21_Labe24_Successed
	MOV		AL, 1
INT21_Labe24_Successed:
	
	JMP		INT21_End
	
INT21_Label25:
	CMP		AH, 25
	JNE		INT21_Label26
	XOR		AX, AX
	CALL	FSSGetFileCreatedDate
	JNC		INT21_Label25_Successed
	MOV		AL, 1
INT21_Label25_Successed:
	JMP		INT21_End
	
INT21_Label26:
	CMP		AH, 26
	JNE		INT21_Label27
	MOV		AH, MainVer
	MOV		AL, Ver
	JMP		INT21_End
	
INT21_Label27:
	CMP		AH, 27
	JNE		INT21_Label28
	MOV		[CS:INT21H_Func0Addr + 2], DS
	MOV		[CS:INT21H_Func0Addr], SI
	JMP		INT21_End
	
INT21_Label28:
	CMP		AH, 28
	JNE		INT21_Label29
	CLI
	MOV		WORD [CS:INT21H_Func0Addr + 2], KernelSegAddr
	MOV		WORD [CS:INT21H_Func0Addr], INT21H_Func0
	STI
	JMP		INT21_End
	
INT21_Label29:
	CMP		AH, 29
	JNE		INT21_Label30
	MOV		AH, FSMainVersion
	MOV		AL, FSVersion
	JMP		INT21_End
	
INT21_Label30:
	CMP		AH, 30
	JNE		INT21_Label31
	CALL	FSSGetFileCount
	JMP		INT21_End		
	
INT21_Label31:
	CMP		AH, 31
	JNE		INT21_Label32
	CALL	FSSGetFileNames
	JMP		INT21_End
	
INT21_Label32:
	CMP		AH, 32
	JNE		INT21_Label33
	PUSH	AX
	MOV		AL, DL
	CALL	OSSPrintHex8Bit
	POP		AX
	JMP		INT21_End
	
INT21_Label33:
	CMP		AH, 33
	JNE		INT21_Label34
	PUSH	AX
	MOV		AX, DX
	CALL	OSSPrintHex16Bit
	POP		AX
	JMP		INT21_End
	
INT21_Label34:
	CMP		AH, 34
	JNE		INT21_Label35
	MOV		AL, [CS:CurrentDisk]
	JMP		INT21_End
	
INT21_Label35:

	CMP		AH, 35
	JNE		INT21_Label36
	CALL	FSSChangeDisk
	MOV		AL, 0
	JNC		INT21_Label35_NoErr
	MOV		AL, 1
	
INT21_Label35_NoErr:
	
	JMP		INT21_End
	
INT21_Label36:

	CMP		AH, 36
	JNE		INT21_Label37
	CALL	FSSCopyFile
	MOV		AL, 0
	JNC		INT21_Label36_NoErr
	MOV		AL, 1
	
INT21_Label36_NoErr:
	
	JMP		INT21_End
	
INT21_Label37:

	CMP		AH, 37
	JNE		INT21_Label38
	CALL	FSSCutFile
	MOV		AL, 0
	JNC		INT21_Label37_NoErr
	MOV		AL, 1
	
INT21_Label37_NoErr:

	JMP		INT21_End
	
INT21_Label38:

	CMP		AH, 38
	JNE		INT21_Label39
	CALL	FSSAppendByte
	MOV		AL, 0
	JNC		INT21_Label38_NoErr
	MOV		AL, 1
	
INT21_Label38_NoErr:
	
	JMP		INT21_End
	
INT21_Label39:

	CMP		AH, 39
	JNE		INT21_Label40
	CALL	FSSGetByte
	MOV		BL, AL
	MOV		AL, 0
	JNC		INT21_Label39_NoErr
	MOV		AL, 1
	
INT21_Label39_NoErr:

	JMP		INT21_End
	
INT21_Label40:

	CMP		AH, 40
	JNE		INT21_Label41
	MOV		BX, [CS:MouseX]
	MOV		CX, [CS:MouseY]
	MOV		DL, [CS:MouseButton]
	JMP		INT21_End
	
INT21_Label41:

	CMP		AH, 41
	JNE		INT21H_Label42
	CMP		BX, 0FFFFH
	JE		INT21H_Label41_NoSetX
	MOV		[CS:MouseX], BX
	
INT21H_Label41_NoSetX:

	CMP		CX, 0FFFFH
	JE		INT21H_Label41_NoSetY
	MOV		[CS:MouseY], CX
	
INT21H_Label41_NoSetY:
	
	JMP		INT21_End

INT21H_Label42:

	CMP		AH, 42
	JNE		INT21H_Label43
	MOV		[CS:MouseState], AL
	JMP		INT21_End
	
INT21H_Label43:

	CMP		AH, 43
	JNE		INT21H_Label44
	XOR		AL, AL
	CALL	FSSGetBytes
	JNC		INT21H_Label43_NoError
	MOV		AL, 1
	
INT21H_Label43_NoError:
	
	JMP		INT21_End
	
INT21H_Label44:

	CMP		AH, 44
	JNE		INT21H_Label45
	CALL	InitDisk
	JMP		INT21_End
	
INT21H_Label45:

	CMP		AH, 45
	JNE		INT21H_Label46
	CALL	WriteDiskSector
	MOV		AL, 0
	JNC		INT21_Label45_NoError
	MOV		AL, 1
	
INT21_Label45_NoError:
	
	JMP		INT21_End
	
INT21H_Label46:

	CMP		AH, 46
	JNE		INT21H_Label47
	CALL	ReadDiskSector
	MOV		AL, 0
	JNC		INT21_Label46_NoError
	MOV		AL, 1
	
INT21_Label46_NoError:
	
	JMP		INT21_End
	
INT21H_Label47:

	CMP		AH, 47
	JNE		INT21H_Label48
	CALL	GetDiskSectorCount
	JMP		INT21_End
	
INT21H_Label48:
	
	FatalError	FEC_UnknowINT21HFuncNum
	
;中断执行结束会跳到这里, 除了功能0
INT21_End:

	;ResumeSSSP	;切换回程序堆栈
	
	IRET
EndProc		INT21H
	
RebootAddr DW 0, 0FFFFH
	
;Function:	Reboot computer
Procedure	Reboot
	MOV 	AX, 40H
	MOV 	DS, AX
	MOV 	BX, 72H
	MOV 	WORD [BX], 0
	MOV 	AX, KernelSegAddr
	MOV 	DS, AX
	JMP 	FAR [RebootAddr]
	RET
EndProc		Reboot

Format_SuccessedMsg DB 'Format successed!', 13, 10, 0
	
;Function:	Delete all file
Procedure	Format
	PUSH 	SI
	MOV		AH, 15
	INT		21H
	MOV 	SI, Format_SuccessedMsg
	CALL 	OSSPrintString
	POP 	SI
	RET
EndProc		Format
	
SetDate_Msg 			DB 'Input new date(Format: 0000/00/00): ', 0
SetDate_UnvalidDateMsg 	DB 13, 10, 'Unvalid date!', 13, 10, 0
	
;Function:	Set new date
Procedure	SetDate
	PUSHF
	PUSH 	AX
	PUSH 	CX
	PUSH 	DX
	PUSH 	SI
	MOV 	SI, SetDate_Msg
	CALL 	OSSPrintString
	XOR 	CX, CX
	XOR 	DX, DX
	;Get year
	;Century
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	MOV 	CH, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	AND 	AL, 0FH
	OR 		CH, AL
	;Year
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	MOV 	CL, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	AND 	AL, 0FH
	OR 		CL, AL
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Get month
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	MOV 	DH, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	AND 	AL, 0FH
	OR 		DH, AL
	
	;Print '/'
	MOV 	AL, '/'
	CALL 	OSSPrintChar
	
	;Get month
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	MOV 	DL, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetDate_Label1
	SUB 	AL, '0'
	AND 	AL, 0FH
	OR 		DL, AL
	
	;Set new date
	MOV 	AH, 5H
	INT 	1AH
	
	CALL 	OSSCR
	CALL 	OSSLF
	
	JMP 	SetDate_Label2
SetDate_Label1:
	;Unvalid date
	MOV 	SI, SetDate_UnvalidDateMsg
	CALL 	OSSPrintString
	
SetDate_Label2:
	
	POP 	SI
	POP 	DX
	POP 	CX
	POP 	AX
	POPF
	RET
EndProc		SetDate
	
SetTime_Msg 			DB 'Input new time(Format: 00:00:00): ', 0
SetTime_UnvalidTimeMsg 	DB 13, 10, 'Unvalid time!', 13, 10, 0
	
;Function	Set new time
Procedure	SetTime
	PUSHF
	PUSH 	CX
	PUSH 	DX
	MOV 	SI, SetTime_Msg
	CALL 	OSSPrintString
	XOR 	CX, CX
	XOR 	DX, DX
	;Get hour
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetTime_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	OR 		CH, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetTime_Label1
	SUB 	AL, '0'
	OR 		CH, AL
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Get minute
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetTime_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	OR 		CL, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetTime_Label1
	SUB 	AL, '0'
	OR 		CL, AL
	
	;Print ':'
	MOV 	AL, ':'
	CALL 	OSSPrintChar
	
	;Get second
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetTime_Label1
	SUB 	AL, '0'
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	SHL 	AL, 1
	OR 		DH, AL
	CALL 	ISSInputChar
	CALL 	IsDigitChar
	JC 		SetTime_Label1
	SUB 	AL, '0'
	OR 		DH, AL
	
	;Standard time
	MOV 	DL, 0
	
	;Set time
	MOV 	AH, 3H
	INT 	1AH
	
	CALL 	OSSCR
	CALL 	OSSLF
	
	JMP 	SetTime_Label2
SetTime_Label1:
	;Unvalid time
	MOV 	SI, SetTime_UnvalidTimeMsg
	CALL 	OSSPrintString
	
SetTime_Label2:
	
	POP 	DX
	POP 	CX
	POPF
	RET
EndProc		SetTime
	
Rename_SrcFileName:		TIMES 20 DB 0	;保存rename命令参数的原文件名
Rename_DstFileName:		TIMES 20 DB 0	;保存rename命令参数的目标文件名
Rename_FailedMsg		DB 'Failed to rename!', 13, 10, 0
Rename_SuccessedMsg		DB 'Rename successfully!', 13, 10, 0
	
;过程名:	Rename
Procedure	Rename
	PUSH	AX
	PUSH	CX
	PUSH	SI
	PUSH	DI
	
	MOV		DI, Rename_SrcFileName
	MOV		CX, MaxFileNameLength
	XOR		AL, AL
	CLD
	REP	STOSB
	MOV		DI, Rename_DstFileName
	MOV		CX, MaxFileNameLength
	REP STOSB
	
	MOV		SI, Parameters
	MOV		DI, Rename_SrcFileName
	MOV		CX, MaxParametersLength
Rename_GetSrc_DstFileName:
	LODSB
	OR		AL, AL
	JE		Rename_GetSrc_DstFileNameEnd
	CMP		AL, ' '
	JNE		Rename_GetSrc_DstFileNameNext
	MOV		DI, Rename_DstFileName
	JMP		Rename_GetSrc_DstFileName
Rename_GetSrc_DstFileNameNext:
	STOSB
	LOOP	Rename_GetSrc_DstFileName
Rename_GetSrc_DstFileNameEnd:	
	MOV		SI, Rename_SrcFileName
	MOV		DI, Rename_DstFileName
	MOV		AH, 19
	INT		21H
	OR		AL, AL
	MOV		SI, Rename_SuccessedMsg
	JE		Rename_Successed
	MOV		SI, Rename_FailedMsg
Rename_Successed:
	CALL	OSSPrintString
	POP		DI
	POP		SI
	POP		CX
	POP		AX
	RET
EndProc		Rename

CD_FailedMsg	DB 'Fail to change disk!', 13, 10, 0

;过程名:	CD
;功能:		实现切换当前使用磁盘
Procedure	CD
	PUSH	AX
	MOV		AL, [CS:Parameters + 1]
	MOV		AH, [CS:Parameters + 0]
	CALL	FSSGetDiskNumber
	CALL	FSSChangeDisk
	JNC		CD_NoErr
	PUSH	SI
	MOV		SI, CD_FailedMsg
	MOV		AH, 4
	INT		21H
	POP		SI
	
CD_NoErr:
	
	POP		AX
	RET
EndProc		CD

Copy_Src:			TIMES MaxFileNameLength DB 0
Copy_Dst:			TIMES 23 DB 0
Copy_DstFileName:	TIMES MaxFileNameLength DB 0
Copy_FailedMsg		DB 'Failed to copy!', 13, 10, 0
Copy_SuccessedMsg	DB 'copy successfully!', 13, 10, 0

;过程名:	Copy
;功能:		复制文件
Procedure	Copy
	PUSHF
	PUSH	AX
	PUSH	CX
	PUSH	SI
	PUSH	DI
	MOV		AL, 0
	CLD
	MOV		DI, Copy_Src
	MOV		CX, MaxFileNameLength
	REP STOSB
	MOV		DI, Copy_Dst
	MOV		CX, 23
	REP STOSB
	MOV		DI, Copy_DstFileName
	MOV		CX, MaxFileNameLength
	REP STOSB
	
	MOV		SI, Parameters
	MOV		DI, Copy_Src
	
Copy_GetSrc:
	
	LODSB
	CMP		AL, ' '
	JE		Copy_Label1
	STOSB
	JMP		Copy_GetSrc
	
Copy_Label1:
	
	MOV		DI, Copy_Dst
	
Copy_GetDst:

	LODSB
	CMP		AL, 0
	JE		Copy_Label2
	STOSB
	JMP		Copy_GetDst
	
Copy_Label2:
	
	MOV		SI, Copy_Dst
	MOV		DI, Copy_DstFileName
	CALL	FSSParsePath
	MOV		SI, Copy_Src
	MOV		DI, Copy_DstFileName
	CALL	FSSCopyFile
	MOV		SI, Copy_SuccessedMsg
	JNC		Copy_NoErr
	MOV		SI, Copy_FailedMsg
	
Copy_NoErr:
	
	MOV		AH, 4
	INT		21H
	
	POP		DI
	POP		SI
	POP		CX
	POP		AX
	POPF
	RET
EndProc		Copy

Cut_Src:			TIMES MaxFileNameLength DB 0
Cut_Dst:			TIMES 23 DB 0
Cut_DstFileName:	TIMES MaxFileNameLength DB 0
Cut_FailedMsg		DB 'Failed to cut!', 13, 10, 0
Cut_SuccessedMsg	DB 'cut successfully!', 13, 10, 0

;过程名:	Cut
;功能:		剪切文件
Procedure	Cut
	PUSHF
	PUSH	AX
	PUSH	CX
	PUSH	SI
	PUSH	DI
	MOV		AL, 0
	CLD
	MOV		DI, Cut_Src
	MOV		CX, MaxFileNameLength
	REP STOSB
	MOV		DI, Cut_Dst
	MOV		CX, 23
	REP STOSB
	MOV		DI, Cut_DstFileName
	MOV		CX, MaxFileNameLength
	REP STOSB
	
	MOV		SI, Parameters
	MOV		DI, Cut_Src
	
Cut_GetSrc:
	
	LODSB
	CMP		AL, ' '
	JE		Cut_Label1
	STOSB
	JMP		Cut_GetSrc
	
Cut_Label1:
	
	MOV		DI, Cut_Dst
	
Cut_GetDst:

	LODSB
	CMP		AL, 0
	JE		Cut_Label2
	STOSB
	JMP		Cut_GetDst
	
Cut_Label2:
	
	MOV		SI, Cut_Dst
	MOV		DI, Cut_DstFileName
	CALL	FSSParsePath
	MOV		SI, Cut_Src
	MOV		DI, Cut_DstFileName
	CALL	FSSCutFile
	MOV		SI, Cut_SuccessedMsg
	JNC		Cut_NoErr
	MOV		SI, Cut_FailedMsg
	
Cut_NoErr:
	
	MOV		AH, 4
	INT		21H
	
	POP		DI
	POP		SI
	POP		CX
	POP		AX
	POPF
	RET
EndProc		Cut

%IFDEF DEBUG
	
	Cmd		DB 'newfile A', 0
	
;Function:	This procedure is test procedure
Procedure	_Test
	MOV		SI, Cmd
	CALL	Console
	;MOV		AH, 20
	;INT		21H
	
	RET
EndProc		_Test
%ENDIF
	
;*--------------------------------------------------------------------------*
;|文件系统																	|
;|																			|
;|版本:	0.0																	|
;|																			|
;|描述:																		|
;|	磁盘类型只能为1.44M的软盘. 只能储存50个文件, 每个文件的最大长度为27KB.	|
;|	文件系统对软盘逻辑结构的Data部分进行进行控制.							|
;|																			|
;|软盘逻辑结构:																|
;|																			|
;|	+---------------------------+											|
;|	|	   		Boot 			|	0										|
;|	+---------------------------+											|
;|	|	  	   Kernel			|	1 ~ 64									|
;|	+---------------------------+											|
;|	|	 	  Reserved			|	65 ~ 128								|
;|	+---------------------------+											|
;|	|	  System parameters		|	129										|
;|	+---------------------------+											|
;|	|			Data			|	130 ~ 2879								|
;|	+---------------------------+											|
;|																			|
;|	其中Data被分为文件参数区域(130~179)和文件数据区域(180~2879). 			|
;|																			|
;|文件参数结构:																|
;|	每个文件参数占据一个扇区(512字节)										|
;|																			|
;|	+---------------------------+											|
;|	|		  File name			|	0 ~ 19									|
;|	+---------------------------+											|
;|	|		 Created year		|	20 ~ 21									|
;|	+---------------------------+											|
;|	|		 Created month		|	22										|
;|	+---------------------------+											|
;|	|		 Created day		|	23										|
;|	+---------------------------+											|
;|	|		 Created hour		|	24										|
;|	+---------------------------+											|
;|	|		 Created minute		|	25										|
;|	+---------------------------+											|
;|	|		 Created second		|	26										|
;|	+---------------------------+											|
;|	|		 Changed year		|	27										|
;|	+---------------------------+											|
;|	|		 Changed month		|	29										|
;|	+---------------------------+											|
;|	|		 Changed day		|	30										|
;|	+---------------------------+											|
;|	|		 Changed hour		|	31										|
;|	+---------------------------+											|
;|	|		 Changed minute		|	32										|
;|	+---------------------------+											|
;|	|		 Changed second		|	33										|
;|	+---------------------------+											|
;|	|		    Length			|	34 ~ 35									|
;|	+---------------------------+											|
;|	|			 Use			|	36										|
;|	+---------------------------+											|
;|	|			Reserve			|	37 ~ 511								|
;|	+---------------------------+											|
;|																			|
;*--------------------------------------------------------------------------*

;当前磁盘
CurrentDisk					DB 0

;版本
FSMainVersion				EQU 0
FSVersion					EQU 0

;Driver
Driver 						EQU 0	;Floppy

;1.44MB Floppy head, cly, sector, sector byte count
MaxHead 					EQU 2
MaxCly 						EQU 80
MaxSector 					EQU 18
SectorByteCount 			EQU 512

;Max file count
MaxFileCount 				EQU 50

;File parameter start sector
FileParameterStartSector 	EQU 130

;File data start sector
FileDataStartSector 		EQU 180

;File data max sector
FileDataMaxSector 			EQU 54

;最大文件长度
MaxFileLength				EQU 27648

;File parameter member
FP_FileName 				EQU 0
FP_CreatedYear 				EQU 20
FP_CreatedMonth 			EQU 22
FP_CreatedDay 				EQU 23
FP_CreatedHour 				EQU 24
FP_CreatedMinute 			EQU 25
FP_CreatedSecond 			EQU 26
FP_ChangedYear 				EQU 27
FP_ChangedMonth 			EQU 29
FP_ChangedDay 				EQU 30
FP_ChangedHour 				EQU 31
FP_ChangedMinute 			EQU 32
FP_ChangedSecond 			EQU 33
FP_Length 					EQU 34
FP_Use 						EQU 36
FP_Reserve 					EQU 37

DiskNames:
	;	驱动器号	盘符
	DB	0, 			'FA'
	DB	1, 			'FB'
	DB	2, 			'FC'
	DB	3,			'FD'
	DB	4,			'FE'
	DB	5,			'FF'
	DB	6,			'FG'
	DB	7,			'FH'
	DB	8,			'FI'
	DB	9,			'FJ'
	DB	10,			'FK'
	DB	11,			'FL'
	DB	12,			'FM'
	DB	13,			'FN'
	DB	14,			'FO'
	DB	15,			'FP'
	DB	16,			'FQ'
	DB	17,			'FR'
	DB	18,			'FS'
	DB	19,			'FT'
	DB	20,			'FU'
	DB	21,			'FV'
	DB	22,			'FW'
	DB	23,			'FX'
	DB	24,			'FY'
	DB	25,			'FZ'
	
	DB	64,			'DA'
	DB	65,			'DB'
	
	DiskNameCount	EQU	($ - DiskNames) / 3
	
;过程名:	FSSGetDiskName
;功能:		获取盘符
;参数:		AL=驱动器号
;返回值:	AL=盘符第二个字节
;			AH=盘符第一个字节
;			当AX=0时则驱动器号错误
Procedure	FSSGetDiskName
	PUSHF
	PUSH	DS
	PUSH	SI
	PUSH	BX
	PUSH	CX
	PUSH	CS
	POP		DS
	MOV		SI, DiskNames
	MOV		CX, DiskNameCount
	MOV		BH, AL
	CLD
	
FSSGetDiskName_Label1:
	
	LODSB
	MOV		BL, AL
	LODSB
	MOV		AH, AL
	LODSB
	CMP		BH, BL
	JE		FSSGetDiskName_Found
	LOOP	FSSGetDiskName_Label1
	XOR		AX, AX
	
FSSGetDiskName_Found:
	
	POP		CX
	POP		BX
	POP		SI
	POP		DS
	POPF
	RET
EndProc		FSSGetDiskName

;过程名:	FSSGetDiskNumber
;功能:		获取盘符对应的驱动器号
;参数:		AH=盘符第一个字节
;			AL=盘符第二个字节
;返回值:	AL=驱动器号
Procedure	FSSGetDiskNumber
	PUSHF
	PUSH	DS
	PUSH	SI
	PUSH	CX
	PUSH	CS
	POP		DS
	MOV		SI, DiskNames
	MOV		CX, DiskNameCount
	
FSSGetDiskNumber_Label1:

	CMP		[SI + 1], AH
	JNE		FSSGetDiskNumber_NotFound
	CMP		[SI + 2], AL
	JNE		FSSGetDiskNumber_NotFound
	MOV		AL, [SI]
	JMP		FSSGetDiskNumber_Found
	
FSSGetDiskNumber_NotFound:
	
	ADD		SI, 3
	LOOP	FSSGetDiskNumber_Label1

FSSGetDiskNumber_Found:
	
	POP		CX
	POP		SI
	POP		DS
	POPF
	RET
EndProc		FSSGetDiskNumber

;过程名:	FSSChangeDisk
;功能:		改变驱动器
;参数:		AL=驱动器号
;返回值:	CF=0成功, 否则失败
Procedure	FSSChangeDisk
	CMP		AL, 64
	JE		FSSChangeDisk_HDisk
	CMP		AL, 65
	JE		FSSChangeDisk_HDisk
	
	PUSH	AX
	PUSH	DX
	MOV		DL, AL
	MOV		AH, 0
	INT		13H
	OR		AH, AH
	STC
	JNZ		FSSChangeDisk_Err
	MOV		[CS:CurrentDisk], AL
	CLC
	
FSSChangeDisk_Err:
	
	POP		DX
	POP		AX
	JMP		FSSChangeDisk_End
	
FSSChangeDisk_HDisk:

	MOV		[CS:CurrentDisk], AL
	CLC
	
FSSChangeDisk_End:
	
	RET
EndProc		FSSChangeDisk

FSSConvert_DL	DB ?

;Function:		Convert address to head, cly, sector
;Parameters:	CX=Address
;Returns:		DH=Head, BH=Cly, BL=Sector		
Procedure	FSSConvert
	PUSHF
	PUSH	AX
	PUSH 	CX
	
	MOV		[CS:FSSConvert_DL], DL
	
	;计算扇区号
	MOV		AX, CX
	XOR		DX, DX
	MOV		BX, MaxSector
	DIV		BX
	MOV		BL, DL
	INC		BL
	
	MOV		CX, AX
	
	;计算磁头号和磁道号
	MOV		AX, CX
	XOR		DX, DX
	MOV		CX, MaxHead
	DIV		CX
	MOV		BH, AL
	MOV		DH, DL
	
	MOV		DL, [CS:FSSConvert_DL]
	
	POP 	CX
	POP		AX
	POPF
	RET
EndProc		FSSConvert

;Function:		Read a sector to memory
;Parameters:	CX=Address
;Returns:		ES:BX=Dest SegAddr:OffAddr
Procedure	FSSReadSector

	;检测当前盘是硬盘还是软盘
	CMP		BYTE [CS:CurrentDisk], 64
	JE		FSSReadSector_HDisk
	CMP		BYTE [CS:CurrentDisk], 65
	JE		FSSReadSector_HDisk
	
	;当前盘为软盘
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	;Init driver
	;MOV 	AL, Driver
	;MOV 	AH, 0
	;INT 	13H
	
	;Safe OffAddr!!!
	PUSH 	BX
	
	;Convert address
	;DH=Head, BH=Cly, BL=Sector
	;This action will change bx!!!
	CALL 	FSSConvert
	
	;Read sector to memory
	MOV 	AL, 1	;Read one sector
	MOV 	CH, BH
	MOV 	CL, BL 
	
	;Safe OffAddr!!!
	POP 	BX
	
	MOV		DL, [CS:CurrentDisk]
	;MOV 	DL, Driver
	MOV 	AH, 2H
	INT 	13H
	
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	JMP		FSSReadSector_End
	
FSSReadSector_HDisk:
	
	;硬盘
	PUSHF
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV		DI, BX
	MOV		AL, [CS:CurrentDisk]
	SUB		AL, 64
	XOR		BL, BL
	MOV		DX, CX
	XOR		CX, CX
	CALL	ReadDiskSector
	POP		AX
	POP		BX
	POP		CX
	POP		DX
	POP		DI
	POPF
	
FSSReadSector_End:
	
	RET
EndProc		FSSReadSector
	
;Function:		Write 512bytes data to sector
;Parameters:	CX=Address, ES:BX=Data SegAddr:OffAddr
Procedure	FSSWriteSector

	;检测当前盘是硬盘还是软盘
	CMP		BYTE [CS:CurrentDisk], 64
	JE		FSSWriteSector_HDisk
	CMP		BYTE [CS:CurrentDisk], 65
	JE		FSSWriteSector_HDisk

	;软盘
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	;Init driver
	;MOV 	AL, Driver
	;MOV 	AH, 0
	;INT 	13H
	
	;Safe OffAddr!!!
	PUSH 	BX
	
	;Convert address
	;DH=Head, BH=Cly, BL=Sector
	;This action will change bx!!!
	CALL 	FSSConvert
	MOV 	AL, 1	;Read one sector
	MOV 	CH, BH
	MOV 	CL, BL
	
	;Safe OffAddr!!!
	POP 	BX
	
	MOV		DL, [CS:CurrentDisk]
	;MOV 	DL, Driver
	MOV 	AH, 3H
	INT 	13H
	
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	JMP		FSSWriteSector_End
	
FSSWriteSector_HDisk:
	
	;硬盘
	PUSHF
	PUSH	SI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	PUSH	DS
	PUSH	ES
	POP		DS
	MOV		SI, BX
	MOV		AL, [CS:CurrentDisk]
	SUB		AL, 64
	XOR		BL, BL
	MOV		DX, CX
	XOR		CX, CX
	CALL	WriteDiskSector
	POP		DS
	POP		AX
	POP		BX
	POP		CX
	POP		DX
	POP		SI
	POPF
	
	FSSWriteSector_End:
	
	RET
EndProc		FSSWriteSector
	
;Function:		Read some sectors to memory
;Parameters:	CX=Address, AX=Count
;Returns:		If CF=0 then successed else failed. ES:BX=Dest SegAddr:OffAddr
Procedure	FSSReadSectors
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
FSSReadSectors_Label1:
	CMP 	AX, 0
	JE 		FSSReadSectors_Label2
	CALL 	FSSReadSector
	INC 	CX
	ADD 	BX, SectorByteCount
	DEC 	AX
	JMP 	FSSReadSectors_Label1
FSSReadSectors_Label2:
	POP 	CX
	POP 	BX
	POP 	AX
	RET
EndProc		FSSReadSectors
	
;Function:		Write some data to disk
;Parameters:	CX=Address, AX=Count, ES:BX=Data SegAddr:OffAddr
Procedure	FSSWriteSectors
	PUSHF
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
FSSWriteSectors_Label1:
	CMP 	AX, 0
	JE 		FSSWriteSectors_Label2
	CALL 	FSSWriteSector
	INC 	CX
	ADD 	BX, SectorByteCount
	DEC 	AX
	JMP 	FSSWriteSectors_Label1
FSSWriteSectors_Label2:
	POP 	CX
	POP 	BX
	POP 	AX
	POPF
	RET
EndProc		FSSWriteSectors
	
;Function:	Get a space
;Returns:	if CF=0, Successed else Failed. AX=File number
Procedure	FSSGetSpace
	PUSH	ES
	PUSH 	BX
	PUSH	BP
	
	;在堆栈上分配空间用于储存文件参数
	AllocS	FileParameterLength
	
	MOV 	AX, MaxFileCount
	MOV 	CX, MaxFileCount
FSSGetSpace_Label1:
	DEC 	AX
	PUSH 	CX
	;Read file parameter
	ADD 	CX, FileParameterStartSector - 1
	MOV 	BX, BP
	PUSH	SS
	POP		ES
	CALL 	FSSReadSector
	
	;Is used or not used?
	CMP 	BYTE [BP + FP_Use], 0
	JNE 	FSSGetSpace_Label3
	;Not used
	JMP 	FSSGetSpace_Label2
	
FSSGetSpace_Label3:
	POP 	CX
	LOOP 	FSSGetSpace_Label1
	;No space
	JMP 	FSSGetSpace_Label4

FSSGetSpace_Label2:
	CLC
	POP 	CX
	JMP 	FSSGetSpace_Label5
FSSGetSpace_Label4:
	STC
FSSGetSpace_Label5:

	;释放内存
	FreeSKeepFlags	FileParameterLength

	POP		BP
	POP 	BX
	POP		ES
	RET
EndProc		FSSGetSpace
	
;Function:		Check file name is exists?
;Parameters:	DS:SI=File name(Length >= 20bytes!!!) SegAddr:OffAddr
;Returns:		if CF=0 then not exists else exists
Procedure	FSSFileExists
	PUSH	ES
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH	BP
	
	;在堆栈上分配空间用于储存文件参数
	AllocS	FileParameterLength
	
	;Get file parameter first sector
	MOV 	CX, FileParameterStartSector
	
	MOV 	AX, MaxFileCount
	PUSH	SS
	POP		ES
FSSFileExists_Label1:
	;Read file parameter
	MOV 	BX, BP
	CALL 	FSSReadSector
	
	;Used?
	CMP 	BYTE [BP + FP_Use], 1
	JNE 	FSSFileExists_Label6

	;Compare file name
	PUSH 	AX
	PUSH 	CX
	PUSH 	SI
	PUSH	BP
	
	MOV 	CX, MaxFileNameLength
FSSFileExists_Label4:
	MOV 	AL, [BP + FP_FileName]
	CMP 	AL, [SI]
	JNE 	FSSFileExists_Label5
	INC 	BP
	INC 	SI
	LOOP 	FSSFileExists_Label4
	;Same
	JMP 	FSSFileExists_Label2
	
	FSSFileExists_Label5:
	;Not same
	
	POP		BP
	POP 	SI
	POP 	CX
	POP 	AX

FSSFileExists_Label6:	;This space not used
	
	INC 	CX	;Next file parameter position
	DEC 	AX
	CMP 	AX, 0
	JNE 	FSSFileExists_Label1
	;Not exists
	CLC
	
	JMP 	FSSFileExists_Label3
FSSFileExists_Label2:
	;Exists
	
	POP		BP
	POP 	SI
	POP 	CX
	POP 	AX
	STC
	
FSSFileExists_Label3:

	;释放内存
	FreeSKeepFlags	FileParameterLength

	POP		BP
	POP 	CX
	POP 	BX
	POP 	AX
	POP		ES
	RET
EndProc		FSSFileExists

;Function:		Create a new file
;Parameters:	DS:SI=File name(Length >= 20bytes!!!) SegAddr:OffAddr
;Returns:		if CF=0, Successed else failed
Procedure	FSSNewFile
	;Exists same name file?
	CALL 	FSSFileExists
	JNC 	FSSNewFile_Lable3
	;Exists
	STC
	RET
	
FSSNewFile_Lable3:

	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	SI
	PUSH	BP
	
	;Exists space?
	CALL 	FSSGetSpace
	JC 		FSSNewFile_Label2
	
	;在堆栈上分配空间用于储存文件参数
	AllocS	FileParameterLength
	
	PUSH 	AX
	
	;Copy file name
	PUSH	BP
	MOV 	CX, MaxFileNameLength
FSSNewFile_Label1:
	MOV 	AL, [SI]
	MOV 	[BP + FP_FileName], AL
	INC 	BP
	INC 	SI
	LOOP 	FSSNewFile_Label1
	POP		BP
	
	;Set created file time and changed file time
	;Year
	MOV 	AL, 9
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	BCDToInteger
	MOV 	[BP + FP_CreatedYear], AL
	MOV 	[BP + FP_ChangedYear], AL
	MOV 	BYTE [BP + FP_CreatedYear + 1], 0
	MOV 	BYTE [BP + FP_ChangedYear + 1], 0
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	MOV 	AH, 4H
	INT 	1AH
	MOV 	AL, CH
	CALL 	BCDToInteger
	MOV 	BL, 100
	MUL 	BL
	ADD 	WORD [BP + FP_CreatedYear], AX
	ADD 	WORD [BP + FP_ChangedYear], AX
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	
	;Month
	MOV 	AL, 8
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	BCDToInteger
	MOV 	[BP + FP_CreatedMonth], AL
	MOV 	[BP + FP_ChangedMonth], AL
	
	;Day
	MOV 	AL, 7
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	BCDToInteger
	MOV 	[BP + FP_CreatedDay], AL
	MOV 	[BP + FP_ChangedDay], AL
	
	;Hour
	MOV 	AL, 4
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	BCDToInteger
	MOV 	[BP + FP_CreatedHour], AL
	MOV 	[BP + FP_ChangedHour], AL
	
	;Minute
	MOV 	AL, 2
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	BCDToInteger
	MOV 	[BP + FP_CreatedMinute], AL
	MOV 	[BP + FP_ChangedMinute], AL
	
	;Second
	MOV 	AL, 0
	OUT 	70H, AL
	IN 		AL, 71H
	CALL 	BCDToInteger
	MOV 	[BP + FP_CreatedSecond], AL
	MOV 	[BP + FP_ChangedSecond], AL
	
	;Set length
	MOV 	BYTE [BP + FP_Length], 0
	
	;Set use flag
	MOV 	BYTE [BP + FP_Use], 1
	
	;Save parameter
	POP 	CX
	PUSH	ES
	PUSH	SS
	POP		ES
	ADD 	CX, FileParameterStartSector
	MOV 	BX, BP
	CALL 	FSSWriteSector
	POP		ES
	
FSSNewFile_Label2:
	;Not exists space
	
	;释放内存
	FreeSKeepFlags	FileParameterLength
	
	POP		BP
	POP 	SI
	POP 	CX
	POP 	BX
	POP 	AX
	RET
EndProc		FSSNewFile
	
;Function:		Delete a file
;Parameters:	DS:SI=File name(Length >= 20bytes!!!) SegAddr:OffAddr
;Returns:		if CF=0, Successed else Failed
Procedure	FSSDeleteFile
	;Check file exists
	CALL 	FSSFileExists
	JC 		FSSDeleteFile_Label1
	;File not exists
	STC
	RET
	
FSSDeleteFile_Label1:
	;File exists
	
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH	ES
	PUSH	BP
	
	;在堆栈上分配空间用于储存文件参数
	AllocS	FileParameterLength
	
	PUSH	SS
	POP		ES
	
	MOV 	CX, FileParameterStartSector
	MOV 	AX, MaxFileCount
	MOV 	BX, BP
FSSDeleteFile_Label2:
	;Read file parameter
	CALL 	FSSReadSector
	
	;Compare file name
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	SI
	PUSH	BP
	
	MOV 	CX, MaxFileNameLength
FSSDeleteFile_Label4:
	MOV 	AL, [BP + FP_FileName]
	CMP 	AL, [SI]
	JE 		FSSDeleteFile_Label5
	
	;文件名不匹配
	POP		BP
	POP 	SI
	POP 	CX
	POP 	BX
	POP 	AX
	
	JMP 	FSSDeleteFile_Label6
	
FSSDeleteFile_Label5:
	INC 	BP
	INC 	SI
	LOOP 	FSSDeleteFile_Label4
	
	;文件名匹配
	POP		BP
	POP 	SI
	POP 	CX
	POP 	BX
	POP 	AX
	JMP 	FSSDeleteFile_Label3
	
FSSDeleteFile_Label6:
	INC 	CX	;Next file parameter sector
	DEC 	AX
	CMP 	AX, 0
	JE 		FSSDeleteFile_Label3
	JMP 	FSSDeleteFile_Label2
FSSDeleteFile_Label3:
	;Found
	CLC
	MOV 	BYTE [BP + FP_Use], 0
	CALL 	FSSWriteSector
	
	;释放内存
	FreeSKeepFlags	FileParameterLength
	
	POP		BP
	POP		ES
	POP 	CX
	POP 	BX
	POP 	AX
	RET
EndProc		FSSDeleteFile
	
;Function:		Get file number
;Parameter:		DS:SI=File name(Length >= 20bytes!!!) SegAddr:OffAddr
;Returns:		If CF=0 then Successed else failed. DX=File number
Procedure	FSSGetFileNumber
	PUSH 	SI
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	ES
	PUSH	BP
	
	;在堆栈上分配空间用于储存文件参数
	AllocS	FileParameterLength
	
	PUSH 	SS
	POP 	ES
	XOR 	DX, DX
	MOV 	BX, BP
	MOV 	CX, FileParameterStartSector
	MOV 	AX, MaxFileCount
FSSGetFileNumber_Label1:
	;Read file parameter
	CALL 	FSSReadSector
	
	;Is used?
	CMP 	BYTE [BP + FP_Use], 0
	JE 		FSSGetFileNumber_Label5
	
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	SI
	PUSH	BP
	
	MOV 	CX, MaxFileNameLength
FSSGetFileNumber_Label3:
	MOV 	AL, [BP + FP_FileName]
	
	CMP 	AL, [SI]
	JNE 	FSSGetFileNumber_Label4
	INC 	BP
	INC 	SI
	LOOP 	FSSGetFileNumber_Label3
	;Equal
	CLC
	POP		BP
	POP 	SI
	POP 	CX
	POP 	BX
	POP 	AX
	
	JMP		FSSGetFileNumber_End
	
FSSGetFileNumber_Label4:

	POP		BP
	POP 	SI
	POP 	CX
	POP 	BX
	POP 	AX

FSSGetFileNumber_Label5:
	
	INC 	CX
	INC 	DX
	DEC 	AX
	CMP 	AX, 0
	JE 		FSSGetFileNumber_Label2
	JMP 	FSSGetFileNumber_Label1
FSSGetFileNumber_Label2:
	;Not exists
	STC

FSSGetFileNumber_End:

	;释放内存
	FreeSKeepFlags	FileParameterLength

	POP		BP
	POP 	ES
	POP 	CX
	POP 	BX
	POP 	AX
	POP 	SI
	RET
EndProc		FSSGetFileNumber
	
FSSWriteData_FP: 				TIMES SectorByteCount DB 0
	
;Function:		Write data to file
;Parameters:	DS:SI=File name(Length >= 20bytes!!!) SegAddr:OffAddr, 
;				ES:DI=Data SegAddr:OffAddr,
;				CX=Data length
;Returns:		If CF=0 then successed else failed
Procedure	FSSWriteData
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	PUSH 	DX
	PUSH 	DI
	PUSH 	DS
	PUSH 	ES
	PUSH 	SI
	PUSH	BP
	CALL 	FSSGetFileNumber
	JNC 	FSSWriteData_Label1
	;Error
	POP		BP
	POP 	SI
	POP 	ES
	POP 	DS
	POP 	DI
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	RET
	
FSSWriteData_Label1:
	;在堆栈上申请一块内存用作文件缓冲区
	AllocS	MaxFileLength

	PUSH 	CX	;Safe data length!!!
	
	;Copy data to file buffer
	MOV 	AX, SS	;文件缓冲区在堆栈上
	MOV 	DS, AX
	MOV 	BX, BP	;刚才在堆栈申请的内存的偏移地址保存在BP
	OR		CX, CX
	JZ		FSSWriteData_NoData
	
FSSWriteData_Label2:

	MOV 	AL, [ES:DI]
	MOV 	[BX], AL
	INC 	DI
	INC 	BX
	LOOP 	FSSWriteData_Label2
	
FSSWriteData_NoData:
	
	;Store file buffer to disk
	PUSH 	DX	;Safe file number!!!
	MOV 	AX, SS	;文件缓冲区在堆栈上
	MOV 	ES, AX
	MOV 	BX, BP	;刚才在堆栈申请的内存的偏移地址保存在BP
	MOV 	AX, DX
	MOV 	DX, FileDataMaxSector
	MUL 	DX
	ADD 	AX, FileDataStartSector
	MOV 	CX, AX
	MOV 	AX, FileDataMaxSector
	CALL 	FSSWriteSectors
	POP 	DX
	
	;Read file parameter
	MOV 	AX, KernelSegAddr
	MOV 	ES, AX
	MOV 	BX, FSSWriteData_FP
	MOV 	CX, FileParameterStartSector
	ADD 	CX, DX
	MOV 	SI, CX	;Safe sector address!!!
	CALL 	FSSReadSector
	
	;Set file parameter - Length
	POP 	CX
	MOV 	[ES:BX + FP_Length], CX
	
	;Set file parameter - Changed Data/Time
	MOV 	AH, 2H
	INT 	1AH
	;Second
	MOV 	AL, DH
	CALL 	BCDToInteger
	MOV 	[ES:BX + FP_ChangedSecond], AL
	;Minute
	MOV 	AL, CL
	CALL 	BCDToInteger
	MOV 	[ES:BX + FP_ChangedMinute], AL
	;Hour
	MOV 	AL, CH
	CALL 	BCDToInteger
	MOV 	[ES:BX + FP_ChangedHour], AL
	MOV 	AH, 4H
	INT 	1AH
	;Day
	MOV 	AL, DL
	CALL 	BCDToInteger
	MOV 	[ES:BX + FP_ChangedDay], AL
	;Month
	MOV 	AL, DH
	CALL 	BCDToInteger
	MOV 	[ES:BX + FP_ChangedMonth], AL
	;Year
	MOV 	AL, CL
	CALL 	BCDToInteger
	MOV 	CL, AL
	MOV 	AL, CH
	CALL 	BCDToInteger
	MOV 	AH, 100
	MUL 	AH
	XOR 	CH, CH
	ADD 	AX, CX
	
	;Save file parameter
	MOV 	CX, SI
	CALL 	FSSWriteSector
	
	CLC
	
	;释放内存
	FreeSKeepFlags	MaxFileLength
	
	POP		BP
	POP 	SI
	POP 	ES
	POP 	DS
	POP 	DI
	POP 	DX
	POP 	CX
	POP 	BX
	POP 	AX
	RET
EndProc		FSSWriteData

FSSReadFile_FP: 				TIMES SectorByteCount DB 0
	
;Function:		Read data from file
;Parameters:	DS:SI=File name(Length >= 20bytes!!!) SegAddr:OffAddr
;Returns:		If CF=0 then successed else failed,
;				ES:DI=Dest SegAddr:OffAddr,
;				CX=Length
Procedure	FSSReadFile
	PUSH 	AX
	PUSH 	DX
	PUSH 	DI
	PUSH 	ES
	PUSH 	BX
	PUSH 	DS
	PUSH	BP
	
	CALL 	FSSGetFileNumber
	JNC 	FSSReadFile_Label1
	;Error
	POP		BP
	POP 	DS
	POP 	BX
	POP 	ES
	POP 	DI
	POP 	DX
	POP 	AX
	RET
	
FSSReadFile_Label1:
	
	;在堆栈上申请一块内存给文件缓冲区
	AllocS	MaxFileLength
	
	PUSH 	ES	;Safe dest SegAddr

	;读取文件到文件缓冲区
	PUSH 	DX		;Safe file number!!!
	MOV 	AX, SS	;文件缓冲区在内核堆栈上
	MOV 	ES, AX
	MOV 	BX, BP	;BP保存着文件缓冲区在内核堆栈上的偏移
	MOV 	AX, DX
	MOV 	DX, FileDataMaxSector
	MUL 	DX
	ADD 	AX, FileDataStartSector
	MOV 	CX, AX
	MOV 	AX, FileDataMaxSector
	CALL 	FSSReadSectors
	POP 	DX
	
	;Get file length
	ADD 	DX, FileParameterStartSector
	MOV 	CX, DX
	MOV 	AX, KernelSegAddr
	MOV 	ES, AX
	MOV 	BX, FSSReadFile_FP
	CALL 	FSSReadSector
	MOV 	CX, [CS:FSSReadFile_FP + FP_Length]
	
	;Copy data to ES:BX
	POP 	ES		;Get dest SegAddr
	PUSH 	CX
	MOV 	AX, SS	;文件缓冲区在内核堆栈上
	MOV 	DS, AX
	MOV 	BX, BP	;BP保存着文件缓冲区在内核堆栈上的偏移
	OR		CX, CX
	JZ		FSSReadFile_FileEmpty
	
FSSReadFile_Label2:
	
	MOV 	AL, [BX]
	MOV 	[ES:DI], AL
	INC 	BX
	INC 	DI
	LOOP FSSReadFile_Label2
	
FSSReadFile_FileEmpty:
	
	POP 	CX
	CLC
	
	;释放内存
	FreeSKeepFlags	MaxFileLength
	
	POP		BP
	POP 	DS
	POP 	BX
	POP 	ES
	POP 	DI
	POP 	DX
	POP 	AX
	RET
EndProc		FSSReadFile

FSSAppendByte_Byte		DB ?
FSSAppendByte_ParamSec	DW ?
FSSAppendByte_FileLen	DW ?

;过程名:	FSSAppendByte
;功能:		向文件尾添加一个字符
;参数:		DS:SI=文件名
;			AL=字符
;返回值:	CF=0成功, 否则失败
Procedure	FSSAppendByte
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	BP
	PUSH	ES
	
	MOV		[CS:FSSAppendByte_Byte], AL
	
	AllocS	SectorByteCount
	
	PUSH	SS
	POP		ES
	
	CALL	FSSGetFileNumber
	JC		FSSAppendByte_End
	MOV		[CS:FSSAppendByte_ParamSec], DX
	ADD		WORD [CS:FSSAppendByte_ParamSec], FileParameterStartSector
	MOV		AL, FileDataMaxSector
	MUL		DL
	ADD		AX, FileDataStartSector
	MOV		BX, AX	;BX=储存着文件开始扇区
	
	CALL	FSSGetFileLen
	MOV		[CS:FSSAppendByte_FileLen], CX
	INC		WORD [CS:FSSAppendByte_FileLen]
	XOR		DX, DX
	MOV		AX, CX
	MOV		CX, SectorByteCount
	DIV		CX	;AX=文件尾所在的扇区, DX=文件尾所在的扇区的字节偏移
	
	PUSH	BX
	PUSH	CX
	MOV		CX, BX
	ADD		CX, AX
	MOV		BX, BP
	CALL	FSSReadSector
	ADD		BX, DX
	MOV		AL, [CS:FSSAppendByte_Byte]
	MOV		[ES:BX], AL
	SUB		BX, DX
	CALL	FSSWriteSector
	POP		CX
	POP		BX
	
	AllocS			SectorByteCount
	
	PUSH	BX
	PUSH	CX
	PUSH	DX
	MOV		BX, BP
	MOV		CX, [CS:FSSAppendByte_ParamSec]
	CALL	FSSReadSector
	MOV		DX, [CS:FSSAppendByte_FileLen]
	MOV		[ES:BX + FP_Length], DX
	CALL	FSSWriteSector
	POP		DX
	POP		CX
	POP		BX
	
	FreeSKeepFlags	SectorByteCount
	
FSSAppendByte_End:

	FreeSKeepFlags	SectorByteCount
	
	POP		ES
	POP		BP
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	RET
EndProc		FSSAppendByte

FSSGetByte_AH	DB ?

;过程名:	FSSGetByte
;描述:		获取文件中的某个字节
;参数:		DS:SI=文件名
;			BX=偏移
;返回值:	CF=0成功, 否则失败
;			AL=字节
Procedure	FSSGetByte
	PUSH	DX
	CALL	FSSGetFileNumber
	JC		FSSGetByte_End
	MOV		[CS:FSSGetByte_AH], AH
	PUSH	BX
	PUSH	CX
	PUSH	ES
	PUSH	BP
	MOV		AX, DX
	MOV		DX, 54
	MUL		DX
	ADD		AX, FileDataStartSector
	MOV		CX, AX
	MOV		AX, BX
	XOR		DX, DX
	MOV		BX, 512
	DIV		BX
	ADD		CX, AX
	
	AllocS	512
	
	PUSH	SS
	POP		ES
	MOV		BX, BP
	CALL	FSSReadSector
	ADD		BX, DX
	MOV		AL, [ES:BX]
	SUB		BX, DX
	
	FreeSKeepFlags	512
	
	CLC
	POP		BP
	POP		ES
	POP		CX
	POP		BX
	MOV		AH, [CS:FSSGetByte_AH]
	
FSSGetByte_End:
	
	POP		DX
	RET
EndProc		FSSGetByte

;过程名:	FSSGetBytes
;描述:		以256字节读取数据
;参数:		DS:SI=文件名
;			BX=扇区偏移
;返回值:	ES:DI=缓冲区
;			CF=0成功, 否则失败
Procedure	FSSGetBytes
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	
	CALL	FSSGetFileNumber
	JC		FSSGetBytes_Error
	MOV		AX, DX
	MOV		DX, FileDataMaxSector
	MUL		DX
	ADD		AX, FileDataStartSector
	ADD		AX, BX
	MOV		CX, AX
	MOV		BX, DI
	CALL	FSSReadSector
	CLC
	
FSSGetBytes_Error:
	
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	RET
EndProc		FSSGetBytes
	
;Function:	Format file data
Procedure	FSSFormat
	PUSHF
	PUSH	ES
	PUSH 	AX
	PUSH 	BX
	PUSH 	CX
	
	;在堆栈上申请一块内存给文件缓冲区
	AllocS	FileParameterLength
	
	PUSH	SS
	POP		ES
	
	XOR 	BX, BX
	MOV 	AX, MaxFileCount
FSSFormat_Label1:
	PUSH 	BX
	;Read file parameter
	MOV 	CX, FileParameterStartSector
	ADD 	CX, BX
	MOV 	BX, BP
	CALL 	FSSReadSector
	
	;Clear FP_Use
	MOV 	BYTE [BP + FP_Use], 0
	
	;Save file parameter
	CALL 	FSSWriteSector
	
	POP 	BX
	INC 	BX
	DEC 	AX
	CMP 	AX, 0
	JE 		FSSFormat_Label2
	JMP 	FSSFormat_Label1
	
FSSFormat_Label2:

	;释放空间
	FreeS	FileParameterLength

	POP 	CX
	POP 	BX
	POP 	AX
	POP		ES
	POPF
	RET
EndProc		FSSFormat
	
;过程名:	FSSRename
;参数:		DS:SI=原文件名(长度必须大于或等于20)
;			ES:DI=目标文件名(长度必须大于或等于20)
;返回值:	如果CF为0则成功, 否则失败.
Procedure	FSSRename
	PUSH	BP
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	ES
	PUSH	DI
	PUSH	DS
	PUSH	SI
	
	AllocS	FileParameterLength
	
	;确认目标文件名是否存在
	XCHG	SI, DI
	MOV		AX, DS
	PUSH	ES
	POP		DS
	MOV		ES, AX
	CALL	FSSFileExists
	JC		FSSRename_End
	XCHG	DI, SI
	MOV		AX, DS
	PUSH	ES
	POP		DS
	MOV		ES, AX
	
	CALL	FSSGetFileNumber
	JC		FSSRename_End
	PUSH	ES
	ADD		DX, FileParameterStartSector
	MOV		CX, DX
	PUSH	SS
	POP		ES
	MOV		BX, BP
	CALL	FSSReadSector
	POP		ES
	
	MOV		CX, MaxFileNameLength
	MOV		SI, DI
	MOV		DI, BP
	ADD		DI, FP_FileName
	
	MOV		AX, ES
	MOV		DS, AX
	MOV		AX, SS
	MOV		ES, AX
	
	CLD
	
FSSRename_CopyFileName:
	LODSB
	STOSB
	LOOP	FSSRename_CopyFileName
	
	MOV		CX, DX
	CALL	FSSWriteSector
	
FSSRename_End:
	
	FreeSKeepFlags	FileParameterLength
	
	POP		SI
	POP		DS
	POP		DI
	POP		ES
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	POP		BP
	RET
EndProc		FSSRename
	
;过程名:	FSSGetFileLen
;功能:		获取文件的长度
;参数:		DS:SI=文件名段地址:偏移地址
;返回值:	CX=文件长度, 如果CF=0则成功否则失败
Procedure	FSSGetFileLen
	PUSH	DX
	PUSH	ES
	PUSH	BX
	PUSH	BP
	
	CALL	FSSGetFileNumber
	JC		FSSGetFileLen_Err
	
	AllocS	FileParameterLength

	PUSH	SS
	POP		ES
	MOV		BX, BP
	MOV		CX, DX
	ADD		CX, FileParameterStartSector
	CALL	FSSReadSector
	
	MOV		CX, [BP + FP_Length]
	
	FreeSKeepFlags	FileParameterLength
	
FSSGetFileLen_Err:	

	POP		BP
	POP		BX
	POP		ES
	POP		DX
	RET
EndProc		FSSGetFileLen
	
;过程名:	FSSGetFileChangedDate
;功能:		获取文件修改日期
;参数:		DS:SI=文件名段地址:偏移地址
;返回值:	ES:DI=日期, 结构如下
;返回值结构:
;   6     5     4     3     2     1     0
;*-----*-----*-----*-----*-----*-----*-----*
;| Sec | Min | Hour| Day | Mon |    Year   |
;*-----*-----*-----*-----*-----*-----*-----*
;
Procedure	FSSGetFileChangedDate
	PUSHF
	PUSH	DX
	PUSH	BX
	PUSH	BP
	
	CALL	FSSGetFileNumber
	JC		FSSGetFileChangedDate_Err
	
	AllocS	FileParameterLength

	PUSH	ES
	PUSH	SS
	POP		ES
	MOV		BX, BP
	MOV		CX, DX
	ADD		CX, FileParameterStartSector
	CALL	FSSReadSector
	POP		ES
	
	MOV		BX, [BP + FP_ChangedYear]
	MOV		[ES:DI + 0], BX
	MOV		BX, [BP + FP_ChangedMonth]
	MOV		[ES:DI + 2], BX
	MOV		BX, [BP + FP_ChangedDay]
	MOV		[ES:DI + 3], BX
	MOV		BX, [BP + FP_ChangedHour]
	MOV		[ES:DI + 4], BX
	MOV		BX, [BP + FP_ChangedMinute]
	MOV		[ES:DI + 5], BX
	MOV		BX, [BP + FP_ChangedSecond]
	MOV		[ES:DI + 6], BX
	
	FreeSKeepFlags	FileParameterLength
	
FSSGetFileChangedDate_Err:	

	POP		BP
	POP		BX
	POP		DX
	POPF
	RET
EndProc		FSSGetFileChangedDate

;过程名:	FSSGetFileCreatedDate
;功能:		获取文件的创建日期
;参数:		DS:SI=文件名段地址:偏移地址
;返回值:	ES:DI=日期, 结构和FSSGetFileChangedDate返回
;			的结构一样
Procedure	FSSGetFileCreatedDate
	PUSHF
	PUSH	DX
	PUSH	BX
	PUSH	BP
	
	CALL	FSSGetFileNumber
	JC		FSSGetFileCreatedDate_Err
	
	AllocS	FileParameterLength

	PUSH	ES
	PUSH	SS
	POP		ES
	MOV		BX, BP
	MOV		CX, DX
	ADD		CX, FileParameterStartSector
	CALL	FSSReadSector
	POP		ES
	
	MOV		BX, [BP + FP_CreatedYear]
	MOV		[ES:DI + 0], BX
	MOV		BX, [BP + FP_CreatedMonth]
	MOV		[ES:DI + 2], BX
	MOV		BX, [BP + FP_CreatedDay]
	MOV		[ES:DI + 3], BX
	MOV		BX, [BP + FP_CreatedHour]
	MOV		[ES:DI + 4], BX
	MOV		BX, [BP + FP_CreatedMinute]
	MOV		[ES:DI + 5], BX
	MOV		BX, [BP + FP_CreatedSecond]
	MOV		[ES:DI + 6], BX
	
	FreeSKeepFlags	FileParameterLength
	
FSSGetFileCreatedDate_Err:	

	POP		BP
	POP		BX
	POP		DX
	POPF
	RET
EndProc		FSSGetFileCreatedDate

;过程名:	FSSGetFileCount
;功能:		获取文件数量.
;参数:		无
;返回值:	CX=文件数量
Procedure	FSSGetFileCount
	PUSHF
	PUSH	ES
	PUSH	BP
	PUSH	AX
	
	AllocS	FileParameterLength
	
	PUSH	SS
	POP		ES
	MOV		BX, BP
	MOV		CX, MaxFileCount
	XOR		AX, AX	;文件计数器
	
FSSGetFileCount_Label1:
	PUSH	CX
	ADD		CX, FileParameterStartSector
	DEC		CX
	CALL	FSSReadSector
	POP		CX
	
	CMP		BYTE [BP + FP_Use], 0	;文件槽是否使用?
	JE		FSSGetFileCount_NotUse
	INC		AX
	
FSSGetFileCount_NotUse:	;文件槽未被使用

	LOOP	FSSGetFileCount_Label1
	
	FreeSKeepFlags	FileParameterLength
	
	MOV		CX, AX
	
	POP		AX
	POP		BP
	POP		ES
	POPF
	RET
EndProc		FSSGetFileCount
	
;过程名:	FSSGetFileNames
;功能:		获取所有文件名.
;参数:		无
;返回值:	ES:DI=文件名缓冲区段地址:偏移地址
Procedure	FSSGetFileNames
	PUSHF
	PUSH	DI
	PUSH	BP
	PUSH	ES
	PUSH	DS
	PUSH	AX
	PUSH	BX
	PUSH	CX
	
	AllocS	FileParameterLength
	
	;把文件名缓冲区段地址临时储存到DS
	PUSH	ES
	POP		DS
	
	PUSH	SS
	POP		ES
	MOV		BX, BP
	MOV		CX, MaxFileCount
	
FSSGetFileNames_Label1:
	;把文件参数读取到分配的空间里
	PUSH	CX
	ADD		CX, FileParameterStartSector
	DEC		CX
	CALL	FSSReadSector
	POP		CX
	
	CMP		BYTE [BP + FP_Use], 0	;文件槽是否被使用?
	JE		FSSGetFileNames_NotUse
	
	PUSH	DS
	PUSH	ES
	PUSH	DI
	PUSH	CX
	
	;取出刚才保存的文件名缓冲区段地址
	PUSH	DS
	POP		ES
	
	PUSH	SS
	POP		DS
	
	MOV		SI, BP
	ADD		SI, FP_FileName
	MOV		CX, MaxFileNameLength
	REP MOVSB
	
	POP		CX
	POP		DI
	POP		ES
	POP		DS
	
	ADD		DI, MaxFileNameLength

FSSGetFileNames_NotUse:	;文件槽未被使用
	
	LOOP	FSSGetFileNames_Label1
	
	FreeSKeepFlags	FileParameterLength
	
	POP		CX
	POP		BX
	POP		AX
	POP		DS
	POP		ES
	POP		BP
	POP		DI
	POPF
	RET
EndProc		FSSGetFileNames

;过程名:	FSSCopyFile
;描述:		复制文件
;参数:		DS:SI=源文件名
;			AL=目标磁盘号
;			ES:DI=目标文件名
;返回值:	CF=0成功, 否则失败		
Procedure	FSSCopyFile
	PUSH	DS
	PUSH	ES
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SI
	PUSH	DI
	PUSH	BP
	
	;检查源文件是否存在
	CALL	FSSFileExists
	JNC		FSSCopyFile_Err_NoAlloc
	
	;在目标文件的位置创建新文件
	PUSH	DS
	PUSH	SI
	MOV		AH, [CS:CurrentDisk]
	CALL	FSSChangeDisk
	PUSH	ES
	POP		DS
	MOV		SI, DI
	CALL	FSSDeleteFile
	CALL	FSSNewFile
	XCHG	AL, AH
	CALL	FSSChangeDisk
	XCHG	AL, AH
	POP		SI
	POP		DS
	
	;把文件读取到缓冲区
	MOV		BX, ES
	MOV		DX, DI
	AllocS	MaxFileLength
	PUSH	SS
	POP		ES
	MOV		DI, BP
	CALL	FSSReadFile
	
	;把缓冲区的内容写到目标文件
	MOV		AH, [CS:CurrentDisk]
	CALL	FSSChangeDisk
	
	MOV		DS, BX
	MOV		SI, DX
	CALL	FSSWriteData
	XCHG	AL, AH
	CALL	FSSChangeDisk
	XCHG	AL, AH
	CLC
	FreeSKeepFlags	MaxFileLength
	JMP		FSSCopyFile_NoErr
	
FSSCopyFile_Err_NoAlloc:

	STC
	
FSSCopyFile_NoErr:
	
	POP		BP
	POP		DI
	POP		SI
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	POP		ES
	POP		DS
	RET
EndProc		FSSCopyFile

;过程名:	FSSCutFile
;描述:		剪切文件
;参数:		DS:SI=源文件名
;			AL=目标磁盘号
;			ES:DI=目标文件名
;返回值:	CF=0成功, 否则失败
Procedure	FSSCutFile
	CALL	FSSCopyFile
	JC		FSSCutFile_Err
	CALL	FSSDeleteFile
	
FSSCutFile_Err:

	RET
EndProc		FSSCutFile

;过程名:	FSSParsePath
;描述:		解析路径
;参数:		DS:SI=路径(长度必须为23字节)
;返回值:	AL=磁盘号
;			ES:DI=文件名(长度必须为20字节)
Procedure	FSSParsePath
	PUSHF
	PUSH	BX
	PUSH	SI
	PUSH	DI
	PUSH	CX
	MOV		BH, AH
	CMP		BYTE [SI + 2], ':'
	JNE		FSSParsePath_NoDisk
	LODSB
	MOV		AH, AL
	LODSB
	CALL	FSSGetDiskNumber
	INC		SI
	
FSSParsePath_NoDisk:
	
	MOV		CX, MaxFileNameLength
	CLD
	REP MOVSB
	
	MOV		AH, BH
	POP		CX
	POP		DI
	POP		SI
	POP		BX
	POPF
	RET
EndProc		FSSParsePath

;----------------------------------------

;String Procedures

;Function:		Get string length
;Parameters:	DS:SI=String SegAddr:OffAddr
;Returns:		CX=String length
Procedure	StrLen
	PUSHF
	PUSH 	SI
	PUSH 	AX
	XOR 	CX, CX
	CLD
StrLen_Label1:
	LODSB
	CMP 	AL, 0
	JE 		StrLen_Label2
	INC 	CX
	JMP 	StrLen_Label1
StrLen_Label2:
	POP 	AX
	POP 	SI
	POPF
	RET
EndProc		StrLen
	
;Function:		Compare string
;Parameters:	DS:SI=String1 SegAddr:OffAddr, ES:DI=String2 SegAddr:OffAddr
;Returns:		If CF = 0, String1 = String2, else String != String2
Procedure	StrCmp
	PUSH 	AX
	PUSH 	SI
	PUSH 	DI
StrCmp_Label1:
	MOV 	AL, [SI]
	MOV 	AH, [ES:DI]
	CMP 	AL, AH
	JNE 	StrCmp_Label2
	CMP 	AL, 0
	JE 		StrCmp_Label3
	INC 	SI
	INC 	DI
	JMP 	StrCmp_Label1
StrCmp_Label2:
	STC
	JMP 	StrCmp_Label4
StrCmp_Label3:
	CLC
StrCmp_Label4:
	POP 	DI
	POP 	SI
	POP 	AX
	RET
EndProc		StrCmp
	
;Function:		Get char position
;Parameters:	DS:SI=String SegAddr:OffAddr, AH=Char, BX=Start position, CX=Length
;Returns:		BX=Char position, if CF=0, found else not found
Procedure	StrPosL
	PUSH 	AX
	PUSH 	CX
	PUSH 	SI
	CLD
	CMP 	CX, 0
	JE 		StrPosL_Label4
	ADD 	SI, BX
StrPosL_Label1:
	LODSB
	CMP 	AH, AL
	JE 		StrPosL_Label2
	INC 	BX
	LOOP 	StrPosL_Label1
StrPosL_Label4:
	STC
	JMP 	StrPosL_Label3
StrPosL_Label2:
	CLC
StrPosL_Label3:
	POP 	SI
	POP 	CX
	POP 	AX
	RET
EndProc		StrPosL

;Function:		Sub string
;Parameters:	DS:SI=Source string SegAddr:OffAddr, AX=Start position, CX=Length
;Returns:		ES:DI=Dest string SegAddr:OffAddr
Procedure	StrMid
	PUSHF
	PUSH 	SI
	PUSH 	DI
	PUSH 	CX
	CMP 	CX, 0
	JE 		StrMid_Label2
	ADD 	SI, AX
	CLC
StrMid_Label1:
	LODSB
	STOSB
	LOOP 	StrMid_Label1
StrMid_Label2:
	MOV 	BYTE [ES:DI], 0
	POP 	CX
	POP 	DI
	POP 	SI
	POPF
	RET
EndProc		StrMid

;Function:		Sub string from left
;Parameters:	DS:SI=Source string SegAddr:OffAddr, CX=Length
;Returns:		ES:DI=Dest string SegAddr:OffAddr
Procedure	StrLeft
	PUSHF
	PUSH 	AX
	MOV 	AX, 0
	CALL 	StrMid
	POP 	AX
	POPF
	RET
EndProc		StrLeft
	
;Function:		Sub string from right
;Parameters:	DS:SI=Source string SegAddr:OffAddr, CX=Length
;Returns:		ES:DI=Dest string SegAddr:OffAddr
Procedure	StrRight
	PUSHF
	PUSH 	AX
	PUSH 	CX
	PUSH 	SI
	PUSH 	DI
	PUSH 	CX
	CALL 	StrLen
	MOV 	AX, CX
	POP 	CX
	SUB 	AX, CX
	CALL 	StrMid
	POP 	DI
	POP 	SI
	POP 	CX
	POP 	AX
	POPF
	RET
EndProc		StrRight
	
;----------------------------------------

;Other Functions:

;Function:		Convert BCD to integer
;Parameters:	AL=BCD
;Returns:		AL=Integer
Procedure	BCDToInteger
	PUSH 	BX
	PUSH 	CX
	;Safe ah
	MOV 	BH, AH
	
	MOV 	CH, AL
	MOV 	CL, 4
	SHR 	AL, CL
	MOV 	BL, 10
	MUL 	BL
	AND 	CH, 0FH
	ADD 	AL, CH
	;Safe ah
	MOV 	AH, BH
	
	POP 	CX
	POP 	BX
	RET
EndProc		BCDToInteger
	
;Function:		Check byte is '0'~'9'
;Parameters:	AL=Byte
;Returns:		If CF=0 yes else no
Procedure	IsDigitChar
	CMP 	AL, '0'
	JB 		IsDigitChar_Label1
	CMP 	AL, '9'
	JA 		IsDigitChar_Label1
	CLC
	JMP 	IsDigitChar_Label2
IsDigitChar_Label1:
	STC
IsDigitChar_Label2:
	RET
EndProc		IsDigitChar
	
;----------------------------------------

;内核堆栈
;堆栈段地址:	2000H
;堆栈偏移地址:	0000H
;堆栈界限:		FFFDH
;堆栈大小:		FFFEH
;
;*----------*
;|			|	0000H	<= 栈顶
;|			|	0001H
;|			|	...
;|			|	FFFCH
;|			|	FFFDH
;|XXXXXXXXXX|	FFFEH	<= 栈底, 堆栈初始化时的位置
;|XXXXXXXXXX|	FFFFH
;*----------*
KernelStackSegAddr	EQU 2000H	;堆栈段地址
KernelStackISTP		EQU 0FFFEH	;堆栈初始化栈顶指针

;INT 21H Address
INT21H_Address 		EQU 21H * 4

AutoExecFileName	DB 'autoexec'
					TIMES 20 - ($ - AutoExecFileName) DB 0
AutoExec:			TIMES 100H DB 0
					
Start:
	
	CLI
	
	;初始化堆栈
	MOV		AX, KernelStackSegAddr
	MOV		SS, AX
	MOV		SP, KernelStackISTP

	;初始化段寄存器
	PUSH 	CS
	POP 	DS
	PUSH 	CS
	POP 	ES
	
	PUSH	CS
	POP		WORD [CS:Exec_Register + ER_DS]
	PUSH	CS
	POP		WORD [CS:Exec_Register + ER_ES]
	
	;Set interrupt vector table
	;INT 21H
	PUSH 	DS
	MOV 	AX, 0
	MOV 	DS, AX
	MOV 	WORD [DS:INT21H_Address], INT21H
	MOV 	WORD [DS:INT21H_Address + 2], KernelSegAddr
	POP 	DS
	
	;Init video mode
	CALL 	OSSInit
	
	;Clear screen
	CALL 	OSSCls
	
	;Print version message
	CALL 	ShowVersion
	CALL 	OSSCR
	CALL 	OSSLF
	
	;初始化鼠标
	CALL	InitMouse
	
	;自动运行
	MOV		SI, AutoExecFileName
	MOV		AH, 14
	INT		21H
	OR		AL, AL
	JZ		AutoExecErr
	MOV		AH, 23
	INT		21H
	CMP		CX, 0
	JE		AutoExecErr
	CMP		CX, 100H
	JA		AutoExecErr
	MOV		DI, AutoExec
	MOV		AH, 13
	INT		21H
	MOV		SI, AutoExec
	MOV		AH, 20
	INT		21H
	
AutoExecErr:
	
	STI
	
	;初始化硬盘
	CALL	InitDisk
	
	;Enter console
	CALL 	EnterConsole

	;运行到这里内核就错了...
	FatalError	FEC_Unknow
	
	;Fill 0
	TIMES 	MaxKernelSize - ($ - $$) DB 0
