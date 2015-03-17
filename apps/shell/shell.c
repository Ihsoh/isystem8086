/*
 * shell.c
 * Ihsoh
 * 2013-7-4
*/

#include "shell.h"

char * HelpInfo	= "exit     Return to system\r\n"\
"tasklist Print all tasks\r\n"\
"killtask kill a task\r\n"\
"nexttask switch to next task\r\n";

char * Commands[] = {	/* System */
						"time",
						"cls",
						"ver",
						"help",
						"newfile",
						"delfile",
						"files",
						"exec",
						"reboot",
						"format",
						"setdate",
						"settime",
						"rename",
						"echooff",
						"echoon",
						"pause",
						"echo",
						"?",
						"cd",
						"copy",
						"cut",
						
						/* Shell */
						"exit",
						"tasklist",
						"killtask",
						"taskswitch",
						"rem",
						
						/* End String */
						"<END>"};
						
void ExecuteBatch(char * FileName);
						
/*
	函数名:	IsCommand
	描述:	确认字符串是否是命令
	参数:	Str=字符串
	返回值:	1=是, 0=不是
*/					
int IsCommand(char * Str)
{
	int i = 0;
	while(!StringCmp(Commands[i], "<END>"))
		if(StringCmp(Commands[i], Str)) return 1;
	return 0;
}

/*
	函数名:	Init
	描述:	初始化Shell
	参数:	无
	返回值:	无
*/
void Init(void)
{
	Console("echooff");
    Console("cls");
    PrintString(MESSAGE);
}

/*
	函数名:	ShellExit
	描述:	退出Shell
	参数:	无
	返回值:	无
*/
void ShellExit(void)
{
	Console("echoon");
	asm	MOV		AH, 0;
	asm	INT		23H;
}

