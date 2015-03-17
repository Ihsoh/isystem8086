/*
	文件名:	form.c
	作者:	Ihsoh
	日期:	2013-9-3
*/

#include "form.h"

Form Forms[MAX_FORM_COUNT];

void InitForm(void)
{
	int i;
	
	for(i = 0; i < MAX_FORM_COUNT; i++)
	{
		Forms[i].Top = 0;
		Forms[i].X = 0;
		Forms[i].Y = 0;
		Forms[i].Width = 0;
		Forms[i].Height = 0;
		CopyString(Forms[i].Title, "");
		Forms[i].Used = 0;
		Forms[i].ID = 0;
		Forms[i].TextCount = 0;
	}
}

int FormIDToIndex(uint ID)
{
	int i;

	for(i = 0; i < MAX_FORM_COUNT; i++)
		if(Forms[i].Used && Forms[i].ID == ID) return i;
	return -1;
}

uint AllocForm(uint X, uint Y, uint Width, uint Height, char * Title)
{
	uint ui;
	uint NewFID = 0;
	int Found;
	
	for(NewFID = 0; NewFID < 0xFFFF; NewFID++)
	{
		Found = 1;
		for(ui = 0; ui < MAX_FORM_COUNT; ui++)
			if(Forms[ui].ID == NewFID)
			{
				Found = 0;
				break;
			}
		if(Found) break;
	}
	for(ui = 0; ui < MAX_FORM_COUNT; ui++)
		Forms[ui].Top = 0;
	for(ui = 0; ui < MAX_FORM_COUNT; ui++)
		if(!Forms[ui].Used)
		{
			Forms[ui].Top = 1;
			Forms[ui].X = X;
			Forms[ui].Y = Y;
			Forms[ui].Width = Width < 9 ? 9 : Width;
			Forms[ui].Height = Height < 10 ? 10 : Height;
			CopyString(Forms[ui].Title, Title);
			Forms[ui].Used = 1;
			Forms[ui].ID = NewFID;
			return NewFID;
		}
	return 0xFFFF;
}

void DestroyForm(uint ID)
{
	int Index = FormIDToIndex(ID);

	Forms[Index].ID = 0;
	Forms[Index].Used = 0;
}

void CopyForm(Form * Dst, Form * Src)
{
	Dst->Top = Src->Top;
	Dst->ID = Src->ID;
	Dst->Used = Src->Used;
	Dst->X = Src->X;
	Dst->Y = Src->Y;
	Dst->Width = Src->Width;
	Dst->Height = Src->Height;
	CopyString(Dst->Title, Src->Title);
}

void DrawStringToForm(uint ID, uint X, uint Y, char * Text)
{
	int Index = FormIDToIndex(ID);
	int TextCount = Forms[Index].TextCount;
	
	Forms[Index].Texts[TextCount].X = X;
	Forms[Index].Texts[TextCount].Y = Y;
	CopyString(Forms[Index].Texts[TextCount].Text, Text);
	Forms[Index].TextCount++;
}

int HasForm(uint ID)
{
	return FormIDToIndex(ID);
}
