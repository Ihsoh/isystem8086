/*
	graph.c
	Ihsoh
	2013-9-7
*/

#include "graph.h"

uint MaxWidth = 0;
uint MaxHeight = 0;
static uint GraphMode = 0;

void GS_InitGraph(uint Mode)
{
	switch(Mode)
	{
		case 0x101:
			MaxWidth = 640;
			MaxHeight = 480;
			break;
		case 0x103:
			MaxWidth = 800;
			MaxHeight = 600;
			break;
		case 0x105:
			MaxWidth = 1024;
			MaxHeight = 768;
			break;
		default:
			return;
	}
	GraphMode = Mode;
	asm	MOV		AX, 4F02H;
	asm	MOV		BX, Mode;
	asm	INT		10H;
}

void GS_SetPalette(uchar Index, uchar R, uchar G, uchar B)
{
	asm	PUSH	AX;
	asm	PUSH	DX;
	asm	MOV		DX, 3C8H;
	asm	MOV		AL, Index;
	asm	OUT		DX, AL;
	asm	MOV		DX, 3C9H;
	asm	MOV		AL, R;
	asm	OUT		DX, AL;
	asm	MOV		AL, G;
	asm	OUT		DX, AL;
	asm	MOV		AL, B;
	asm	OUT		DX, AL;
	asm	POP		DX
	asm	POP		AX;
}

static uint FillPixel_Page = 0;
static uint FillPixel_Offset = 0;

void GS_FillPixel(uchar Pixel)
{	
	asm	PUSH	ES;
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	MOV		AX, 0A000H;
	asm	MOV		ES, AX;
	asm	MOV		AL, Pixel;
	asm	MOV		BX, FillPixel_Offset;
	asm	MOV		ES:[BX], AL;
	asm	POP		BX;
	asm	POP		AX;
	asm	POP		ES;
	if(FillPixel_Offset == 65535)
	{
		FillPixel_Page++;
		asm	PUSH	AX;
		asm	PUSH	BX;
		asm	PUSH	DX;
		asm	MOV		AX, 4F05H;
		asm	XOR		BX, BX;
		asm	MOV		DX, FillPixel_Page;
		asm	INT		10H;
		asm	POP		DX;
		asm	POP		BX;
		asm	POP		AX;
		FillPixel_Offset = 0;
	}
	else FillPixel_Offset++;
}

void GS_NextFrame(void)
{
	FillPixel_Page = 0;
	FillPixel_Offset = 0;
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	DX;
	asm	MOV		AX, 4F05H;
	asm	XOR		BX, BX;
	asm	XOR		DX, DX;
	asm	INT		10H;
	asm	POP		DX;
	asm	POP		BX;
	asm	POP		AX;
}
