/*
	ilib_gra.c
	Ihsoh
	2013-2-25
*/

#include "ilib.h"

/*
	函数名:	InitVideo
	描述:	初始化视频模式
	参数:	Mode=视频模式
	返回值:	无
*/
void InitVideo(VideoMode Mode)
{
	asm PUSH		AX;
	_AL = (uchar)Mode;
	asm MOV			AH, 0;
	asm INT			10H;
	asm POP			AX;
}

uchar _GetBit(uchar * Data, ulong Offset)
{
	uint Index = Offset / 8;
	uint BitOffset = Offset % 8;
	return (*(Data + Index) >> BitOffset) & 0x01;
}

void _SetBit(uchar * Data, ulong Offset, uchar Bit)
{
	uint Index = Offset / 8;
	uint BitOffset = Offset % 8;
	if(Bit) 
	{
		Bit <<= BitOffset;
		*(Data + Index) |= Bit;
	}
	else 
	{
		Bit = 1;
		Bit <<= BitOffset;
		Bit = ~Bit;
		*(Data + Index) &= Bit;
	}
}

/*
	函数名:	GetPixel_Grayness
	功能:	获取灰度图的像素
	参数:	Data=数据
			X=X坐标
			Y=Y坐标
			Width=图片宽度
			Height=图片高度
	返回值:	像素
*/
uchar GetPixel_Grayness(uchar * Data, uint X, uint Y, uint Width, uint Height)
{
	if(X >= Width || Y >= Height) return 0;
	return _GetBit(Data, (ulong)X + (ulong)Y * (ulong)Width);
}

/*
	函数名:	SetPixel_Grayness
	功能:	设置灰度图的像素
	参数:	Data=数据
			X=X坐标
			Y=Y坐标
			Width=图片宽度
			Height=图片高度
			Pixel=像素
	返回值:	无
	
*/
void SetPixel_Grayness(uchar * Data, uint X, uint Y, uint Width, uint Height, uchar Pixel)
{
	if(X < Width && Y < Height) _SetBit(Data, (ulong)X + (ulong)Y * (ulong)Width, Pixel);
}

/*
	函数名:	HLine_Grayness
	功能:	画灰度图水平线
	参数:	Data=数据
			X=X坐标
			Y=Y坐标
			Length=长度
			Width=图片宽度
			Height=图片高度
			Pixel=像素
	返回值:	无
*/
void HLine_Grayness(uchar * Data, uint X, uint Y, uint Length, uint Width, uint Height, uchar Pixel, LineType Type)
{
	uint ui;
	
	for(ui = X; ui < X + Length; ui++)
		if(Type == LineType_Normal)
			SetPixel_Grayness(Data, ui, Y, Width, Height, Pixel);
		else 
			SetPixel_Grayness(Data, ui, Y, Width, Height, Pixel = !Pixel);
}

/*
	函数名:	VLine_Grayness
	功能:	画灰度图垂直线
	参数:	Data=数据
			X=X坐标
			Y=Y坐标
			Length=长度
			Width=图片宽度
			Height=图片高度
			Pixel=像素
	返回值:	无
*/
void VLine_Grayness(uchar * Data, uint X, uint Y, uint Length, uint Width, uint Height, uchar Pixel, LineType Type)
{
	uint ui;
	
	for(ui = Y; ui < Y + Length; ui++)
		if(Type == LineType_Normal) 
			SetPixel_Grayness(Data, X, ui, Width, Height, Pixel);
		else
			SetPixel_Grayness(Data, X, ui, Width, Height, Pixel = !Pixel);
}

void DrawImage_Grayness(uchar * Dst,
						uint DstX,
						uint DstY,
						uint DstWidth,
						uint DstHeight,
						uchar * Src,
						uint SrcWidth,
						uint SrcHeight)
{
	uint X, Y;
	
	for(X = 0; X < SrcWidth; X++)
		for(Y = 0; Y < SrcHeight; Y++)
		{
			uchar Pixel = GetPixel_Grayness(Src, X, Y, SrcWidth, SrcHeight);
			SetPixel_Grayness(Dst, DstX + X, DstY + Y, DstWidth, DstHeight, Pixel);
		}
}

/*
	------------------------------VMG13H------------------------------
*/

