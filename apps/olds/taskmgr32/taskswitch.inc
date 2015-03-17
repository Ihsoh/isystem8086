;*------------------------------------------------------*
;|文件名:	taskswitch.inc								|
;|作者:		Ihsoh										|
;|创建时间:	2013-6-4									|
;|														|
;|概述:													|
;|	任务切换程序										|
;*------------------------------------------------------*

;过程名:	TaskSwitch
;描述:		任务切换
;参数:		无
;返回值:	无
Procedure	TaskSwitch
	BITS	32
	
	MOV		SI, [CurrentTaskID]
	INC		SI

TaskSwitch_Loop:

	CMP		BYTE [UsedFlags + SI], 1
	JE		TaskSwitch_Found
	INC		SI
	CMP		SI, MaxTaskCount - 1
	JAE		TaskSwitch_Loop
	XOR		ESI, ESI
	
TaskSwitch_Loop1:

	CMP		BYTE [UsedFlags + SI], 1
	JE		TaskSwitch_Found
	INC		SI
	CMP		SI, MaxTaskCount - 1
	JAE		TaskSwitch_Loop1
	
TaskSwitch_Found:	;找到可用的任务

	MOV		[CurrentTaskID], SI
	
	;清堆栈数据
	MOV		ESP, TaskSwitchStackLen	
	
	;设置要跳转的任务选择子
	SHL		SI, 3
	ADD		SI, FirstTaskSel
	MOV		[TaskSwitch_Selector], SI
	
	;结束中断
	MOV		AL, 20H
	OUT		20H, AL
	
	DB	0EAH
	DD	0
TaskSwitch_Selector:
	DW	0
	
	BITS	16
EndProc		TaskSwitch
