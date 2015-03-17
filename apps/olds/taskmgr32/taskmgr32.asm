;*------------------------------------------------------*
;|文件名:	taskmgr32.asm								|
;|作者:		Ihsoh										|
;|创建时间:	2013-5-11									|
;|														|
;|概述:													|
;|	任务管理器主文件									|
;*------------------------------------------------------*

ORG		0
BITS 	16
CPU 	386

%INCLUDE	'..\..\common\common.mac'
%INCLUDE	'..\..\common\386.mac'
%INCLUDE	'..\..\common\386.inc'
%INCLUDE	'taskmgr32.inc'
%INCLUDE	'taskmgr32.mac'

SECTION	.bss
	Stack	RESB 512
StackTop:

	ProdStackLen	EQU 512
	ProdStack		RESB ProdStackLen
ProdStackTop:

	INT21HStackLen	EQU 512
	INT21HStack		RESB INT21HStackLen
INT21HStackTop:

	TaskSwitchStackLen	EQU 512
	TaskSwitchStack		RESB TaskSwitchStackLen
TaskSwitchStackTop:

	TempStackLen	EQU 512
	TempStack		RESB TempStackLen
TempStackTop:

SECTION	.data

;全局段描述符表
GDT:
	;空的全局描述符
	Desc	0, 0, 0
	;实模式下的段描述符
	NormalSel		EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATDW | DPL0, 0
	;任务管理器核心所在的段的段描述符(代码段)
	CoreCSel		EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATCE | DPL0 | D32, CoreAddress
	;任务管理器核心所在的段的段描述符(数据段)
	CoreDSel		EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATDW | DPL0, CoreAddress
	;任务管理器所在的段的段描述符(代码段)
	Taskmgr32CSel	EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATCE | DPL0 | D32, Taskmgr32Address
	;任务管理器所在的段的段描述符(数据段)
	Taskmgr32DSel	EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATDW | DPL0, Taskmgr32Address
	;任务管理器所在的段的段描述符(代码段, 16位)
	Taskmgr32C16Sel	EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATCE | DPL0, Taskmgr32Address
	;保护模式下的堆栈
	ProdStackSel	EQU ($ - GDT) | RPL0
	ProdStackDesc:	Desc	ProdStackLen, ATDW | DPL0 | D32, 0
	;缓冲区所在的段的段描述符(数据段)
	BufferSel		EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATDW | DPL0, (BufferSeg << 4) + BufferOff
	;INT21H中断处理过程的堆栈描述符
	INT21HStackSel	EQU ($ - GDT) | RPL0
	INT21HStackDesc:	Desc	INT21HStackLen - 1, ATDW | DPL0 | D32, 0
	;任务管理器任务状态段描述符
	Taskmgr32TSSSel	EQU ($ - GDT) | RPL0
	Taskmgr32TSSDesc:	Desc	Taskmgr32TSSLength - 1, AT386TSS | DPL0, 0
	;INT21H中断处理程序任务状态段描述符
	INT21HTSSSel	EQU ($ - GDT) | RPL0
	INT21HTSSDesc:	Desc	INT21HTSSLength - 1, AT386TSS | DPL0, 0
	;INT21H中断程序任务门
	INT21HTSSGateSel	EQU ($ - GDT) | RPL0
	Gate	0, INT21HTSSSel, 0, ATTaskGate
	;任务切换过程的堆栈描述符
	TaskSwitchStackSel	EQU ($ - GDT) | RPL0
	TaskSwitchStackDesc:	Desc	TaskSwitchStackLen - 1, ATDW | DPL0 | D32, 0
	;任务切换程序任务状态描述符
	TaskSwitchTSSSel		EQU ($ - GDT) | RPL0
	TaskSwitchTSSDesc:	Desc TaskSwitchTSSLength - 1, AT386TSS | DPL0, 0
	;公共数据段描述符, 0H ~ 18FFF000H
	CommonDataSel	EQU ($ - GDT) | RPL0
	CommonDataDesc:	Desc 18FFFH, ATDW | G | DPL0, 0
	
	FirstTaskSel	EQU	($ - GDT) | RPL0
