;*------------------------------------------------------*
;|文件名:	em.asm										|
;|作者:		Ihsoh										|
;|创建时间:	2013-7-22									|
;|														|
;|描述:													|
;|	提供在实模式下访问扩展内存的功能					|
;*------------------------------------------------------*

ORG		0
BITS	16
CPU		386

%INCLUDE	'em.mac'
%INCLUDE	'..\..\common\common.mac'
%INCLUDE	'..\..\common\common.inc'
%INCLUDE	'..\..\common\386.mac'
%INCLUDE	'..\..\common\386.inc'

SECTION	.bss
	RMStackLen	EQU 512
	RMStack		RESB RMStackLen
RMStackTop:

	PMStackLen	EQU 512
	PMStack		RESB PMStackLen
PMStackTop:

	TotalBlockCount	DW 0	;块的总数

SECTION	.data

	;进入实模式后把所有的寄存器的值存到下面的变量
	RealMode_SS		DW ?
	RealMode_DS		DW ?
	RealMode_ES		DW ?
	RealMode_FS		DW ?
	RealMode_GS		DW ?
	RealMode_EAX	DD ?
	RealMode_EBX	DD ?
	RealMode_ECX	DD ?
	RealMode_EDX	DD ?
	RealMode_ESI	DD ?
	RealMode_EDI	DD ?
	RealMode_ESP	DD ?
	RealMode_Flags	DD ?
	
	;WriteByte过程用于在实模式和保护模式交互的变量
	WriteByte_BlockID	DW	?
	WriteByte_Offset	DW	?
	WriteByte_Byte		DB	?
	
	;ReadByte过程用于在实模式和保护模式交互的变量
	ReadByte_BlockID	DW	?
	ReadByte_Offset		DW	?
	ReadByte_Byte		DB	?
	
	;WriteBytes过程用于在实模式和保护模式交互的变量
	WriteBytes_BlockID	DW	?
	WriteBytes_Offset	DW	?
	WriteBytes_Length	DW	?
	WriteBytes_Buffer:	TIMES 1024 DB ?
	
	;ReadBytes过程用于在实模式和保护模式交互的变量
	ReadBytes_BlockID	DW	?
	ReadBytes_Offset	DW	?
	ReadBytes_Length	DW	?
	ReadBytes_Buffer:	TIMES 1024 DB ?
	
	FastMode	DB 0
	
	FastWriteByte_BlockID	DW	?
	FastWriteByte_Offset	DW	?
	FastWriteByte_Byte		DB	?
	
	FastReadByte_BlockID	DW	?
	FastReadByte_Offset		DW	?
	FastReadByte_Byte		DB	?
	
	FastWriteBytes_BlockID	DW	?
	FastWriteBytes_Offset	DW	?
	FastWriteBytes_Length	DW	?
	FastWriteBytes_ES		DW 	?
	FastWriteBytes_DI		DW	?

	FastReadBytes_BlockID	DW	?
	FastReadBytes_Offset	DW	?
	FastReadBytes_Length	DW	?
	FastReadBytes_ES		DW	?
	FastReadBytes_DI		DW	?
	
	;记录所有块的使用情况
Blocks:
	TIMES	((1024 / 64) / 8) DB 0FFH	;块0 ~ 1023为常规内存的块, 始终被占用
	TIMES	(65536 / 8) - ((1024 / 64) / 8)  DB 0

;全局描述符表
GDT:
	;空的全局描述符
	Desc	0, 0, 0
	;实模式下的数据段
	NormalSel	EQU ($ - GDT) | RPL0
	Desc	0FFFFH, ATDW | RPL0, 0
	;代码段
	CodeSel		EQU ($ - GDT) | RPL0
	CodeDesc:	Desc	0FFFFH, ATCE | DPL0 | D32, ?
	;16位代码段
	Code16Sel	EQU ($ - GDT) | RPL0
	Code16Desc:	Desc	0FFFFH, ATCE | DPL0, ?
	;数据段
	DataSel		EQU ($ - GDT) | RPL0
	DataDesc:	Desc	0FFFFH, ATDW | DPL0 | D32, ?
	;4G内存空间
	MemorySel	EQU ($ - GDT) | RPL0
	Desc	0FFFFFH, ATDW | DPL0 | D32 | G, 0
	;堆栈段
	StackSel	EQU ($ - GDT) | RPL0
	StackDesc:	Desc	RMStackLen, ATDW | RPL0 | D32, ?
	GDTLength	EQU $ - GDT
	
	;全局段描述符表虚描述符
	VGDTR:	VDesc	GDTLength - 1, ?
	
