#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-9									|
#|														|
#|概述:													|
#|	用于构建tomta16.exe									|
#*------------------------------------------------------*

TargetDir=bin
Out=tomta16.exe
Target=$(TargetDir)/$(Out)
CC=GCC

$(Target): tomta16.c tomta16.h
	gcc tomta16.c -o $(Target)
	
.PHONY:Clear

Clear:
	-del $(Objs) $(Target)