TaskDescs:
	%REP	MaxTaskCount
		Desc	105, AT386TSS | DPL0, 0
	%ENDREP
	
	FirstTaskStackSel	EQU ($ - GDT) | RPL0
TaskStackDescs:
	%REP	MaxTaskCount
		;Desc	0FFFFFH
	%ENDREP
	
	GDTLength		EQU $ - GDT
	
;中断描述符表
IDT:
	%REP	(0CH - 0H) + 1
		Gate	0, Taskmgr32CSel, 0, AT386TGate
	%ENDREP
	;INT 0DH
	GlobalProtectedGate:	Gate	0, Taskmgr32CSel, 0, AT386TGate
	%REP	(20H - 0EH) + 1
		Gate	0, Taskmgr32CSel, 0, AT386TGate
	%ENDREP
	;INT 21H
	TempINT21HGate:	Gate	0, Taskmgr32CSel, 0, AT386TGate
	%REP	(3FH - 22H) + 1
		Gate	0, Taskmgr32CSel, 0, AT386TGate
	%ENDREP
	;INT 40H
	Gate	0, TaskSwitchTSSSel, 0, ATTaskGate
	%REP	(0FFH - 41H) + 1
		Gate	0, Taskmgr32CSel, 0, AT386TGate
	%ENDREP
	IDTLength		EQU $ - IDT
	
;任务管理器的任务状态段
Taskmgr32TSS:
	TSS
	DB	0FFH
	
	Taskmgr32TSSLength	EQU $ - Taskmgr32TSS
	
;任务切换过程的任务状态
TaskSwitchTSS:
	DW	0, 0					;Link
	DD	TaskSwitchStackLen		;ESP0
	DW	TaskSwitchStackSel, 0	;SS0
	DD	0						;ESP1
	DW	0, 0					;SS1
	DD	0						;ESP2
	DW	0, 0					;SS2
	DD	0						;CR3
	DD	TaskSwitch				;EIP
	DD	0						;Flags
	DD	0						;EAX
	DD	0						;ECX
	DD	0						;EDX
	DD	0						;EBX
	DD	TaskSwitchStackLen		;ESP
	DD	0						;EBP
	DD	0						;ESI
	DD	0						;EDI
	DW	Taskmgr32DSel, 0		;ES
	DW	Taskmgr32CSel, 0		;CS
	DW	TaskSwitchStackSel, 0	;SS
	DW	Taskmgr32DSel, 0		;DS
	DW	Taskmgr32DSel, 0		;FS
	DW	Taskmgr32DSel, 0		;GS
	DW	0, 0					;LDT
	DW	0						;T
	DW	$ + 2					;IO Map
	DB	0FFH
	
	TaskSwitchTSSLength	EQU $ - TaskSwitchTSS
	
;INT21H的任务状态段
INT21HTSS:
	DW	0, 0				;Link
	DD	INT21HStackLen		;ESP0
	DW	INT21HStackSel, 0	;SS0
	DD	0					;ESP1
	DW	0, 0				;SS1
	DD	0					;ESP2
	DW	0, 0				;SS2
	DD	0					;CR3
	DD	INT21H				;EIP
	DD	0					;Flags
	DD	0					;EAX
	DD	0					;ECX
	DD	0					;EDX
	DD	0					;EBX
	DD	INT21HStackLen		;ESP
	DD	0					;EBP
	DD	0					;ESI
	DD	0					;EDI
	DW	NormalSel, 0		;ES
	DW	Taskmgr32CSel, 0	;CS
	DW	INT21HStackSel, 0	;SS
	DW	NormalSel, 0		;DS
	DW	NormalSel, 0		;FS
	DW	NormalSel, 0		;GS
	DW	0, 0				;LDT
	DW	0					;T
	DW	$ + 2				;IO Map
	DB	0FFH
	
	INT21HTSSLength	EQU $ - INT21HTSS

