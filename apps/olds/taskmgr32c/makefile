#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-13									|
#|														|
#|概述:													|
#|用于构建taskmgr32c.bin								|
#*------------------------------------------------------*

TargetDir=bin
Out=taskmgr32c.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target): 	taskmgr32c.asm \
			..\..\common\common.mac ..\..\common\386.mac ..\..\common\386.inc
	$(ASM) -f bin taskmgr32c.asm -o $(Target)
	
.PHONT:Clear

Clear:
	cd bin && del /Q *.*
