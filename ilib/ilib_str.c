/*
	ilib_str.c
	Ihsoh
	2013-2-17
*/

#include "ilib.h"

uint GetStringLen(const char * String)
{
	uint Len;
	for(Len = 0; *(String++) != '\0'; Len++);
	return Len;
}

void CopyString(char * Dest, const char * Source)
{
	while(*Source != '\0')
		*(Dest++) = *(Source++);
	*Dest = '\0';
}

void LinkString(char * Dest, const char * Source)
{
	CopyString(Dest + GetStringLen(Dest), Source);
}

int FindChar(const char * String, uint Start, char Char)
{
	uint ui;
	uint StringLen = GetStringLen(String);
	if(Start >= StringLen) return -1;
	for(ui = Start; ui < StringLen; ui++)
		if(String[ui] == Char) return ui;
	return -1;
}

void RemoveChar(char * String, uint Index)
{
	uint StringLen = GetStringLen(String);
	uint ui;
	if(Index >= StringLen) return;
	for(ui = Index; ui < StringLen - 1; ui++)
		String[ui] = String[ui + 1];
	String[StringLen - 1] = '\0';
}

void RemoveCharN(char * String, uint Index, uint Len)
{
	uint ui;
	for(ui = 0; ui < Len; ui++)
		RemoveChar(String, Index);
}

void LTrim(char * String)
{
	uint StringLen = GetStringLen(String);
	uint FirstNotSpacePos;
	if(StringLen == 0) return;
	for(FirstNotSpacePos = 0; FirstNotSpacePos < StringLen; FirstNotSpacePos++)
		if(String[FirstNotSpacePos] != ' ') break;
	RemoveCharN(String, 0, FirstNotSpacePos);
}

void RTrim(char * String)
{
	uint StringLen = GetStringLen(String);
	uint FirstNotSpacePos;
	if(StringLen == 0) return;
	for(FirstNotSpacePos = StringLen - 1; FirstNotSpacePos > 0; FirstNotSpacePos--)
		if(String[FirstNotSpacePos] != ' ') break;
	if(FirstNotSpacePos == 0 && String[0] == ' ')
	{
		String[0] = '\0';
		return;
	}
	RemoveCharN(String, FirstNotSpacePos + 1, StringLen - FirstNotSpacePos - 1);
}

void Trim(char * String)
{
	LTrim(String);
	RTrim(String);
}

void SubString(char * Dest, uint Start, uint Len, const char * Source)
{
	uint ui;
	for(ui = Start; ui < Start + Len; ui++)
		*(Dest++) = Source[ui];
}

void InsertChar(char * String, uint Index, char Char)
{
	uint ui;
	uint StringLen = GetStringLen(String);
	for(ui = StringLen; ui >= Index; ui--)
	{
		String[ui + 1] = String[ui];
		if(ui == 0) break;
	}
	String[Index] = Char;
}

void LeftPad(char * String, char Char, uint Len)
{
	uint StringLen = GetStringLen(String);
	uint ui;
	for(ui = 0; ui < Len - StringLen; ui++)
		InsertChar(String, 0, Char);
}

void RightPad(char * String, char Char, uint Len)
{
	uint StringLen;
	while((StringLen = GetStringLen(String)) < Len)
		InsertChar(String, StringLen, Char);
}

void ToUpper(char * String)
{
	char Char;
	while((Char = *String) != '\0')
	{
		if(Char >= 'a' && Char <= 'z')
			*String -= 0x20;
		String++;
	}
}

void ToLower(char * String)
{
	char Char;
	while((Char = *String) != '\0')
	{
		if(Char >= 'A' && Char <= 'Z')
			*String += 0x20;
		String++;
	}
}

int StringCmp(const char * String1, const char * String2)
{
	do
	{
		if(*String1 != *(String2++)) return 0;
	}while(*(String1++) != '\0');
	return 1;
}

int StringCaseCmp(const char * String1, const char * String2)
{
	char Char1, Char2;
	do
	{
		Char1 = *(String1++);
		Char2 = *(String2++);
		Char1 = Char1 >= 'a' && Char1 <= 'z' ? Char1 - 0x20 : Char1;
		Char2 = Char2 >= 'a' && Char2 <= 'z' ? Char2 - 0x20 : Char2;
		if(Char1 != Char2) return 0;
	}while(Char1 != '\0');
	return 1;
}

