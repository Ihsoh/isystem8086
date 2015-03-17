/*
	文件名:	buffer.h
	作者:	Ihsoh
	日期:	2013-8-20
	描述:
		缓冲区操作
*/

#ifndef	BUFFER_H_
#define	BUFFER_H_

#include "ilib.h"

extern void InitBuffer(void);
extern void DestroyBuffer(void);
extern void PushCharToBuffer(char Char);
extern void PopCharFromBuffer(void);
extern void InsertCharToBuffer(uint Offset, char Char);
extern void DeleteCharFromBuffer(uint Offset);
extern char GetCharFromBuffer(uint Offset);
extern void SetCharToBuffer(uint Offset, char Char);
extern void SaveBufferToFile(char * FileName);
extern void LoadBufferFromFile(char * FileName);
extern void GetLinesFromBuffer(uint Row, uint Line, char Lines[]);
extern uint GetLineCountFromBuffer(void);
extern uint GetOffsetByRow(uint Row);

#endif
