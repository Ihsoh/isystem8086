/*
 * parser.c
 * Ihsoh
 * 2013-11-8
*/

#include "parser.h"

#define	GETTOKEN(t) {	\
						CurrentLine = GetToken(t);	\
					}
#define	ERROR(m) 	{	\
						if(CurrentOperat != 2)	\
						{	\
							ErrorWithLine(m, CurrentLine + 1); \
						}	\
						else	\
						{	\
							Error(m);	\
						}	\
					}

static uint CurrentLine = 0;
static int CurrentOperat = OPT_NONE;

void InitParser(void)
{
	InitLexer();
	InitEncoder();
	InitSTable();
}

void DestroyParser(void)
{
	DestroyLexer();
	DestroyEncoder();
	DestroySTable();
}

void AddCharToParser(char Char)
{
	AddCharToLexer(Char);
}

static int IsNumber(char * Token)
{
	return Token[0] >= '0' && Token[0] <= '9';
}

static int IsReg(char * Token)
{
	return Token[0] == '%';
}

static int IsLabel(char * Token)
{
	return (Token[0] >= 'A' && Token[0] <= 'Z') || (Token[0] >= 'a' && Token[0] <= 'z');
}

static int IsConstant(char * Token)
{
	return IsNumber(Token) || IsLabel(Token);
}

static int IsMem(char * Token)
{
	return Token[0] == '[' && Token[GetStringLen(Token) - 1] == ']';
}

static int IsString(char * Token)
{
	return Token[0] == '\'' && Token[GetStringLen(Token) - 1] == '\'';
}

static int IsReg8(char * Token)
{
	return	StringCmp(Token, INS_REG_PREFIX REGS_AL)	||
			StringCmp(Token, INS_REG_PREFIX REGS_BL)	||
			StringCmp(Token, INS_REG_PREFIX REGS_CL)	||
			StringCmp(Token, INS_REG_PREFIX REGS_DL)	||
			StringCmp(Token, INS_REG_PREFIX REGS_AH)	||
			StringCmp(Token, INS_REG_PREFIX REGS_BH)	||
			StringCmp(Token, INS_REG_PREFIX REGS_CH)	||
			StringCmp(Token, INS_REG_PREFIX REGS_DH);
}

static int IsReg16(char * Token)
{
	return	StringCmp(Token, INS_REG_PREFIX REGS_AX)	||
			StringCmp(Token, INS_REG_PREFIX REGS_BX)	||
			StringCmp(Token, INS_REG_PREFIX REGS_CX)	||
			StringCmp(Token, INS_REG_PREFIX REGS_DX)	||
			StringCmp(Token, INS_REG_PREFIX REGS_SI)	||
			StringCmp(Token, INS_REG_PREFIX REGS_DI)	||
			StringCmp(Token, INS_REG_PREFIX REGS_BP)	||
			StringCmp(Token, INS_REG_PREFIX REGS_SP);
}

static uint Number(char * Str)
{
	uint Len = GetStringLen(Str);
	uint N;
	char Temp;
	
	if(Str[Len - 1] == 'h' || Str[Len - 1] == 'H')
	{
		Temp = Str[Len - 1];
		Str[Len - 1] = '\0';
		N = HexStringToUInteger(Str);
		Str[Len - 1] = Temp;
	}
	else if(Str[Len - 1] == 'b' || Str[Len - 1] == 'B')
	{
		Temp = Str[Len - 1];
		Str[Len - 1] = '\0';
		N = BinStringToUInteger(Str);
		Str[Len - 1] = Temp;
	}
	else if(Str[Len - 1] >= '0' && Str[Len - 1] <= '9')
		N = StringToUInteger(Str);
	else 
		ERROR("Invalid number");
	return N;
}

static uint GetConstant(char * Constant)
{
	if(IsNumber(Constant)) return Number(Constant);
	else if(IsLabel(Constant))
	{
		int Found;
		uint Value;
		
		if(CurrentOperat == OPT_SCAN) return 0xFFFF;
		Value = GetLabelOffsetFromSTable(Constant, &Found);
		if(Found) return Value;
		else ERROR("Unknow label");
	}
}

