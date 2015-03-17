/*
	createimg.c
	Ihsoh
	2013-1-10
	
	Image Creator1
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAXFILECOUNT 50
#define MAXFILENAMELENGTH 20
#define MAXLINELENGTH 300
#define IMGSIZE 1474560
#define KERNELSIZE 32768
#define BOOTSIZE 512
#define MAXUSERFILESIZE 27648
#define FILEPARAMETERSIZE 512
#define BOOTSTARTPOS 0
#define KERNELSTARTPOS 512
#define MAXMSGLENGTH 200
#define USERFILEDATASTARTPOS 92160
#define USERFILEPARAMETERSTARTPOS 66560

#define FP_FILENAME 0
#define FP_CREATEDYEAR 20
#define FP_CREATEDMONTH 22
#define FP_CREATEDDAY 23
#define FP_CREATEDHOUR 24
#define FP_CREATEDMINUTE 25
#define FP_CREATEDSECOND 26
#define FP_CHANGEDYEAR 27
#define FP_CHANGEDMONTH 29
#define FP_CHANGEDDAY 30
#define FP_CHANGEDHOUR 31
#define FP_CHANGEDMINUTE 32
#define FP_CHANGEDSECOND 33
#define FP_LENGTH 34
#define FP_USE 36
#define FP_RESERVE 37

#define uchar unsigned char

#define USERFILE 0
#define USERFILENAME 1

void Die(char * Msg)
{
	printf("Error: %s", Msg);
	exit(-1);
}

void GetUserFile(const char * Line, char * UserFile, char * UserFileName)
{
	int i;
	int Type = USERFILE;
	int UserFileIndex = 0, UserFileNameIndex = 0;
	for(i = 0; i < strlen(Line); i++)
	{
		if(Line[i] == '|' && Type == USERFILE)
		{
			Type = USERFILENAME;
			continue;
		}
		else if(Line[i] == '|' && Type == USERFILENAME) Die("Script error!");
		if(Type == USERFILE) UserFile[UserFileIndex++] = Line[i];
		else UserFileName[UserFileNameIndex++] = Line[i];
	}
	if(UserFileIndex == 0 || UserFileNameIndex == 0) Die("Script error!");
	UserFile[UserFileIndex] = 0;
	UserFileName[UserFileNameIndex] = 0;
}

int GetFileSize(char * FileName)
{
	char Msg[MAXMSGLENGTH] = {0};
	FILE * FilePtr = fopen(FileName, "rb");
	int Len = 0;
	if(FilePtr == NULL)
	{
		strcpy(Msg, "Cannot open '");
		strcat(Msg, FileName);
		strcat(Msg, "'!");
		Die(Msg);
	}
	fgetc(FilePtr);
	while(!feof(FilePtr))
	{
		fgetc(FilePtr);
		Len++;
	}
	fclose(FilePtr);
	return Len;
}

int main(int Argc, char * Args[])
{
	uchar * Img;
	uchar * Boot;
	uchar * Kernel;
	uchar * UserFile;
	FILE * FilePtr;
	char Files[MAXFILECOUNT][200];
	char FileNames[MAXFILECOUNT][MAXFILENAMELENGTH + 1];
	int FileCount = 0;
	int i, i1;
	char Msg[MAXMSGLENGTH] = {0};
	char * BootFile;
	char * KernelFile;
	char * ScriptFile;
	char * ImgFile;
	char Line[MAXLINELENGTH + 1];
	
	/*Get parameters*/
	if(Argc != 5) Die("Parameter error!");
	BootFile = Args[1];
	KernelFile = Args[2];
	ScriptFile = Args[3];
	ImgFile = Args[4];
	
	/*Alloc imemory*/
	Img = (uchar *)malloc(IMGSIZE);
	Boot = (uchar *)malloc(BOOTSIZE);
	Kernel = (uchar *)malloc(KERNELSIZE);
	UserFile = (uchar *)malloc(MAXUSERFILESIZE);
	if(	Img == NULL 	|| 
		Boot == NULL 	|| 
		Kernel == NULL	||
		UserFile == NULL) Die("Alloc memory error!");
		
	/*Init image*/
	for(i = 0; i < IMGSIZE; i++)
		*(Img + i) = 0;
	
	/*Read script file*/
	FilePtr = fopen(ScriptFile, "r");
	if(FilePtr == NULL) Die("Cannot open script file!");
	while(!feof(FilePtr))
	{
		if(FileCount == MAXFILECOUNT) Die("File count cannot greater than 50!");
		fgets(Line, MAXLINELENGTH + 1, FilePtr);
		for(i = 0; i < strlen(Line); i++)
			if(Line[i] == '\r' || Line[i] == '\n') Line[i] = 0;
		if(strcmp(Line, "") == 0) 
		{
			FileNames[FileCount][0] = 0;
			continue;
		}
		GetUserFile(Line, Files[FileCount], FileNames[FileCount]);
		FileCount++;
	}
	fclose(FilePtr);
	
	/*Read boot file*/
	FilePtr = fopen(BootFile, "rb");
	if(FilePtr == NULL) Die("Cannot open boot file!");
	fread(Boot, sizeof(uchar), BOOTSIZE, FilePtr);
	fclose(FilePtr);
	
	/*Read Kernel file*/
	FilePtr = fopen(KernelFile, "rb");
	if(FilePtr == NULL) Die("Cannot open kernel file!");
	fread(Kernel, sizeof(uchar), KERNELSIZE, FilePtr);
	fclose(FilePtr);
	
	/*Store boot loader to image*/
	for(i = 0; i < BOOTSIZE; i++)
		*(Img + BOOTSTARTPOS + i) = *(Boot + i);
		
	/*Store kernel to image*/
	for(i = 0; i < KERNELSIZE; i++)
		*(Img + KERNELSTARTPOS + i) = *(Kernel + i);
	
	/*Store user file to image*/
	for(i = 0; i < FileCount; i++)
	{
		if(strcmp(FileNames[i], "") == 0) continue;
		int FileParameterStartPos = FILEPARAMETERSIZE * i;
		time_t Time = time(NULL);
		struct tm * TimeS = localtime(&Time);
		/*Get file length*/
		int Len = GetFileSize(Files[i]);
		/*Write file parameter*/
		char FileName[MAXFILENAMELENGTH + 1];
		for(i1 = 0; i1 < MAXFILENAMELENGTH + 1; i1++)
			FileName[i1] = 0;
		strcpy(FileName, FileNames[i]);
		for(i1 = 0; i1 < MAXFILENAMELENGTH; i1++)
			*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_FILENAME + i1) = FileName[i1];
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_LENGTH) = (uchar)Len;
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_LENGTH + 1) = (uchar)((Len >> 8) & 0xFF);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_USE) = 1;
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDYEAR) = (uchar)(1900 + TimeS->tm_year);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDYEAR + 1) = (uchar)((1900 + TimeS->tm_year) >> 8);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDMONTH) = (uchar)(TimeS->tm_mon + 1);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDDAY) = (uchar)(TimeS->tm_mday);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDHOUR) = (uchar)(TimeS->tm_hour);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDMINUTE) = (uchar)(TimeS->tm_min);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CREATEDSECOND) = (uchar)(TimeS->tm_sec);

		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDYEAR) = (uchar)(1900 + TimeS->tm_year);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDYEAR + 1) = (uchar)((1900 + TimeS->tm_year) >> 8);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDMONTH) = (uchar)(TimeS->tm_mon + 1);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDDAY) = (uchar)(TimeS->tm_mday);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDHOUR) = (uchar)(TimeS->tm_hour);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDMINUTE) = (uchar)(TimeS->tm_min);
		*(Img + USERFILEPARAMETERSTARTPOS + FileParameterStartPos + FP_CHANGEDSECOND) = (uchar)(TimeS->tm_sec);
	
		FilePtr = fopen(Files[i], "rb");
		if(FilePtr == NULL)
		{
			strcpy(Msg, "Cannot open'");
			strcat(Msg, Files[i]);
			strcat(Msg, "'!");
			Die(Msg);
		}
		for(i1 = 0; i1 < MAXUSERFILESIZE; i1++)
			*(UserFile + i1) = 0;
		fread(UserFile, sizeof(uchar), MAXUSERFILESIZE, FilePtr);
		fclose(FilePtr);
		for(i1 = 0; i1 < MAXUSERFILESIZE; i1++)
			*(Img + USERFILEDATASTARTPOS + i * MAXUSERFILESIZE + i1) = *(UserFile + i1);
	}
	
	/*Store image to file*/
	FilePtr = fopen(ImgFile, "wb");
	if(FilePtr == NULL) Die("Cannot output image file!");
	fwrite(Img, sizeof(uchar), IMGSIZE, FilePtr);
	fclose(FilePtr);
	
	/*Free memoey*/
	free(Boot);
	free(Img);
	free(Kernel);
	free(UserFile);
	
	return 0;
}
