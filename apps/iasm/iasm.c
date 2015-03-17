/*
 * iasm.c
 * Ihsoh
 * 2013-10-7
*/

#include "parser.h"

void main(void)
{
	char Filename[50];
	uint FileLen;
	uint ui;
	char Char;
	
	InitParser();
	Argument(Filename);
	FileLen = GetFileLength(Filename);
	for(ui = 0; ui < FileLen; ui++)
	{
		ReadByteFromFile(Filename, ui, &Char);
		AddCharToParser(Char);
	}
	Scan();
	Parse();
	LinkString(Filename, ".bin");
	SaveToFile(Filename);
	DestroyParser();
	
	
	
}