int StringCmpN(const char * String1, const char * String2, uint Len)
{
	uint ui;
	for(ui = 0; ui < Len; ui++)
		if(String1[ui] != String2[ui]) return 0;
	return 1;
}

int StringCaseCmpN(const char * String1, const char * String2, uint Len)
{
	uint ui;
	char Char1, Char2;
	for(ui = 0; ui < Len; ui++)
	{
		Char1 = *(String1++);
		Char2 = *(String2++);
		Char1 = Char1 >= 'a' && Char1 <= 'z' ? Char1 - 0x20 : Char1;
		Char2 = Char2 >= 'a' && Char2 <= 'z' ? Char2 - 0x20 : Char2;
		if(Char1 != Char2) return 0;
	}
	return 1;
}

void FillChar(char * String, char Char, uint Len)
{
	uint ui;
	for(ui = 0; ui < Len; ui++)
		String[ui] = Char;
}

void AddChar(char * String, char Char)
{
	InsertChar(String, GetStringLen(String), Char);
}

uint SplitString(char Strings[][SPLITSTRING_MAXBUFFERLEN], char * String, char Separator, SplitMode Mode)
{
	char Buffer[SPLITSTRING_MAXBUFFERLEN] = {0};
	char Char;
	uint Count = 0;
	while((Char = *(String++)) != '\0')
		if(Separator != Char) AddChar(Buffer, Char);
		else if(Mode == SplitMode_Normal || (Mode == SplitMode_RemoveEmpty && Buffer[0] != '\0'))
		{
			CopyString(Strings[Count], Buffer);
			Buffer[0] = '\0';
			Count++;
		}
	if(Mode == SplitMode_Normal || (Mode == SplitMode_RemoveEmpty && Buffer[0] != '\0'))
	{
		CopyString(Strings[Count], Buffer);
		Count++;
	}
	return Count;
}

uint StringToUInteger(char * Str)
{
	uint Number = 0;
	int i = GetStringLen(Str) - 1;
	uint ui1 = 1;
	for(; i >= 0; i--)
	{
		char Chr = Str[i];
		if(Chr >= '0' && Chr <= '9')
		{
			Number += (Chr - '0') * ui1;
			ui1 *= 10;
		}
		else return 0;
	}
	return Number;
}

uint HexStringToUInteger(char * Str)
{
	uint Number = 0;
	int i = GetStringLen(Str) - 1;
	uint ui1 = 1;
	for(; i >= 0; i--)
	{
		char Chr = Str[i];
		if(Chr >= '0' && Chr <= '9') Number += (Chr - '0') * ui1;
		else if(Chr >= 'a' && Chr <= 'f') Number += (Chr - 'a' + 10) * ui1;
		else if(Chr >= 'A' && Chr <= 'F') Number += (Chr - 'A' + 10) * ui1;
		else return 0;
		ui1 *= 0x10;
	}
	return Number;
}

uint BinStringToUInteger(char * Str)
{
	uint Number = 0;
	int i = GetStringLen(Str) - 1;
	uint ui1 = 1;
	for(; i >= 0; i--)
	{
		char Chr = Str[i];
		if(Chr == '1') Number += ui1;
		else if(Chr != '_' && Chr != '0') return 0;
		if(Chr != '_') ui1 <<= 1;
	}
	return Number;
}

int StringToInteger(char * Str)
{
	if(Str[0] == '-') return -StringToUInteger(Str + 1);
	else if(Str[0] == '+') return StringToUInteger(Str + 1);
	else return StringToUInteger(Str);
}

void UIntegerToString(uint Number, char * SNumber)
{
	uint ui, ui1 = 0;
	uint Dividends[] = {10000, 1000, 100, 10, 1};
	uint Result;
	int Flag = 0;
	
	if(Number == 0)
	{
		SNumber[0] = '0';
		SNumber[1] = 0;
		return;
	}
	for(ui = 0; ui < 5; ui++)
	{
		Result = Number / Dividends[ui];
		if(Result != 0) Flag = 1;
		if(Flag) SNumber[ui1++] = (char)Result + '0';
		Number %= Dividends[ui];
	}
	SNumber[ui1] = 0;
}
