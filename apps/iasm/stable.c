/*
 * stable.h
 * Ihsoh
 * 2013-11-9
*/

#include "stable.h"

static uint BufferBID = 0;
static uint SymbolCount = 0;

void InitSTable(void)
{
	BufferBID = AllocBlocks(1);
}

void DestroySTable(void)
{
	FreeBlocks(BufferBID, 1);
}

void AddLabelToSTable(char * Name, uint Offset)
{
	Symbol S;
	uint ui;
	uchar * UCPtr;
	
	CopyString(S.Name, Name);
	S.Type = SYMBOL_TYPE_LABEL;
	S.Offset = Offset;
	UCPtr = (uchar *)&S;
	for(ui = SymbolCount * sizeof(Symbol); ui < SymbolCount * sizeof(Symbol) + sizeof(Symbol); ui++)
		WriteByte(BufferBID, ui, *(UCPtr++));
	SymbolCount++;
}

uint GetLabelOffsetFromSTable(char * Name, int * Found)
{
	Symbol S;
	uint ui, ui1;
	uchar * UCPtr;
	
	*Found = 1;
	for(ui1 = 0; ui1 < SymbolCount; ui1++)
	{
		UCPtr = (uchar *)&S;
		for(ui = ui1 * sizeof(Symbol); ui < ui1 * sizeof(Symbol) + sizeof(Symbol); ui++)
			*(UCPtr++) = ReadByte(BufferBID, ui);
		if(StringCmp(Name, S.Name)) return S.Offset;
	}
	*Found = 0;
}

int HasLabel(char * Name)
{
	Symbol S;
	uint ui, ui1;
	uchar * UCPtr;
	
	for(ui1 = 0; ui1 < SymbolCount; ui1++)
	{
		UCPtr = (uchar *)&S;
		for(ui = ui1 * sizeof(Symbol); ui < ui1 * sizeof(Symbol) + sizeof(Symbol); ui++)
			*(UCPtr++) = ReadByte(BufferBID, ui);
		if(StringCmp(Name, S.Name)) return 1;
	}
	return 0;
}
