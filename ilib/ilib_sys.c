/*
	ilib_sys.c
	Ihsoh
	2013-3-27
*/

#include "ilib.h"

extern void _ISVer(uchar * Ver);

void ISVer(ISystemVer * Ver)
{
	uchar V[2];
	_ISVer(V);
	Ver->MainVer = (uint)(V[1]);
	Ver->Ver = (uint)(V[0]);
}

void Argument(char * Buffer)
{
	char * Src = 0x7;
	
	do
		*(Buffer++) = *Src;
	while(*(Src++) != 0);
}

uint AllocBlocks(uint Count)
{
	uint BlockID;

	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	CX;
	_CX = Count;
	asm	MOV		AH, 0;
	asm	INT		25H;
	if(_AL == 0) BlockID = _BX;
	else BlockID = 0;
	asm	POP		CX;
	asm	POP		BX;
	asm	POP		AX;
	return BlockID;
}

void FreeBlocks(uint BlockID, uint Count)
{
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	CX;
	_BX = BlockID;
	_CX = Count;
	asm	MOV		AH, 1;
	asm	INT		25H;
	asm	POP		CX;
	asm	POP		BX;
	asm	POP		AX;
}

void WriteByte(uint BlockID, uint Offset, uchar Byte)
{
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	SI;
	_BX = BlockID;
	_SI = Offset;
	_AL = Byte;
	asm	MOV		AH, 2;
	asm	INT		25H;
	asm	POP		SI;
	asm	POP		BX;
	asm	POP		AX;
}

uchar ReadByte(uint BlockID, uint Offset)
{
	uchar Byte;

	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	SI;
	_BX = BlockID;
	_SI = Offset;
	asm	MOV		AH, 3;
	asm	INT		25H;
	Byte = _BL;
	asm	POP		SI;
	asm	POP		BX;
	asm	POP		AX;
	return Byte;
}

void WriteBytes(uint BlockID, uint Offset, uint Length, uchar * Bytes)
{
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	CX;
	asm	PUSH	SI;
	asm	PUSH	DI;
	_BX = BlockID;
	_SI = Offset;
	_CX = Length;
	_DI = Bytes;
	asm	MOV		AH, 4;
	asm	INT		25H;
	asm	POP		DI;
	asm	POP		SI;
	asm	POP		CX;
	asm	POP		BX;
	asm	POP		AX;
}

void ReadBytes(uint BlockID, uint Offset, uint Length, uchar * Bytes)
{
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	CX;
	asm	PUSH	SI;
	asm	PUSH	DI;
	_BX = BlockID;
	_SI = Offset;
	_CX = Length;
	_DI = Bytes;
	asm	MOV		AH, 5;
	asm	INT		25H;
	asm	POP		DI;
	asm	POP		SI;
	asm	POP		CX;
	asm	POP		BX;
	asm	POP		AX;
}
