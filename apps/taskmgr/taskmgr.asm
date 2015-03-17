;*------------------------------------------------------*
;|文件名:	taskmgr.asm									|
;|作者:		Ihsoh										|
;|创建时间:	2013-3-28									|
;|														|
;|描述:													|
;|任务管理器. 总共可以容纳6个任务(实际5个, 任务管理器本	|
;|身也是一个任务). 通过中断向别的任务提供功能.			|
;*------------------------------------------------------*

%INCLUDE	'..\..\common\common.mac'
%INCLUDE	'..\..\common\common.inc'
%INCLUDE	'taskmgr.inc'
%INCLUDE	'tasktbl.mac'

ORG		0

JMP		Start	;转移到真正开始的地方

TaskTable			Tasks, MaxTaskCount		;任务表
TaskCount			DB 1					;任务数, 任务管理器也是一个任务
CurrentTaskID		DB 0					;当前任务ID
TaskMgrFileName		DB TASKMGR				;任务管理器文件名
TaskMgrCfgFileName	DB TASKMGRCFG			;任务管理器配置文件的文件名
OldINT24H			DW 0, 0					;原INT24H中断过程地址
OldINT23H			DW 0, 0					;原INT23H中断过程地址
StartApp:			TIMES 20 DB 0			;启动的程序
StartAppArg			DB 0

LoadErrMsg				DB 'Taskmgr Error: Cannot load application!', 13, 10, 0
LostConfigFileErrMsg	DB 'Taskmgr Error: Lost config file!', 13, 10, 0
AppNameToLongErrMsg		DB 'Taskmgr Error: Application name too long!', 13, 10, 0
NoMTA16ErrMsg			DB 'Taskmgr Error: No MTA16!', 13, 10, 0

;宏名:		SetINT24H
;功能:		把过程装入到INT24H中断
;参数:		参数1=过程名
;返回值:	无
%MACRO	SetINT24H 1
	PUSHF
	CLI
	PUSH	ES
	PUSH	AX
	;保存INT24H中断过程地址
	MOV		AX, 0
	MOV		ES, AX
	MOV		AX, [ES:INT24HAddr + 2]	;段地址
	MOV		[CS:OldINT24H + 2], AX
	MOV		AX, [ES:INT24HAddr]		;偏移地址
	MOV		[CS:OldINT24H], AX
	
	;设置新的INT24H中断过程地址
	MOV		WORD [ES:INT24HAddr], %1	;中断过程偏移地址
	MOV		AX, CS
	MOV		[ES:INT24HAddr + 2], AX	;段地址
	
	POP		AX
	POP		ES
	POPF
%ENDMACRO

;宏名:		ResumeINT24H
;功能:		恢复INT24H中断过程地址
;参数:		无
;返回值:	无
%MACRO	ResumeINT24H 0
	PUSHF
	CLI
	CMP		WORD [CS:OldINT24H + 2], 0	;如果段地址为0则INT24HH的中断过程地址就没有被修改
	JE		%%ResumeINT24H_NoSet
	PUSH	ES
	PUSH	AX
	MOV		AX, 0
	MOV		ES, AX
	MOV		AX, [CS:OldINT24H + 2]
	MOV		[ES:INT24HAddr + 2], AX
	MOV		AX, [CS:OldINT24H]
	MOV		[ES:INT24HAddr], AX
	POP		AX
	POP		ES
%%ResumeINT24H_NoSet:
	POPF
%ENDMACRO

;宏名:		SetINT23H
;功能:		把过程装入到INT23H中断
;参数:		参数1=中断过程
;返回值:	无
%MACRO	SetINT23H 1
	PUSHF
	CLI
	PUSH	ES
	PUSH	AX
	MOV		AX, 0
	MOV		ES, AX
	;保存INT23H中断过程地址
	MOV		AX, [ES:INT23HAddr + 2]
	MOV		[CS:OldINT23H + 2], AX
	MOV		AX, [ES:INT23HAddr]
	MOV		[CS:OldINT23H], AX
	
	;设置新的INT32H中断过程地址
	MOV		AX, CS
	MOV		[ES:INT23HAddr + 2], AX
	MOV		WORD [ES:INT23HAddr], %1
	
	POP		AX
	POP		ES
	POPF
%ENDMACRO

