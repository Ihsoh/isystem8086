/*
 * encoder.c
 * Ihsoh
 * 2013-10-7
*/

#include "encoder.h"

static uint BufferBID = 0;
static uint CurrentPos = 0;
static int Debug = 1;

void EnableDebug(void)
{
	Debug = 1;
}

void DisableDebug(void)
{
	Debug = 0;
}

void InitEncoder(void)
{
	BufferBID = AllocBlocks(1);
}

void DestroyEncoder(void)
{
	FreeBlocks(BufferBID, 1);
}

void SaveToFile(char * Filename)
{
	uint ui;
	
	DelFile(Filename);
	NewFile(Filename); 
	PrintString("\r\nTotal Length: ");
	PrintUInteger(CurrentPos);
	PrintString("\r\n");
	for(ui = 0; ui < CurrentPos; ui++)
		AppendFile(Filename, ReadByte(BufferBID, ui));
}

static uchar TempBuffer[20];
static uint TempBufferPos = 0;

static void ToBuffer(uint BlockID, uint Offset, uchar Byte)
{
	WriteByte(BlockID, Offset, Byte);
	if(Debug) TempBuffer[TempBufferPos++] = Byte;
}

static uint AddressCounter = 0;

static void InstructionEnd(void)
{
	uint ui;

	if(Debug)
	{
		PrintHex16(AddressCounter);
		PrintString(": ");
		for(ui = 0; ui < TempBufferPos; ui++)
		{
			PrintHex8((uint)TempBuffer[ui]);
			PrintChar(' ');
		}
		AddressCounter += TempBufferPos;
		TempBufferPos = 0;
		PrintString(" \r\n");
	}
}

static void GetMem_Mod_RM(	uchar Reg1, 	/*第一个寄存器*/
							uchar Reg2, 	/*第二个寄存器*/
							uint OffType, 	/*偏移地址类型, 0=无, 1=D8, 2=D16*/
							uint DirectOff,	/*直接寻址*/
							uchar * Mod, 
							uchar * RM)
{
	*Mod = OffType;
	if(	(Reg1 == REG_BX && Reg2 == REG_SI)	||
		(Reg1 == REG_SI && Reg2 == REG_BX))
		*RM = 0x0;
	else if((Reg1 == REG_BX && Reg2 == REG_DI)	||
			(Reg1 == REG_DI && Reg2 == REG_BX))
		*RM = 0x1;
	else if((Reg1 == REG_BP && Reg2 == REG_SI)	||
			(Reg1 == REG_SI && Reg2 == REG_BP))
		*RM = 0x2;
	else if((Reg1 == REG_BP && Reg2 == REG_DI)	||
			(Reg1 == REG_DI && Reg2 == REG_BP))
		*RM = 0x3;
	else if((Reg1 == REG_SI && Reg2 == REG_NONE)	||
			(Reg1 == REG_NONE && Reg2 == REG_SI))
		*RM = 0x4;
	else if((Reg1 == REG_DI && Reg2 == REG_NONE)	||
			(Reg1 == REG_NONE && Reg2 == REG_DI))
		*RM = 0x5;
	else if(DirectOff)
	{
		*Mod = 0;
		*RM = 0x6;
	}
	else if((Reg1 == REG_BX && Reg2 == REG_NONE)	||
			(Reg1 == REG_NONE && Reg2 == REG_BX))
		*RM = 0x7;
	else if(((Reg1 == REG_BP && Reg2 == REG_NONE)	||
			(Reg1 == REG_NONE && Reg2 == REG_BP))		&& 
			(OffType == 1 || OffType == 2))
		*RM = 0x6;
}

static void GetReg_Mod_RM(	uchar Reg,
							uchar * Mod, 
							uchar * RM)
{
	*Mod = 0x3;
	if(Reg == REG_AL || Reg == REG_AX) *RM = 0x0;
	else if(Reg == REG_CL || Reg == REG_CX) *RM = 0x1;
	else if(Reg == REG_DL || Reg == REG_DX) *RM = 0x2;
	else if(Reg == REG_BL || Reg == REG_BX) *RM = 0x3;
	else if(Reg == REG_AH || Reg == REG_SP) *RM = 0x4;
	else if(Reg == REG_CH || Reg == REG_BP) *RM = 0x5;
	else if(Reg == REG_DH || Reg == REG_SI) *RM = 0x6;
	else if(Reg == REG_BH || Reg == REG_DI) *RM = 0x7;
}

