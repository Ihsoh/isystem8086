/*
 * lexer.h
 * Ihsoh
 * 2013-11-8
*/

#ifndef	LEXER_H_
#define	LEXER_H_

#include "ilib.h"

extern void InitLexer(void);
extern void DestroyLexer(void);
extern void AddCharToLexer(char Char);
extern void ResetLexer(void);
extern uint GetToken(char * Token);

#endif