;宏名:		ResumeINT23H
;功能:		恢复INT23H中断过程地址
;参数:		无
;返回值:	无
%MACRO	ResumeINT23H 0
	PUSHF
	CLI
	PUSH	ES
	PUSH	AX
	MOV		AX, 0
	MOV		ES, AX
	MOV		AX, [CS:OldINT23H + 2]
	MOV		[ES:INT23HAddr + 2], AX
	MOV		AX, [CS:OldINT23H]
	MOV		[ES:INT23HAddr], AX
	POP		AX
	POP		ES
	POPF
%ENDMACRO

;过程名:	Init
;功能:		初始化任务管理器
;参数:		无
;返回值:	无
Procedure	Init
	PUSHF
	PUSH	AX
	PUSH	DS
	PUSH	BX
	PUSH	SI
	PUSH	DI
	PUSH	CX
	
	;把自己当成任务
	MOV		BYTE [Tasks + TaskMgrID + TT_Used], 1	;设置为使用
	PUSHF
	POP		AX
	OR		AX, 00000010_00000000B	;IF = 1
	MOV		[Tasks + TaskMgrID + TT_Flags], AX		;设置任务管理器的任务的标志寄存器
	MOV		SI, TaskMgrFileName
	MOV		DI, Tasks + TaskMgrID + TT_FileName
	MOV		CX, 20
	CLD
	REP MOVSB	;复制任务名
	
	;设置INT21H功能号0的执行过程
	MOV		SI, INT21H_Func0
	PUSH	CS
	POP		DS
	MOV		AH, 27
	INT		21H
	
	POP		CX
	POP		DI
	POP		SI
	POP		BX
	POP		DS
	POP		AX
	POPF
	
	RET
EndProc		Init
	
;过程名:	INT21H_Func0
;描述:		用于替换默认的21H号中断的0号功能的执行过程.
;			任务自杀
;参数:		无
;返回值:	无
Procedure	INT21H_Func0
	PUSH	AX
	MOV		AL, [CS:CurrentTaskID]
	CALL	KillTask
	POP		AX
	INT		24H
EndProc		INT21H_Func0

;用于在切换任务时临时保存上一个任务的各寄存器
INT24H_AX		DW 0
INT24H_BX		DW 0
INT24H_CX		DW 0
INT24H_DX		DW 0
INT24H_SI		DW 0
INT24H_DI		DW 0
INT24H_BP		DW 0
INT24H_SP		DW 0
INT24H_DS		DW 0
INT24H_SS		DW 0
INT24H_ES		DW 0
INT24H_CS		DW 0
INT24H_IP		DW 0
INT24H_Flags	DW 0
	
;过程名:	INT24H
;功能:		INT 24H中断程序
;参数:		无
;返回值:	无
Procedure	INT24H
	;取上一个任务的IP, CS和Flags
	PUSH	BP
	MOV		BP, SP
	PUSH	AX
	MOV		AX, [BP + 2]
	MOV		[CS:INT24H_IP], AX	;IP
	MOV		AX, [BP + 4]
	MOV		[CS:INT24H_CS], AX	;CS
	MOV		AX, [BP + 6]
	MOV		[CS:INT24H_Flags], AX	;Flags
	POP		AX
	POP		BP

	;保存上个任务的各寄存器
	MOV		[CS:INT24H_AX], AX
	MOV		[CS:INT24H_BX], BX
	MOV		[CS:INT24H_CX], CX
	MOV		[CS:INT24H_DX], DX
	MOV		[CS:INT24H_SI], SI
	MOV		[CS:INT24H_DI], DI
	MOV		[CS:INT24H_BP], BP
	MOV		[CS:INT24H_SP], SP
	ADD		WORD [CS:INT24H_SP], 6
	MOV		[CS:INT24H_DS], DS
	MOV		[CS:INT24H_SS], SS
	MOV		[CS:INT24H_ES], ES
	
	PUSH	BP
	MOV		BP, SP
	
	;检测当前任务是否存活?
	PUSH	AX
	PUSH	BX
	MOV		AL, [CS:CurrentTaskID]
	MOV		AH, TaskTableItemLen
	MUL		AH
	MOV		BX, AX
	CMP		BYTE [CS:Tasks + BX + TT_Used], 0
	JE		INT24H_CurrentTaskDie
	POP		BX
	POP		AX
	
	;检测是否只有一个任务?
	CMP		BYTE [CS:TaskCount], 1
	JA		INT24H_MoreOneTask
	MOV		SP, BP
	POP		BP
	PUSH	AX
	MOV		AL, 20H
	OUT		20H, AL
	POP		AX
	IRET	;不做任务切换直接返回
	
