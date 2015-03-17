/*
	文件名:	form.h
	作者:	Ihsoh
	日期:	2013-9-3
*/

#ifndef	FORM_H_
#define	FORM_H_

#include "ilib.h"
#include "button.h"

#define	MAX_FORM_COUNT		32
#define TITLEBAR_HEIGHT		16
#define	MAX_FORM_TEXT_COUNT	10

typedef struct
{
	uint X;
	uint Y;
	char Text[11];
}Form_Text;

typedef struct
{
	int Top;
	uint ID;
	uint Used;
	uint X;
	uint Y;
	uint Width;
	uint Height;
	char Title[11];
	uint TextCount;
	Form_Text Texts[MAX_FORM_TEXT_COUNT];
	
}Form;

extern Form Forms[MAX_FORM_COUNT];

extern void InitForm(void);
extern int FormIDToIndex(uint ID);
extern uint AllocForm(uint X, uint Y, uint Width, uint Height, char * Title);
extern void DestroyForm(uint ID);
extern void CopyForm(Form * Dst, Form * Src);
extern void DrawStringToForm(uint FID, uint X, uint Y, char * Text);

#endif
