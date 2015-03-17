/*
 * stable.h
 * Ihsoh
 * 2013-11-9
*/

#ifndef	STABLE_H_
#define	STABLE_H_

#include "ilib.h"
#include "lexer.h"

#define	SYMBOL_TYPE_LABEL	0

typedef struct
{
	char Name[50];
	int Type;
	uint Offset;
}Symbol;

extern void InitSTable(void);
extern void DestroySTable(void);
extern void AddLabelToSTable(char * Name, uint Offset);
extern uint GetLabelOffsetFromSTable(char * Name, int * Found);
extern int HasLabel(char * Name);

#endif
