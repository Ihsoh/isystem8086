#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-3-28							|
#|描述:											|
#|	构建 taskmgr.bin							|
#*----------------------------------------------*	

TargetDir=bin
Out=taskmgr.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target):	taskmgr.asm taskmgr.inc tasktbl.mac	\
			..\..\common\common.mac
	$(ASM) -f bin taskmgr.asm -o $(Target)
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