static void OpcodeW_Mem_X(	uint Opcode,
							uchar Reg1,
							uchar Reg2,
							uchar OffType,
							uint Off,
							uint ImmType,
							uint Imm)
{
	uchar Mod, RM;
	
	/* !!!!直接寻址时偏移地址必须是16位!!!! */
	if(Reg1 == REG_NONE && Reg2 == REG_NONE && (OffType == 2 || OffType == 1))
	{
		GetMem_Mod_RM(REG_NONE, REG_NONE, 0, 1, &Mod, &RM);
		Opcode |= (uint)RM & 0xFF;
		Opcode |= ((uint)Mod << 6) & 0xFF;
		ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
		ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
		ToBuffer(BufferBID, CurrentPos++, (uchar)Off);
		ToBuffer(BufferBID, CurrentPos++, (uchar)(Off >> 8));
		if(ImmType == 2)
		{
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
			ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm >> 8));
		}
		else if(ImmType == 1)
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
	}
	else if(Reg1 != REG_NONE && OffType == 0)
	{
		GetMem_Mod_RM(Reg1, Reg2, 0, 0, &Mod, &RM);
		Opcode |= (uint)RM & 0xFF;
		Opcode |= ((uint)Mod << 6) & 0xFF;
		ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
		ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
		if(ImmType == 2)
		{
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
			ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm >> 8));
		}
		else if(ImmType == 1)
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
	}
	else if(Reg1 != REG_NONE && OffType == 1)
	{
		GetMem_Mod_RM(Reg1, Reg2, 1, 0, &Mod, &RM);
		Opcode |= (uint)RM & 0xFF;
		Opcode |= ((uint)Mod << 6) & 0xFF;
		ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
		ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
		ToBuffer(BufferBID, CurrentPos++, (uchar)Off);
		if(ImmType == 2)
		{
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
			ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm >> 8));
		}
		else if(ImmType == 1)
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
	}
	else if(Reg1 != REG_NONE && OffType == 2)
	{
		GetMem_Mod_RM(Reg1, Reg2, 2, 0, &Mod, &RM);
		Opcode |= (uint)RM & 0xFF;
		Opcode |= ((uint)Mod << 6) & 0xFF;
		ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
		ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
		ToBuffer(BufferBID, CurrentPos++, (uchar)Off);
		ToBuffer(BufferBID, CurrentPos++, (uchar)(Off >> 8));
		if(ImmType == 2)
		{
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
			ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm >> 8));
		}
		else if(ImmType == 1)
			ToBuffer(BufferBID, CurrentPos++, (uchar)Imm);
	}
}

static void OpcodeW_Reg(uint Opcode, uchar Reg)
{
	uchar Mod, RM;
	
	GetReg_Mod_RM(Reg, &Mod, &RM);
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
}

/*
	用于编码算术类指令的过程
*/
static void OpcodeW_Reg8_Reg8(uint Opcode, uchar DstReg8, uchar SrcReg8)
{
	Opcode |= ((uint)DstReg8 << 3) & 0xFF;
	OpcodeW_Reg(Opcode, SrcReg8);
}

static void OpcodeW_Reg16_Reg16(uint Opcode, uchar DstReg16, uchar SrcReg16)
{
	Opcode |= ((uint)DstReg16 << 3) & 0xFF;
	OpcodeW_Reg(Opcode, SrcReg16);
}

static void OpcodeW_Mem8_Reg8(	uint Opcode,
								uchar Reg1, 
								uchar Reg2, 
								uint OffType, 
								uint Off,
								uchar SrcReg)
{
	Opcode |= ((uint)SrcReg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
}

static void OpcodeW_Mem16_Reg16(uint Opcode,
								uchar Reg1, 
								uchar Reg2, 
								uint OffType, 
								uint Off,
								uchar SrcReg)
{
	Opcode |= ((uint)SrcReg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
}

static void OpcodeW_Reg8_Mem8(	uint Opcode,
								uchar DstReg8,
								uchar Reg1, 
								uchar Reg2, 
								uint OffType, 
								uint Off)
{
	Opcode |= ((uint)DstReg8 << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
}

static void OpcodeW_Reg16_Mem16(uint Opcode,
								uchar DstReg16,
								uchar Reg1, 
								uchar Reg2, 
								uint OffType, 
								uint Off)
{
	Opcode |= ((uint)DstReg16 << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
}

static void OpcodeB_Acc8_Imm8(uchar Opcode, uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, Opcode);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
}

static void OpcodeB_Acc16_Imm16(uchar Opcode, uint Imm16)
{
	ToBuffer(BufferBID, CurrentPos++, Opcode);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Imm16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm16 >> 8));
}

static void OpcodeW_Reg_Imm8(uint Opcode, uint RegW, uchar Reg, uchar Imm8)
{
	uchar Mod, RM;
	
	if(RegW) Opcode |= 0x100;
	GetReg_Mod_RM(Reg, &Mod, &RM);
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
}

static void OpcodeW_Mem_Imm8(	uint Opcode, 
								uint MemW,
								uchar Reg1,
								uchar Reg2,
								uint OffType,
								uint Off,
								uchar Imm8)
{
	if(MemW) Opcode |= 0x100;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 1, Imm8);
}

static void OpcodeW_Reg8_Imm8(uint Opcode, uchar Reg8, uchar Imm8)
{
	uchar Mod, RM;
	
	GetReg_Mod_RM(Reg8, &Mod, &RM);
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
}

