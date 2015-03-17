/*
	ilib_fs.c
	Ihsoh
	2013-2-20
*/

#include "ilib.h"

#define MAX_DISK_COUNT		28

typedef struct _DiskName
{
	uchar Number;
	char Name[3];
}DiskName;

DiskName DiskNames[MAX_DISK_COUNT] = {	{0, "FA"}, {1, "FB"}, {2, "FC"}, {3, "FD"},
										{4, "FE"}, {5, "FF"}, {6, "FG"}, {7, "FH"},
										{8, "FI"}, {9, "FJ"}, {10, "FK"}, {11, "FL"},
										{12, "FM"}, {13, "FN"}, {14, "FO"}, {15, "FP"},
										{16, "FQ"}, {17, "FR"}, {18, "FS"}, {19, "FT"},
										{20, "FU"}, {21, "FV"}, {22, "FW"}, {23, "FX"},
										{24, "FY"}, {25, "FZ"}, {64, "DA"}, {65, "DB"}};

/*
	过程名:	GetDiskName
	描述:	获取盘符
	参数:	DiskNumber=磁盘号
			DiskName=用于储存盘符的指针
	返回值:	无
*/
void GetDiskName(uchar DiskNumber, char * DiskName)
{
	uchar uc;
	for(uc = 0; uc < MAX_DISK_COUNT; uc++)
		if(DiskNames[uc].Number == DiskNumber)
		{
			CopyString(DiskName, DiskNames[uc].Name);
			break;
		}
}

/*
	过程名:	GetDiskNumber
	描述:	获取磁盘号
	参数:	DiskName=用于储存盘符的指针
	返回值:	磁盘号
*/
uchar GetDiskNumber(char * DiskName)
{
	uchar uc;
	for(uc = 0; uc < MAX_DISK_COUNT; uc++)
		if(StringCmp(DiskNames[uc].Name, DiskName)) return DiskNames[uc].Number;
	return 0xFF;
}

void FixFileName(char * Dest, const char * Source)
{
	uint ui;
	for(ui = 0; ui < MAXFILENAMELEN; ui++)
		Dest[ui] = '\0';
	for(ui = 0; ui < MAXFILENAMELEN && Source[ui] != '\0'; ui++)
		Dest[ui] = Source[ui];
}

uchar OldDiskNumber;

void SwitchDisk(char FileName[], char FN[])
{
	uchar DiskNumber;
	uint Len;
	
	Len = GetStringLen(FileName);
	if(Len > 3 && FileName[2] == ':')
	{
		FileName[2] = 0;
		if((DiskNumber = GetDiskNumber(FileName)) == 0xFF)
		{
			OldDiskNumber = 0xFF;
			FixFileName(FN, FileName);
		}
		else
		{
			OldDiskNumber = GetCurrentDisk();
			ChangeDisk(DiskNumber);
			FixFileName(FN, FileName + 3);
		}
	}
	else 
	{
		OldDiskNumber = 0xFF;
		FixFileName(FN, FileName);
	}
}

void ResumeDisk(void)
{
	if(OldDiskNumber != 0xFF) ChangeDisk(OldDiskNumber);
}

extern int _NewFile(char * FileName);

int NewFile(char * FileName)
{
	char FN[MAXFILENAMELEN];
	int RV;
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return 1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _NewFile(FN);
	ResumeDisk();
	return RV;
}

extern int _DelFile(char * FileName);

int DelFile(char * FileName)
{
	char FN[MAXFILENAMELEN];
	int RV;
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _DelFile(FN);
	ResumeDisk();
	return RV;
}

extern int _WriteFile(char * FileName, uchar * Data, uint Len);

int WriteFile(char * FileName, uchar * Data, uint Len)
{
	char FN[MAXFILENAMELEN];
	int RV;
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _WriteFile(FN, Data, Len);
	ResumeDisk();
	return RV;
}

int AppendFile(char * FileName, uchar Byte)
{
	char FN[MAXFILENAMELEN];
	int RV;
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	asm	PUSH	SI;
	_SI = FN;
	_AL = Byte;
	asm	MOV		AH, 38;
	asm	INT		21H;
	asm	POP		SI;
	RV = 0x01 & _AX;
	ResumeDisk();
	return RV;
}

