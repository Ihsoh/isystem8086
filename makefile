#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-2-15									|
#|														|
#|概述:													|
#|用于构建isystem.img									|
#*------------------------------------------------------*

TargetDir=isystem
Out=isystem.img
Target=$(TargetDir)\$(Out)
CreateImg=tools\createimg1\bin\createimg1.exe
Make=mingw32-make

$(Target):
#构建ilib
	cd ilib && $(Make)

#构建boot
	cd boot && $(Make)
	
#构建kernel
	cd kernel && $(Make)
	
#构建所有应用程序
	cd apps && $(Make)
	
#构建软盘影像
	$(CreateImg) boot\bin\boot.bin kernel\bin\kernel.bin userfiles.txt $(Target)
	
.PHONY:Clear

Clear:
#删除ilib
	cd ilib && $(Make) Clear

#删除boot
	cd boot && $(Make) Clear
	
#删除kernel
	cd kernel && $(Make) Clear
	
#删除所有应用程序
	cd apps && $(Make) Clear
	
#删除软盘影像
	-del $(Target)