static uchar GetReg(char * RegName)
{
	uchar Reg;
	char * Name = RegName + 1;
	
	if(StringCmp(Name, "AX")) Reg = REG_AX;
	else if(StringCmp(Name, "AL")) Reg = REG_AL;
	else if(StringCmp(Name, "BX")) Reg = REG_BX;
	else if(StringCmp(Name, "BL")) Reg = REG_BL;
	else if(StringCmp(Name, "CX")) Reg = REG_CX;
	else if(StringCmp(Name, "CL")) Reg = REG_CL;
	else if(StringCmp(Name, "DX")) Reg = REG_DX;
	else if(StringCmp(Name, "DL")) Reg = REG_DL;
	else if(StringCmp(Name, "SP")) Reg = REG_SP;
	else if(StringCmp(Name, "AH")) Reg = REG_AH;
	else if(StringCmp(Name, "DI")) Reg = REG_DI;
	else if(StringCmp(Name, "BH")) Reg = REG_BH;
	else if(StringCmp(Name, "BP")) Reg = REG_BP;
	else if(StringCmp(Name, "CH")) Reg = REG_CH;
	else if(StringCmp(Name, "SI")) Reg = REG_SI;
	else if(StringCmp(Name, "DH")) Reg = REG_DH;
	else if(StringCmp(Name, "CS")) Reg = REG_CS;
	else if(StringCmp(Name, "DS")) Reg = REG_DS;
	else if(StringCmp(Name, "ES")) Reg = REG_ES;
	else if(StringCmp(Name, "SS")) Reg = REG_SS;
	else ERROR("Invalid register");
	
	return Reg;
}

static void GetMem(char * Mem, uchar * Reg1, uchar * Reg2, uint * Offset)
{
	char Strs[3][SPLITSTRING_MAXBUFFERLEN];
	uchar R1, R2;
	int i;
	int CommaCount = 0;
	uint S0Len, S1Len, S2Len;
	
	for(i = 0; i < GetStringLen(Mem); i++)
		if(Mem[i] == ',') CommaCount++;
	if(CommaCount != 2) ERROR("Invalid memory reference");
	Mem[GetStringLen(Mem) - 1] = '\0';
	SplitString(Strs, Mem + 1, ',', SplitMode_Normal);
	Trim(Strs[0]);
	Trim(Strs[1]);
	Trim(Strs[2]);
	S0Len = GetStringLen(Strs[0]);
	S1Len = GetStringLen(Strs[1]);
	S2Len = GetStringLen(Strs[2]);
	if(S0Len == 0 && S1Len == 0 && S2Len == 0) ERROR("Invalid memory reference");
	if(S1Len != 0) R1 = GetReg(Strs[0]);
	else R1 = REG_NONE;
	if(S1Len!= 0) R2 = GetReg(Strs[1]);
	else R2 = REG_NONE;
	if(S2Len != 0) *Offset = GetConstant(Strs[2]);
	else *Offset = 0;
	
	/* 检测内存引用的两个寄存器和偏移是否为合法组合 */
	if(	(R1 == REG_BX && R2 == REG_SI)		||
		(R1 == REG_SI && R2 == REG_BX)		||
		(R1 == REG_BX && R2 == REG_DI)		||
		(R1 == REG_DI && R2 == REG_BX)		||
		(R1 == REG_BP && R2 == REG_SI)		||
		(R1 == REG_SI && R2 == REG_BP)		||
		(R1 == REG_BP && R2 == REG_DI)		||
		(R1 == REG_DI && R2 == REG_BP)		||
		(R1 == REG_SI && R2 == REG_NONE)	||
		(R1 == REG_NONE && R2 == REG_SI)	||
		(R1 == REG_DI && R2 == REG_NONE)	||
		(R1 == REG_NONE && R2 == REG_DI)	||
		(R1 == REG_BX && R2 == REG_NONE)	||
		(R1 == REG_NONE && R2 == REG_BX)	||
		(R1 == REG_BP && R2 == REG_NONE)	||
		(R1 == REG_NONE && R2 == REG_BP)	||
		(R1 == REG_NONE && R2 == REG_NONE))
	{
		Mem[GetStringLen(Mem)] = ']';
		*Reg1 = R1;
		*Reg2 = R2;
	}
	else ERROR("Invalid memory reference");
}

static void GetString(char * Token, char * String)
{
	int i;
	
	for(i = 1; i < GetStringLen(Token) - 1; i++)
		*(String++) = Token[i];
	*String = '\0';
}

static uint GetOffType(uint Off)
{
	return Off == 0 ? 0 : (Off <= 0xFF ? 1 : 2);
}

static int IsSeg(char * Name)
{
	return 	StringCmp(Name + 1, REGS_CS) 	|| 
			StringCmp(Name + 1, REGS_ES) 	|| 
			StringCmp(Name + 1, REGS_SS) 	|| 
			StringCmp(Name + 1, REGS_DS);
}

static void InvalidInstruction(void)
{
	ERROR("Invalid instruction");
}

