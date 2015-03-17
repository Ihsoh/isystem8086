/*
	文件名:	tomta16.c
	作者:	Ihsoh
	日期:	2013-7-10
	描述:	转换为MTA16程序
*/

#include "tomta16.h"

void Die(char * Message)
{
	printf("ToMTA16 Error: ");
	printf(Message);
	printf("\n");
	exit(-1);
}

int GetFileLen(char * FileName)
{
	FILE * FilePtr;
	int Len = 0;
	
	FilePtr = fopen(FileName, "rb");
	if(FilePtr == NULL) Die("Cannot open file!");
	fgetc(FilePtr);
	while(!feof(FilePtr))
	{
		Len++;
		fgetc(FilePtr);
	}
	fclose(FilePtr);
	return Len;
}

int main(int Argc, char * Args[])
{
	char * Header, * Src, * Dst; 
	uchar * Buffer;
	int SrcAppLen = 0;
	FILE * HeaderFilePtr, * SrcFilePtr, * DstFilePtr;

	if(Argc != 4) Die("Arg error!");
	Header = Args[1];
	Src = Args[2];
	Dst = Args[3];
	SrcAppLen = GetFileLen(Src);
	Buffer = (uchar *)malloc((HEADER_LEN + SrcAppLen) * sizeof(uchar));
	if(Buffer == NULL) Die("Cannot alloc memory!");
	HeaderFilePtr = fopen(Header, "rb");
	if(HeaderFilePtr == NULL) Die("Cannot open header file!");
	fread(Buffer, sizeof(uchar), HEADER_LEN, HeaderFilePtr);
	fclose(HeaderFilePtr);
	SrcFilePtr = fopen(Src, "rb");
	if(SrcFilePtr == NULL) Die("Cannot open src file!");
	fread(Buffer + HEADER_LEN, sizeof(uchar), SrcAppLen, SrcFilePtr);
	fclose(SrcFilePtr);
	DstFilePtr = fopen(Dst, "wb");
	if(DstFilePtr == NULL) Die("Cannot open dst file!");
	fwrite(Buffer, sizeof(uchar), HEADER_LEN + SrcAppLen, DstFilePtr);
	fclose(DstFilePtr);
	free(Buffer);
	printf("ToMTA16 Message: Successful!");
	return 0;
}