TaskTSSs:
	%REP	MaxTaskCount
		TSS
		DB	0FFH
	%ENDREP
	
	CurrentTaskID	DW 0					;当前任务ID
	UsedFlags:		TIMES MaxTaskCount DB 0	;是否使用?
	
	RealMode_SS		DW 0
	RealMode_DS		DW 0
	RealMode_ES		DW 0
	RealMode_FS		DW 0
	RealMode_GS		DW 0
	RealMode_EAX	DD 0
	RealMode_EBX	DD 0
	RealMode_ECX	DD 0
	RealMode_EDX	DD 0
	RealMode_ESI	DD 0
	RealMode_EDI	DD 0
	RealMode_ESP	DD 0
	RealMode_Flags	DD 0
	
	ProdMode_SS		DW 0
	ProdMode_DS		DW 0
	ProdMode_ES		DW 0
	ProdMode_FS		DW 0
	ProdMode_GS		DW 0
	ProdMode_EAX	DD 0
	ProdMode_EBX	DD 0
	ProdMode_ECX	DD 0
	ProdMode_EDX	DD 0
	ProdMode_ESI	DD 0
	ProdMode_EDI	DD 0
	ProdMode_ESP	DD 0
	ProdMode_Flags	DD 0
	
	; GP_SS		DW 0
	; GP_DS		DW 0
	; GP_ES		DW 0
	; GP_FS		DW 0
	; GP_GS		DW 0
	; GP_EAX		DD 0
	; GP_EBX		DD 0
	; GP_ECX		DD 0
	; GP_EDX		DD 0
	; GP_ESI		DD 0
	; GP_EDI		DD 0
	; GP_ESP		DD 0
	; GP_Flags	DD 0		
	
	;全局段描述符表虚描述符
	VGDTR:	VDesc	GDTLength - 1, GDT + Taskmgr32Address

	;中断描述符表虚描述符
	VIDTR:	VDesc	1024, IDT + Taskmgr32Address
	
	;实模式下的中断描述符表虚描述符
	RealVIDTR:	VDesc	0, 0

	UnvalidInterruptMsg	DB 'Taskmgr32 error: Unvalid interrupt!', 13, 10, 0

;任务管理器核心文件名
Taskmgr32c	DB 'taskmgr32c'
			TIMES 20 - ($ - Taskmgr32c) DB 0
			
;错误信息
ErrMsg1		DB "Missing 'taskmgr32c'!", 13, 10, 0

SECTION	.text

	MOV		AX, CS
	MOV		SS, AX
	MOV		DS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		SP, StackTop
	
	CLI
	CALL	LoadGDT
	CALL	EnableA20
	CALL	LoadTaskmgr32Core
	AND		ECX, 0FFFFH
	CALL	LoadIDT
	
	;储存实模式下各个寄存器的值
	MOV		[RealMode_SS], SS
	MOV		[RealMode_DS], DS
	MOV		[RealMode_ES], ES
	MOV		[RealMode_FS], FS
	MOV		[RealMode_GS], GS
	MOV		[RealMode_EAX], EAX
	MOV		[RealMode_EBX], EBX
	MOV		[RealMode_ECX], ECX
	MOV		[RealMode_EDX], EDX
	MOV		[RealMode_ESI], ESI
	MOV		[RealMode_EDI], EDI
	MOV		[RealMode_ESP], ESP
	MOV		WORD [RealMode_ESP + 2], 0
	PUSHFD
	POP		DWORD [RealMode_Flags]
	
	MOV		EAX, CR0
	OR		EAX, 1
	MOV		CR0, EAX
	
	Jump16	Taskmgr32CSel, Prod
	