static void OpcodeW_Reg16_Imm16(uint Opcode, uchar Reg16, uint Imm16)
{
	uchar Mod, RM;
	
	GetReg_Mod_RM(Reg16, &Mod, &RM);
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Imm16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm16 >> 8));
}

static void OpcodeW_Mem8_Imm8(	uint Opcode,
								uchar Reg1,
								uchar Reg2,
								uint OffType,
								uint Off,
								uchar Imm8)
{
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 1, (uchar)Imm8);
}

static void OpcodeW_Mem16_Imm16(uint Opcode,
								uchar Reg1,
								uchar Reg2,
								uint OffType,
								uint Off,
								uint Imm16)
{
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 2, Imm16);
}

/*
	Opt X1 X2
*/
void EncodeOpt_Reg8_Reg8(uint OptOpcode, uchar DstReg8, uchar SrcReg8)
{
	OpcodeW_Reg8_Reg8(OptOpcode, DstReg8, SrcReg8);
	InstructionEnd();
}

void EncodeOpt_Reg16_Reg16(uint OptOpcode, uchar DstReg16, uchar SrcReg16)
{
	OpcodeW_Reg16_Reg16(OptOpcode, DstReg16, SrcReg16);
	InstructionEnd();
}

void EncodeOpt_Mem8_Reg8(	uint OptOpcode,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off,
							uchar SrcReg)
{
	OpcodeW_Mem8_Reg8(OptOpcode, Reg1, Reg2, OffType, Off, SrcReg);
	InstructionEnd();
}

void EncodeOpt_Mem16_Reg16(	uint OptOpcode,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off,
							uchar SrcReg)
{
	OpcodeW_Mem16_Reg16(OptOpcode, Reg1, Reg2, OffType, Off, SrcReg);
	InstructionEnd();
}

void EncodeOpt_Reg8_Mem8(	uint OptOpcode,
							uchar DstReg8,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	OpcodeW_Reg8_Mem8(OptOpcode, DstReg8, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

void EncodeOpt_Reg16_Mem16(	uint OptOpcode,
							uchar DstReg16,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	OpcodeW_Reg16_Mem16(OptOpcode, DstReg16, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

void EncodeOpt_Acc8_Imm8(uchar OptOpcode, uchar Imm8)
{
	OpcodeB_Acc8_Imm8(OptOpcode, Imm8);
	InstructionEnd();
}

void EncodeOpt_Acc16_Imm16(uchar OptOpcode, uint Imm16)
{
	OpcodeB_Acc16_Imm16(OptOpcode, Imm16);
	InstructionEnd();
}

void EncodeOpt_Reg_Imm8(uint OptOpcode, uint RegW, uchar Reg, uchar Imm8)
{
	OpcodeW_Reg_Imm8(OptOpcode, RegW, Reg, Imm8);
	InstructionEnd();
}

void EncodeOpt_Mem_Imm8(	uint OptOpcode,
								uint MemW,
								uchar Reg1,
								uchar Reg2,
								uint OffType,
								uint Off,
								uchar Imm8)
{
	OpcodeW_Mem_Imm8(OptOpcode, MemW, Reg1, Reg2, OffType, Off, Imm8);
	InstructionEnd();
}

void EncodeOpt_Reg8_Imm8(uint OptOpcode, uchar Reg8, uchar Imm8)
{
	OpcodeW_Reg8_Imm8(OptOpcode, Reg8, Imm8);
	InstructionEnd();
}

void EncodeOpt_Reg16_Imm16(uint OptOpcode, uchar Reg16, uint Imm16)
{
	OpcodeW_Reg16_Imm16(OptOpcode, Reg16, Imm16);
	InstructionEnd();
}

void EncodeOpt_Mem8_Imm8(uint OptOpcode,
								uchar Reg1,
								uchar Reg2,
								uint OffType,
								uint Off,
								uchar Imm8)
{
	OpcodeW_Mem8_Imm8(OptOpcode, Reg1, Reg2, OffType, Off, Imm8);
	InstructionEnd();
}

void EncodeOpt_Mem16_Imm16(	uint OptOpcode,
									uchar Reg1,
									uchar Reg2,
									uint OffType,
									uint Off,
									uint Imm16)
{
	OpcodeW_Mem16_Imm16(OptOpcode, Reg1, Reg2, OffType, Off, Imm16);
	InstructionEnd();
}

/*
	{DEC|INC} X
*/
static void EncodeDI_RegW(uchar OpcodeB, uchar RegW)
{
	ToBuffer(BufferBID, CurrentPos++, OpcodeB | RegW);
	InstructionEnd();
}

static void EncodeDI_Reg(uint OpcodeW, uchar Reg)
{
	OpcodeW_Reg(OpcodeW, Reg);
	InstructionEnd();
}

static void EncodeDI_Mem(	uint OpcodeW,
							uchar Reg1,
							uchar Reg2,
							uint OffType,
							uint Off)
{
	OpcodeW_Mem_X(OpcodeW, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}


/*
	{DIV|IDIV|MUL|IMUL} X
*/
static void EncodeDM_Reg(	uint OpcodeW,
							uchar Reg)
{
	OpcodeW_Reg(OpcodeW, Reg);
	InstructionEnd();
}

static void EncodeDM_Mem(	uint OpcodeW,
							uchar Reg1,
							uchar Reg2,
							uint OffType,
							uint Off)
{
	OpcodeW_Mem_X(OpcodeW, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void ResetEncoder(void)
{
	CurrentPos = 0;
}

uint GetCurrentPos(void)
{
	return CurrentPos;
}

/*
	DB
*/
void EncodeDB(uchar Byte)
{
	ToBuffer(BufferBID, CurrentPos++, Byte);
	InstructionEnd();
}

/*
	DW
*/
void EncodeDW(uint Word)
{
	ToBuffer(BufferBID, CurrentPos++, (uchar)Word);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Word >> 8));
	InstructionEnd();
}

/*
	LOCK
*/
void EncodePrefixLOCK(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_LOCK);
	InstructionEnd();
}

/*
	REP
*/
void EncodePrefixREP(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_REP);
	InstructionEnd();
}

/*
	REPNZ
*/
void EncodePrefixREPNZ(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_REPNZ);
	InstructionEnd();
}

/*
	CS:
*/
void EncodePrefixCS(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_CS);
	InstructionEnd();
}

/*
	DS:
*/
void EncodePrefixDS(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_DS);
	InstructionEnd();
}