INT24H_CurrentTaskDie:
	
	POP		BX
	POP		AX
	
INT24H_MoreOneTask:	;多余一个任务
	
	;获取下一个任务的ID
	MOV		AL, [CS:CurrentTaskID]
	MOV		BL, TaskTableItemLen
	MUL		BL
	MOV		BX, AX
	ADD		BX, TaskTableItemLen
	MOV		AL, [CS:CurrentTaskID]
	XOR		AH, AH
	INC		AX
	
INT24H_Find:	;正向搜索出一个可用的任务

	CMP		AX, MaxTaskCount
	JE		INT24H_Find_Exit
	CMP		BYTE [CS:Tasks + BX + TT_Used], 0
	JE		INT24H_Find_NotUsed
	JMP		INT24H_Found

INT24H_Find_NotUsed:	;未被使用

	ADD		BX, TaskTableItemLen
	INC		AX
	CMP		AX, MaxTaskCount
	JNE		INT24H_Find
	
INT24H_Find_Exit:	;正向搜索未找到

	MOV		BX, 0
	MOV		AL, 0
	XOR		AH, AH
	
INT24H_Find1:	;从头开始搜索是否有可用的任务

	CMP		BYTE [CS:Tasks + BX + TT_Used], 0
	JE		INT24H_Find1_NotUsed
	JMP		INT24H_Found
	
INT24H_Find1_NotUsed:	;未被使用

	ADD		BX, TaskTableItemLen
	INC		AX
	JMP		INT24H_Find1

INT24H_Found:	;找到
	
	;保存当前任务的现场
	MOV		CX, AX
	MOV		BL, [CS:CurrentTaskID]
	MOV		AL, TaskTableItemLen
	MUL		BL
	MOV		BX, AX
	
	MOV		AX, [CS:INT24H_Flags]				;Flags
	MOV		[CS:Tasks + BX + TT_Flags], AX
	MOV		AX, [CS:INT24H_CS]				;CS
	MOV		[CS:Tasks + BX + TT_CS], AX
	MOV		AX, [CS:INT24H_IP]				;IP
	MOV		[CS:Tasks + BX + TT_IP], AX
	MOV		AX, [CS:INT24H_AX]				;AX
	MOV		[CS:Tasks + BX + TT_AX], AX
	MOV		AX, [CS:INT24H_BX]				;BX
	MOV		[CS:Tasks + BX + TT_BX], AX
	MOV		AX, [CS:INT24H_CX]				;CX
	MOV		[CS:Tasks + BX + TT_CX], AX
	MOV		AX, [CS:INT24H_DX]				;DX
	MOV		[CS:Tasks + BX + TT_DX], AX
	MOV		AX, [CS:INT24H_SI]				;SI
	MOV		[CS:Tasks + BX + TT_SI], AX
	MOV		AX, [CS:INT24H_DI]				;DI
	MOV		[CS:Tasks + BX + TT_DI], AX
	MOV		AX, [CS:INT24H_BP]				;BP
	MOV		[CS:Tasks + BX + TT_BP], AX
	MOV		AX, [CS:INT24H_SP]				;SP
	MOV		[CS:Tasks + BX + TT_SP], AX
	MOV		AX, [CS:INT24H_DS]				;DS
	MOV		[CS:Tasks + BX + TT_DS], AX
	MOV		AX, [CS:INT24H_SS]				;SS
	MOV		[CS:Tasks + BX + TT_SS], AX
	MOV		AX, [CS:INT24H_ES]				;ES
	MOV		[CS:Tasks + BX + TT_ES], AX
	
	;恢复下一个任务的部分现场
	MOV		AX, CX
	MOV		[CS:CurrentTaskID], AL
	MOV		BL, TaskTableItemLen
	MUL		BL
	MOV		BX, AX
	
	MOV		CX, [CS:Tasks + BX + TT_CX]	;CX
	MOV		DX, [CS:Tasks + BX + TT_DX]	;DX
	MOV		SI, [CS:Tasks + BX + TT_SI]	;SI
	MOV		DI, [CS:Tasks + BX + TT_DI]	;DI
	MOV		ES, [CS:Tasks + BX + TT_ES]	;ES
	
	MOV		SP, BP
	POP		BP
	
	;切换为下一个任务的堆栈
	MOV		SP, [CS:Tasks + BX + TT_SP]	;SP
	MOV		SS, [CS:Tasks + BX + TT_SS]	;SS
	
	;准备返回地址和Flags
	MOV		AX, [CS:Tasks + BX + TT_Flags]	;Flags
	PUSH	AX
	MOV		AX, [CS:Tasks + BX + TT_CS]		;CS
	PUSH	AX
	MOV		AX, [CS:Tasks + BX + TT_IP]		;IP
	PUSH	AX
	
	MOV		AX, [CS:Tasks + BX + TT_AX]	;AX
	MOV		BP, [CS:Tasks + BX + TT_BP]	;BP
	MOV		DS, [CS:Tasks + BX + TT_DS]	;DS
	MOV		BX, [CS:Tasks + BX + TT_BX]	;BX
	
	PUSH	AX
	MOV		AL, 20H
	OUT		20H, AL
	POP		AX
	
	IRET
