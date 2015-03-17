#*----------------------------------------------*
#|文件名:	makefile							|
#|作者:		Ihsoh								|
#|日期:		2013-8-15							|
#|描述:											|
#|	构建 em.bin									|
#*----------------------------------------------*	

TargetDir=bin
Out=em.bin
Target=$(TargetDir)\$(Out)
ASM=NASM

$(Target):	em.asm em.mac	\
			..\..\common\common.mac	\
			..\..\common\common.inc	\
			..\..\common\386.mac	\
			..\..\common\386.inc
	$(ASM) -f bin em.asm -o $(Target)
	
.PHONY:Clear

Clear:
	cd bin && del /Q *.*
