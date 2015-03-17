#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-11									|
#|														|
#|概述:													|
#|用于构建taskmgr32.bin									|
#*------------------------------------------------------*

TargetDir=bin
Out=taskmgr32.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target):	taskmgr32.asm taskmgr32.inc taskmgr32.mac int21h.inc taskswitch.inc \
			..\..\common\common.mac ..\..\common\386.mac ..\..\common\386.inc
	$(ASM) -f bin taskmgr32.asm -o $(Target)

.PHONY:Clear

Clear:
	cd bin && del /Q *.*
	