/*
	ES:
*/
void EncodePrefixES(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_ES);
	InstructionEnd();
}

/*
	SS:
*/
void EncodePrefixSS(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PREFIX_SS);
	InstructionEnd();
}

/*
	AAA
*/
void EncodeAAA(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_AAA);
	InstructionEnd();
}

/*
	AAD
*/
void EncodeAAD(void)
{
	ToBuffer(BufferBID, CurrentPos++, (uchar)(OPCODE_AAD >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)OPCODE_AAD);
	InstructionEnd();
}

/*
	AAM
*/
void EncodeAAM(void)
{
	ToBuffer(BufferBID, CurrentPos++, (uchar)(OPCODE_AAM >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)OPCODE_AAM);
	InstructionEnd();
}

/*
	AAS
*/
void EncodeAAS(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_AAS);
	InstructionEnd();
}

/*
	ADC
*/
DefineEncodeOpt_X_X(ADC)

/*
	ADD
*/
DefineEncodeOpt_X_X(ADD)

/*
	AND
*/
DefineEncodeOpt_X_X(AND)

/*
	CBW
*/
void EncodeCBW(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CBW);
	InstructionEnd();
}

/*
	CLC
*/
void EncodeCLC(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CLC);
	InstructionEnd();
}

/*
	CLD
*/
void EncodeCLD(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CLD);
	InstructionEnd();
}

/*
	CLI
*/
void EncodeCLI(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CLI);
	InstructionEnd();
}

/*
	CMC
*/
void EncodeCMC(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CMC);
	InstructionEnd();
}

/*
	CMP
*/
DefineEncodeOpt_X_X(CMP)

/*
	CMPSB
*/
void EncodeCMPSB(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CMPSB);
	InstructionEnd();
}

/*
	CMPSW
*/
void EncodeCMPSW(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CMPSW);
	InstructionEnd();
}

/*
	CWD
*/
void EncodeCWD(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CWD);
	InstructionEnd();
}

/*
	DAA
*/
void EncodeDAA(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_DAA);
	InstructionEnd();
}

/*
	DAS
*/
void EncodeDAS(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_DAS);
	InstructionEnd();
}

/*
	DEC
*/
void EncodeDEC_RegW(uchar RegW)
{
	EncodeDI_RegW(OPCODE_DEC_REGW, RegW);
}

void EncodeDEC_Reg8(uchar Reg8)
{
	EncodeDI_Reg(OPCODE_DEC_REG8, Reg8);
}

void EncodeDEC_Reg16(uchar Reg16)
{
	EncodeDI_Reg(OPCODE_DEC_REG16, Reg16);
}

void EncodeDEC_Mem8(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDI_Mem(OPCODE_DEC_MEM8, Reg1, Reg2, OffType, Off);
}

void EncodeDEC_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDI_Mem(OPCODE_DEC_MEM8, Reg1, Reg2, OffType, Off);
}

/*
	DIV
*/
void EncodeDIV_Reg8(uchar Reg8)
{
	EncodeDM_Reg(OPCODE_DIV_REG8, Reg8);
}

void EncodeDIV_Reg16(uchar Reg16)
{
	EncodeDM_Reg(OPCODE_DIV_REG16, Reg16);
}

void EncodeDIV_Mem8(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDM_Mem(OPCODE_DIV_MEM8, Reg1, Reg2, OffType, Off);
}

void EncodeDIV_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDM_Mem(OPCODE_DIV_MEM16, Reg1, Reg2, OffType, Off);
}

/*
	HLT
*/
void EncodeHLT(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_HLT);
	InstructionEnd();
}

