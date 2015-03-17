#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-12-26							|
#|描述:											|
#|	构建 iasmldr.bin							|
#*----------------------------------------------*	

TargetDir=bin
Out=iasmldr.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target):	iasmldr.asm
	$(ASM) -f bin iasmldr.asm -o $(Target)
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
