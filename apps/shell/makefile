#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-7-3							|
#|描述:											|
#|	构建 shell.bin								|
#*----------------------------------------------*	

TargetDir=bin
Out=shell.bin
Target=$(TargetDir)\$(Out)
CC=TCC
ASM=ML
ToMTA16=..\..\tools\tomta16\bin\tomta16.exe

$(Target):	bin\shell.obj bin\shella.obj bin\error.obj \
		..\..\ilib\bin\entry16.obj \
		..\..\ilib\bin\mta16h.hdr \
		..\..\ilib\bin\ilib.lib
	$(ASM) /AT /Febin\_shell.bin ..\..\ilib\bin\entry16.obj bin\shell.obj bin\shella.obj bin\error.obj ..\..\ilib\bin\ilib.lib
	$(ToMTA16) ..\..\ilib\bin\mta16h.hdr bin\_shell.bin bin\shell.bin
	
bin\shell.obj: shell.c shell.h
	$(CC) -c -mt -obin\shell.obj -I..\..\ilib\include shell.c
	
bin\shella.obj: shella.asm
	$(ASM) /c /AT /Fobin\shella.obj shella.asm
	
bin\error.obj: error.c error.h
	$(CC) -c -mt -obin\error.obj -I..\..\ilib\include error.c
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
