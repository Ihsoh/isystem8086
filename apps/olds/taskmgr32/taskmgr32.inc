;*------------------------------------------------------*
;|文件名:	taskmgr32.inc								|
;|作者:		Ihsoh										|
;|创建时间:	2013-5-11									|
;|														|
;|概述:													|
;|	任务管理器定义文件									|
;*------------------------------------------------------*

%IFNDEF	_TASKMGR32_INC_
%DEFINE	_TASKMGR32_INC_

CoreAddress			EQU 100000H								;任务管理器核心地址
Taskmgr32Seg		EQU 4000H								;任务管理器段地址
Taskmgr32Off		EQU 0H									;任务管理器偏移地址
Taskmgr32Address	EQU (Taskmgr32Seg << 4) + Taskmgr32Off	;任务管理器地址
BufferSeg			EQU 5000H								;缓冲区段地址
BufferOff			EQU 0H									;缓冲区偏移地址

MaxFileNameLen		EQU 20		;文件名最长长度
MaxTaskCount		EQU 64		;最大任务数
TaskMemorySize		EQU 400000H	;任务内存大小, 4MB


%ENDIF
