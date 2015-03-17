/*
	ilib_86.c
	Ihsoh
	2013-3-27
*/

#include "ilib.h";

extern void _GetRegisters(uchar * Regs);

void __GetRegisters(Registers * Regs)
{
	uchar Rs[24];
	_GetRegisters(Rs);
	Regs->Flags = *(uint*)(Rs + 0);
	Regs->AX = *(uint*)(Rs + 2);
	Regs->BX = *(uint*)(Rs + 4);
	Regs->CX = *(uint*)(Rs + 6);
	Regs->DX = *(uint*)(Rs + 8);
	Regs->SI = *(uint*)(Rs + 10);
	Regs->DI = *(uint*)(Rs + 12);
	Regs->BP = *(uint*)(Rs + 14);
	Regs->SP = *(uint*)(Rs + 16);
	Regs->DS = *(uint*)(Rs + 18);
	Regs->SS = *(uint*)(Rs + 20);
	Regs->ES = *(uint*)(Rs + 22);
}