/*
	函数名:	Shell
	描述:	处理命令
	参数:	Command=命令
	返回值:	0=正常返回
			1=输入的命令请求退出
*/
int Shell(char * Command)
{
	char Strs[10][SPLITSTRING_MAXBUFFERLEN];
	uint StrCount;
	uint CommandLen;
	
	Trim(Command);
	if(GetStringLen(Command) == 0) return 0;
	StrCount = SplitString(Strs, Command, ' ', SplitMode_RemoveEmpty);
	/* exit */
    if(StringCmp(Strs[0], COMMAND_EXIT)) 
		if(StrCount != 1) Error_ArgErr();
		else return 1;
	/* newfile */
	else if(StringCmp(Strs[0], COMMAND_NEWFILE))
	{
		char NewFileCommand[MAX_COMMAND_LEN];
		if(StrCount != 2) Error_ArgErr();
		else
		{
			CopyString(NewFileCommand, COMMAND_NEWFILE);
			LinkString(NewFileCommand, " ");
			LinkString(NewFileCommand, Strs[1]);
			Console(NewFileCommand);
		}
	}
	/* delfile */
	else if(StringCmp(Strs[0], COMMAND_DELFILE))
	{
		char DelFileCommand[MAX_COMMAND_LEN];
		if(StrCount != 2) Error_ArgErr();
		else
		{
			CopyString(DelFileCommand, COMMAND_DELFILE);
			LinkString(DelFileCommand, " ");
			LinkString(DelFileCommand, Strs[1]);
			Console(DelFileCommand);
		}
	}
	/* rename */
	else if(StringCmp(Strs[0], COMMAND_RENAME))
	{
		char RenameCommand[MAX_COMMAND_LEN];
		if(StrCount != 3) Error_ArgErr();
		else
		{
			CopyString(RenameCommand, COMMAND_RENAME);
			LinkString(RenameCommand, " ");
			LinkString(RenameCommand, Strs[1]);
			LinkString(RenameCommand, " ");
			LinkString(RenameCommand, Strs[2]);
			Console(RenameCommand);
		}
	}
	/* cd */
	else if(StringCmp(Strs[0], COMMAND_CD))
	{
		char CDCommand[MAX_COMMAND_LEN];
		if(StrCount != 2) Error_ArgErr();
		else
		{
			CopyString(CDCommand, COMMAND_CD);
			LinkString(CDCommand, " ");
			LinkString(CDCommand, Strs[1]);
			Console(CDCommand);
		}
	}
	/* copy */
	else if(StringCmp(Strs[0], COMMAND_COPY))
	{
		char CopyCommand[MAX_COMMAND_LEN];
		if(StrCount != 3) Error_ArgErr();
		else
		{
			CopyString(CopyCommand, COMMAND_COPY);
			LinkString(CopyCommand, " ");
			LinkString(CopyCommand, Strs[1]);
			LinkString(CopyCommand, " ");
			LinkString(CopyCommand, Strs[2]);
			PrintHex16(_SP);
			PrintChar(' ');
			if(CopyFile(Strs[1], Strs[2])) Error_CannotCopyFile();
			PrintHex16(_SP);
		}
	}
	/* cut */
	else if(StringCmp(Strs[0], COMMAND_CUT))
	{
		char CutCommand[MAX_COMMAND_LEN];
		if(StrCount != 3) Error_ArgErr();
		else
		{
			CopyString(CutCommand, COMMAND_CUT);
			LinkString(CutCommand, " ");
			LinkString(CutCommand, Strs[1]);
			LinkString(CutCommand, " ");
			LinkString(CutCommand, Strs[2]);
			if(CutFile(Strs[1], Strs[2])) Error_CannotCutFile();
		}
	}
	/* exec */
	else if(StringCmp(Strs[0], COMMAND_EXEC))
	{
		char FileName[20];
		char Argument[128];
		int Flag = 0;
		uint ui, ui1 = 0;
		
		for(ui = 5; ui < GetStringLen(Command); ui++)
		{
			if(Command[ui] == ' ')
			{
				Flag = 1;
				continue;
			}
			if(Flag) 
				if(ui1 == 127) Error_ArgumentLengthTooLong();
				else Argument[ui1++] = Command[ui];
		}
		Argument[ui1] = 0;
		if(StrCount < 2) Error_ArgErr();
		else
		{
			FixFileName(FileName, Strs[1]);
			if(ExecMTA16(FileName, Argument)) Error_CannotLoadApp();
		}
	}
	/* tasklist */
	else if(StringCmp(Strs[0], COMMAND_TASKLIST))
	{
		uchar uc;
		if(StrCount != 1) Error_ArgErr();
		else
			for(uc = 0; uc < 6; uc++)
			{
				uint TaskID = uc;
				if(TaskSlotIsUsed(uc))
				{
					char TaskName[20];
					PrintStringP("[", CharColor_Yellow);
					PrintUIntegerP(TaskID, CharColor_Yellow);
					PrintStringP("] ", CharColor_Yellow);
					GetTaskName(uc, TaskName);
					PrintStringP(TaskName, CharColor_Yellow);
					PrintString("\r\n");
				}
				else
				{
					PrintString("[");
					PrintUInteger(TaskID);
					PrintString("] ");
					PrintString("--------------------\r\n");
				}
			}
	}
	/* killtask */
	else if(StringCmp(Strs[0], COMMAND_KILLTASK))
	{
		if(StrCount != 2) Error_ArgErr();
		else KillTask((uchar)StringToUInteger(Strs[1]));
	}
	/* nexttask */
	else if(StringCmp(Strs[0], COMMAND_NEXTTASK))
	{
		if(StrCount != 1) Error_ArgErr();
		else asm INT	24H;
	}
	/* help, ? */
	else if(StringCmp(Strs[0], COMMAND_HELP1) || StringCmp(Strs[0], COMMAND_HELP2))
	{
		Console(Strs[0]);
		Console("pause");
		PrintStringP(HelpInfo, CharColor_Yellow);
	}
	/* rem */
	else if(StringCmp(Strs[0], COMMAND_REM));
	
	
	else if(StringCmp(Strs[0], "test"))
	{
		uchar Buffer[512];
		Buffer[0] = 'A';
		Buffer[1] = 'B';
		Buffer[2] = 'C';
		Buffer[3] = '\r';
		Buffer[4] = '\n';
		Buffer[5] = 0;
		InitDisk();
		WriteDiskSector(0, 0, 0, 0, Buffer);
		Buffer[0] = 0;
		Buffer[1] = 0;
		Buffer[2] = 0;
		Buffer[3] = 0;
		Buffer[4] = 0;
		Buffer[5] = 0;
		ReadDiskSector(0, 0, 0, 0, Buffer);
		PrintString(Buffer);
		
	}
	
	
	/* *: */
	else if(GetStringLen(Strs[0]) == 3 && Strs[0][2] == ':')
	{
		char DiskName[3] = {0, 0, 0};
		uchar DiskNumber;
		DiskName[0] = Strs[0][0];
		DiskName[1] = Strs[0][1];
		DiskNumber = GetDiskNumber(DiskName);
		if(DiskNumber != 0xFF) 
		{
			if(ChangeDisk(DiskNumber)) Error_UnvalidDisk();
		}
		else Error_UnvalidDisk();
	}
	/* *.bat */
	else if((CommandLen = GetStringLen(Strs[0])) > 4	&&
			Strs[0][CommandLen - 4] == '.'				&&
			Strs[0][CommandLen - 3] == 'b'				&&
			Strs[0][CommandLen - 2] == 'a'				&&
			Strs[0][CommandLen - 1] == 't')
		ExecuteBatch(Strs[0]);
	/* Application */
	else if(FileExists(Strs[0]))
	{
		char FileName[20];
		char Argument[128];
		int Flag = 0;
		uint ui, ui1 = 0;
		
		for(ui = 0; ui < GetStringLen(Command); ui++)
		{
			if(Command[ui] == ' ')
			{
				Flag = 1;
				continue;
			}
			if(Flag) 
				if(ui1 == 127) Error_ArgumentLengthTooLong();
				else Argument[ui1++] = Command[ui];
		}
		Argument[ui1] = 0;
		FixFileName(FileName, Strs[0]);
		if(ExecMTA16(FileName, Argument)) Error_CannotLoadApp();
	}
    else Console(Command);
	return 0;
}