/*
	IDIV
*/
void EncodeIDIV_Reg8(uchar Reg8)
{
	EncodeDM_Reg(OPCODE_IDIV_REG8, Reg8);
}

void EncodeIDIV_Reg16(uchar Reg16)
{
	EncodeDM_Reg(OPCODE_IDIV_REG16, Reg16);
}

void EncodeIDIV_Mem8(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDM_Mem(OPCODE_IDIV_MEM8, Reg1, Reg2, OffType, Off);
}

void EncodeIDIV_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDM_Mem(OPCODE_IDIV_MEM16, Reg1, Reg2, OffType, Off);
}

/*
	IMUL
*/
void EncodeIMUL_Reg8(uchar Reg8)
{
	EncodeDM_Reg(OPCODE_IMUL_REG8, Reg8);
}

void EncodeIMUL_Reg16(uchar Reg16)
{
	EncodeDM_Reg(OPCODE_IMUL_REG16, Reg16);
}

void EncodeIMUL_Mem8(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDM_Mem(OPCODE_IMUL_MEM8, Reg1, Reg2, OffType, Off);
}

void EncodeIMUL_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDM_Mem(OPCODE_IMUL_MEM16, Reg1, Reg2, OffType, Off);
}

/*
	IN
*/
void EncodeINB_Imm8(uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INB_Imm8);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
	InstructionEnd();
}

void EncodeINW_Imm8(uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INW_Imm8);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
	InstructionEnd();
}

void EncodeINB_DX(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INB_DX);
	InstructionEnd();
}

void EncodeINW_DX(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INW_DX);
	InstructionEnd();
}

/*
	INT
*/
void EncodeINT_3(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INT_3);
	InstructionEnd();
}

void EncodeINT_Imm8(uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INT_IMM8);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
	InstructionEnd();
}

void EncodeINTO(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_INTO);
	InstructionEnd();
}

/*
	INC
*/
void EncodeINC_RegW(uchar RegW)
{
	EncodeDI_RegW(OPCODE_INC_REGW, RegW);
}

void EncodeINC_Reg8(uchar Reg8)
{
	EncodeDI_Reg(OPCODE_INC_REG8, Reg8);
}

void EncodeINC_Reg16(uchar Reg16)
{
	EncodeDI_Reg(OPCODE_INC_REG16, Reg16);
}

void EncodeINC_Mem8(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDI_Mem(OPCODE_INC_MEM8, Reg1, Reg2, OffType, Off);
}

void EncodeINC_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	EncodeDI_Mem(OPCODE_INC_MEM8, Reg1, Reg2, OffType, Off);
}

/*
	IRET
*/
void EncodeIRET(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_IRET);
	InstructionEnd();
}

/*
	LAHF
*/
void EncodeLAHF(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_LAHF);
	InstructionEnd();
}

/*
	LDS
*/
void EncodeLDS_Reg16_Mem32(	uchar Reg16, 
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	OpcodeW_Reg16_Mem16(OPCODE_LDS_REG16_MEM32, Reg16, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

/*
	LES
*/
void EncodeLES_Reg16_Mem32(	uchar Reg16, 
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	OpcodeW_Reg16_Mem16(OPCODE_LES_REG16_MEM32, Reg16, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

/*
	LEA
*/
void EncodeLEA_Reg16_Mem(	uchar Reg16,
							uchar Reg1,
							uchar Reg2,
							uint OffType,
							uint Off)
{
	OpcodeW_Reg16_Mem16(OPCODE_LEA_REG16_MEM, Reg16, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

/*
	LODSB
*/
void EncodeLODSB(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_LODSB);
	InstructionEnd();
}

/*
	LODSW
*/
void EncodeLODSW(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_LODSB);
	InstructionEnd();
}

/* 
	MOV
*/
void EncodeMOV_MemOff_Acc8(uint MemOff)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOV_MEMOFF_ACC8);
	ToBuffer(BufferBID, CurrentPos++, (uchar)MemOff);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(MemOff >> 8));
	InstructionEnd();
}

void EncodeMOV_MemOff_Acc16(uint MemOff)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOV_MEMOFF_ACC16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)MemOff);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(MemOff >> 8));
	InstructionEnd();
}

void EncodeMOV_Acc8_MemOff(uint MemOff)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOV_ACC8_MEMOFF);
	ToBuffer(BufferBID, CurrentPos++, (uchar)MemOff);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(MemOff >> 8));
	InstructionEnd();
}

void EncodeMOV_Acc16_MemOff(uint MemOff)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOV_ACC16_MEMOFF);
	ToBuffer(BufferBID, CurrentPos++, (uchar)MemOff);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(MemOff >> 8));
	InstructionEnd();
}

void EncodeMOV_Reg8_Imm8(uchar Reg8, uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOV_REG8_IMM8 | Reg8);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
	InstructionEnd();
}

void EncodeMOV_Reg16_Imm16(uchar Reg16, uint Imm16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOV_REG16_IMM16 | Reg16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Imm16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm16 >> 8));
	InstructionEnd();
}

