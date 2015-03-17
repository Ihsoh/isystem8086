/*
	ilib_sys.h
	Ihsoh
	2013-2-25
*/

#ifndef ILIB_SYS_H_
#define ILIB_SYS_H_

#include "ilib_tpy.h"

typedef struct
{
	uint MainVer;
	uint Ver;
}ISystemVer;

extern void Exit(void);
extern void Console(char * Command);
extern void ISVer(ISystemVer * Ver);
extern void Argument(char * Buffer);

extern uint AllocBlocks(uint Count);
extern void FreeBlocks(uint BlockID, uint Count);
extern void WriteByte(uint BlockID, uint Offset, uchar Byte);
extern uchar ReadByte(uint BlockID, uint Offset);
extern void WriteBytes(uint BlockID, uint Offset, uint Length, uchar * Bytes);
extern void ReadBytes(uint BlockID, uint Offset, uint Length, uchar * Bytes);

#endif
