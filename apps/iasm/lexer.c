/*
 * lexer.c
 * Ihsoh
 * 2013-11-8
*/

#include "lexer.h"

static uint BufferBID = 0;
static uint CurrentPos = 0;

void InitLexer(void)
{
	BufferBID = AllocBlocks(1);
}

void DestroyLexer(void)
{
	FreeBlocks(BufferBID, 1);
}

void AddCharToLexer(char Char)
{
	WriteByte(BufferBID, CurrentPos++, Char);
}

uint GetToken(char * Token)
{
	static int Pos = 0;
	static uint Line = 0;
	int State = 0;	/* 0:空白, 1:记号的一部分, 2:字符串, 3:内存引用, 4:换行 */
	int End = 0;
	
	if(Token == NULL) 
	{
		Pos = 0;
		Line = 0;
	}
	else 
		while(Pos != CurrentPos && !End)
		{
			char Char = ReadByte(BufferBID, Pos++);
			
			switch(Char)
			{
				case '\n':
					Line++;
					End = 1;
					*(Token++) = '\n';
					break;
				case '\r':
				case '\t':
				case ' ':
					if(State == 1)  End = 1;
					else if(State == 2 || State == 3) *(Token++) = Char;
					break;
				case ',':
					if(State != 3)
					{
						if(State == 1) 
						{
							Pos--;
							End = 1;
						}
						else if(State == 2) *(Token++) = Char;
						else 
						{
							*(Token++) = Char;
							End = 1;
						}
					}
					else *(Token++) = Char;
					break;
				case '\'':
					if(State == 0)
					{
						*(Token++) = '\'';
						State = 2;
					}
					else if(State == 2)
					{
						*(Token++) = '\'';
						End = 1;
					}
					break;
				case '[':
					if(State == 0)
					{
						*(Token++) = '[';
						State = 3;
					}
					else if(State == 2) *(Token++) = '[';
					break;
				case ']':
					if(State == 3)
					{
						*(Token++) = ']';
						End = 1;
					}
					else if(State == 2) *(Token++) = ']';
					break;
				case ';':
					if(State == 0)
						while(1)
						{
							Char = ReadByte(BufferBID, Pos++);
							if(Char == '\n' || Pos == CurrentPos) break;
						}
					else *(Token++) = ';';
					break;
				default:
					*(Token++) = Char;
					if(State != 2 && State != 3) State = 1;
					break;
			}
		}
		
	*Token = '\0';
	return Line;
}

void ResetLexer(void)
{
	GetToken(NULL);
}
