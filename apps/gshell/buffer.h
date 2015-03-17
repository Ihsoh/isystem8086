/*
	文件名:	buffer.h
	作者:	Ihsoh
	日期:	2013-9-13
*/

#ifndef	BUFFER_H_
#define	BUFFER_H_

#include "ilib.h"
#include "graph.h"

extern void InitBuffer(void);
extern void DestroyBuffer(void);
extern void SetPixelToBuffer(uint X, uint Y, uchar Pixel);
extern uchar GetPixelFromBuffer(uint X, uint Y);
extern void GetPixelsFromBuffer(uint X, uint Y, uint Count, uchar * Pixels);
extern void DrawHLineToBuffer(uint X, uint Y, uint Length, uchar Pixel, uint Point);
extern void DrawVLineToBuffer(uint X, uint Y, uint Length, uchar Pixel, uint Point);
extern void DrawGraynessImageToBuffer(uint X, uint Y, uchar * Image, uint Width, uint Height);

#endif
