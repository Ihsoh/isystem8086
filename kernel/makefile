#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-8									|
#|														|
#|概述:													|
#|用于构建kernel.bin									|
#*------------------------------------------------------*

TargetDir=bin
Out=kernel.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target): 	kernel.asm kernel.mac fe.inc disk.inc mouse.inc \
			..\common\common.mac
	$(ASM) -f bin kernel.asm -o $(Target)
	
.PHONY:Clear

Clear:
	-del $(Target)
