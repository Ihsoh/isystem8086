/*
	文件名:	pak.c
	作者:	Ihsoh
	日期:	2013-12-26
	描述:	分包程序
*/
#include "pak.h"

void Die(char * Message)
{
	printf("PAK Error: ");
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
	if(Argc != 2) Die("Bad parameter!");
	char * Filename = Args[1];
	int FileLen = GetFileLen(Filename);
	if(FileLen > 2 * MAX_FILE_LEN) Die("File too big!");
	else if(FileLen <= MAX_FILE_LEN) Die("File too small!");
	unsigned char FileData[2 * MAX_FILE_LEN];
	FILE * FilePtr;
	FilePtr = fopen(Filename, "rb");
	if(FilePtr == NULL) Die("Cannot open file!");
	fread(FileData, sizeof(unsigned char), FileLen, FilePtr);
	fclose(FilePtr);
	char PakA[300];
	strcpy(PakA, Filename);
	strcat(PakA, ".a.pak");
	FilePtr = fopen(PakA, "wb");
	fwrite(FileData, sizeof(unsigned char), MAX_FILE_LEN, FilePtr);
	fclose(FilePtr);
	char PakB[300];
	strcpy(PakB, Filename);
	strcat(PakB, ".b.pak");
	FilePtr = fopen(PakB, "wb");
	if(FilePtr == NULL) Die("Cannot open file!");
	fwrite(FileData + MAX_FILE_LEN, sizeof(unsigned char), FileLen - MAX_FILE_LEN, FilePtr);
	fclose(FilePtr);
	return 0;
}











