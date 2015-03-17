/*
 * shell.h
 * Ihsoh
 * 2013-7-4
*/

#ifndef SHELL_H_
#define SHELL_H_

#include <ilib.h>
#include "error.h"

#define VER "0"
#define SUBVER "0"
#define MESSAGE "ISystem Shell [Version "VER"."SUBVER"]\r\n"\
"Ihsoh Software, 2013-7-4\r\n\r\n"

#define MAX_COMMAND_LEN 0x110

#define _AUTOEXEC	"_autoexec"

#define COMMAND_EXIT 		"exit"
#define COMMAND_NEWFILE		"newfile"
#define COMMAND_DELFILE		"delfile"
#define COMMAND_EXEC 		"exec"
#define COMMAND_RENAME		"rename"
#define COMMAND_CD			"cd"
#define COMMAND_COPY		"copy"
#define COMMAND_CUT			"cut"
#define COMMAND_TASKLIST	"tasklist"
#define COMMAND_KILLTASK	"killtask"
#define	COMMAND_NEXTTASK	"nexttask"
#define COMMAND_HELP1		"help"
#define COMMAND_HELP2		"?"
#define	COMMAND_REM			"rem"

extern int ExecMTA16(char * FileName, char * Argument);
extern int TaskSlotIsUsed(uchar TaskID);
extern int KillTask(uchar TaskID);

#endif

