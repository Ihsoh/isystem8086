#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-2-17							|
#|描述:											|
#|	构建 ilib.lib, entry.obj, entry16.obj		|
#|	mta16h.hdr									|
#*----------------------------------------------*	

TargetDir=bin
Out=ilib.lib
Target=$(TargetDir)\$(Out)
CC=TCC
ASM=ML
LIB=TLIB
IncludeFiles=	include\ilib.h include\ilib_86.h include\ilib_fs.h include\ilib_gra.h \
				include\ilib_io.h include\ilib_str.h include\ilib_sys.h include\ilib_tpy.h

$(Target): 	bin\ilib_io.obj bin\ilib_str.obj bin\ilib_fs.obj bin\ilib_gra.obj bin\ilib_sys.obj \
			bin\ilib_86.obj bin\ilib_asm.obj
	$(LIB) $(Target) +bin\ilib_io.obj +bin\ilib_str.obj +bin\ilib_fs.obj 
	$(LIB) $(Target) +bin\ilib_gra.obj +bin\ilib_sys.obj +bin\ilib_86.obj +bin\ilib_asm.obj
	$(ASM) /c /AT /Fobin\entry.obj entry.asm
	$(ASM) /c /AT /Fobin\entry16.obj entry16.asm
	$(ASM) /AT /Febin\mta16h.hdr mta16h.asm
	
bin\ilib_io.obj: ilib_io.c $(IncludeFiles)
	$(CC) -c -mt -obin\ilib_io.obj -Iinclude ilib_io.c
	
bin\ilib_str.obj: ilib_str.c $(IncludeFiles)
	$(CC) -c -mt -obin\ilib_str.obj -Iinclude ilib_str.c 

bin\ilib_fs.obj: ilib_fs.c $(IncludeFiles)
	$(CC) -c -mt -obin\ilib_fs.obj -Iinclude ilib_fs.c 
		
bin\ilib_gra.obj: ilib_gra.c $(IncludeFiles)
	$(CC) -c -mt -obin\ilib_gra.obj -Iinclude ilib_gra.c
	
bin\ilib_sys.obj: ilib_sys.c $(IncludeFiles)
	$(CC) -c -mt -obin\ilib_sys.obj -Iinclude ilib_sys.c
	
bin\ilib_86.obj: ilib_86.c $(IncludeFiles)
	$(CC) -c -mt -obin\ilib_86.obj -Iinclude ilib_86.c
	
bin\ilib_asm.obj: 
	$(ASM) /c /Fobin\ilib_asm.obj ilib.asm
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
	