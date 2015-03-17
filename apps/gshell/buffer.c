/*
	文件名:	buffer.c
	作者:	Ihsoh
	日期:	2013-9-13
*/

asm	.386;

#include "buffer.h"

static uint BufferBID;

void InitBuffer(void)
{
	asm	PUSH	AX;
	asm	MOV		AH, 6;
	asm	INT		25H;
	asm	POP		AX;
	BufferBID = AllocBlocks(12);
	if(BufferBID == 0)
	{
		PrintStringP("GShell: Cannot alloc memory!\r\n", CharColor_Red);
		asm	JMP		$;
	}
}

void DestroyBuffer(void)
{
	FreeBlocks(BufferBID, 12);
}

void SetPixelToBuffer(uint X, uint Y, uchar Pixel)
{
	if(X >= MaxWidth || Y >= MaxHeight) return;

	asm	PUSH	EAX;
	asm	PUSH	EDX;
	
	/* EAX = BufferBID + Y / 64 */
	asm	XOR		EAX, EAX;
	asm	MOV		AX, Y;
	asm	MOV		DL, 64;
	asm	DIV		DL;
	asm	XOR		AH, AH;
	asm	ADD		AX, BufferBID;

	asm	MOV		EDX, 65536;
	asm	MUL		EDX;
	
	/* EDX = Y % 64 * MaxWidth + X */
	asm	PUSH	EAX;
	asm	MOV		AX, Y;
	asm	MOV		DL, 64;
	asm	DIV		DL;
	asm	MOVZX	EAX, AH;
	asm	MOVZX	EDX, WORD PTR MaxWidth;
	asm	MUL		EDX;
	asm	MOVZX	EDX, WORD PTR X;
	asm	ADD		EAX, EDX;
	asm	MOV		EDX, EAX;
	asm	POP		EAX;
	
	asm	ADD		EAX, EDX;
	asm	MOV		DL, Pixel;
	
	
	asm	CMP		EAX, 100000H;
	asm	JAE		NoErr;
	asm	CLI;
	asm	MOV		BX, X;
	asm	MOV		CX, Y;
	asm	MOV		DX, CS:[BufferBID];
	asm	HLT;
	
NoErr:
	
	asm	MOV		FS:[EAX], DL;
	asm	POP		EDX;
	asm	POP		EAX;
}

uchar GetPixelFromBuffer(uint X, uint Y)
{
	uchar Pixels[1];
	
	GetPixelsFromBuffer(X, Y, 1, Pixels);
	return Pixels[0];
}

void GetPixelsFromBuffer(uint X, uint Y, uint Count, uchar * Pixels)
{
	asm	PUSH	EAX;
	asm	PUSH	ECX;
	asm	PUSH	EDX;
	asm	PUSH	DI;
	
	/* EAX = BufferBID + Y / 64 */
	asm	XOR		EAX, EAX;
	asm	MOV		AX, Y;
	asm	MOV		DL, 64;
	asm	DIV		DL;
	asm	XOR		AH, AH;
	asm	ADD		AX, BufferBID;

	asm	MOV		EDX, 65536;
	asm	MUL		EDX;
	
	/* EDX = Y % 64 * MaxWidth + X */
	asm	PUSH	EAX;
	asm	MOV		AX, Y;
	asm	MOV		DL, 64;
	asm	DIV		DL;
	asm	MOVZX	EAX, AH;
	asm	MOVZX	EDX, WORD PTR MaxWidth;
	asm	MUL		EDX;
	asm	MOVZX	EDX, WORD PTR X;
	asm	ADD		EAX, EDX;
	asm	MOV		EDX, EAX;
	asm	POP		EAX;
	
	asm	ADD		EAX, EDX;
	asm	MOV		CX, Count;
	asm	MOV		DI, Pixels;
	asm	CLD;
	
CopyPixel:
	
	asm	MOV		DL, FS:[EAX];
	asm	MOV		[DI], DL;
	asm	INC		EAX;
	asm	INC		DI;
	asm	LOOP	CopyPixel;
	
	asm	POP		DI;
	asm	POP		EDX;
	asm	POP		ECX;
	asm	POP		EAX;
}

void DrawHLineToBuffer(uint X, uint Y, uint Length, uchar Pixel, uint Point)
{
	uint ui;

	Pixel = Pixel == 0 ? 0 : 0xFF;
	for(ui = X; ui < X + Length; ui++)
		if(!Point) SetPixelToBuffer(ui, Y, Pixel);
		else if(ui % 2 != 0) SetPixelToBuffer(ui, Y, Pixel);
}

void DrawVLineToBuffer(uint X, uint Y, uint Length, uchar Pixel, uint Point)
{
	uint ui;
	
	Pixel = Pixel == 0 ? 0 : 0xFF;
	for(ui = Y; ui < Y + Length; ui++)
		if(!Point) SetPixelToBuffer(X, ui, Pixel);
		else if(ui % 2 != 0) SetPixelToBuffer(X, ui, Pixel);
}

void DrawGraynessImageToBuffer(uint X, uint Y, uchar * Image, uint Width, uint Height)
{
	uint ui, ui1;

	for(ui = X; ui < X + Width && ui < MaxWidth; ui++)
		for(ui1 = Y; ui1 < Y + Height && ui1 < MaxHeight; ui1++)
		{
			uchar Pixel = GetPixel_Grayness(Image, ui - X, ui1 - Y, Width, Height);
			Pixel = Pixel == 0 ? 0 : 0xFF;
			SetPixelToBuffer(ui, ui1, Pixel);
		}
}