EndProc		INT24H

;*------------------------------------------------------*
;|多任务应用程序文件格式								|
;|														|
;|	+---------------+									|
;|	|	 'MTA16'	|	0 ~ 4							|
;|	+---------------+									|
;|	| Start Address	|	5 ~ 6							|
;|	+---------------+									|
;|	|    Argument	|	7 ~ 134							|
;|	+---------------+									|
;|	|    Reserve	|	135 ~ 154						|
;|	+---------------+									|
;|	|     User      |	155 ~ 255						|
;|	+---------------+									|
;|														|
;|描述:													|
;|	'MTA16':											|
;|		多任务应用程序文件识别字符串.					|
;|														|
;|	Start Address:										|
;|		开始执行地址.									|
;|														|
;|	Argument:											|
;|		参数.											|
;|														|
;|	Reserve:											|
;|		保留.											|
;|														|
;|	User:												|
;|		用户空间.										|
;*------------------------------------------------------*

MTA16FileFormat_Check			EQU 0
MTA16FileFormat_StartAddress	EQU 5
MAT16FileFormat_Reserve			EQU 6

Load_ES	DW ?
Load_DI	DW ?

;过程名:	Load
;功能:		装载程序
;参数:		DS:SI=文件名
;			ES:DI=参数
;返回值:	如果CF为0则成功, 否则失败
Procedure	Load
	PUSH	DI
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	BP
	PUSH	ES
	PUSH	DS
	
	MOV		[CS:Load_ES], ES
	MOV		[CS:Load_DI], DI
	
	PUSH	CS
	POP		ES
	
	;搜索可用的任务槽
	MOV		CX, MaxTaskCount
	XOR		BX, BX	;用于保存任务表的偏移
	XOR		BP, BP	;用于保存任务ID
	
Load_Loop:

	CMP		BYTE [CS:Tasks + BX + TT_Used], 0
	JNE		Load_Used	;是否使用?
	;复制文件名
	MOV		DI, Tasks
	ADD		DI, BX
	ADD		DI, TT_FileName
	MOV		CX, 20
	PUSH	SI
	CLD
	
Load_CopyFileName:

	LODSB
	STOSB
	LOOP	Load_CopyFileName
	POP		SI
	
	;设置Flags, CS和IP
	MOV		AX, TaskOffAddr
	MUL		BP
	ADD		AX, TaskSegAddr
	
	MOV		[CS:Tasks + BX + TT_CS], AX
	PUSH	AX
	PUSHF
	POP		AX
	OR		AX, 00000010_00000000B	;IF = 1
	MOV		[CS:Tasks + BX + TT_Flags], AX
	POP		AX
	
	;装载程序
	PUSH	AX
	PUSH	CX
	MOV		ES, AX
	MOV		DI, 0
	MOV		AH, 13
	INT		21H
	OR		AL, AL
	JZ		Load_NoErr
	POP		CX
	POP		AX
	STC
	JMP		Load_End
	