extern int _ReadFile(char * FileName, uchar * Data);

int ReadFile(char * FileName, uchar * Data)
{
	char FN[MAXFILENAMELEN];
	int RV;
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _ReadFile(FN, Data);
	ResumeDisk();
	return RV;
}

int ReadByteFromFile(char * FileName, uint Offset, uchar * Byte)
{
	int RV;
	char FN[MAXFILENAMELEN];
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	SI;
	_SI = FN;
	_BX = Offset;
	asm	MOV		AH, 39;
	asm	INT		21H;
	RV = (int)_AL;
	/* !!!Byte = xxx操作会使用BX!!! */
	_AL = _BL;
	*Byte = _AL;
	
	asm	POP		SI;
	asm	POP		BX;
	asm	POP		AX;
	ResumeDisk();
	return RV;
}

extern int _FileExists(char * FileName);

int FileExists(char * FileName)
{
	char FN[MAXFILENAMELEN];
	int RV;
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _FileExists(FN);
	ResumeDisk();
	return RV;
}

extern int _Rename(char * SrcFileName, char * DstFileName);

int Rename(char * SrcFileName, char * DstFileName)
{
	char SrcFN[MAXFILENAMELEN], DstFN[MAXFILENAMELEN];
	char Buffer[256];
	int RV;
	
	if(!(GetStringLen(SrcFileName) && GetStringLen(DstFileName))) return -1;
	CopyString(Buffer, SrcFileName);
	SwitchDisk(Buffer, SrcFN);
	FixFileName(DstFN, DstFileName);
	RV = _Rename(SrcFN, DstFN);
	ResumeDisk();
	return RV;
}

extern int _GetFileLength(char * FileName);

int GetFileLength(char * FileName)
{
	char FN[MAXFILENAMELEN];
	char Buffer[256];
	int RV;
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _GetFileLength(FN);
	ResumeDisk();
	return RV;
}

extern int _GetFileCD(char * FileName, uchar * ChangedDate);

int GetFileChangedDate(char * FileName, FileDate * FD)
{
	char FN[MAXFILENAMELEN];
	int RV;
	uint ui;
	uchar ChangedDate[7];
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _GetFileCD(FN, ChangedDate);
	FD->Year = *(uint*)(ChangedDate + 0);
	FD->Month = (uint)*(ChangedDate + 2);
	FD->Day = (uint)*(ChangedDate + 3);
	FD->Hour = (uint)*(ChangedDate + 4);
	FD->Minute = (uint)*(ChangedDate + 5);
	FD->Second = (uint)*(ChangedDate + 6);
	ResumeDisk();
	return RV;
}

extern int _GetFileCrD(char * FileName, uchar * CreatedDate);

int GetFileCreatedDate(char * FileName, FileDate * FD)
{
	char FN[MAXFILENAMELEN];
	int RV;
	uint ui;
	uchar CreatedDate[7];
	char Buffer[256];
	
	if(GetStringLen(FileName) == 0) return -1;
	CopyString(Buffer, FileName);
	SwitchDisk(Buffer, FN);
	RV = _GetFileCrD(FN, CreatedDate);
	FD->Year = *(uint*)(CreatedDate + 0);
	FD->Month = (uint)*(CreatedDate + 2);
	FD->Day = (uint)*(CreatedDate + 3);
	FD->Hour = (uint)*(CreatedDate + 4);
	FD->Minute = (uint)*(CreatedDate + 5);
	FD->Second = (uint)*(CreatedDate + 6);
	ResumeDisk();
	return RV;
}

uchar ParsePath(char * Path, char * FileName)
{
	char DiskName[3] = {0, 0, 0};
	uint Len = GetStringLen(Path);
	if(Len >= 3)
		if(Path[2] == ':')
		{
			uchar DiskNumber;
			DiskName[0] = Path[0];
			DiskName[1] = Path[1];
			DiskNumber = GetDiskNumber(DiskName);
			CopyString(FileName, Path + 3);
			return DiskNumber;
		}
		else CopyString(FileName, Path);
	else CopyString(FileName, Path);
	return 0;
}

