#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-9									|
#|														|
#|概述:													|
#|	用于构建tomta16.exe									|
#*------------------------------------------------------*

TargetDir=bin
Out=pak.exe
Target=$(TargetDir)/$(Out)
CC=GCC

$(Target): pak.c pak.h
	gcc pak.c -o $(Target)
	
.PHONY:Clear

Clear:
	-del $(Objs) $(Target)