/*
	函数名:	SetPalette_VMG13H
	描述:	设置调色板
	参数:	Index=索引号
			R=红色分量
			G=绿色分量
			B=蓝色分量
	返回值:	无
*/
void SetPalette_VMG13H(uchar Index, uchar R, uchar G, uchar B)
{
	asm	PUSH	AX;
	asm	PUSH	DX;
	_AL = Index;
	asm	MOV		DX, 03C8H;
	asm	OUT		DX, AL;
	_AL = R;
	asm	MOV		DX, 03C9H;
	asm	OUT		DX, AL;
	_AL = G;
	asm	OUT		DX, AL;
	_AL = B;
	asm	OUT		DX, AL;
	asm	POP		DX;
	asm	POP		AX;
}

/*
	函数名:	DrawPixel_VMG13H
	描述:	画像素
	参数:	X=X坐标
			Y=Y坐标
			Index=索引号
	返回值:	无
*/
void DrawPixel_VMG13H(uint X, uint Y, uchar Index)
{
	uint Offset = Offset_VMG13H(X, Y);
	asm	PUSH	DS;
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	MOV		BX, 0A000H;
	asm	MOV		DS, BX;
	_BX = Offset;
	_AL = Index;
	asm	MOV		[BX], AL;
	asm	POP		BX;
	asm	POP		AX;
	asm	POP		DS;
}

/*
	函数名:	GetPixel_VMG13H
	描述:	获取像素
	参数:	X=X坐标
			Y=Y坐标
	返回值:	无
*/
uchar GetPixel_VMG13H(uint X, uint Y)
{
	uint Offset = Offset_VMG13H(X, Y);
	asm	PUSH	DS;
	asm	PUSH	BX;
	asm	MOV		BX, 0A000H;
	asm	MOV		DS, BX;
	_BX = Offset;
	asm	MOV		AL, [BX];
	asm	POP		BX;
	asm	POP		DS;
}

/*
	函数名:	DrawRectangle_VMG13H
	描述:	画矩形
	参数:	X=X坐标
			Y=Y坐标
			Width=宽度
			Height=高度
			Index=索引号
	返回值:	无
*/
void DrawRectangle_VMG13H(uint X, uint Y, uint Width, uint Height, uchar Index)
{
	uint i, i1;
	for(i = X; i < X + Width && i < MAX_WIDTH_VMG13H; i++)
		for(i1 = Y; i1 < Y + Height && i1 < MAX_HEIGHT_VMG13H; i1++)
		{
			asm	PUSH	DS;
			asm	PUSH	AX;
			asm	PUSH	BX;
			asm	MOV		BX, 0A000H;
			asm	MOV		DS, BX;
			_BX = Offset_VMG13H(i, i1);
			_AL = Index;
			asm	MOV		[BX], AL;
			asm	POP		BX;
			asm	POP		AX;
			asm	POP		DS;
		}
}

/*
	函数名:	DrawToScreen_Grayness_VMG13H
	描述:	画灰度图
	参数:	X=X坐标
			Y=Y坐标
			Width=宽度
			Height=高度
			Image=图片. 图片的长度为: Width * Height / 8
	返回值:	无
*/
void DrawToScreen_Grayness_VMG13H(uint X, uint Y, uint Width, uint Height, uchar * Image)
{
	uint ui, ui1;
	for(ui = X; ui < X + Width && ui < MAX_WIDTH_VMG13H; ui++)
		for(ui1 = Y; ui1 < Y + Height && ui1 < MAX_HEIGHT_VMG13H; ui1++)
			DrawPixel_VMG13H(ui, ui1, GetPixel_Grayness(Image, ui - X, ui1 - Y, Width, Height));
}

/*
	函数名:	DrawToScreen_256Colors_VMG13H
	描述:	画256色图
	参数:	X=X坐标
			Y=Y坐标
			Width=宽度
			Height=高度
			Image=图片. 图片的长度为: Width * Height
	返回值:	无
*/
void DrawToScreen_256Colors_VMG13H(uint X, uint Y, uint Width, uint Height, uchar * Image)
{
	uint ui, ui1;
	for(ui = X; ui < X + Width && ui < MAX_WIDTH_VMG13H; ui++)
		for(ui1 = Y; ui1 < Y + Height && ui1 < MAX_HEIGHT_VMG13H; ui1++)
			DrawPixel_VMG13H(ui, ui1, Image[Offset_VMG13H(ui - X, ui1 - Y)]);
}
