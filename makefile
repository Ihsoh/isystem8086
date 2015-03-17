#*------------------------------------------------------*
#|�ļ���:	makefile									|
#|����:		Ihsoh										|
#|����ʱ��:	2013-2-15									|
#|														|
#|����:													|
#|���ڹ���isystem.img									|
#*------------------------------------------------------*

TargetDir=isystem
Out=isystem.img
Target=$(TargetDir)\$(Out)
CreateImg=tools\createimg1\bin\createimg1.exe
Make=mingw32-make

$(Target):
#����ilib
	cd ilib && $(Make)

#����boot
	cd boot && $(Make)
	
#����kernel
	cd kernel && $(Make)
	
#��������Ӧ�ó���
	cd apps && $(Make)
	
#��������Ӱ��
	$(CreateImg) boot\bin\boot.bin kernel\bin\kernel.bin userfiles.txt $(Target)
	
.PHONY:Clear

Clear:
#ɾ��ilib
	cd ilib && $(Make) Clear

#ɾ��boot
	cd boot && $(Make) Clear
	
#ɾ��kernel
	cd kernel && $(Make) Clear
	
#ɾ������Ӧ�ó���
	cd apps && $(Make) Clear
	
#ɾ������Ӱ��
	-del $(Target)
