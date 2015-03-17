/*
	文件名:	screen.c
	作者:	Ihsoh
	日期:	2013-8-20
	描述:
		屏幕操作
*/

#include "screen.h"

/*
	函数名:	SetCharToScreen
	功能:	设置一个字符到屏幕
	参数:	X=X坐标
			Y=Y坐标
			Char=字符
			Property=属性
	返回值:	无
*/
void SetCharToScreen(uint X, uint Y, char Char, uchar Property)
{
	uint Offset = ((80 * Y) + X) * 2;
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	DS;
	asm	MOV		BX, 0B800H;
	asm	MOV		DS, BX;
	_BX = Offset;
	_AL = Char;
	asm	MOV		[BX + 0], AL;
	_AL = Property;
	asm	MOV		[BX + 1], AL;
	asm	POP		DS;
	asm	POP		BX;
	asm	POP		AX;
}
