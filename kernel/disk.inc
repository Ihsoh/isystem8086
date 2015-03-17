;*------------------------------------------------------*
;|文件名:	disk.inc									|
;|作者:		Ihsoh										|
;|创建时间:	2013-9-1									|
;|														|
;|描述:													|
;|	硬盘操作.											|
;*------------------------------------------------------*

DiskOperatorPorts	DW	0, 0, 0, 0, 0, 0, 0, 0, 0

Disk1Arguments:	TIMES 10H DB 0
Disk2Arguments:	TIMES 10H DB 0

;过程名:	InitDisk
;功能:		初始化硬盘
;参数:		无
;返回值:	无
Procedure	InitDisk
	PUSH	ES
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	AX
	PUSH	BX
	PUSH	CX
	
	MOV		AX, 0
	MOV		ES, AX
	MOV		AX, [ES:104H + 0H]
	MOV		BX, [ES:104H + 2H]
	MOV		DS, BX
	MOV		SI, AX
	PUSH	CS
	POP		ES
	MOV		DI, Disk1Arguments
	MOV		CX, 10H
	CLD
	REP MOVSB
	
	MOV		AX, 0
	MOV		ES, AX
	MOV		AX, [ES:118H + 0H]
	MOV		BX, [ES:118H + 2H]
	MOV		DS, BX
	MOV		SI, AX
	PUSH	CS
	POP		ES
	MOV		DI, Disk2Arguments
	MOV		CX, 10H
	CLD
	REP MOVSB
	
	POP		CX
	POP		BX
	POP		AX
	POP		DI
	POP		SI
	POP		DS
	POP		ES
	RET
EndProc		InitDisk

;过程名:	WriteDiskSector
;功能:		写硬盘扇区
;参数:		AL=硬盘号. 0=第一, 1=第二
;			BL=主从. 0=主, 1=从
;			CX=扇区逻辑地址(高12位)
;			DX=扇区逻辑地址(低16位)
;			DS:SI=数据
;返回值:	CF=0成功, 否则失败
Procedure	WriteDiskSector
	PUSH	AX
	PUSH	BX
	PUSH	DX
	PUSH	CX
	PUSH	SI
	
	CMP		AL, 0
	JNE		WriteDiskSector_Disk2
	MOV		WORD [CS:DiskOperatorPorts + 0], 1F0H
	MOV		WORD [CS:DiskOperatorPorts + 2], 1F1H
	MOV		WORD [CS:DiskOperatorPorts + 4], 1F2H
	MOV		WORD [CS:DiskOperatorPorts + 6], 1F3H
	MOV		WORD [CS:DiskOperatorPorts + 8], 1F4H
	MOV		WORD [CS:DiskOperatorPorts + 10], 1F5H
	MOV		WORD [CS:DiskOperatorPorts + 12], 1F6H
	MOV		WORD [CS:DiskOperatorPorts + 14], 1F7H
	MOV		WORD [CS:DiskOperatorPorts + 16], 3F6H
	MOV		AL, [CS:Disk1Arguments + 08H]
	JMP		WriteDiskSector_Disk1
	
WriteDiskSector_Disk2:
	
	MOV		WORD [CS:DiskOperatorPorts + 0], 170H
	MOV		WORD [CS:DiskOperatorPorts + 2], 171H
	MOV		WORD [CS:DiskOperatorPorts + 4], 172H
	MOV		WORD [CS:DiskOperatorPorts + 6], 173H
	MOV		WORD [CS:DiskOperatorPorts + 8], 174H
	MOV		WORD [CS:DiskOperatorPorts + 10], 175H
	MOV		WORD [CS:DiskOperatorPorts + 12], 176H
	MOV		WORD [CS:DiskOperatorPorts + 14], 177H
	MOV		WORD [CS:DiskOperatorPorts + 16], 376H
	MOV		AL, [CS:Disk2Arguments + 08H]
	
WriteDiskSector_Disk1:

	;用AH和BH保存扇区逻辑地址(低16位)
	MOV		AH, DH
	MOV		BH, DL

	MOV		DX, [CS:DiskOperatorPorts + 16]
	;不知道为什么输出控制字节不行.
	;输出控制字节的话Bochs可以, VM和VBox都失败...
	OUT		DX, AL
	;输出4和0的话Bochs, VM和VBox都可以...
	;MOV		AL, 4
	;OUT		DX, AL
	;MOV		AL, 0
	;OUT		DX, AL
	
	MOV		DX, [CS:DiskOperatorPorts + 2]
	MOV		AL, 0
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 4]
	MOV		AL, 1
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 6]
	MOV		AL, BH
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 8]
	MOV		AL, AH
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 10]
	MOV		AL, CL
	OUT		DX, AL
	OR		CH, 11100000B
	AND		CH, 11101111B
	CMP		BL, 0
	JE		WriteDiskSector_Master
	OR	 	CH, 00010000B
	
WriteDiskSector_Master:
	
	MOV		AL, CH
	MOV		DX, [CS:DiskOperatorPorts + 12]
	OUT		DX, AL
	MOV		AL, 30H
	MOV		DX, [CS:DiskOperatorPorts + 14]
	OUT		DX, AL
	
WriteDiskSector_Wait:

	IN		AL, DX
	TEST	AL, 8
	JZ		WriteDiskSector_Wait
	
	MOV		DX, [CS:DiskOperatorPorts + 0]
	MOV		CX, 512 / 2
	CLD
	