extern int _CopyFile(char * Src, char * Dst, uchar DstDiskNumber);

int CopyFile(char * Src, char * Dst)
{
	char SrcFN[MAXFILENAMELEN], DstFN[MAXFILENAMELEN + 1];
	uchar DstDiskNumber;
	char Buffer[256];
	int RV;
	
	CopyString(Buffer, Src);
	SwitchDisk(Buffer, SrcFN);
	DstDiskNumber = ParsePath(Dst, DstFN);
	RV = _CopyFile(SrcFN, DstFN, DstDiskNumber);
	ResumeDisk();
	return RV;
}

extern int _CutFile(char * Src, char * Dst, uchar DstDiskNumber);

int CutFile(char * Src, char * Dst)
{
	char SrcFN[MAXFILENAMELEN], DstFN[MAXFILENAMELEN + 1];
	uchar DstDiskNumber;
	char Buffer[256];
	int RV;
	
	CopyString(Buffer, Src);
	SwitchDisk(Buffer, SrcFN);
	DstDiskNumber = ParsePath(Dst, DstFN);
	RV = _CutFile(SrcFN, DstFN, DstDiskNumber);
	ResumeDisk();
	return RV;
}

uint GetFileCount(void)
{
	uint FileCount;
	asm	PUSH	AX;
	asm	PUSH	CX;
	asm	MOV		AH, 30;
	asm	INT		21H;
	FileCount = _CX;
	asm	POP		CX;
	asm	POP		AX;
	return FileCount;
}

uint GetFileName_FileCount = 0;
char GetFileName_FileNames[MAXFILENAMELEN * MAXFILECOUNT];

int GetFileName(char * FileName)
{
	if(FileName == NULL)
	{
		GetFileName_FileCount = GetFileCount();
		asm	PUSH	AX;
		asm	PUSH	DI;
		asm	MOV		DI, OFFSET GetFileName_FileNames;
		asm	MOV		AH, 31;
		asm	INT		21H;
		asm	POP		DI;
		asm	POP		AX;
	}
	else
		if(GetFileName_FileCount == 0) return 0;
		else
		{
			CopyString(FileName, GetFileName_FileNames + (GetFileName_FileCount - 1) * MAXFILENAMELEN);
			GetFileName_FileCount--;
		}
	return 1;
}

void InitDisk(void)
{
	asm	PUSH	AX;
	asm	MOV		AH, 44;
	asm	INT		21H;
	asm	POP		AX;
}

int WriteDiskSector(uchar DiskNumber,
					uchar MS,
					uint SectorOffsetH,
					uint SectorOffsetL,
					uchar * Buffer)
{
	asm	PUSH	BX;
	asm	PUSH	CX;
	asm	PUSH	DX;
	asm	PUSH	SI;
	_AL = DiskNumber;
	_CX = SectorOffsetH;
	_DX = SectorOffsetL;
	_SI = Buffer;
	_BL = MS;
	asm	MOV		AH, 45;
	asm	INT		21H;
	asm	POP		SI;
	asm	POP		DX;
	asm	POP		CX;
	asm	POP		BX;
	asm	XOR		AH, AH;
}

int ReadDiskSector(	uchar DiskNumber,
					uchar MS,
					uint SectorOffsetH,
					uint SectorOffsetL,
					uchar * Buffer)
{
	asm	PUSH	BX;
	asm	PUSH	CX;
	asm	PUSH	DX;
	asm	PUSH	SI;
	_AL = DiskNumber;
	_CX = SectorOffsetH;
	_DX = SectorOffsetL;
	_DI = Buffer;
	_BL = MS;
	asm	MOV		AH, 46;
	asm	INT		21H;
	asm	POP		SI;
	asm	POP		DX;
	asm	POP		CX;
	asm	POP		BX;
	asm	XOR		AH, AH;
}

void GetDiskSectorCount(uchar DiskNumber, uint * CountH, uint * CountL)
{
	asm	PUSH	AX;
	asm	PUSH	DX;
	asm	MOV		AH, 47;
	asm	INT		21H;
	*CountH = _DX;
	*CountL = _AX;
	asm	POP		DX;
	asm	POP		AX;
}
