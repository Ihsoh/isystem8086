/*
	文件名:	iedtr16.c
	作者:	Ihsoh
	日期:	2013-8-20
	描述:
		IEditor16主文件
*/

#include "iedtr16.h"

uint CaretX = 0, CaretY = 0;
uint ScreenTop = 0;
char Lines[24 * 80];
char FileName[21] = {	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void SaveFile(void);


/*
	函数名:	Init
	功能:	初始化
	参数:	无
	返回值:	无
*/
void Init(void)
{
	InitBuffer();
	Console("cls");
}

/*
	函数名:	Destroy
	功能:	删除分配的资源
	参数:	无
	返回值:	无
*/
void Destroy(void)
{
	DestroyBuffer();
	Console("cls");
}

/*
	函数名:	PrintLines
	功能:	打印行
	参数:	无
	返回值:	无
*/
void PrintLines(void)
{
	uint X, Y;

	for(X = 0; X < 80; X++)
		for(Y = 0; Y < 24; Y++)
		{
			char Char = Lines[Y * 80 + X];
			switch(Char)
			{
				case '\t':
					SetCharToScreen(X, Y, 26, 0x0E);
					break;
				default:	
					SetCharToScreen(X, Y, Char, 7);
					break;
			}
		}
}

/*
	函数名:	GetLineLength
	功能:	获取行的长度
	参数:	行号
	返回值:	长度
*/
uint GetLineLength(uint Row)
{
	uint ui;
	
	for(ui = 0; ui < 80; ui++)
		if(Lines[(Row * 80) + ui] == 0) break;
	return ui;
}

/*
	函数名:	PrintStatusBar
	功能:	打印状态条
	参数:	无
	返回值:	无
*/
void PrintStatusBar(void)
{
	char X[6] = {0, 0, 0, 0, 0, 0};
	char Y[6] = {0, 0, 0, 0, 0, 0};
	uint ui;

	UIntegerToString(CaretX, X);
	UIntegerToString(ScreenTop + CaretY, Y);
	
	asm	PUSH	ES;
	asm	PUSH	DI;
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	MOV		AX, 0B800H;
	asm	MOV		ES, AX;
	
	/* X坐标(5) */
	asm	MOV		AL, [X + 0];
	asm	MOV		ES:[24 * 80 * 2 + 0], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 1], 70H;
	asm	MOV		AL, [X + 1];
	asm	MOV		ES:[24 * 80 * 2 + 2], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 3], 70H;
	asm	MOV		AL, [X + 2];
	asm	MOV		ES:[24 * 80 * 2 + 4], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 5], 70H;
	asm	MOV		AL, [X + 3];
	asm	MOV		ES:[24 * 80 * 2 + 6], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 7], 70H;
	asm	MOV		AL, [X + 4];
	asm	MOV		ES:[24 * 80 * 2 + 8], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 9], 70H;
	
	/* |(1) */
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 10], '|';
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 11], 70H;
	
	/* Y坐标(5) */
	asm	MOV		AL, [Y + 0];
	asm	MOV		ES:[24 * 80 * 2 + 12], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 13], 70H;
	asm	MOV		AL, [Y + 1];
	asm	MOV		ES:[24 * 80 * 2 + 14], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 15], 70H;
	asm	MOV		AL, [Y + 2];
	asm	MOV		ES:[24 * 80 * 2 + 16], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 17], 70H;
	asm	MOV		AL, [Y + 3];
	asm	MOV		ES:[24 * 80 * 2 + 18], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 19], 70H;
	asm	MOV		AL, [Y + 4];
	asm	MOV		ES:[24 * 80 * 2 + 20], AL;
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 21], 70H;
	
	/* |(1) */
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 22], '|';
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 23], 70H;
	
	/* 文件名(20) */
	asm	PUSH	AX;
	asm	PUSH	BX;
	for(ui = 0; ui < 20; ui++)
	{
		_AL = FileName[ui];
		_BX = ui * 2;
		if(_AL != 0) 
			asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 24 + BX], AL;
		else 
			asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 24 + BX], ' ';
		asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 24 + BX + 1], 70H;
	}
	asm	POP		BX;
	asm	POP		AX;
	
	/* |(1) */
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 64], '|';
	asm	MOV		BYTE PTR ES:[24 * 80 * 2 + 65], 70H;
	
	/* 填充(47) */
	asm	PUSH	BX;
	for(ui = 66; ui < 80 * 2; ui += 2)
	{
		_BX = ui;
		asm	MOV		BYTE PTR ES:[24 * 80 * 2 + BX], ' ';
		asm	MOV		BYTE PTR ES:[24 * 80 * 2 + BX + 1], 70H;
	}
	asm	POP		BX;
	
	asm	POP		BX;
	asm	POP		AX;
	asm	POP		DI;
	asm	POP		ES;
}

