#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-8									|
#|														|
#|概述:													|
#|用于构建boot.bin										|
#*------------------------------------------------------*

TargetDir=bin
Out=boot.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target): boot.asm
	$(ASM) -f bin boot.asm -o $(Target)

.PHONY:Clear

Clear:
	-del $(Target)
	