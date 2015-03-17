/*
 * error.c
 * Ihsoh
 * 2013-11-15
*/

#include "error.h"

void ErrorWithLine(char * Message, uint Line)
{
	PrintStringP("IASM Error(", CharColor_Red);
	PrintUIntegerP(Line, CharColor_Red);
	PrintStringP("): ", CharColor_Red);
	PrintStringP(Message, CharColor_Red);
	PrintString("\r\n");
	Exit();
}

void Error(char * Message)
{
	PrintStringP("IASM Error: ", CharColor_Red);
	PrintStringP(Message, CharColor_Red);
	PrintString("\r\n");
	Exit();
}
