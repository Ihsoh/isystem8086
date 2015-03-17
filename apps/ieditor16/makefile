#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-7-23							|
#|描述:											|
#|	构建 iedtr16.bin							|
#*----------------------------------------------*	

TargetDir=bin
Out=iedtr16.bin
Target=$(TargetDir)\$(Out)
CC=TCC
ASM=ML
ToMTA16=..\..\tools\tomta16\bin\tomta16.exe

$(Target):	bin\iedtr16.obj bin\buffer.obj bin\screen.obj \
			..\..\ilib\bin\entry16.obj \
			..\..\ilib\bin\mta16h.hdr \
			..\..\ilib\bin\ilib.lib
	$(ASM) 	/AT /Febin\_$(Out) ..\..\ilib\bin\entry16.obj bin\iedtr16.obj \
			bin\buffer.obj bin\screen.obj ..\..\ilib\bin\ilib.lib
	$(ToMTA16) ..\..\ilib\bin\mta16h.hdr bin\_$(Out) $(Target)

bin\iedtr16.obj: iedtr16.c iedtr16.h
	$(CC) -1 -c -mt -obin\iedtr16.obj -I..\..\ilib\include iedtr16.c
			
bin\buffer.obj: buffer.c buffer.h
	$(CC) -c -mt -obin\buffer.obj -I..\..\ilib\include buffer.c
	
bin\screen.obj: screen.c screen.h
	$(CC) -c -mt -obin\screen.obj -I..\..\ilib\include screen.c
			
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
			