Load_NoErr:

	POP		CX
	POP		AX
	
	INC		BYTE [CS:TaskCount]
	
	PUSH	DS
	PUSH	AX
	MOV		DS, AX
	
	CMP		BYTE [MTA16FileFormat_Check + 0], 'M'
	JNE		Load_NoMTA16
	CMP		BYTE [MTA16FileFormat_Check + 1], 'T'
	JNE		Load_NoMTA16
	CMP		BYTE [MTA16FileFormat_Check + 2], 'A'
	JNE		Load_NoMTA16
	CMP		BYTE [MTA16FileFormat_Check + 3], '1'
	JNE		Load_NoMTA16
	CMP		BYTE [MTA16FileFormat_Check + 4], '6'
	JNE		Load_NoMTA16
	
	PUSH	ES
	PUSH	DS
	PUSH	DI
	PUSH	SI
	
	MOV		ES, AX
	MOV		DI, 7
	MOV		DS, [CS:Load_ES]
	MOV		SI, [CS:Load_DI]
	CLD
	
Load_CopyArgument:

	LODSB
	STOSB
	OR		AL, AL
	JZ		Load_CopyEnd
	
	JMP		Load_CopyArgument
	
Load_CopyEnd:
	
	POP		SI
	POP		DI
	POP		DS
	POP		ES
	
	MOV		AX, [MTA16FileFormat_StartAddress]
	MOV		WORD [CS:Tasks + BX + TT_IP], AX	;设置起始地址
	POP		AX
	POP		DS
	MOV		BYTE [CS:Tasks + BX + TT_Used], 1	;设置为使用
	
	CLC
	JMP		Load_End
	
Load_Used:	;已经被使用

	ADD		BX, TaskTableItemLen	;任务表偏移指针指向下一个位置
	INC		BP						;任务ID递增
	DEC		CX
	OR		CX, CX
	JNZ		Load_Loop
	STC
	JMP		Load_End
	
Load_NoMTA16:
	
	STC
	POP		AX
	POP		DS
	
Load_End:	;结束

	POP		DS
	POP		ES 
	POP		BP
	POP		DX
	POP		CX
	POP		BX
	POP		AX
	POP		DI
	
	RET
EndProc		Load

;过程名:	KillTask
;功能:		杀死任务
;参数:		AL=任务ID
;返回值:	如果CF为0则成功, 否则失败
Procedure	KillTask
	PUSH	BX
	CMP		AL, MaxTaskCount		;检测任务ID是否合法?
	JL		KillTask_ValidTaskID
	STC
	JMP		KillTask_End	;不合法
	
KillTask_ValidTaskID:	;合法的任务ID
	
	;计算任务在任务表内的偏移
	MOV		BL, TaskTableItemLen
	MUL		BL	
	MOV		BX, AX
	
	;检测任务槽是否被使用?
	CMP		BYTE [CS:Tasks + BX + TT_Used], 0
	JNE		KillTask_Has
	STC
	JMP		KillTask_End
	
KillTask_Has:	;任务槽被使用

	MOV		BYTE [CS:Tasks + BX + TT_Used], 0	;杀死任务
	DEC		BYTE [CS:TaskCount]
	
	CLC

KillTask_End:
	
	POP		BX
	STI
	
	RET
EndProc		KillTask
	
;过程名:	ReturnSystem
;功能:		返回系统
;参数:		无
;返回值:	无
Procedure	ReturnSystem
	MOV		AH, 28
	INT		21H
	ResumeINT23H
	ResumeINT24H
	;返回系统
	MOV		AH, 0
	INT		21H
EndProc		ReturnSystem

;过程名:	GetTaskSlotState
;功能:		获取任务槽是否被使用
;参数:		AL=任务号
;返回值:	AL为0时被使用, 否则没有被使用
Procedure	GetTaskSlotState
	PUSHF
	PUSH	CX
	MOV		CH, AH
	PUSH	BX
	XOR		AH, AH
	MOV		BL, TaskTableItemLen
	MUL		BL
	MOV		BX, AX
	MOV		AL, [CS:Tasks + BX + TT_Used]
	POP		BX
	MOV		AH, CH
	POP		CX
	POPF
	RET
EndProc		GetTaskSlotState