static int ParseJcc(char * Token)
{
	if(0);
	/* JO */
	JCC(O, XXXX, XXXX)
	
	/* JNO */
	JCC(NO, XXXX, XXXX)
	
	/* JC, JB, JNAE */
	JCC(C, B, NAE)
	
	/* JNC, JAE, JNB */
	JCC(NC, AE, NB)
	
	/* JE, JZ */
	JCC(E, Z, XXXX)
	
	/* JNE, JNZ */
	JCC(NE, NZ, XXXX)
	
	/* JBE, JNA */
	JCC(BE, NA, XXXX)
	
	/* JA, JNBE */
	JCC(A, NBE, XXXX)
	
	/* JS */
	JCC(S, XXXX, XXXX)
	
	/* JNS */
	JCC(NS, XXXX, XXXX)
	
	/* JP, JPE */
	JCC(P, PE, XXXX)
	
	/* JNP, JPO */
	JCC(NP, PO, XXXX)
	
	/* JL, JNGE */
	JCC(L, NGE, XXXX)
	
	/* JGE, JNL */
	JCC(GE, NL, XXXX)
	
	/* JLE, JNG */
	JCC(LE, NG, XXXX)
	
	/* JG, JNLE */
	JCC(G, NLE, XXXX)
	else return 0;
	return 1;
}

static int ParseOperator(char * Token)
{
	if(0);
	
	/* ADC */
	OPT(ADC)
	
	/* ADD */
	OPT(ADD)
	
	/* AND */
	OPT(AND)
	
	/* CMP */
	OPT(CMP)
		
	/* OR */
	OPT(OR)
		
	/* SBB */
	OPT(SBB)
	
	/* SUB */
	OPT(SUB)
	
	/* TEST */
	OPT(TEST)
		
	/* XOR */
	OPT(XOR)
	
	else return 0;
	return 1;
}

static int ParseShift(char * Token)
{
	if(0);
	
	/* RCL */
	SHIFT(RCL)
	
	/* RCR */
	SHIFT(RCR)
	
	/* ROL */
	SHIFT(ROL)
	
	/* ROR */
	SHIFT(ROR)
	
	/* SAL */
	SHIFT(SAL)
	
	/* SAR */
	SHIFT(SAR)
	
	/* SHL */
	SHIFT(SHL)
	
	/* SHR */
	SHIFT(SHR)
	
	else return 0;
	return 1;
}

