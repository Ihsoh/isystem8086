/*
	文件名:	ifont.c
	作者:	Ihsoh
	日期:	2013-9-4
*/

#include "ifont.h"

uchar IFontBuffer[8192];

void InitIFont(char * FileName)
{
	if(ReadFile(FileName, IFontBuffer) == -1)
	{
		PrintStringP("Cannot load font!", CharColor_Red);
		asm	CLI;
		asm	HLT;
	}
}

void GetCharImage(uchar ASCII, uchar * Data)
{
	int i;
	
	for(i = 0; i < 32; i++)
		Data[i] = IFontBuffer[ASCII * 32 + i];
}