Prod:
	BITS	32
	CALL	Set8259A
	
	;装载任务管理器核心所使用的段描述符选择子
	MOV		AX, CoreDSel
	MOV		DS, AX
	MOV		ES, AX
	MOV		GS, AX
	MOV		FS, AX
	
	;装载保护模式的堆栈
	MOV		AX, ProdStackSel
	MOV		SS, AX
	MOV		ESP, ProdStackLen	
	
	;装载任务状态段
	MOV		AX, Taskmgr32TSSSel
	LTR		AX
	
	;复制任务管理器核心
	CALL	CopyTaskmgr32Core
	
	;跳入任务管理器核心代码
	STI
	JMP		$
	;Jump32	CoreCSel, 0
	
	BITS	16
	
	;过程名:	LoadGDT
	;描述:		加载全局描述符表
	;参数:		无
	;返回值:	无
	Procedure	LoadGDT
		PUSHF
		PUSH	EAX
		PUSH	EBX
		PUSH	ECX
		PUSH	EDX
		Desc_Set_Base	CS, ProdStackDesc, Taskmgr32Address + ProdStack
		Desc_Set_Base	CS, INT21HStackDesc, Taskmgr32Address + INT21HStack
		Desc_Set_Base	CS, Taskmgr32TSSDesc, Taskmgr32Address + Taskmgr32TSS
		Desc_Set_Base	CS, INT21HTSSDesc, Taskmgr32Address + INT21HTSS
		Desc_Set_Base	CS, TaskSwitchStackDesc, Taskmgr32Address + TaskSwitchStack
		Desc_Set_Base	CS, TaskSwitchTSSDesc, Taskmgr32Address + TaskSwitchTSS
		
		;初始化任务状态段段描述符
		MOV		CX, MaxTaskCount
		MOV		EDX, Taskmgr32Address + TaskTSSs
		MOV		EBX, TaskDescs
LoadGDT_Loop:
		Desc_Set_Base	CS, EBX, EDX
		ADD		EBX,  8
		ADD		EDX, 105
		LOOP	LoadGDT_Loop
		
		LGDT	[VGDTR]
		POP		EDX
		POP		ECX
		POP		EBX
		POP		EAX
		POPF
		RET
	EndProc		LoadGDT
	
	;过程名:	EnableA20
	;描述:		开A20地址线
	;参数:		无
	;返回值:	无
	Procedure	EnableA20
		PUSHF
		PUSH	AX
		IN		AL, 92H
		OR		AL, 2
		OUT		92H, AL
		POP		AX
		POPF
		RET
	EndProc		EnableA20
	
	;过程名:	LoadTaskgr32Core
	;描述:		加载任务管理器核心
	;参数:		无
	;返回值:	无
	Procedure	LoadTaskmgr32Core
		PUSHF
		PUSH	ES
		PUSH	DS
		PUSH	SI
		PUSH	DI
		PUSH	AX
		
		MOV		AX, CS
		MOV		DS, AX
		MOV		SI, Taskmgr32c
		MOV		AX, BufferSeg
		MOV		ES, AX
		MOV		DI, BufferOff
		MOV		AH, 13
		INT		21H
		OR		AL, AL
		JZ		LoadTaskmgr32Core_NoErr
		MOV		SI, ErrMsg1
		MOV		AH, 4
		INT		21H
		MOV		AH, 0
		INT		21H
		
LoadTaskmgr32Core_NoErr:
		
		POP		AX
		POP		DI
		POP		SI
		POP		DS
		POP		ES
		POPF
		RET
	EndProc		LoadTaskmgr32Core
	
	;过程名:	CopyTaskmgr32Core
	;描述:		从常规内存复制任务管理器核心到扩展内存
	;参数:		ECX=长度
	;返回值:	无
	Procedure	CopyTaskmgr32Core
		BITS	32
		PUSHF
		PUSH	ES
		PUSH	DS
		PUSH	EDI
		PUSH	ESI
		
		MOV		AX, BufferSel
		MOV		DS, AX
		MOV		ESI, BufferOff
		MOV		AX, CoreDSel
		MOV		ES, AX
		XOR		EDI, EDI
		CLD
		
		REP	MOVSB
		
		POP		ESI
		POP		EDI
		POP		DS
		POP		ES
		POPF
		RET
		BITS	16
	EndProc		CopyTaskmgr32Core
	
	;过程名:	LoadIDT
	;描述:		加载中断描述符表
	;参数:		无
	;返回值:	无
	Procedure	LoadIDT
		PUSHF
		PUSH	EAX
		PUSH	CX
		PUSH	SI
		
		SIDT	[CS:RealVIDTR]
		
		MOV		EAX, UnvalidInterrupt
		MOV		SI, IDT
		MOV		CX, 100H
		