void EncodeMOV_Mem8_Imm8(	uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off,
							uchar Imm8)
{
	OpcodeW_Mem_X(OPCODE_MOV_MEM8_IMM8, Reg1, Reg2, OffType, Off, 1, Imm8);
	InstructionEnd();
}

void EncodeMOV_Mem16_Imm16(	uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off,
							uint Imm16)
{
	OpcodeW_Mem_X(OPCODE_MOV_MEM16_IMM16, Reg1, Reg2, OffType, Off, 2, Imm16);
	InstructionEnd();
}

void EncodeMOV_Reg8_Reg8(uchar DstReg, uchar SrcReg)
{
	uint Opcode;
	uchar Mod, RM;
	
	Opcode = OPCODE_MOV_REG8_REG8;
	Opcode |= ((uint)SrcReg << 3) & 0xFF;
	GetReg_Mod_RM(DstReg, &Mod, &RM);
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	InstructionEnd();
}

void EncodeMOV_Reg16_Reg16(uchar DstReg, uchar SrcReg)
{
	uint Opcode;
	uchar Mod, RM;
	
	Opcode = OPCODE_MOV_REG16_REG16;
	Opcode |= ((uint)SrcReg << 3) & 0xFF;
	GetReg_Mod_RM(DstReg, &Mod, &RM);
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	InstructionEnd();
}

