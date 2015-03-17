#*------------------------------------------------------*
#|文件名:	makefile									|
#|作者:		Ihsoh										|
#|创建时间:	2013-5-9									|
#|														|
#|概述:													|
#|用于构建所有应用程序									|
#*------------------------------------------------------*

Make=mingw32-make

apps:

#构建任务管理器(16位)
	cd taskmgr && $(Make)
	
#构建Shell
	cd shell && $(Make)
	
#构建GShell
	cd gshell && $(Make)
	
#构建EM
	cd em && $(Make)
	
#构建IEditor16
	cd ieditor16 && $(Make)
	
#构建IASM
	cd iasm && $(Make)
	
#构建IASM Loader
	cd iasmldr && $(Make)
	
.PHONY:Clear

Clear:

#清理构建任务管理器(16位)产生的文件
	cd taskmgr && $(Make) Clear
	
#清理构建Shell产生的文件
	cd shell && $(Make) Clear
	
#清除构建GShell产生的文件
	cd gshell && $(Make) Clear
	
#清除构建EM产生的文件
	cd em && $(Make) Clear
	
#清除构建IEditor16产生的文件
	cd ieditor16 && $(Make) Clear
	
#清除构建IASM产生的文件
	cd iasm && $(Make) Clear
	
#清除构建IASM Loader产生的文件
	cd iasmldr && $(Make) Clear
	