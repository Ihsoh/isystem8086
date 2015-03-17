;*------------------------------------------------------*
;|文件名:	ieditor.inc									|
;|作者:		Ihsoh										|
;|创建时间:	2013-4-1									|
;|														|
;|描述:													|
;|编辑器常量文件										|
;*------------------------------------------------------*

%IFNDEF	_IEDITOR_INC_
%DEFINE	_IEDITOR_INC_

ScreenWidth		EQU 80	;屏幕字符总列数
ScreenHeight	EQU 24	;屏幕字符总行数

MaxCol			EQU 78	;最大列数, 剩下的两列保存\r\n
MaxRow			EQU 340	;最大行数

BufferSegAddr	EQU 5000H					;缓冲区段地址
BufferOffAddr	EQU 0000H					;缓冲区偏移地址
BufferLen		EQU (MaxCol + 2) * MaxRow	;缓冲区长度

PrintASCBeg		EQU 32		;可打印字符ASCII码开始
PrintASCEnd		EQU 127		;可打印字符ASCII码结束

;键盘扫描码
Key_Up			EQU 72
Key_Down		EQU 80
Key_Left		EQU 75
Key_Right		EQU 77
Key_F1			EQU 59
Key_F2			EQU 60
Key_F3			EQU 61
Key_Esc			EQU 1
Key_Backspace	EQU 14
Key_Home		EQU 71
Key_End			EQU 79
Key_PageUp		EQU 73
Key_PageDown	EQU 81
Key_Tab			EQU 15
Key_Enter		EQU 28
Key_N			EQU 49
Key_S			EQU 31
Key_L			EQU 38
Key_O			EQU 24

StatusBarProp	EQU 70H	;状态栏字符属性

MainVer			EQU '0'	;主版本
Ver				EQU '0'	;从版本

%ENDIF