void EncodeMOV_Reg8_Mem8(	uchar DstReg,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	uint Opcode;
	
	Opcode = OPCODE_MOV_REG8_MEM8;
	Opcode |= ((uint)DstReg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeMOV_Reg16_Mem16(	uchar DstReg,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	uint Opcode;
	
	Opcode = OPCODE_MOV_REG16_MEM16;
	Opcode |= ((uint)DstReg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeMOV_Mem8_Reg8(	uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off, 
							uchar SrcReg)
{
	uint Opcode;
	
	Opcode = OPCODE_MOV_MEM8_REG8;
	Opcode |= ((uint)SrcReg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeMOV_Mem16_Reg16(	uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off, 
							uchar SrcReg)
{
	uint Opcode;
	
	Opcode = OPCODE_MOV_MEM16_REG16;
	Opcode |= ((uint)SrcReg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeMOV_Reg16_Seg(uchar Reg16, uchar Seg)
{
	uint Opcode;
	uchar Mod, RM;
	
	Opcode = OPCODE_MOV_REG16_SEG;
	GetReg_Mod_RM(Reg16, &Mod, &RM);
	Opcode |= ((uint)Seg << 3) & 0xFF;
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	InstructionEnd();
}

void EncodeMOV_Seg_Reg16(uchar Seg, uchar Reg16)
{
	uint Opcode;
	uchar Mod, RM;
	
	Opcode = OPCODE_MOV_SEG_REG16;
	GetReg_Mod_RM(Reg16, &Mod, &RM);
	Opcode |= ((uint)Seg << 3) & 0xFF;
	Opcode |= (uint)RM & 0xFF;
	Opcode |= ((uint)Mod << 6) & 0xFF;
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Opcode >> 8));
	ToBuffer(BufferBID, CurrentPos++, (uchar)Opcode);
	InstructionEnd();
}

void EncodeMOV_Mem16_Seg(	uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off, 
							uchar Seg)
{
	uint Opcode;
	
	Opcode = OPCODE_MOV_MEM16_SEG;
	Opcode |= ((uint)Seg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeMOV_Seg_Mem16(	uchar Seg,
							uchar Reg1, 
							uchar Reg2, 
							uint OffType, 
							uint Off)
{
	uint Opcode;
	
	Opcode = OPCODE_MOV_SEG_MEM16;
	Opcode |= ((uint)Seg << 3) & 0xFF;
	OpcodeW_Mem_X(Opcode, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

/*
	MOVSB
*/
void EncodeMOVSB(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOVSB);
	InstructionEnd();
}

/*
	MOVSW
*/
void EncodeMOVSW(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_MOVSW);
	InstructionEnd();
}

/*
	MUL
*/
void EncodeMUL_Reg8(uchar Reg8)
{
	OpcodeW_Reg(OPCODE_MUL_REG8, Reg8);
	InstructionEnd();
}

void EncodeMUL_Mem8(uchar Reg1, 
					uchar Reg2, 
					uint OffType, 
					uint Off)
{
	OpcodeW_Mem_X(OPCODE_MUL_MEM8, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeMUL_Reg16(uchar Reg16)
{
	OpcodeW_Reg(OPCODE_MUL_REG16, Reg16);
	InstructionEnd();
}

void EncodeMUL_Mem16(	uchar Reg1, 
						uchar Reg2, 
						uint OffType, 
						uint Off)
{
	OpcodeW_Mem_X(OPCODE_MUL_MEM16, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

/*
	NEG
*/
void EncodeNEG_Reg8(uchar Reg8)
{
	OpcodeW_Reg(OPCODE_NEG_REG8, Reg8);
	InstructionEnd();
}

void EncodeNEG_Mem8(uchar Reg1, 
					uchar Reg2, 
					uint OffType, 
					uint Off)
{
	OpcodeW_Mem_X(OPCODE_NEG_MEM8, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeNEG_Reg16(uchar Reg16)
{
	OpcodeW_Reg(OPCODE_NEG_REG16, Reg16);
	InstructionEnd();
}

void EncodeNEG_Mem16(	uchar Reg1, 
						uchar Reg2, 
						uint OffType, 
						uint Off)
{
	OpcodeW_Mem_X(OPCODE_NEG_MEM16, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

/*
	NOP
*/
void EncodeNOP(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_NOP);
	InstructionEnd();
}

/*
	NOT
*/
void EncodeNOT_Reg8(uchar Reg8)
{
	OpcodeW_Reg(OPCODE_NOT_REG8, Reg8);
	InstructionEnd();
}

void EncodeNOT_Mem8(uchar Reg1, 
					uchar Reg2, 
					uint OffType, 
					uint Off)
{
	OpcodeW_Mem_X(OPCODE_NOT_MEM8, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeNOT_Reg16(uchar Reg16)
{
	OpcodeW_Reg(OPCODE_NOT_REG16, Reg16);
	InstructionEnd();
}

void EncodeNOT_Mem16(	uchar Reg1, 
						uchar Reg2, 
						uint OffType, 
						uint Off)
{
	OpcodeW_Mem_X(OPCODE_NOT_MEM16, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

/*
	OR
*/
DefineEncodeOpt_X_X(OR)

/*
	OUT
*/
void EncodeOUTB_Imm8(uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_OUTB_Imm8);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
	InstructionEnd();
}

void EncodeOUTW_Imm8(uchar Imm8)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_OUTW_Imm8);
	ToBuffer(BufferBID, CurrentPos++, Imm8);
	InstructionEnd();
}

void EncodeOUTB_DX(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_OUTB_DX);
	InstructionEnd();
}

void EncodeOUTW_DX(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_OUTW_DX);
	InstructionEnd();
}

/*
	POP
*/
void EncodePOP_Reg16(uchar Reg16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_POP_REG16 | Reg16);
	InstructionEnd();
}

void EncodePOP_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	OpcodeW_Mem_X(OPCODE_POP_MEM16, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodePOP_Seg(uchar Seg)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_POP_SEG | (Seg << 3));
	InstructionEnd();
}

/*
	POPF
*/
void EncodePOPF(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_POPF);
	InstructionEnd();
}

/*
	PUSH
*/
void EncodePUSH_Reg16(uchar Reg16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PUSH_REG16 | Reg16);
	InstructionEnd();
}

void EncodePUSH_Mem16(uchar Reg1, uchar Reg2, uint OffType, uint Off)
{
	OpcodeW_Mem_X(OPCODE_PUSH_MEM16, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodePUSH_Seg(uchar Seg)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PUSH_SEG | (Seg << 3));
	InstructionEnd();
}

/*
	PUSHF
*/
void EncodePUSHF(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_PUSHF);
	InstructionEnd();
}

/*
	RCL
*/
DefineEncodeShift_X_X(RCL)

/*
	RCR
*/
DefineEncodeShift_X_X(RCR)

/*
	RET
*/
void EncodeRET_Near(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_RET_NEAR);
	InstructionEnd();
}

void EncodeRET_Imm16Near(uint Imm16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_RET_IMM16NEAR);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Imm16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm16 >> 8));
	InstructionEnd();
}

void EncodeRET_Far(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_RET_FAR);
	InstructionEnd();
}

void EncodeRET_Imm16Far(uint Imm16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_RET_IMM16FAR);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Imm16);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Imm16 >> 8));
	InstructionEnd();
}

/*
	ROL
*/
DefineEncodeShift_X_X(ROL)

/*
	ROR
*/
DefineEncodeShift_X_X(ROR)

/*
	SAHF
*/
void EncodeSAHF(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_SAHF);
	InstructionEnd();
}

/*
	SAL
*/
DefineEncodeShift_X_X(SAL)

/*
	SAR
*/
DefineEncodeShift_X_X(SAR)

/*
	SHL
*/
DefineEncodeShift_X_X(SHL)

/*
	SHR
*/
DefineEncodeShift_X_X(SHR)

/*
	SBB
*/
DefineEncodeOpt_X_X(SBB)

/*
	SCANSB
*/
void EncodeSCANSB(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_SCANSB);
	InstructionEnd();
}

/*
	SCANSW
*/
void EncodeSCANSW(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_SCANSW);
	InstructionEnd();
}

/*
	STC
*/
void EncodeSTC(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_STC);
	InstructionEnd();
}

/*
	STD
*/
void EncodeSTD(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_STD);
	InstructionEnd();
}

/*
	STI
*/
void EncodeSTI(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_STI);
	InstructionEnd();
}

