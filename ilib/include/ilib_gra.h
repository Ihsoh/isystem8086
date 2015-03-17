/*
	ilib_gra.h
	Ihsoh
	2013-2-25
*/

#ifndef ILIB_GRA_H_
#define ILIB_GRA_H_

#include "ilib_tpy.h"

typedef enum _VideoMode
{
	VideoMode_System 			= 0x03,
	VideoMode_Graphics_0DH		= 0x0D,
	VideoMode_Graphics_13H		= 0x13
}VideoMode;

typedef enum _Color
{
	Color_Black = 0,
	Color_Blue,
	Color_Green,
	Color_DarkBlue,
	Color_Red,
	Color_Magenta,
	Color_Brown,
	Color_GrayWhite,
	Color_LightGray,
	Color_LightBlue,
	Color_LightGreen,
	Color_LightCyan,
	Color_LightRed,
	Color_LightMagenta,
	Color_Yellow,
	Color_White,
	Color_Bg = 0
}Color;

typedef enum
{
	LineType_Normal,
	LineType_Point
}LineType;

extern void InitVideo(VideoMode Mode);
extern uchar GetPixel_Grayness(uchar * Data, uint X, uint Y, uint Width, uint Height);
extern void SetPixel_Grayness(uchar * Data, uint X, uint Y, uint Width, uint Height, uchar Pixel);
extern void HLine_Grayness(uchar * Data, uint X, uint Y, uint Length, uint Width, uint Height, uchar Pixel, LineType Type);
extern void VLine_Grayness(uchar * Data, uint X, uint Y, uint Length, uint Width, uint Height, uchar Pixel, LineType Type);
extern void DrawImage_Grayness(uchar * Dst, uint DstX, uint DstY, uint DstWidth, uint DstHeight, uchar * Src, uint SrcWidth, uint SrcHeight);
extern void ClearBg(Color Clr);
extern void DrawPixel(uint X, uint Y, uchar Color);
extern uchar GetPixel(uint X, uint Y);

/*
	------------------------------VMG13H------------------------------
*/

#define	MAX_WIDTH_VMG13H	320
#define MAX_HEIGHT_VMG13H	200

#define Offset_VMG13H(X, Y) ((X) + (Y) * MAX_WIDTH_VMG13H)

typedef struct _Image_1Bit	/* VMG13HÏÂµÄÍ¼Æ¬ */
{
	uint Width;		/* Í¼Æ¬¿í¶È */
	uint Height;	/* Í¼Æ¬¸ß¶È */
	uchar * Pixels;	/* ÏñËØ¼¯ºÏ. ÏñËØÆ«ÒÆ = X + (Y * Width) */
}Image_VMG13H;

extern void SetPalette_VMG13H(uchar Index, uchar R, uchar G, uchar B);
extern void DrawPixel_VMG13H(uint X, uint Y, uchar Index);
extern uchar GetPixel_VMG13H(uint X, uint Y);
extern void DrawRectangle_VMG13H(uint X, uint Y, uint Width, uint Height, uchar Index);
extern void DrawToScreen_Grayness_VMG13H(uint X, uint Y, uint Width, uint Height, uchar * Image);
extern void DrawToScreen_256Colors_VMG13H(uint X, uint Y, uint Width, uint Height, uchar * Image);

#endif
