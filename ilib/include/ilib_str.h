/*
	ilib_str.h
	Ihsoh
	2013-2-25
*/

#ifndef ILIB_STR_H_
#define ILIB_STR_H_

#include "ilib_tpy.h"

#define SPLITSTRING_MAXBUFFERLEN 64			

typedef enum _SplitMode						
{
	SplitMode_Normal,						
	SplitMode_RemoveEmpty					
}SplitMode;

extern uint GetStringLen(const char * String);
extern void CopyString(char * Dest, const char * Source);
extern void LinkString(char * Dest, const char * Source);
extern int FindChar(const char * String, uint Start, char Char);
extern void RemoveChar(char * String, uint Index);
extern void RemoveCharN(char * String, uint Index, uint Len);
extern void LTrim(char * String);
extern void RTrim(char * String);
extern void Trim(char * String);
extern void SubString(char * Dest, uint Start, uint Len, const char * Source);
extern void InsertChar(char * String, uint Index, char Char);
extern void LeftPad(char * String, char Char, uint Len);
extern void RightPad(char * String, char Char, uint Len);
extern void ToUpper(char * String);
extern void ToLower(char * String);
extern int StringCmp(const char * String1, const char * String2);
extern int StringCaseCmp(const char * String1, const char * String2);
extern int StringCmpN(const char * String1, const char * String2, uint Len);
extern int StringCaseCmpN(const char * String1, const char * String2, uint Len);
extern void FillChar(char * String, char Char, uint Len);
extern void AddChar(char * String, char Char);
extern uint SplitString(char Strings[][SPLITSTRING_MAXBUFFERLEN], char * String, char Separator, SplitMode Mode);
extern uint StringToUInteger(char * Str);
extern uint HexStringToUInteger(char * Str);
extern uint BinStringToUInteger(char * Str);
extern void UIntegerToString(uint Number, char * SNumber);

#endif
