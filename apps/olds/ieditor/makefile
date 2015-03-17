#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-4-1									|
#|														|
#|概述:													|
#|用于构建ieditor.bin									|
#*------------------------------------------------------*

TargetDir=bin
Out=ieditor.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target): ieditor.asm ieditor.mac ieditor.inc ..\..\common\common.mac
	$(ASM) -f bin ieditor.asm -o $(Target)

.PHNOY:Clear
	
Clear:
	cd bin && del /Q *.* 
	