LoadIDT_Label1:

		Gate_Set_Offset	CS, SI, EAX
		ADD		SI, Gate_SLength
		LOOP	LoadIDT_Label1
		
		Gate_Set_Offset	CS, TempINT21HGate, TempINT21H
		; Gate_Set_Offset	CS, GlobalProtectedGate, GlobalProtected
		Gate_Set_Offset	CS, GlobalProtectedGate, UnvalidInterrupt
		
		LIDT	[CS:VIDTR]
		POP		SI
		POP		CX
		POP		EAX
		POPF
		RET
	EndProc		LoadIDT
	
	;过程名:	Set8259A
	;描述:		设置8259A
	;参数:		无
	;返回值:	无
	Procedure	Set8259A
		BITS	32
		
		PUSHF
		PUSH	AX
		
		;主8259A
		;ICW1
		MOV		AL, 0001_0001B	;边沿触发, 级联, 需要写ICW4
		OUT		20H, AL
		;ICW2
		MOV		AL, 0100_0000B	;40H
		OUT		21H, AL
		;ICW3
		MOV		AL, 0000_0100B	;主8259A的IRQ2接从8259A
		OUT		21H, AL
		;ICW4
		MOV		AL, 0001_0001B	;特殊完全嵌套, 非缓冲, 自动结束
		OUT		21H, AL
		
		;从8259A
		;ICW1
		MOV		AL, 0001_0001B	;边沿触发, 级联, 需要写ICW4
		OUT		0A0H, AL
		;ICW2
		MOV		AL, 0111_0000B	;70H
		OUT		0A1H, AL
		;ICW3
		MOV		AL, 0000_0010B	;接主8259A的IRQ2
		OUT		0A1H, AL
		;ICW4
		MOV		AL, 0000_0001B	;特殊完全嵌套, 非缓冲, 非自动结束
		OUT		0A1H, AL
		
		POP		AX
		POPF
		RET
		
		BITS	16
	EndProc		Set8259A
	
	;过程名:	Resume8259A
	;描述:		恢复8259A的设置
	;参数:		无
	;返回值:	无
	Procedure	Resume8259A
		BITS	32
		PUSHF
		PUSH	AX
		
		;主8259A
		;ICW1
		MOV		AL, 0001_0001B	;边沿触发, 级联, 需要写ICW4
		OUT		20H, AL
		;ICW2
		MOV		AL, 0000_1000B	;8H
		OUT		21H, AL
		;ICW3
		MOV		AL, 0000_0100B	;主8259A的IRQ2接从8259A
		OUT		21H, AL
		;ICW4
		MOV		AL, 0001_0001B	;特殊完全嵌套, 非缓冲, 自动结束
		OUT		21H, AL
		
		;从8259A
		;ICW1
		MOV		AL, 0001_0001B	;边沿触发, 级联, 需要写ICW4
		OUT		0A0H, AL
		;ICW2
		MOV		AL, 0111_0000B	;70H
		OUT		0A1H, AL
		;ICW3
		MOV		AL, 0000_0010B	;接主8259A的IRQ2
		OUT		0A1H, AL
		;ICW4
		MOV		AL, 0000_0001B	;特殊完全嵌套, 非缓冲, 非自动结束
		OUT		0A1H, AL
		
		POP		AX
		POPF
		BITS	16
		RET
	EndProc		Resume8259A
	
	;过程名:	AddTask
	;描述:		添加任务
	;参数:		DS:ESI=程序名选择子:偏移地址
	;返回值:	CF=1失败, 否则成功
	;			AX=任务ID
	Procedure	AddTask
		BITS	32
		PUSHAD
		
		;检测程序是否存在
		MOV		AH, 14
		INT		21H
		OR		AL, AL
		JNZ		AddTask_Err
		
		;获取空闲的任务槽
		XOR		SI, SI
		MOV		AX, Taskmgr32DSel
		MOV		DS, AX
		MOV		ECX, MaxTaskCount

