#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-7-23							|
#|描述:											|
#|	构建 gshell.bin								|
#*----------------------------------------------*	

TargetDir=bin
Out=gshell.bin
Target=$(TargetDir)\$(Out)
CC=TCC
ASM=ML
ToMTA16=..\..\tools\tomta16\bin\tomta16.exe

$(Target): 	bin\gshell.obj bin\images.obj bin\keyboard.obj bin\form.obj	\
			bin\ifont.obj bin\graph.obj bin\buffer.obj	\
			..\..\ilib\bin\entry16.obj \
			..\..\ilib\bin\mta16h.hdr \
			..\..\ilib\bin\ilib.lib
	$(ASM) 	/AT /Febin\_$(Out) ..\..\ilib\bin\entry16.obj bin\gshell.obj \
			bin\images.obj bin\keyboard.obj bin\form.obj bin\ifont.obj	\
			bin\graph.obj bin\buffer.obj	\
			..\..\ilib\bin\ilib.lib
	$(ToMTA16) ..\..\ilib\bin\mta16h.hdr bin\_$(Out) $(Target)
	
bin\gshell.obj: gshell.c gshell.h
	$(CC) -c -mt -obin\gshell.obj -I..\..\ilib\include gshell.c
	
bin\images.obj: images.c images.h
	$(CC) -c -mt -obin\images.obj -I..\..\ilib\include images.c

bin\keyboard.obj: keyboard.c keyboard.h
	$(CC) -c -mt -obin\keyboard.obj -I..\..\ilib\include keyboard.c
	
bin\form.obj: form.c form.h
	$(CC) -c -mt -obin\form.obj -I..\..\ilib\include form.c
	
bin\ifont.obj: ifont.c ifont.h
	$(CC) -c -mt -obin\ifont.obj -I..\..\ilib\include ifont.c
	
bin\graph.obj: graph.c graph.h
	$(CC) -c -mt -obin\graph.obj -I..\..\ilib\include graph.c
	
bin\buffer.obj: buffer.c buffer.h
	$(CC) -c -mt -obin\buffer.obj -I..\..\ilib\include buffer.c
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*