static int _Parse_1(char * Token)
{
	if(StringCmp(Token, INS_CBW)) EncodeCBW();
	else if(StringCmp(Token, INS_CLC)) EncodeCLC();
	else if(StringCmp(Token, INS_CLD)) EncodeCLD();
	else if(StringCmp(Token, INS_CLI)) EncodeCLI();
	else if(StringCmp(Token, INS_CMC)) EncodeCMC();
	else if(StringCmp(Token, INS_CMPSB)) EncodeCMPSB();
	else if(StringCmp(Token, INS_CMPSW)) EncodeCMPSW();
	else if(StringCmp(Token, INS_CWD)) EncodeCWD();
	else if(StringCmp(Token, INS_DAS)) EncodeDAS();
	else if(StringCmp(Token, INS_DEC INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_DEC_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeDEC_Reg8(GetReg(OPRD));
		/* OPCODE_DEC_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeDEC_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_DEC INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_DEC_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeDEC_Reg16(GetReg(OPRD));
		/* OPCODE_DEC_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeDEC_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_DIV INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_DIV_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeDIV_Reg8(GetReg(OPRD));
		/* OPCODE_DIV_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeDIV_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_DIV INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_DIV_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeDIV_Reg16(GetReg(OPRD));
		/* OPCODE_DIV_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeDIV_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	/* OPCODE_HLT */
	else if(StringCmp(Token, INS_HLT)) EncodeHLT();
	else if(StringCmp(Token, INS_IDIV INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_IDIV_REG8 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeIDIV_Reg8(GetReg(OPRD));
		/* OPCODE_IDIV_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeIDIV_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_IDIV INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_IDIV_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeIDIV_Reg16(GetReg(OPRD));
		/* OPCODE_IDIV_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeIDIV_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_IMUL INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_IMUL_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeIMUL_Reg8(GetReg(OPRD));
		/* OPCODE_IMUL_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeIMUL_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_IMUL INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_IMUL_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeIMUL_Reg16(GetReg(OPRD));
		/* OPCODE_IMUL_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeIMUL_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_IN INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_INB_Imm8 */
		if(IsConstant(OPRD)) EncodeINB_Imm8((uchar)GetConstant(OPRD));
		/* OPCODE_INB_DX */
		else if(StringCmp(Token, INS_REG_PREFIX REGS_DX))
			EncodeINB_DX();
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_IN INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_INW_Imm8 */
		if(IsConstant(OPRD)) EncodeINW_Imm8(GetConstant(OPRD));
		/* OPCODE_INW_DX */
		else if(StringCmp(Token, INS_REG_PREFIX REGS_DX))
			EncodeINW_DX();
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_INC INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_INC_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeINC_Reg8(GetReg(OPRD));
		/* OPCODE_INC_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeINC_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_INC INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_INC_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeINC_Reg16(GetReg(OPRD));
		/* OPCODE_INC_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeINC_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_IRET))
		EncodeIRET();
	else if(StringCmp(Token, INS_LAHF))
		EncodeLAHF();
	else if(StringCmp(Token, INS_LDS))
	{
		char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
		
		GETTOKEN(OPRD1);
		GETTOKEN(OPRD2);
		GETTOKEN(OPRD2);
		
		/* OPCODE_LDS_REG16_MEM32 */
		if(IsReg(OPRD1) && IsReg16(OPRD1) && IsMem(OPRD2))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD2, &Reg1, &Reg2, &Offset);
			EncodeLDS_Reg16_Mem32(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_LES))
	{
		char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
		
		GETTOKEN(OPRD1);
		GETTOKEN(OPRD2);
		GETTOKEN(OPRD2);
		
		/* OPCODE_LES_REG16_MEM32 */
		if(IsReg(OPRD1) && IsReg16(OPRD1) && IsMem(OPRD2))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD2, &Reg1, &Reg2, &Offset);
			EncodeLES_Reg16_Mem32(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_LEA))
	{
		char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
		
		GETTOKEN(OPRD1);
		GETTOKEN(OPRD2);
		GETTOKEN(OPRD2);
		
		/* OPCODE_LEA_REG16_MEM */
		if(IsReg(OPRD1) && IsReg16(OPRD1) && IsMem(OPRD2))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD2, &Reg1, &Reg2, &Offset);
			EncodeLEA_Reg16_Mem(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_LODSB))
		EncodeLODSB();
	/* OPCODE_LODSW */
	else if(StringCmp(Token, INS_LODSW))
		EncodeLODSW();
	/* OPCODE_MOVSB */
	else if(StringCmp(Token, INS_MOVSB))
		EncodeMOVSB();
	/* OPCODE_MOVSW */
	else if(StringCmp(Token, INS_MOVSW))
		EncodeMOVSW();
	else if(StringCmp(Token, INS_MUL INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_MUL_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeMUL_Reg8(GetReg(OPRD));
		/* OPCODE_MUL_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeMUL_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_MUL INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_MUL_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeMUL_Reg16(GetReg(OPRD));
		/* OPCODE_MUL_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeMUL_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_NEG INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_NEG_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeNEG_Reg8(GetReg(OPRD));
		/* OPCODE_NEG_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeNEG_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_NEG INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_NEG_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeNEG_Reg16(GetReg(OPRD));
		/* OPCODE_NEG_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeNEG_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	/* OPCODE_NOP */
	else if(StringCmp(Token, INS_NOP))
		EncodeNOP();
	else if(StringCmp(Token, INS_NOT INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_NOT_REG8 */
		if(IsReg(OPRD) && IsReg8(OPRD)) EncodeNOT_Reg8(GetReg(OPRD));
		/* OPCODE_NOT_MEM8 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeNOT_Mem8(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_NOT INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_NOT_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD)) EncodeNOT_Reg16(GetReg(OPRD));
		/* OPCODE_NOT_MEM16 */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeNOT_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_OUT INS_BYTE))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_OUTB_Imm8 */
		if(IsConstant(OPRD)) EncodeOUTB_Imm8(GetConstant(OPRD));
		/* OPCODE_OUTB_DX */
		else if(StringCmp(Token, INS_REG_PREFIX REGS_DX))
			EncodeOUTB_DX();
		else InvalidInstruction();
	}
	else if(StringCmp(Token, INS_OUT INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		/* OPCODE_OUTW_Imm8 */
		if(IsConstant(OPRD)) EncodeOUTW_Imm8(GetConstant(OPRD));
		/* OPCODE_OUTW_DX */
		else if(StringCmp(Token, INS_REG_PREFIX REGS_DX))
			EncodeOUTW_DX();
		else InvalidInstruction();
	}
	/* POPW */
	else if(StringCmp(Token, INS_POP INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		
		/* OPCODE_POP_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD))
			EncodePOP_Reg16(GetReg(OPRD));
		/* OPCODE_POP_MEM16 */
		else if(IsSeg(OPRD))
			EncodePOP_Seg(GetReg(OPRD));
		/* OPCODE_POP_SEG */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodePOP_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	/* POPF */
	else if(StringCmp(Token, INS_POPF))
		EncodePOPF();
	/* PUSHW */
	else if(StringCmp(Token, INS_PUSH INS_WORD))
	{
		char OPRD[OPRD_SIZE];
		
		GETTOKEN(OPRD);
		
		/* OPCODE_PUSH_REG16 */
		if(IsReg(OPRD) && IsReg16(OPRD))
			EncodePUSH_Reg16(GetReg(OPRD));
		/* OPCODE_PUSH_MEM16 */
		else if(IsSeg(OPRD))
			EncodePUSH_Seg(GetReg(OPRD));
		/* OPCODE_PUSH_SEG */
		else if(IsMem(OPRD))
		{
			uchar Reg1, Reg2;
			uint Offset;
			
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodePUSH_Mem16(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		else InvalidInstruction();
	}
	/* PUSHF */
	else if(StringCmp(Token, INS_PUSHF))
		EncodePUSHF();
	else return 0;
	return 1;
}

static void _Parse(void)
{
	char Token[50];
	
	while(1)
	{
		GETTOKEN(Token);
		
		if(StringCmp(Token, INS_EOF)) break;
		else if(StringCmp(Token, INS_END)) break;
		else if(StringCmp(Token, INS_LF));
		else if(StringCmp(Token, INS_DB))
		{
			char OPRD[OPRD_SIZE];
			uint Byte;
			
			GETTOKEN(OPRD);
			Byte = GetConstant(OPRD);
			EncodeDB((uchar)Byte);
		}
		else if(StringCmp(Token, INS_DW))
		{
			char OPRD[OPRD_SIZE];
			uint Word;
			
			GETTOKEN(OPRD);
			Word = GetConstant(OPRD);
			EncodeDW(Word);
		}
		else if(StringCmp(Token, INS_SET))
		{
			char Label[OPRD_SIZE];
			char Value[OPRD_SIZE];
			
			GETTOKEN(Label);
			GETTOKEN(Value);
			if(CurrentOperat == OPT_SCAN)
				AddLabelToSTable(Label, GetConstant(Value));
		}
		else if(StringCmp(Token, INS_LABEL))
		{
			char Label[OPRD_SIZE];
			
			GETTOKEN(Label);
			if(CurrentOperat == OPT_SCAN && HasLabel(Label)) 
				ERROR("Redefine label");
			if(CurrentOperat == OPT_SCAN) 
				AddLabelToSTable(Label, GetCurrentPos()); 
		}
		else if(StringCmp(Token, INS_DBS))
		{
			char OPRD[OPRD_SIZE];
			uint Count;
			int i;
			
			GETTOKEN(OPRD);
			Count = GetConstant(OPRD);
			for(i = 0; i < Count; i++)
				EncodeDB(0);
		}
		else if(StringCmp(Token, INS_DWS))
		{
			char OPRD[OPRD_SIZE];
			uint Count;
			int i;
			
			GETTOKEN(OPRD);
			Count = GetConstant(OPRD);
			for(i = 0; i < Count; i++)
				EncodeDW(0);
		}
		else if(StringCmp(Token, INS_STR))
		{
			char String[STRING_SIZE];
			char OPRD[STRING_SIZE];
			
			GETTOKEN(OPRD);
			if(IsString(OPRD))
			{
				int i;
				
				GetString(OPRD, String);
				for(i = 0; i < GetStringLen(String); i++)
					EncodeDB(String[i]);
			}
		}
		/* OPCODE_AAA */
		else if(StringCmp(Token, INS_AAA))
			EncodeAAA();
		/* OPCODE_AAD */	
		else if(StringCmp(Token, INS_AAD))
			EncodeAAD();
		/* OPCODE_AAM */
		else if(StringCmp(Token, INS_AAM))
			EncodeAAM();
		/* OPCODE_AAS */
		else if(StringCmp(Token, INS_AAS))
			EncodeAAS();
		
		/* Operator */
		else if(ParseOperator(Token));
		
		/* OPCODE_INT_3 */
		else if(StringCmp(Token, INS_INT3))
			EncodeINT_3();
		/* OPCODE_INT_IMM8 */
		else if(StringCmp(Token, INS_INT))
		{
			char OPRD[OPRD_SIZE];
			uint FuncNumber;
			
			GETTOKEN(OPRD);
			FuncNumber = GetConstant(OPRD);
			EncodeINT_Imm8((uchar)FuncNumber);
		}
		/* OPCODE_INTO */
		else if(StringCmp(Token, INS_INTO))
			EncodeINTO();
		else if(StringCmp(Token, INS_MOV INS_BYTE))
		{
			char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
			
			GETTOKEN(OPRD1);
			GETTOKEN(OPRD2);
			GETTOKEN(OPRD2);
			
			if(IsMem(OPRD1) && IsReg(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				GetMem(OPRD1, &Reg1, &Reg2, &Offset);
				
				/* OPCODE_MOV_MEMOFF_ACC8 */
				if(Reg1 == REG_NONE && Reg2 == REG_NONE && StringCmp(OPRD2, INS_REG_PREFIX REGS_AL)) 
					EncodeMOV_MemOff_Acc8(Offset);
				/* OPCODE_MOV_MEM8_REG8 */
				else if(IsReg8(OPRD2))
					EncodeMOV_Mem8_Reg8(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));
				else InvalidInstruction();
				
			}
			else if(IsReg(OPRD1) && IsMem(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				GetMem(OPRD2, &Reg1, &Reg2, &Offset);
				
				/* OPCODE_MOV_ACC8_MEMOFF */
				if(Reg1 == REG_NONE && Reg2 == REG_NONE && StringCmp(OPRD2, INS_REG_PREFIX REGS_AL)) 
					EncodeMOV_Acc8_MemOff(Offset);
				/* OPCODE_MOV_REG8_MEM8 */
				else if(IsReg8(OPRD1))
					EncodeMOV_Reg8_Mem8(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
				else InvalidInstruction();
			}
			/* OPCODE_MOV_REG8_IMM8 */
			else if(IsReg(OPRD1) && IsReg8(OPRD1) && IsConstant(OPRD2))
			{
				uchar Reg = GetReg(OPRD1);
				uchar Imm8 = (uchar)GetConstant(OPRD2);
				EncodeMOV_Reg8_Imm8(Reg, Imm8);
			}
			/* OPCODE_MOV_MEM8_IMM8 */
			else if(IsMem(OPRD1) && IsConstant(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				uchar Imm8 = (uchar)GetConstant(OPRD2);
				GetMem(OPRD1, &Reg1, &Reg2, &Offset);
				EncodeMOV_Mem8_Imm8(Reg1, Reg2, GetOffType(Offset), Offset, Imm8);
			}
			/* OPCODE_MOV_REG8_REG8 */
			else if(IsReg(OPRD1) && IsReg8(OPRD1) && IsReg(OPRD2) && IsReg8(OPRD2))
				EncodeMOV_Reg8_Reg8(GetReg(OPRD1), GetReg(OPRD2));
			else InvalidInstruction();
			
		}
		else if(StringCmp(Token, INS_MOV INS_WORD))
		{
			char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
			
			GETTOKEN(OPRD1);
			GETTOKEN(OPRD2);
			GETTOKEN(OPRD2);
			
			if(IsMem(OPRD1) && IsReg(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				GetMem(OPRD1, &Reg1, &Reg2, &Offset);
				
				/* OPCODE_MOV_MEMOFF_ACC16 */
				if(Reg1 == REG_NONE && Reg2 == REG_NONE && StringCmp(OPRD2, INS_REG_PREFIX REGS_AX))
					EncodeMOV_MemOff_Acc16(Offset);
				/* OPCODE_MOV_MEM16_SEG */
				else if(IsSeg(OPRD2))
					EncodeMOV_Mem16_Seg(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));
				/* OPCODE_MOV_MEM16_REG16 */
				else if(IsReg16(OPRD2))
					EncodeMOV_Mem16_Reg16(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));
				else InvalidInstruction();
				
			}
			else if(IsReg(OPRD1) && IsMem(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				GetMem(OPRD2, &Reg1, &Reg2, &Offset);
				
				/* OPCODE_MOV_ACC16_MEMOFF */
				if(Reg1 == REG_NONE && Reg2 == REG_NONE && StringCmp(OPRD1, INS_REG_PREFIX REGS_AX))
					EncodeMOV_Acc16_MemOff(Offset);
				/* OPCODE_MOV_SEG_MEM16 */
				else if(IsSeg(OPRD1))
					EncodeMOV_Seg_Mem16(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
				/* OPCODE_MOV_REG16_MEM16 */
				else if(IsReg16(OPRD1))
					EncodeMOV_Reg16_Mem16(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
				else InvalidInstruction();
			}
			/* OPCODE_MOV_REG16_IMM16 */
			else if(IsReg(OPRD1) && IsReg16(OPRD1) && IsConstant(OPRD2))
			{
				uchar Reg = GetReg(OPRD1);
				uint Imm16 = GetConstant(OPRD2);
				EncodeMOV_Reg16_Imm16(Reg, Imm16);
			}
			/* OPCODE_MOV_MEM16_IMM16 */
			else if(IsMem(OPRD1) && IsConstant(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				uint Imm16 = GetConstant(OPRD2);
				GetMem(OPRD1, &Reg1, &Reg2, &Offset);
				EncodeMOV_Mem16_Imm16(Reg1, Reg2, GetOffType(Offset), Offset, Imm16);
			}
			/* OPCODE_MOV_REG16_SEG */
			else if(IsReg(OPRD1) && IsReg16(OPRD1) && IsSeg(OPRD2))
				EncodeMOV_Reg16_Seg(GetReg(OPRD1), GetReg(OPRD2));
			/* OPCODE_MOV_SEG_REG16 */
			else if(IsSeg(OPRD1) && IsReg(OPRD2) && IsReg16(OPRD2))
				EncodeMOV_Seg_Reg16(GetReg(OPRD1), GetReg(OPRD2));
			/* OPCODE_MOV_REG16_REG16 */
			else if(IsReg(OPRD1) && IsReg16(OPRD1) && IsReg(OPRD2) && IsReg16(OPRD2))
				EncodeMOV_Reg16_Reg16(GetReg(OPRD1), GetReg(OPRD2));
			else InvalidInstruction();
		}
		/* _Parse的一部分 */
		else if(_Parse_1(Token));
		/* OPCODE_SAHF */
		else if(StringCmp(Token, INS_SAHF))
			EncodeSAHF();
		/* Shift instruction */
		else if(ParseShift(Token));
		/* OPCODE_SCANSB */
		else if(StringCmp(Token, INS_SCANSB))
			EncodeSCANSB();
		/* OPCODE_SCANSW */
		else if(StringCmp(Token, INS_SCANSW))
			EncodeSCANSW();
		/* OPCODE_STC */
		else if(StringCmp(Token, INS_STC))
			EncodeSTC();
		/* OPCODE_STD */
		else if(StringCmp(Token, INS_STD))
			EncodeSTD();
		/* OPCODE_STI */
		else if(StringCmp(Token, INS_STI))
			EncodeSTI();
		/* OPCODE_STOSB */
		else if(StringCmp(Token, INS_STOSB))
			EncodeSTOSB();
		/* OPCODE_STOSW */
		else if(StringCmp(Token, INS_STOSW))
			EncodeSTOSW();
		/* OPCODE_WAIT */
		else if(StringCmp(Token, INS_WAIT))
			EncodeWAIT();
		/* XCHGB */
		else if(StringCmp(Token, INS_XCHG INS_BYTE))
		{
			char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
			
			GETTOKEN(OPRD1);
			GETTOKEN(OPRD2);
			GETTOKEN(OPRD2);
			
			/* OPCODE_XCHG_REG8_REG8 */
			if(IsReg(OPRD1) && IsReg8(OPRD1) && IsReg(OPRD2) && IsReg8(OPRD2))
				EncodeXCHG_Reg8_Reg8(GetReg(OPRD1), GetReg(OPRD2));
			/* OPCODE_XCHG_MEM8_REG8 */
			else if(IsMem(OPRD1) && IsReg(OPRD2) && IsReg8(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				
				GetMem(OPRD1, &Reg1, &Reg2, &Offset);
				EncodeXCHG_Mem8_Reg8(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));
			}
			/* OPCODE_XCHG_REG8_MEM8 */
			else if(IsReg(OPRD1) && IsReg8(OPRD1) && IsMem(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				
				GetMem(OPRD2, &Reg1, &Reg2, &Offset);
				EncodeXCHG_Reg8_Mem8(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
			}
			else InvalidInstruction();
		}
		/* XCHGW */
		else if(StringCmp(Token, INS_XCHG INS_WORD))
		{
			char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];
			
			GETTOKEN(OPRD1);
			GETTOKEN(OPRD2);
			GETTOKEN(OPRD2);
			
			/* OPCODE_XCHG_ACC16_REG16 */
			if(StringCmp(OPRD1, INS_REG_PREFIX REGS_AX) && IsReg(OPRD2) && IsReg16(OPRD2))
				EncodeXCHG_Acc16_Reg16(GetReg(OPRD2));
			/* OPCODE_XCHG_REG16_ACC16 */
			else if(IsReg(OPRD1) && IsReg16(OPRD1) && StringCmp(OPRD2, INS_REG_PREFIX REGS_AX))
				EncodeXCHG_Reg16_Acc16(GetReg(OPRD1));
			/* OPCODE_XCHG_REG16_REG16 */
			else if(IsReg(OPRD1) && IsReg16(OPRD1) && IsReg(OPRD2) && IsReg16(OPRD2))
				EncodeXCHG_Reg16_Reg16(GetReg(OPRD1), GetReg(OPRD2));
			/* OPCODE_XCHG_MEM16_REG16 */
			else if(IsMem(OPRD1) && IsReg(OPRD2) && IsReg16(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				
				GetMem(OPRD1, &Reg1, &Reg2, &Offset);
				EncodeXCHG_Mem16_Reg16(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));
			}
			/* OPCODE_XCHG_REG16_MEM16 */
			else if(IsReg(OPRD1) && IsReg16(OPRD1) && IsMem(OPRD2))
			{
				uchar Reg1, Reg2;
				uint Offset;
				
				GetMem(OPRD2, &Reg1, &Reg2, &Offset);
				EncodeXCHG_Reg16_Mem16(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);
			}
			else InvalidInstruction();
		}
		/* OPCODE_XLAT */
		else if(StringCmp(Token, INS_XLAT))
			EncodeXLAT();
		/* OPCODE_RET_NEAR */
		else if(StringCmp(Token, INS_RET INS_NEAR))
			EncodeRET_Near();
		/* OPCODE_RET_IMM16NEAR */
		else if(StringCmp(Token, INS_RET INS_NEAR INS_WITH_IMM))
		{
			char OPRD[OPRD_SIZE];
			
			GETTOKEN(OPRD);
			EncodeRET_Imm16Near(GetConstant(OPRD));
		}
		/* OPCODE_RET_FAR */
		else if(StringCmp(Token, INS_RET INS_FAR))
			EncodeRET_Far();
		/* OPCODE_RET_IMM16FAR */
		else if(StringCmp(Token, INS_RET INS_FAR INS_WITH_IMM))
		{
			char OPRD[OPRD_SIZE];
			
			GETTOKEN(OPRD);
			EncodeRET_Imm16Far(GetConstant(OPRD));
		}
		/* OPCODE_CALL_MEMFAR */
		else if(StringCmp(Token, INS_CALL INS_FAR))
		{
			char OPRD[OPRD_SIZE];
			uchar Reg1, Reg2;
			uint Offset;
			
			GETTOKEN(OPRD);
			if(!IsMem(OPRD)) ERROR("Invalid " INS_CALL INS_FAR);
			GetMem(OPRD, &Reg1, &Reg2, &Offset);
			EncodeCALL_MemFar(Reg1, Reg2, GetOffType(Offset), Offset);
		}
		/* OPCODE_CALL_NEAR */
		else if(StringCmp(Token, INS_CALL INS_NEAR))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 3);
			EncodeCALL_Near(Offset);
		}
		/* OPCODE_JCC_SHORT */
		else if(ParseJcc(Token));
		/* OPCODE_JMP_SHORT */
		else if(StringCMP(Token, INS_JMP INS_SHORT))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 2);
			EncodeJMP_SHORT(Offset);
		}
		/* OPCODE_JMP_NEAR */
		else if(StringCmp(Token, INS_JMP INS_NEAR))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 3);
			EncodeJMP_NEAR(Offset);
		}
		/* OPCODE_JCXZ_SHORT */
		else if(StringCmp(Token, INS_JCXZ))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 2);
			EncodeJCXZ_SHORT((uchar)Offset);
		}
		/* OPCODE_LOOP_SHORT */
		else if(StringCmp(Token, INS_LOOP))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 2);
			EncodeLOOP_SHORT((uchar)Offset);
		}
		/* OPCODE_LOOPZ_SHORT */
		else if(StringCmp(Token, INS_LOOP))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 2);
			EncodeLOOPZ_SHORT((uchar)Offset);
		}
		/* OPCODE_LOOPNZ_SHORT */
		else if(StringCmp(Token, INS_LOOP))
		{
			char OPRD[OPRD_SIZE];
			uint Offset;
			uint CurrentPos = GetCurrentPos();
			
			GETTOKEN(OPRD);
			Offset = GetConstant(OPRD);
			Offset = Offset - (CurrentPos + 2);
			EncodeLOOPNZ_SHORT((uchar)Offset);
		}
		/* OPCODE_PREFIX_LOCK */
		else if(StringCmp(Token, INS_LOCK)) EncodePrefixLOCK();
		/* OPCODE_PREFIX_REP */
		else if(StringCmp(Token, INS_REP)) EncodePrefixREP();
		/* OPCODE_PREFIX_REPNZ */
		else if(StringCmp(Token, INS_REPNZ)) EncodePrefixREPNZ();
		/* CS */
		else if(StringCmp(Token, INS_CS)) EncodePrefixCS();
		/* DS */
		else if(StringCmp(Token, INS_DS)) EncodePrefixDS();
		/* ES */
		else if(StringCmp(Token, INS_ES)) EncodePrefixES();
		/* SS */
		else if(StringCmp(Token, INS_SS)) EncodePrefixSS();
		/* 违法的指令 */
		else 
			InvalidInstruction();
	}
}

void Scan(void)
{
	CurrentOperat = OPT_SCAN;
	DisableDebug();
	_Parse();
	ResetLexer();
	ResetEncoder();
	CurrentOperat = OPT_NONE;
}

void Parse(void)
{
	CurrentOperat = OPT_PARSE;
	DisableDebug();
	_Parse();
	ResetLexer();
	CurrentOperat = OPT_NONE;
}