;过程名:	GetAppName
;功能:		获取任务的程序名
;参数:		AL=任务号
;返回值:	ES:DI=程序名
Procedure	GetAppName
	PUSHF
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	AX
	PUSH	CX
	MOV		AH, TaskTableItemLen
	MUL		AH
	MOV		SI, AX
	ADD		SI, TT_FileName
	ADD		SI, Tasks
	PUSH	CS
	POP		DS
	CLD
	MOV		CX, 20
	REP MOVSB
	POP		CX
	POP		AX
	POP		DI
	POP		SI
	POP		DS
	POPF
	RET
EndProc		GetAppName

;过程名:	INT23H
;功能:		中断INT21H处理程序.
;			0:	退出任务管理器
;			1:	创建任务
;			2:	杀死任务
;			3:	打印任务信息列表
;参数:		AH=功能号
;返回值:	
Procedure	INT23H
	CMP		AH, 0
	;
	;功能号:	0
	;描述:		返回系统
	;
	JNE		INT23H_Label1
	JMP		ReturnSystem
	
INT23H_Label1:

	CMP		AH, 1
	;
	;功能号:	1
	;描述:		加载程序
	;参数:		DS:SI=程序名
	;			ES:DI=参数
	;返回值:	AL=0则成功, 否则失败
	;
	JNE		INT23H_Label2
	XOR		AL, AL
	CALL	Load
	JNC		INT23H_Label1_NoErr
	MOV		AL, 1
	
INT23H_Label1_NoErr:
	
	JMP		INT23H_End
	
INT23H_Label2:

	CMP		AH, 2
	;
	;功能号:	2
	;描述:		杀死任务
	;参数:		AL=任务号
	;返回值:	AL=0则成功, 否则失败
	;
	JNE		INT23H_Label3
	CALL	KillTask
	JNC		INT23H_Label2_NoErr
	MOV		AL, 1
	JMP		INT23_Label_Err
	
INT23H_Label2_NoErr:

	XOR		AL, AL
	
INT23_Label_Err:
	
	JMP		INT23H_End
	
INT23H_Label3:

	CMP		AH, 3
	;
	;功能号:	3
	;描述:		获取任务槽是否被使用
	;参数:		AL=任务号
	;返回值:	AL为1被使用, 否则没有被使用
	;
	JNE		INT23H_Label4
	CALL	GetTaskSlotState
	JMP		INT23H_End
	
INT23H_Label4:

	CMP		AH, 4
	;
	;功能号:	4
	;描述:		获取任务的程序名
	;参数:		AL=任务号
	;返回值:	ES:DI=程序名
	;
	JNE		INT23H_Label5
	CALL	GetAppName
	JMP		INT23H_End
	
INT23H_Label5:

	CMP		AH, 5
	;
	;功能号:	5
	;描述:		获取当前的任务号
	;参数:		无
	;返回值:	AL=任务号
	;
	JNE		INT23H_Label6
	MOV		AL, [CS:CurrentTaskID]
	JMP		INT23H_End
	
INT23H_Label6:
	
INT23H_End:	;INT23中断处理程序处理结束
	
	IRET
EndProc		INT23H

;
;程序开始
;
Start:
	
	MOV		AX, CS
	MOV		SS, AX
	MOV		ES, AX
	MOV		DS, AX
	MOV		SP, 0FFFEH
	
	MOV		SI, TaskMgrCfgFileName
	MOV		AH, 14
	INT		21H
	OR		AL, AL
	JNZ		ConfigFileExists
	MOV		SI, LostConfigFileErrMsg
	MOV		AH, 4
	INT		21H
	JMP		NoInitEnd
	
ConfigFileExists:
	
	MOV		AH, 23
	INT		21H
	CMP		CX, 20
	JNA		ConfigNoErr
	MOV		SI, AppNameToLongErrMsg
	MOV		AH, 4
	INT		21H
	JMP		NoInitEnd

ConfigNoErr:

	MOV		DI, StartApp
	MOV		AH, 13
	INT		21H
	
	CALL	Init	;初始化
	
	MOV		DI, StartAppArg
	MOV		SI, StartApp
	CALL	Load
	
	SetINT23H	INT23H
	SetINT24H	INT24H
	
TaskSwitch:
	
	INT		24H
	JMP		TaskSwitch
	
NoInitEnd:
	
	MOV		AH, 0
	INT		21H
	