AddTask_Loop:
		
		CMP		BYTE [UsedFlags + SI], 0
		JE		AddTask_FoundID
		INC		SI
		LOOP	AddTask_Loop
		JMP		AddTask_Err
		
AddTask_FoundID:
	
		;装载程序
		MOV		BYTE [UsedFlags + SI], 1
		XOR		EAX, EAX
		MOV		AX, SI
		MOV		BX, TaskMemorySize
		MUL		BX
		SHL		EDX, 16
		OR		EAX, EDX
		ADD		EAX, 100000H
		MOV		EDI, EAX
		MOV		AX, CommonDataSel
		MOV		DS, AX
		MOV		AH, 13
		INT		21H
		OR		AL, AL
		JNZ		AddTask_Err
		
		;设置TSS
		
		
AddTask_Err:

		STC
		
AddTask_End:
		
		POPAD
		RET
		BITS	16
	EndProc		AddTask
	
	;过程名:	UnvalidInterrupt
	;描述:		处理无效的中断
	;参数:		无
	;返回值:	无
	Procedure	UnvalidInterrupt
		BITS	32
		
		MOV		AX, Taskmgr32DSel
		MOV		DS, AX
		
		MOV		[ProdMode_SS], SS
		MOV		[ProdMode_ESP], ESP
		PUSHFD
		POP		DWORD [ProdMode_Flags]
		
		MOV		AX, NormalSel
		MOV		DS, AX
		MOV		SS, AX
		
		Jump32	Taskmgr32C16Sel, UnvalidInterrupt_Prod16
		
UnvalidInterrupt_Prod16:
		BITS	16
		
		MOV		EAX, CR0
		AND		EAX, 0FFFFFFFEH
		MOV		CR0, EAX
		
		Jump16	Taskmgr32Seg, UnvalidInterrupt_Real

		
UnvalidInterrupt_Real:

		MOV		AX, Taskmgr32Seg
		MOV		DS, AX

		LIDT	[RealVIDTR]
		
		MOV		SS, [RealMode_SS]
		MOV		ESP, [RealMode_ESP]

		MOV		AH, 2
		INT		21H
		MOV		AH, 3
		INT		21H
		
		MOV		AX, Taskmgr32Seg
		MOV		DS, AX
		MOV		SI, UnvalidInterruptMsg
		MOV		AH, 4
		INT		21H

		MOV		AH, 2
		INT		21H
		MOV		AH, 3
		INT		21H
		
		LIDT	[VIDTR]
		
		MOV		EAX, CR0
		OR		EAX, 1
		MOV		CR0, EAX
		
		Jump16	Taskmgr32CSel, UnvalidInterrupt_Prod
		
		BITS	32
UnvalidInterrupt_Prod:
		
		MOV		AX, Taskmgr32DSel
		MOV		DS, AX
		
		MOV		SS, [ProdMode_SS]
		MOV		ESP, [ProdMode_ESP]
		PUSH	DWORD [ProdMode_Flags]
		POPFD
		
		HLT
		BITS	16
	EndProc		UnvalidInterrupt

	;过程名:	GlobalProtected
	;描述:		通用保护异常处理过程
	;参数:		无
	;返回值:	无
	; Procedure	GlobalProtected
		; BITS	32
		
	
		; BITS	16
	; EndProc		GlobalProtected
	
	%INCLUDE	'int21h.inc'
	%INCLUDE	'taskswitch.inc'
	