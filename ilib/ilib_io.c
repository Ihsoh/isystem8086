/*
	ilib_io.c
	Ihsoh
	2013-2-17
*/

#include "ilib.h"

uchar GetControlKeys(void)
{
	asm	MOV		AH, 2;
	asm	INT		16H;
}

void PrintString(const char * String)
{
	while(*String != '\0')
	{
		char Char = *(String++);
		if(Char == '\r') CR();
		else if(Char == '\n') LF();
		else PrintChar(Char);
	}
}

void PrintStringP(const char * String, CharProperty Property)
{
	while(*String != '\0')
	{
		char Char = *(String++);
		if(Char == '\r') CR();
		else if(Char == '\n') LF();
		else PrintCharP(Char, Property);
	}
}

void PrintUIntegerP(uint UInteger, uchar Property)
{
	uint Dividend[5] = {10000, 1000, 100, 10, 1};
	int i;
	char Char;
	int ThrowZero = 1;
	if(UInteger == 0)
	{
		PrintCharP('0', Property);
		return;
	}
	for(i = 0; i < 5; i++)
	{
		Char = (char)(UInteger / Dividend[i]) + '0';
		if(Char == '0' && !ThrowZero) PrintCharP('0', Property);
		else if(Char != '0')
		{
			PrintCharP(Char, Property);
			ThrowZero = 0;
		}
		UInteger %= Dividend[i];
	}
}

void PrintUInteger(uint UInteger)
{
	PrintUIntegerP(UInteger, CharColor_GrayWhite);
}

void PrintIntegerP(int Integer, uchar Property)
{
	if(Integer == 0) 
	{
		PrintCharP('0', Property);
		return;
	}
	if(Integer < 0) 
	{
		PrintCharP('-', Property);
		PrintUIntegerP(~((uint)Integer - 1), Property);
	}
	else PrintUIntegerP((uint)Integer, Property);
}

void PrintInteger(int Integer)
{
	PrintIntegerP(Integer, CharColor_GrayWhite);
}

void PrintHex8(uchar Hex)
{
	asm	PUSH	AX;
	asm	PUSH	DX;
	asm	MOV		DL, Hex;
	asm	MOV		AH, 32;
	asm	INT		21H;
	asm	POP		DX;
	asm	POP		AX;
}

void PrintHex16(uint Hex)
{
	asm	PUSH	AX;
	asm	PUSH	DX;
	asm	MOV		DX, Hex;
	asm	MOV		AH, 33;
	asm	INT		21H;
	asm	POP		DX;
	asm	POP		AX;
}