/*
	过程名:	ExecuteBatch
	描述:	执行脚本
	参数:	FileName=脚本文件名
	返回值:	无
*/
void ExecuteBatch(char * FileName)
{
	char Buffer[12800];
	char Lines[200][SPLITSTRING_MAXBUFFERLEN];
	uint Line;
	if(FileExists(FileName) && GetFileLength(FileName) <= 12800)
	{
		int FileLength = ReadFile(FileName, (uchar *)Buffer);
		int i;
		
		Buffer[FileLength] = '\0';
		Line = SplitString(Lines, Buffer, '\n', SplitMode_RemoveEmpty);
		for(i = 0; i < Line; i++)
		{
			uint Last = GetStringLen(Lines[i]) - 1;
			if(Lines[i][Last] == '\r') Lines[i][Last] = '\0';
			Shell(Lines[i]);
		}
	}
}

/*
	过程名:	AutoExecute
	描述:	自动执行
	参数:	无
	返回值:	无
*/
void AutoExecute(void)
{
	ExecuteBatch(_AUTOEXEC);
}

char Arg[128];

/*
	过程名:	main
	描述:	主函数
	参数:	无
	返回值:	无
*/
void main(void)
{	
    char Command[MAX_COMMAND_LEN];

    Init();
	
	Argument(Arg);
	if(StringCmp(Arg, "") || !StringCmp(Arg, "noautoexec"))
		AutoExecute();
	
    while(1)
    {
		char CurrentDiskName[3];
		uchar CurrentDiskNumber;
		CurrentDiskNumber = GetCurrentDisk();
		GetDiskName(CurrentDiskNumber, CurrentDiskName);
		PrintString("Shell[");
		PrintString(CurrentDiskName);
		PrintString("]> ");
		GetLine(Command);
		if(Shell(Command) == 1) break;
		asm	INT		24H;
    }
	
	ShellExit();
}
