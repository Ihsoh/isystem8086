;*------------------------------------------------------*
;|文件名:	shella.asm									|
;|作者:		Ihsoh										|
;|创建时间:	2013-7-11									|
;|描述:													|
;|	用ASM编写的过程										|
;*------------------------------------------------------*

.MODEL TINY
OPTION CASEMAP:NONE
.CODE
	;过程名:	_ExecMTA16
	;描述:		执行MTA16类型的程序
	;函数原型:	int ExecMTA16(char * FileName, char * Argument);
	PUBLIC _ExecMTA16
	_ExecMTA16	PROC NEAR
		PUSH	BP
		MOV		BP, SP
		PUSH	SI
		MOV		SI, [BP + 4]
		MOV		DI, [BP + 6]
		MOV		AH, 1
		INT		23H
		XOR		AH, AH
		POP		SI
		MOV		SP, BP
		POP		BP
		RET
	_ExecMTA16	ENDP
	
	
	;过程名:	_TaskSlotIsUsed
	;描述:		任务槽是否被使用
	;函数原型:	int TaskSlotIsUsed(uchar TaskID);
	PUBLIC	_TaskSlotIsUsed
	_TaskSlotIsUsed	PROC NEAR
		PUSH	BP
		MOV		BP, SP
		MOV		AL, [BP + 4]
		MOV		AH, 3
		INT		23H
		XOR		AH, AH
		MOV		SP, BP
		POP		BP
		RET
	_TaskSlotIsUsed	ENDP
	
	;过程名:	_GetTaskName
	;描述:		获取任务名
	;函数原型:	void GetTaskName(uchar  TaskID, char * TaskName);
	PUBLIC	_GetTaskName
	_GetTaskName	PROC NEAR
		PUSH	BP
		MOV		BP, SP
		PUSH	AX
		PUSH	DI
		MOV		AL, [BP + 4]
		MOV		DI, [BP + 6]
		MOV		AH, 4
		INT		23H
		POP		DI
		POP		AX
		MOV		SP, BP
		POP		BP
		RET
	_GetTaskName	ENDP
	
	;过程名:	_KillTask
	;描述:		杀死任务
	;函数原型:	int KillTask(uchar TaskID);
	PUBLIC	_KillTask
	_KillTask	PROC NEAR
		PUSH	BP
		MOV		BP, SP
		MOV		AL, [BP + 4]
		MOV		AH, 2
		INT		23H
		XOR		AH, AH
		MOV		SP, BP
		POP		BP
		RET
	_KillTask	ENDP
	
END