WriteDiskSector_Write:
	
	LODSW
	OUT		DX, AX
	LOOP	WriteDiskSector_Write
	
	CLC
	POP		SI
	POP		CX
	POP		DX
	POP		BX
	POP		AX
	RET
EndProc		WriteDiskSector

;过程名:	ReadDiskSector
;功能:		读硬盘扇区
;参数:		AL=硬盘号. 0=第一, 1=第二
;			BL=主从. 0=主, 1=从
;			CX=扇区逻辑地址(高12位)
;			DX=扇区逻辑地址(低16位)
;返回值:	CF=0成功, 否则失败
;			ES:DI=数据
Procedure	ReadDiskSector
	PUSH	AX
	PUSH	BX
	PUSH	DX
	PUSH	CX
	PUSH	DI
	
	CMP		AL, 0
	JNE		ReadDiskSector_Disk2
	MOV		WORD [CS:DiskOperatorPorts + 0], 1F0H
	MOV		WORD [CS:DiskOperatorPorts + 2], 1F1H
	MOV		WORD [CS:DiskOperatorPorts + 4], 1F2H
	MOV		WORD [CS:DiskOperatorPorts + 6], 1F3H
	MOV		WORD [CS:DiskOperatorPorts + 8], 1F4H
	MOV		WORD [CS:DiskOperatorPorts + 10], 1F5H
	MOV		WORD [CS:DiskOperatorPorts + 12], 1F6H
	MOV		WORD [CS:DiskOperatorPorts + 14], 1F7H
	MOV		WORD [CS:DiskOperatorPorts + 16], 3F6H
	MOV		AL, [CS:Disk1Arguments + 08H]
	JMP		ReadDiskSector_Disk1
	
ReadDiskSector_Disk2:
	
	MOV		WORD [CS:DiskOperatorPorts + 0], 170H
	MOV		WORD [CS:DiskOperatorPorts + 2], 171H
	MOV		WORD [CS:DiskOperatorPorts + 4], 172H
	MOV		WORD [CS:DiskOperatorPorts + 6], 173H
	MOV		WORD [CS:DiskOperatorPorts + 8], 174H
	MOV		WORD [CS:DiskOperatorPorts + 10], 175H
	MOV		WORD [CS:DiskOperatorPorts + 12], 176H
	MOV		WORD [CS:DiskOperatorPorts + 14], 177H
	MOV		WORD [CS:DiskOperatorPorts + 16], 376H
	MOV		AL, [CS:Disk2Arguments + 08H]
	
ReadDiskSector_Disk1:

	;用AH和BH保存扇区逻辑地址(低16位)
	MOV		AH, DH
	MOV		BH, DL

	MOV		DX, [CS:DiskOperatorPorts + 16]
	OUT		DX, AL
	;MOV		AL, 4
	;OUT		DX, AL
	;MOV		AL, 0
	;OUT		DX, AL
	
	MOV		DX, [CS:DiskOperatorPorts + 2]
	MOV		AL, 0
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 4]
	MOV		AL, 1
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 6]
	MOV		AL, BH
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 8]
	MOV		AL, AH
	OUT		DX, AL
	MOV		DX, [CS:DiskOperatorPorts + 10]
	MOV		AL, CL
	OUT		DX, AL
	OR		CH, 11100000B
	AND		CH, 11101111B
	CMP		BL, 0
	JE		ReadDiskSector_Master
	OR	 	CH, 00010000B
	
ReadDiskSector_Master:
	
	MOV		AL, CH
	MOV		DX, [CS:DiskOperatorPorts + 12]
	OUT		DX, AL
	MOV		AL, 20H
	MOV		DX, [CS:DiskOperatorPorts + 14]
	OUT		DX, AL
	
	IN		AL, DX
	TEST	AL, 1
	JZ		ReadDiskSector_NoErr
	STC
	JMP		ReadDiskSector_End
	
ReadDiskSector_NoErr:
	
ReadDiskSector_Wait:

	IN		AL, DX
	TEST	AL, 8
	JZ		ReadDiskSector_Wait
	
	MOV		DX, [CS:DiskOperatorPorts + 0]
	MOV		CX, 512 / 2
	CLD
	
ReadDiskSector_Read:

	IN		AX, DX
	STOSW
	LOOP	ReadDiskSector_Read	
	CLC
	
ReadDiskSector_End:
	
	POP		DI
	POP		CX
	POP		DX
	POP		BX
	POP		AX
	RET
EndProc		ReadDiskSector

;过程名:	GetDiskSectorCount
;功能:		获取硬盘扇区数量
;参数:		AL=硬盘号. 0=第一硬盘, 1=第二硬盘
;返回值:	DX=扇区数量(高16位)
;			AX=扇区数量(低16位)
Procedure	GetDiskSectorCount
	PUSH	BX
	PUSH	CX
	CMP		AL, 0
	JNE		GetDiskSectorCount_Disk2
	MOV		BX, Disk1Arguments 
	JMP		GetDiskSectorCount_Disk1
	
GetDiskSectorCount_Disk2:
	
	MOV		BX, Disk2Arguments
	
GetDiskSectorCount_Disk1:
	
	MOV		AL, [CS:BX + 02H]	;磁头数
	MOV		AH, [CS:BX + 0EH]	;每磁道扇区数
	MUL		AH
	MOV		CX, [CS:BX + 0H]	;柱面数
	MUL		CX
	POP		CX
	POP		BX
	RET
EndProc		GetDiskSectorCount