/*
	STOSB
*/
void EncodeSTOSB(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_STOSB);
	InstructionEnd();
}

/*
	STOSW
*/
void EncodeSTOSW(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_STOSW);
	InstructionEnd();
}

/*
	SUB
*/
DefineEncodeOpt_X_X(SUB)

/*
	TEST
*/
DefineEncodeOpt_X_X(TEST)

/*
	WAIT
*/
void EncodeWAIT(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_WAIT);
	InstructionEnd();
}

/*
	XCHG
*/
void EncodeXCHG_Acc16_Reg16(uchar Reg16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_XCHG_ACC16_REG16 | Reg16);
	InstructionEnd();
}

void EncodeXCHG_Reg16_Acc16(uchar Reg16)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_XCHG_REG16_ACC16 | Reg16);
	InstructionEnd();
}

void EncodeXCHG_Reg8_Reg8(uchar DstReg, uchar SrcReg)
{
	OpcodeW_Reg8_Reg8(OPCODE_XCHG_REG8_REG8, DstReg, SrcReg);
	InstructionEnd();
}

void EncodeXCHG_Reg16_Reg16(uchar DstReg, uchar SrcReg)
{
	OpcodeW_Reg16_Reg16(OPCODE_XCHG_REG16_REG16, DstReg, SrcReg);
	InstructionEnd();
}

void EncodeXCHG_Mem8_Reg8(	uchar Reg1,
							uchar Reg2,
							uint OffType,
							uchar Off,
							uchar SrcReg)
{
	OpcodeW_Mem8_Reg8(OPCODE_XCHG_MEM8_REG8, Reg1, Reg2, OffType, Off, SrcReg);
	InstructionEnd();
}

void EncodeXCHG_Mem16_Reg16(uchar Reg1,
							uchar Reg2,
							uint OffType,
							uchar Off,
							uchar SrcReg)
{
	OpcodeW_Mem16_Reg16(OPCODE_XCHG_MEM16_REG16, Reg1, Reg2, OffType, Off, SrcReg);
	InstructionEnd();
}

void EncodeXCHG_Reg8_Mem8(	uchar DstReg,
							uchar Reg1,
							uchar Reg2,
							uint OffType,
							uchar Off)
{
	OpcodeW_Reg8_Mem8(OPCODE_XCHG_REG8_MEM8, DstReg, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

void EncodeXCHG_Reg16_Mem16(uchar DstReg,
							uchar Reg1,
							uchar Reg2,
							uint OffType,
							uchar Off)
{
	OpcodeW_Reg16_Mem16(OPCODE_XCHG_REG16_MEM16, DstReg, Reg1, Reg2, OffType, Off);
	InstructionEnd();
}

/*
	XLAT
*/
void EncodeXLAT(void)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_XLAT);
	InstructionEnd();
}

/*
	XOR
*/
DefineEncodeOpt_X_X(XOR)

/*
	CALL
*/
void EncodeCALL_MemFar(	uchar Reg1,
						uchar Reg2,
						uint OffType,
						uint Off)
{
	OpcodeW_Mem_X(OPCODE_CALL_MEMFAR, Reg1, Reg2, OffType, Off, 0, 0);
	InstructionEnd();
}

void EncodeCALL_Near(uint Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_CALL_NEAR);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Offset);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Offset >> 8));
	InstructionEnd();
}

/*
	Jcc
*/
void EncodeJcc_SHORT(uchar CCCC, uchar Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_JCC_SHORT | CCCC);
	ToBuffer(BufferBID, CurrentPos++, Offset);
	InstructionEnd();
}

/*
	JMP
*/
void EncodeJMP_SHORT(uchar Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_JMP_SHORT);
	ToBuffer(BufferBID, CurrentPos++, Offset);
	InstructionEnd();
}

void EncodeJMP_NEAR(uint Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_JMP_NEAR);
	ToBuffer(BufferBID, CurrentPos++, (uchar)Offset);
	ToBuffer(BufferBID, CurrentPos++, (uchar)(Offset >> 8));
	InstructionEnd();
}

/*
	JCXZ
*/
void EncodeJCXZ_SHORT(uchar Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_JCXZ_SHORT);
	ToBuffer(BufferBID, CurrentPos++, Offset);
	InstructionEnd();
}

/*
	LOOP
*/
void EncodeLOOP_SHORT(uchar Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_LOOP_SHORT);
	ToBuffer(BufferBID, CurrentPos++, Offset);
	InstructionEnd();
}

/*
	LOOPZ
*/
void EncodeLOOPZ_SHORT(uchar Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_LOOPZ_SHORT);
	ToBuffer(BufferBID, CurrentPos++, Offset);
	InstructionEnd();
}

/*
	LOOPNZ
*/
void EncodeLOOPNZ_SHORT(uchar Offset)
{
	ToBuffer(BufferBID, CurrentPos++, OPCODE_LOOPNZ_SHORT);
	ToBuffer(BufferBID, CurrentPos++, Offset);
	InstructionEnd();
}
