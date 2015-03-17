/*
 * error.c
 * Ihsoh
 * 2013-7-11
*/

#include "error.h"

void Error_ArgErr(void)
{
	PrintStringP(	"Shell Error: Arguments error!\r\n\r\n", 
					CharColor_Red);
}

void Error_CannotLoadApp(void)
{
	PrintStringP(	"Shell Error: Cannot load this application!\r\n\r\n",
					CharColor_Red);
}

void Error_UnvalidDisk(void)
{
	PrintStringP(	"Shell Error: Unvalid disk!\r\n\r\n",
					CharColor_Red);
}

void Error_CannotCopyFile(void)
{
	PrintStringP(	"Shell Error: Cannot copy file!\r\n\r\n",
					CharColor_Red);
}

void Error_CannotCutFile(void)
{
	PrintStringP(	"Shell Error: Cannot cut file!\r\n\r\n",
					CharColor_Red);
}

void Error_ArgumentLengthTooLong(void)
{
	PrintStringP(	"Shell Error: Argument length too long!\r\n\r\n",
					CharColor_Red);
}
