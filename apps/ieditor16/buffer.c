/*
	文件名:	buffer.c
	作者:	Ihsoh
	日期:	2013-8-20
	描述:
		缓冲区操作
*/

#include "buffer.h"

uint BufferBID = 0;
uint BufferLength = 0;

/*
	函数名:	InitBuffer
	功能:	初始化缓冲区
	参数:	无
	返回值:	无
*/
void InitBuffer(void)
{
	uint ui;
	uint ui1 = 0;
	
	asm	MOV		AH, 6
	asm	INT		25H;
	
	BufferBID = AllocBlocks(1);
	if(BufferBID == 0)
	{
		PrintStringP("Cannot alloc memory!", CharColor_Red);
		Exit();
	}
	for(ui = 0; ui < 0xFFFF; ui++)
		WriteByte(BufferBID, ui, 0);
	
	BufferLength = ui1;
}

/*
	函数名:	DestroyBuffer
	功能:	销毁缓冲区
	参数:	无
	返回值:	无
*/
void DestroyBuffer(void)
{
	FreeBlocks(BufferBID, 1);
}

/*
	函数名:	InsertCharToBuffer
	功能:	插入字符到缓冲区
	参数:	Offset=偏移
			Char=字符
	返回值:	无
*/
void InsertCharToBuffer(uint Offset, char Char)
{
	uint ui;
	
	if(BufferLength == 0xFFFF) return;
	for(ui = BufferLength; ui >= Offset; ui--)
	{
		char c = ReadByte(BufferBID, ui);
		WriteByte(BufferBID, ui + 1, c);
		if(ui == Offset) break;
	}
	WriteByte(BufferBID, Offset, Char);
	BufferLength++;
}

/*
	函数名:	DeleteCharFromBuffer
	功能:	从缓冲区删除一个字符
	参数:	Offset=偏移
	返回值:	无
*/
void DeleteCharFromBuffer(uint Offset)
{
	uint ui;
	
	for(ui = Offset; ui < BufferLength - 1; ui++)
	{
		char c = ReadByte(BufferBID, ui + 1);
		WriteByte(BufferBID, ui, c);
	}
	BufferLength--;
}

/*
	函数名:	GetCharFromBuffer
	功能:	从缓冲区获取字符
	参数:	Offset=偏移
	返回值:	字符
*/
char GetCharFromBuffer(uint Offset)
{
	return ReadByte(BufferBID, Offset);
}

/*
	函数名:	SetCharToBuffer
	功能:	设置字符到缓冲区
	参数:	Offset=偏移
			Char=字符
	返回值:	字符
*/
void SetCharToBuffer(uint Offset, char Char)
{
	WriteByte(BufferBID, Offset, Char);
}

/*
	函数名:	SaveBufferToFile
	功能:	保存缓冲区到文件
	参数:	FileName=文件名
	返回值:	无
*/
void SaveBufferToFile(char * FileName)
{
	uint ui;
	uchar Byte;
	
	for(ui = 0; ui < BufferLength; ui++)
	{
		Byte = ReadByte(BufferBID, ui);
		AppendFile(FileName, Byte);
	}
}

/*
	函数名:	LoadBufferFromFile
	功能:	从文件中加载缓冲区
	参数:	FileName=文件名
	返回值:	无
*/
void LoadBufferFromFile(char * FileName)
{
	uint ui;
	uint Length;
	uchar Byte;
	
	Length = GetFileLength(FileName);
	BufferLength = Length;
	for(ui = 0; ui < Length; ui++)
	{
		if(ReadByteFromFile(FileName, ui, &Byte))
		{
			PrintStringP("Cannot load file!", CharColor_Red);
			Exit();
		}
		WriteByte(BufferBID, ui, Byte);
		/*PrintHex8(Byte);*/
	}
}

/*
	函数名:	GetLinesFromBuffer
	功能:	从缓冲区内获取行
	参数:	Row=起始行, 从0开始
			Line=要获取的行数
			Lines=获取的行的缓冲区
	返回值:	无
*/
void GetLinesFromBuffer(uint Row, uint Line, char Lines[])
{
	char L[80];
	uint L_Position = 0;
	uint CurrentRow = 0;
	char CurrentChar;
	uint ui, ui1;
	
	for(ui = 0; ui < 80; ui++)
		L[ui] = 0;
	for(ui = 0; ui < BufferLength; ui++)
	{
		CurrentChar = GetCharFromBuffer(ui);
		if(CurrentChar == '\r')
		{
			if(CurrentRow >= Row)
			{
				for(ui1 = 0; ui1 < 80; ui1++)
				{
					Lines[(CurrentRow - Row) * 80 + ui1] = L[ui1];
					L[ui1] = 0;
				}
				if(CurrentRow - Row + 1 == Line) return;
				L_Position = 0;
			}
			CurrentRow++;
			ui++;
			continue;
		}
		if(CurrentRow >= Row) L[L_Position++] = CurrentChar;
	}
	if(L_Position != 0)
		for(ui = 0; ui < 80; ui++)
			Lines[(CurrentRow - Row) * 80 + ui] = L[ui];
}

/*
	函数名:	GetLineCountFromBuffer
	描述:	获取缓冲区内文本的行数.
	参数:	无
	返回值:	行数
*/
uint GetLineCountFromBuffer(void)
{
	uint ui;
	uint LineCount = 0;

	for(ui = 0; ui < BufferLength; ui++)
	{
		char Char = GetCharFromBuffer(ui);
		if(Char == '\r')
		{
			LineCount++;
			ui++;
		}
	}
	return LineCount + 1;
}

/*
	函数名:	GetOffsetByRow
	描述:	获取某行的首个字符的偏移.
	参数:	Row=行
	返回值:	偏移
*/
uint GetOffsetByRow(uint Row)
{
	uint ui;
	uint Offset = 0;
	uint CurrentRow = 0;
	char Char;
	
	for(ui = 0; ui < BufferLength; ui++)
	{
		if(CurrentRow == Row) return Offset;
		Char = GetCharFromBuffer(ui);
		if(Char == '\r')
		{
			CurrentRow++;
			ui++;
			Offset += 2;
		}
		else Offset++;
	}
	return Offset;
}
