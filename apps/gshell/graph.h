/*
	graph.h
	Ihsoh
	2013-9-7
*/

#ifndef	GRAPH_H_
#define	GRAPH_H_

#include "ilib.h"

extern uint MaxWidth;
extern uint MaxHeight;

extern void GS_InitGraph(uint Mode);
extern void GS_SetPalette(uchar Index, uchar R, uchar G, uchar B);
extern void GS_FillPixel(uchar Pixel);
extern void GS_NextFrame(void);

#endif
