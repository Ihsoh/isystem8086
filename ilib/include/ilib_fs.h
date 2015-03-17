/*
	ilib_fs.h
	Ihsoh
	2013-2-25
*/

#ifndef ILIB_FS_H_
#define ILIB_FS_H_

#include "ilib_tpy.h"

typedef struct
{
	uint Year;
	uint Month;
	uint Day;
	uint Hour;
	uint Minute;
	uint Second;
}FileDate;

#define MAXFILENAMELEN	20
#define MAXFILECOUNT	50

void GetDiskName(uchar DiskNumber, char * DiskName);
uchar GetDiskNumber(char * DiskName);
extern void FixFileName(char * Dest, const char * Source);
extern int NewFile(char * FileName);
extern int DelFile(char * FileName);
extern int WriteFile(char * FileName, uchar * Data, uint Len);
extern int AppendFile(char * FileName, uchar Byte);
extern int ReadFile(char * FileName, uchar * Data);
extern int ReadByteFromFile(char * FileName, uint Offset, uchar * Byte);
extern int FileExists(char * FileName);
extern void Format(void);
extern int Rename(char * SrcFileName, char * DstFileName);
extern int GetFileLength(char * FileName);
extern int GetFileChangedDate(char * FileName, FileDate * FD);
extern int GetFileCreatedDate(char * FileName, FileDate * FD);
extern int CopyFile(char * Src, char * Dst);
extern int CutFile(char * Src, char * Dst);
extern int ChangeDisk(uchar DiskNumber);
extern uchar GetCurrentDisk(void);
extern int GetFileName(char * FileName);

#endif
