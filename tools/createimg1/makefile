#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-9									|
#|														|
#|概述:													|
#|用于构建createimg1.exe								|
#*------------------------------------------------------*

TargetDir=bin
Out=createimg1.exe
Target=$(TargetDir)/$(Out)
CC=GCC
CFlags=-o2
Objs=bin\createimg1.obj

$(Target): $(Objs)
	$(CC) $(CFlags) $(Objs) -o $(Target)

bin\createimg1.obj: createimg1.c
	$(CC) $(CFlags) -c createimg1.c -o bin\createimg1.obj
	
.PHONY:Clear

Clear:
	-del $(Objs) $(Target)
