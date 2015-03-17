/*
	ilib_86.h
	Ihsoh
	2013-3-27
*/

#ifndef ILIB_KNL_H_
#define ILIN_KNL_H_

#include "ilib_tpy.h"

/*
	寄存器结构体
*/
typedef struct
{
	uint Flags;
	uint AX;
	uint BX;
	uint CX;
	uint DX;
	uint SI;
	uint DI;
	uint BP;
	uint SP;
	uint DS;
	uint SS;
	uint ES;
}Registers; 

extern void __GetRegisters(Registers * Regs);
extern uint GetAX(void);
extern uint SetAX(uint Value);

#define	GetRegisters(Regs)					\
			asm PUSH	AX;					\
			asm PUSH	BX;					\
			asm PUSH	CX;					\
			asm PUSH	DX;					\
			__GetRegisters(Regs);			\
			asm POP		DX;					\
			asm POP		CX;					\
			asm POP		BX;					\
			asm POP		AX;					\
			(Regs)->AX = GetAX();
			
#define CPointer(Seg, Off) (((uint)(Seg)) << 4 + ((uint)(Off)))

#endif
