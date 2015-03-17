/*
	createimg.c
	Ihsoh
	2013-1-3
	
	Image Creator
*/

#include <stdio.h>
#include <stdlib.h>

#define BOOTLOADERFILE "boot.bin"
#define KERNELFILE "kernel.bin"
#define IMGFILE "isystem.img"
#define BOOTLOADERLENGTH 512
#define KERNELLENGTH 32768
#define IMGFILELENGTH 1474560

#define uchar unsigned char

int main()
{
	uchar * Img;
	uchar * BootLoader, * Kernel;
	int i;
	Img = (uchar *)malloc(IMGFILELENGTH);
	BootLoader = (uchar *)malloc(BOOTLOADERLENGTH);
	Kernel = (uchar *)malloc(KERNELLENGTH);
	if(Img == NULL || BootLoader == NULL || Kernel == NULL)
	{
		printf("Alloc memory error!");
		getchar();
		return -1;
	}
	FILE * FilePtr = fopen(BOOTLOADERFILE, "rb");
	if(FilePtr == NULL)
	{
		printf("Open \"");
		printf(BOOTLOADERFILE);
		printf("\" error!");
		getchar();
		return -1;
	}
	fread(BootLoader, sizeof(uchar), BOOTLOADERLENGTH, FilePtr);
	fclose(FilePtr);
	FilePtr = fopen(KERNELFILE, "rb");
	if(FilePtr == NULL)
	{
		printf("Open \"");
		printf(KERNELFILE);
		printf("\" error!");
		getchar();
		return -1;
	}
	fread(Kernel, sizeof(uchar), KERNELLENGTH, FilePtr);
	fclose(FilePtr);
	for(i = 0; i < BOOTLOADERLENGTH; i++)
		*(Img + i) = *(BootLoader + i);
	for(i = 0; i < KERNELLENGTH; i++)
		*(Img + BOOTLOADERLENGTH + i) = *(Kernel + i);
	FilePtr = fopen(IMGFILE, "wb");
	fwrite(Img, sizeof(uchar), IMGFILELENGTH, FilePtr);
	fclose(FilePtr);
	return 0;
}
