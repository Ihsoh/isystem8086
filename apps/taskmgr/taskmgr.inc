;*------------------------------------------------------*
;|文件名:	taskmgr.inc									|
;|作者:		Ihsoh										|
;|创建时间:	2013-3-28									|
;|														|
;|描述:													|
;|进程管理器常量.										|
;*------------------------------------------------------*

MaxTaskCount		EQU 6		;最大任务数
TaskMgrID			EQU 0		;任务管理器ID
INT24HAddr			EQU 24H * 4	;24号中断地址
INT23HAddr			EQU 23H * 4	;23号中断地址
TaskSegAddr			EQU 4000H	;任务管理器段地址
TaskOffAddr			EQU 1000H	;任务管理器偏移地址

%DEFINE TASKMGR		'taskmgr', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
%DEFINE	TASKMGRCFG	'taskmgr.cfg', 0, 0, 0, 0, 0, 0, 0, 0, 0