/*
	函数名:	Edit
	功能:	进入编辑模式
	参数:	无
	返回值:	无
*/
void Edit(void)
{
	char Has;
	uint Key;
	char Char;
	uint ui;
	
	while(1)
	{
		Key = CheckKey(&Has);
		if(Has)
		{
			uint LineCount;
			uint CRLFOffset;
			uint CurrentOffset;
			
			switch((uchar)(Key >> 8))
			{
				case KEY_UP:
					if(CaretY > 0) 
					{
						uint PrevLineLength;
						
						CaretY--;
						PrevLineLength = GetLineLength(CaretY);
						CaretX = CaretX <= PrevLineLength ? CaretX : PrevLineLength;
					}
					if(CaretY == 0 && ScreenTop != 0)  ScreenTop--;
					break;
				case KEY_DOWN:
					LineCount = GetLineCountFromBuffer();
					if(	CaretY == 23				&& 
						LineCount > 24				&&
						ScreenTop < LineCount - 24 - 1)  ScreenTop++;
					if(CaretY + 1 < 24 && CaretY < LineCount - 1)
					{
						uint NextLineLength;
						
						CaretY++;
						NextLineLength = GetLineLength(CaretY);
						CaretX = CaretX <= NextLineLength ? CaretX : NextLineLength;
					}
					break;
				case KEY_LEFT:
					if(CaretX > 0) CaretX--;
					break;
				case KEY_RIGHT:
					if(	CaretX + 1 < 80 					&& 
						Lines[(CaretY * 80) + CaretX] != 0) CaretX++;
					break;
				case KEY_BACKSPACE:
					if(CaretX > 0)
					{
						DeleteCharFromBuffer(GetOffsetByRow(ScreenTop + CaretY) + CaretX - 1);
						CaretX--;
					}
					else if(CaretX == 0 && ScreenTop + CaretY != 0)
					{
						CRLFOffset = GetOffsetByRow(ScreenTop + CaretY);
						DeleteCharFromBuffer(CRLFOffset);
						DeleteCharFromBuffer(CRLFOffset);
						CaretY--;
						CaretX = GetLineLength(CaretY);
					}
					break;
				case KEY_ENTER:
					CurrentOffset = GetOffsetByRow(ScreenTop + CaretY) + CaretX;
					InsertCharToBuffer(CurrentOffset, '\n');
					InsertCharToBuffer(CurrentOffset, '\r');
					LineCount = GetLineCountFromBuffer();
					if(CaretY == 23) ScreenTop++;
					if(CaretY < 24 - 1) CaretY++;
					CaretX = 0;
					break;
				case KEY_PAGEUP:
					if(ScreenTop > 24) ScreenTop -= 24;
					CaretX = 0;
					break;
				case KEY_PAGEDOWN:
					LineCount = GetLineCountFromBuffer();
					if(ScreenTop + CaretY + 24 < LineCount)
						ScreenTop += 24;
					CaretX = 0;
					break;
				case KEY_HOME:
					CaretX = 0;
					break;
				case KEY_END:
					CaretX = GetLineLength(CaretY);
					break;
				case KEY_DEL:
					if(CaretX != GetLineLength(CaretY))
						DeleteCharFromBuffer(GetOffsetByRow(ScreenTop + CaretY) + CaretX);
					break;
				case KEY_ESC:
					GetCharNP();
					return;
				case KEY_F2:
					SaveFile();
					break;
				/*case KEY_TAB:
					for(ui = 0; ui < 4 && CaretX < 79; ui++, CaretX++)
						InsertCharToBuffer(GetOffsetByRow(ScreenTop + CaretY) + CaretX, ' ');
					break;*/
				default:
					Char = (char)Key;
					InsertCharToBuffer(GetOffsetByRow(ScreenTop + CaretY) + CaretX, Char);
					CaretX++;
					break;
			}
			GetCharNP();
		}
		MoveCaret(CaretX, CaretY);
		for(ui = 0; ui < 80 * 24; ui++)
			Lines[ui] = 0;
		GetLinesFromBuffer(ScreenTop, 24, Lines);
		PrintLines();
		PrintStatusBar();
	}
}

/*
	函数名:	LoadFile
	功能:	加载文件
	参数:	无
	返回值:	无
*/
void LoadFile(char * FN)
{
	if(StringCmp(FN, ""))
	{
		PrintString("Please file name: ");
		GetLine(FileName);
		if(GetStringLen(FileName) == 0) Exit();
	}
	else CopyString(FileName, FN);
	if(!FileExists(FileName)) NewFile(FileName);
	LoadBufferFromFile(FileName);
}

/*
	函数名:	SaveFile
	功能:	保存文件
	参数:	无
	返回值:	无
*/
void SaveFile(void)
{
	DelFile(FileName);
	NewFile(FileName);
	SaveBufferToFile(FileName);
}
	
/*
	函数名:	main
	功能:	主过程
	参数:	无
	返回值:	无
*/
void main(void)
{
	char Buffer[128];
	
	Init();
	Argument(Buffer);
	LoadFile(Buffer);
	Edit();
	Destroy();
}