SECTION .text
	
Header:

	DB	'MTA16'
	DW	Start
	TIMES 256 - ($ - Header) DB 0

Start:
	
	MOV		AX, CS
	MOV		DS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		SS, AX
	MOV		SP, PMStackTop
	
	CALL	GetTotalBlockCount
	
	CLI
	
	;设置全局描述符表
	MOVZX	EAX, AX
	SHL		EAX, 4
	Desc_Set_Base	CodeDesc, EAX
	Desc_Set_Base	Code16Desc, EAX
	Desc_Set_Base	DataDesc, EAX
	PUSH	EAX
	ADD		EAX, PMStack
	Desc_Set_Base	StackDesc, EAX
	POP		EAX

	;设置全局描述符表虚描述符
	ADD		EAX, GDT
	MOV		[CS:VGDTR + 2], EAX
	
	MOV		AX, CS
	MOV		DS, AX
	MOV		BX, INT25H
	MOV		AL, 25H
	MOV		AH, 17
	INT		21H
	
NextTask:

	INT		24H
	JMP		NextTask
	
	;过程名:	SetBlock
	;描述:		设置块
	;参数:		BX=块ID
	;返回值:	无
	Procedure	SetBlock
		PUSHF
		PUSH	AX
		PUSH	BX
		PUSH	CX
		PUSH	DX
		XOR		DX, DX
		MOV		AX, BX
		MOV		BX, 8
		DIV		BX
		MOV		BX, AX
		MOV		CL, DL
		MOV		AL, 1
		SHL		AL, CL
		OR		[CS:Blocks + BX], AL
		POP		DX
		POP		CX
		POP		BX
		POP		AX
		POPF
		RET
	EndProc		SetBlock
	
	;过程名:	ResetBlock
	;描述:		复位块
	;参数:		BX=块ID
	;返回值:	无
	Procedure	ResetBlock
		PUSHF
		PUSH	AX
		PUSH	BX
		PUSH	CX
		PUSH	DX
		XOR		DX, DX
		MOV		AX, BX
		MOV		BX, 8
		DIV		BX
		MOV		BX, AX
		MOV		CL, DL
		MOV		AL, 1
		SHL		AL, CL
		NOT		AL
		AND		[CS:Blocks + BX], AL
		POP		DX
		POP		CX
		POP		BX
		POP		AX
		POPF
		RET
	EndProc		ResetBlock
	
	;过程名:	GetBlock
	;描述:		获取块
	;参数:		BX块ID
	;返回值:	AL=0复位, AL=1置位
	Procedure	GetBlock
		PUSHF
		PUSH	BX
		PUSH	AX
		PUSH	CX
		PUSH	DX
		XOR		DX, DX
		MOV		AX, BX
		MOV		BX, 8
		DIV		BX
		MOV		BX, AX
		MOV		CL, DL
		MOV		AL, [CS:Blocks + BX]
		SHR		AL, CL
		AND		AL, 1
		MOV		BL, AL
		POP		DX
		POP		CX
		POP		AX
		MOV		AL, BL
		POP		BX
		POPF
		RET
	EndProc		GetBlock
	
	;过程名:	GetBlocks
	;描述:		获取可用的块
	;参数:		CX=块数
	;返回值:	BX=块ID
	;			AL=0成功, 否则失败
	Procedure	GetBlocks
		PUSHF
		
		CMP		[CS:TotalBlockCount], CX
		JB		GetBlocks_NoEnoughBlocks
		SUB		[CS:TotalBlockCount], CX
		
		PUSH	DX
		PUSH	AX
		PUSH	CX
		XOR		BX, BX
		MOV		DX, CX
		
GetBlocks_Label1:

		PUSH	BX
		
GetBlocks_Label2:
		
		CALL	GetBlock
		CMP		AL, 1
		JE		GetBlocks_NextBlock
		INC		BX
		LOOP	GetBlocks_Label2
		POP		BX
		PUSH	BX
		MOV		CX, DX

GetBlocks_Label3:

		CALL	SetBlock
		INC		BX
		LOOP	GetBlocks_Label3
		POP		BX
		MOV		DL, 0
		JMP		GetBlocks_Found
		
