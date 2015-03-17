#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-10-7									|
#|														|
#|概述:													|
#|用于构建IASM											|
#*------------------------------------------------------*

TargetDir=bin
Out=iasm.bin
Target=$(TargetDir)\$(Out)
CC=TCC
ASM=ML
ToMTA16=..\..\tools\tomta16\bin\tomta16.exe
PAK=..\..\..\tools\pak\bin\pak.exe

$(Target): 	bin\iasm.obj bin\encoder.obj bin\lexer.obj bin\parser.obj bin\stable.obj	\
			bin\error.obj	\
			..\..\ilib\bin\entry16.obj 		\
			..\..\ilib\bin\mta16h.hdr 		\
			..\..\ilib\bin\ilib.lib
	$(ASM) /AT /Febin\_iasm.bin ..\..\ilib\bin\entry16.obj bin\iasm.obj bin\lexer.obj  bin\error.obj bin\stable.obj bin\parser.obj bin\encoder.obj ..\..\ilib\bin\ilib.lib
	$(ToMTA16) ..\..\ilib\bin\mta16h.hdr bin\_iasm.bin bin\iasm.bin
	cd bin && $(PAK) iasm.bin
	
bin\error.obj: error.c error.h
	$(CC) -c -mt -obin\error.obj -I..\..\ilib\include error.c
	
bin\stable.obj: stable.c stable.h lexer.h
	$(CC) -c -mt -obin\stable.obj -I..\..\ilib\include stable.c
	
bin\parser.obj: parser.c parser.h lexer.h encoder.h
	$(CC) -c -mt -obin\parser.obj -I..\..\ilib\include parser.c
	
bin\lexer.obj: lexer.c lexer.h
	$(CC) -c -mt -obin\lexer.obj -I..\..\ilib\include lexer.c
	
bin\iasm.obj: iasm.c iasm.h
	$(CC) -c -mt -obin\iasm.obj -I..\..\ilib\include iasm.c
	
bin\encoder.obj: encoder.c encoder.h opcode.h reg.h
	$(CC) -c -mt -obin\encoder.obj -I..\..\ilib\include encoder.c
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
	