/*
	文件名:	keyboard.c
	作者:	Ihsoh
	日期:	2013-8-4
*/

#include "keyboard.h"

uchar Keyboards[256];
uint KeyboardsLength;

void ScanKeyboard(void)
{
	uint Key;
	uchar Has;
	int ui;
	
	KeyboardsLength = 0;
	while(1)
	{
		Key = CheckKey(&Has);
		if(!Has) break;
		Keyboards[KeyboardsLength++] = (uchar)(Key >> 8);
		GetCharNP();
		for(ui = 0; ui < 0xFFFF; ui++);
	}
}

uint HasKey(uchar Scancode)
{
	int i;
	
	for(i = 0; i < KeyboardsLength; i++)
		if(Keyboards[i] == Scancode) return 1;
	return 0;
}