GetBlocks_NextBlock:
		
		POP		BX
		INC		BX
		PUSH	BX
		ADD		BX, CX
		JC		GetBlocks_NotFound
		POP		BX
		
		MOV		CX, DX
		JMP		GetBlocks_Label1
		
GetBlocks_NotFound:

		POP		BX
		MOV		DL, 1
		
GetBlocks_Found:
		
		POP		CX
		POP		AX
		MOV		AL, DL
		POP		DX
		
		JMP		GetBlocks_End
		
GetBlocks_NoEnoughBlocks:

		MOV		AL, 1
		
GetBlocks_End:
		
		POPF
		RET
	EndProc		GetBlocks
	
	;过程名:	FreeBlocks
	;描述:		释放块
	;参数:		BX=块ID
	;			CX=块数
	;返回值:	AL=0成功, 否则失败
	Procedure	FreeBlocks
		PUSHF
		PUSH	BX
		PUSH	CX

		ADD		[CS:TotalBlockCount], CX
		
FreeBlocks_Label1:

		CALL	ResetBlock
		INC		BX
		LOOP	FreeBlocks_Label1
		POP		CX
		POP		BX
		POPF
		RET
	EndProc		FreeBlocks
	
	;过程名:	WriteByte
	;描述:		写字节
	;参数:		BX=块ID
	;			SI=块内偏移
	;			AL=数据
	;返回值:	AL=0成功, 否则失败
	Procedure	WriteByte
		PUSHF
		
		MOV		[CS:WriteByte_BlockID], BX
		MOV		[CS:WriteByte_Offset], SI
		MOV		[CS:WriteByte_Byte], AL
		
		;检测是不是写常规内存. 如果是则出错返回
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		WriteByte_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		WriteByte_End
		
		EnterProd	WriteByte_RealSeg
		
		MOV		AX, DataSel
		MOV		DS, AX
		MOV		AX, [DS:WriteByte_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [DS:WriteByte_Offset]
		MOVZX	EBX, BX
		MOV		CX, MemorySel
		MOV		ES, CX
		MOV		CL, [DS:WriteByte_Byte]
		MOV		[ES:EAX + EBX], CL
		
		LeaveProd	WriteByte_RealSeg
		
		XOR		AL, AL
		
WriteByte_End:
		
		POPF
		RET
	EndProc		WriteByte
	
	;过程名:	ReadByte
	;描述:		读字节
	;参数:		BX=块ID
	;			SI=块内偏移
	;返回值:	BL=数据
	;			AL=0成功, 否则失败
	Procedure	ReadByte
		PUSHF
		
		MOV		[CS:ReadByte_BlockID], BX
		MOV		[CS:ReadByte_Offset], SI
		
		;检测是不是写常规内存. 如果是则出错返回
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		ReadByte_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		ReadByte_End
		
		EnterProd	ReadByte_RealSeg
		
		MOV		AX, DataSel
		MOV		DS, AX
		MOV		AX, [DS:ReadByte_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [DS:ReadByte_Offset]
		MOVZX	EBX, BX
		MOV		CX, MemorySel
		MOV		ES, CX
		MOV		AL, [ES:EAX + EBX]
		MOV		[DS:ReadByte_Byte], AL
		
		LeaveProd	ReadByte_RealSeg
		
		MOV		BL, [CS:ReadByte_Byte]
		XOR		AL, AL
		
ReadByte_End:
		
		POPF
		RET
	EndProc		ReadByte
	
	;过程名:	WriteBytes
	;描述:		写字节序列
	;参数:		BX=块ID
	;			SI=块内偏移
	;			ES:DI=数据
	;			CX=数据长度(最大值为1024)
	;返回值:	AL=0成功, 否则失败
	Procedure	WriteBytes
		PUSHF
		PUSH	BX
		
		MOV		[CS:WriteBytes_BlockID], BX
		MOV		[CS:WriteBytes_Offset], SI
		MOV		[CS:WriteBytes_Length], CX
		XOR		AL, AL
		OR		CX, CX
		JZ		WriteBytes_End
		MOV		BX, WriteBytes_Buffer
		
WriteBytes_Label1:
		
		MOV		AL, [ES:DI]
		MOV		[CS:BX], AL
		INC		DI
		INC		BX
		LOOP	WriteBytes_Label1
		
		;检测是不是写常规内存. 如果是则出错返回
		MOV		BX, [CS:WriteBytes_BlockID]
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		WriteBytes_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		WriteBytes_End
		
		;检测写字节序列时有没有超过块的界限. 如果超过则出错返回
		MOV		CX, [CS:WriteBytes_Length]
		ADD		SI, CX
		MOV		AL, 1
		JC		WriteBytes_End
		
		EnterProd	WriteBytes_RealSeg
		
		MOV		AX, DataSel
		MOV		DS, AX
		MOV		AX, [DS:WriteBytes_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [DS:WriteBytes_Offset]
		MOVZX	EBX, BX
		MOV		CX, MemorySel
		MOV		ES, CX
		MOV		CX, [DS:WriteBytes_Length]
		MOVZX	ECX, CX
	
		XOR		ESI, ESI
		
WriteBytes_Label2:

		MOV		DL, [DS:WriteBytes_Buffer + ESI]
		MOV		[ES:EAX + EBX], DL
		INC		EBX
		INC		ESI
		LOOP	WriteBytes_Label2
		
		LeaveProd	WriteBytes_RealSeg
		
		XOR		AL, AL
		
WriteBytes_End:
		
		POP		BX
		POPF
		RET
	EndProc		WriteBytes
	
	;过程名:	ReadBytes
	;描述:		读字节序列
	;参数:		BX=块ID
	;			SI=块内偏移
	;			CX=数据长度(最大值为1024)
	;返回值:	ES:DI=数据
	;			AL=0成功, 否则失败
	Procedure	ReadBytes
		PUSHF
		PUSH	BX
		
		MOV		[CS:ReadBytes_BlockID], BX
		MOV		[CS:ReadBytes_Offset], SI
		MOV		[CS:ReadBytes_Length], CX
		XOR		AL, AL
		OR		CX, CX
		JZ		ReadBytes_End
		MOV		BX, ReadBytes_Buffer
		
		;检测是不是写常规内存. 如果是则出错返回
		MOV		BX, [CS:ReadBytes_BlockID]
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		ReadBytes_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		ReadBytes_End
		
		;检测写字节序列时有没有超过块的界限. 如果超过则出错返回
		MOV		CX, [CS:ReadBytes_Length]
		ADD		SI, CX
		MOV		AL, 1
		JC		ReadBytes_End
		
		EnterProd	ReadBytes_RealSeg
		
		MOV		AX, DataSel
		MOV		DS, AX
		MOV		AX, [DS:ReadBytes_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [DS:ReadBytes_Offset]
		MOVZX	EBX, BX
		MOV		CX, MemorySel
		MOV		ES, CX
		MOV		CX, [DS:ReadBytes_Length]
		MOVZX	ECX, CX
		
		XOR		EDI, EDI
		
ReadByteBytes_Label1:
		
		MOV		DL, [ES:EAX + EBX]
		MOV		[DS:ReadBytes_Buffer + EDI], DL
		INC		EBX
		INC		EDI
		LOOP	ReadByteBytes_Label1
		
		LeaveProd	ReadBytes_RealSeg
		
		PUSH	DS
		PUSH	CS
		POP		DS
		PUSH	SI
		MOV		SI, ReadBytes_Buffer
		CLD
		REP MOVSB
		POP		SI
		POP		DS
		
		XOR		AL, AL
		
ReadBytes_End:
		
		POP		BX
		POPF
		RET
	EndProc		ReadBytes
	
	;过程名:	INT25H
	;描述:		25号中断处理程序
	;参数:		/
	;返回值:	/
	Procedure	INT25H
		CMP		AH, 0
		JNE		INT25H_Label1
		CALL	GetBlocks
		JMP		INT25H_End
		
INT25H_Label1:	

		CMP		AH, 1
		JNE		INT25H_Label2
		CALL	FreeBlocks
		JMP		INT25H_End
		
INT25H_Label2:

		CMP		AH, 2
		JNE		INT25H_Label3
		CMP		BYTE [CS:FastMode], 1
		JE		INT25H_FastWriteByte
		CALL	WriteByte
		JMP		INT25H_NotFastWriteByte
		
INT25H_FastWriteByte:

		CALL	FastWriteByte
		
INT25H_NotFastWriteByte:
		
		JMP		INT25H_End
		
INT25H_Label3:

		CMP		AH, 3
		JNE		INT25H_Label4
		CMP		BYTE [CS:FastMode], 1
		JE		INT25_FastReadByte
		CALL	ReadByte
		JMP		INT25_NotFastReadByte
		
INT25_FastReadByte:

		CALL	FastReadByte

INT25_NotFastReadByte:
		
		JMP		INT25H_End
		
INT25H_Label4:

		CMP		AH, 4
		JNE		INT25H_Label5
		CMP		BYTE [CS:FastMode], 1
		JE		INT25H_FastWriteBytes
		CALL	WriteBytes
		JMP		INT25H_WriteBytes
		
INT25H_FastWriteBytes:

		CALL	FastWriteBytes

INT25H_WriteBytes:
		
		JMP		INT25H_End
		
INT25H_Label5:
		
		CMP		AH, 5
		JNE		INT25H_Label6
		CMP		BYTE [CS:FastMode], 1
		JE		INT25H_FastReadBytes
		CALL	ReadBytes
		JMP		INT25H_ReadBytes
		
INT25H_FastReadBytes:

		CALL	FastReadBytes

INT25H_ReadBytes:
		
		JMP		INT25H_End

INT25H_Label6:
		
		CMP		AH, 6
		JNE		INT25H_Label7
		CALL	InitFastMode
		JMP		INT25H_End
		
INT25H_Label7:

		CMP		AH, 7
		JNE		INT25H_Label8
		MOV		BYTE [CS:FastMode], 0
		JMP		INT25H_End
		
INT25H_Label8:
		
INT25H_End:
	
		IRET
	EndProc		INT25H

	;过程名:	LoadGDT
	;描述:		加载全局描述符表
	;参数:		无
	;返回值:	无
	Procedure	LoadGDT
		PUSHF
		LGDT	[CS:VGDTR]
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
	
	;过程名:	GetTotalBlockCount
	;描述:		获取块的总数
	;参数:		无
	;返回值:	TotalBlockCount=块的总数
	Procedure	GetTotalBlockCount
		PUSHF
		PUSH	AX
		PUSH	BX
		PUSH	CX
		PUSH	DX
		MOV		AX, 0E801H
		INT		15H
		MOV		[CS:TotalBlockCount], BX
		MOV		BL, 64
		DIV		BL
		XOR		AH, AH
		ADD		[CS:TotalBlockCount], AX
		POP		DX
		POP		CX
		POP		BX
		POP		AX
		POPF
		RET
	EndProc		GetTotalBlockCount
	
	;过程名:	InitFastMode
	;描述:		初始化为快速模式
	;参数:		无
	;返回值:	无
	Procedure	InitFastMode
		EnterProd	InitFastMode_RealSeg
		
		MOV		AX, MemorySel
		MOV		FS, AX
		
		LeaveProd	InitFastMode_RealSeg
		
		MOV		BYTE [CS:FastMode], 1
		RET
	EndProc		InitFastMode
	
	;过程名:	FastWriteByte
	;描述:		快速写字节
	;参数:		BX=块ID
	;			SI=块内偏移
	;			AL=数据
	;返回值:	AL=0成功, 否则失败
	Procedure	FastWriteByte
		PUSHF
		MOV		[CS:FastWriteByte_BlockID], BX
		MOV		[CS:FastWriteByte_Offset], SI
		MOV		[CS:FastWriteByte_Byte], AL
		
		;检测是不是写常规内存. 如果是则出错返回
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		FastWriteByte_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		FastWriteByte_End
		
		MOV		AX, [CS:FastWriteByte_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [CS:FastWriteByte_Offset]
		MOVZX	EBX, BX
		MOV		CL, [CS:FastWriteByte_Byte]
		MOV		[FS:EAX + EBX], CL
		
		XOR		AL, AL
		
FastWriteByte_End:
		
		POPF
		RET
	EndProc		FastWriteByte
	
	;过程名:	FastReadByte
	;描述:		快速读字节
	;参数:		BX=块ID
	;			SI=块内偏移
	;返回值:	BL=数据
	;			AL=0成功, 否则失败
	Procedure	FastReadByte
		PUSHF
		
		MOV		[CS:FastReadByte_BlockID], BX
		MOV		[CS:FastReadByte_Offset], SI
		
		;检测是不是写常规内存. 如果是则出错返回
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		FastReadByte_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		FastReadByte_End
		
		MOV		AX, [CS:FastReadByte_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [CS:FastReadByte_Offset]
		MOVZX	EBX, BX
		MOV		AL, [FS:EAX + EBX]
		MOV		[CS:FastReadByte_Byte], AL
		
		MOV		BL, [CS:FastReadByte_Byte]
		XOR		AL, AL
		
FastReadByte_End:
		
		POPF
		RET
	EndProc		FastReadByte
	
	;过程名:	FastWriteBytes
	;描述:		快速写字节序列
	;参数:		BX=块ID
	;			SI=块内偏移
	;			ES:DI=数据
	;			CX=数据长度
	;返回值:	AL=0成功, 否则失败
	Procedure	FastWriteBytes
		PUSHF
		PUSH	BX
		
		MOV		[CS:FastWriteBytes_BlockID], BX
		MOV		[CS:FastWriteBytes_Offset], SI
		MOV		[CS:FastWriteBytes_Length], CX
		MOV		[CS:FastWriteBytes_ES], ES
		MOV		[CS:FastWriteBytes_DI], DI
		XOR		AL, AL
		OR		CX, CX
		JZ		FastWriteBytes_End
		
		;检测是不是写常规内存. 如果是则出错返回
		MOV		BX, [CS:FastWriteBytes_BlockID]
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		FastWriteBytes_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		FastWriteBytes_End
		
		;检测写字节序列时有没有超过块的界限. 如果超过则出错返回
		MOV		CX, [CS:FastWriteBytes_Length]
		ADD		SI, CX
		MOV		AL, 1
		JC		FastWriteBytes_End
		
		MOV		AX, [CS:FastWriteBytes_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [CS:FastWriteBytes_Offset]
		MOVZX	EBX, BX
		MOV		CX, [CS:FastWriteBytes_Length]
		MOVZX	ECX, CX
	
		MOV		ES, [CS:FastWriteBytes_ES]
		MOV		DI, [CS:FastWriteBytes_DI]
		CLD
		
FastWriteBytes_Label2:

		MOV		DL, [ES:DI]
		MOV		[FS:EAX + EBX], DL
		INC		EBX
		INC		DI
		LOOP	FastWriteBytes_Label2
		
		XOR		AL, AL
		
FastWriteBytes_End:
		
		POP		BX
		POPF
		RET
	EndProc		FastWriteBytes
	
	;过程名:	FastReadBytes
	;描述:		读字节序列
	;参数:		BX=块ID
	;			SI=块内偏移
	;			CX=数据长度
	;返回值:	ES:DI=数据
	;			AL=0成功, 否则失败
	Procedure	FastReadBytes
		PUSHF
		PUSH	BX
		
		MOV		[CS:FastReadBytes_BlockID], BX
		MOV		[CS:FastReadBytes_Offset], SI
		MOV		[CS:FastReadBytes_Length], CX
		MOV		[CS:FastReadBytes_ES], ES
		MOV		[CS:FastReadBytes_DI], DI
		XOR		AL, AL
		OR		CX, CX
		JZ		FastReadBytes_End
		
		;检测是不是写常规内存. 如果是则出错返回
		MOV		BX, [CS:FastReadBytes_BlockID]
		CMP		BX, 1024 / 64 - 1
		MOV		AL, 1
		JBE		FastReadBytes_End
		
		;检测块是否被使用. 如果没有被使用则出错返回
		CALL	GetBlock
		OR		AL, AL
		MOV		AL, 1
		JZ		FastReadBytes_End
		
		;检测写字节序列时有没有超过块的界限. 如果超过则出错返回
		MOV		CX, [CS:FastReadBytes_Length]
		ADD		SI, CX
		MOV		AL, 1
		JC		FastReadBytes_End
		
		MOV		AX, [CS:FastReadBytes_BlockID]
		MOVZX	EAX, AX
		MOV		EBX, 64 * 1024
		MUL		EBX
		MOV		BX, [CS:FastReadBytes_Offset]
		MOVZX	EBX, BX
		MOV		CX, [CS:FastReadBytes_Length]
		MOVZX	ECX, CX
		
		MOV		DI, [CS:FastReadBytes_DI]
		MOV		ES, [CS:FastReadBytes_ES]
		CLD
		
FastReadByteBytes_Label1:
		
		MOV		DL, [FS:EAX + EBX]
		MOV		[ES:DI], DL
		INC		EBX
		INC		DI
		LOOP	FastReadByteBytes_Label1
		
		XOR		AL, AL
		
FastReadBytes_End:
		
		POP		BX
		POPF
		RET
	EndProc		FastReadBytes
	