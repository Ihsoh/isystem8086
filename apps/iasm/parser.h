/*
 * parser.h
 * Ihsoh
 * 2013-11-8
*/

#ifndef	PARSER_H_
#define	PARSER_H_

#include "ilib.h"
#include "lexer.h"
#include "encoder.h"
#include "stable.h"
#include "reg.h"

#define	OPRD_SIZE	50
#define	STRING_SIZE	100

#define	OPT_SCAN	0
#define	OPT_PARSE	1
#define	OPT_NONE	2

#define	REGS_AX		"AX"
#define	REGS_BX		"BX"
#define	REGS_CX		"CX"
#define	REGS_DX		"DX"
#define	REGS_AL		"AL"
#define	REGS_BL		"BL"
#define	REGS_CL		"CL"
#define	REGS_DL		"DL"
#define	REGS_AH		"AH"
#define	REGS_BH		"BH"
#define	REGS_CH		"CH"
#define	REGS_DH		"DH"
#define	REGS_SI		"SI"
#define	REGS_DI		"DI"
#define	REGS_BP		"BP"
#define	REGS_SP		"SP"
#define	REGS_SS		"SS"
#define	REGS_DS		"DS"
#define	REGS_ES		"ES"
#define	REGS_CS		"CS"

#define	INS_BYTE	"B"
#define	INS_WORD	"W"

#define	INS_SHORT	"S"
#define	INS_NEAR	"N"
#define	INS_FAR		"F"

#define	INS_WITH_IMM	"N"

#define	INS_REG_PREFIX	"%"

#define	INS_LF		"\n"
#define	INS_EOF		""
#define	INS_END		"END"
#define	INS_LABEL	"LABEL"
#define	INS_SET		"SET"
#define	INS_DB		"DB"
#define	INS_DW		"DW"
#define	INS_DBS		"DBS"
#define	INS_DWS		"DBW"
#define	INS_STR		"STR"
#define	INS_AAA		"AAA"
#define	INS_AAD		"AAD"
#define	INS_AAM		"AAM"
#define	INS_AAS		"AAS"
#define	INS_ADC		"ADC"
#define	INS_ADD		"ADD"
#define	INS_AND		"AND"
#define	INS_CBW		"CBW"
#define	INS_CLC		"CLC"
#define	INS_CLD		"CLD"
#define	INS_CLI		"CLI"
#define	INS_CMC		"CMC"
#define	INS_CMP		"CMP"
#define	INS_CMPSB	"CMPSB"
#define	INS_CMPSW	"CMPSW"
#define	INS_CWD		"CWD"
#define	INS_DAA		"DAA"
#define	INS_DAS		"DAS"
#define	INS_DEC		"DEC"
#define	INS_DIV		"DIV"
#define	INS_HLT		"HLT"
#define	INS_IDIV	"IDIV"
#define	INS_IMUL	"IMUL"
#define	INS_IN		"IN"
#define	INS_INT3	"INT3"
#define	INS_INT		"INT"
#define	INS_INTO	"INTO"
#define	INS_INC		"INC"
#define	INS_IRET	"IRET"
#define	INS_LAHF	"LAHF"
#define	INS_LDS		"LDS"
#define	INS_LES		"LES"
#define	INS_LEA		"LEA"
#define	INS_LODSB	"LODSB"
#define	INS_LODSW	"LODSW"
#define	INS_MOV		"MOV"
#define	INS_MOVSB	"MOVSB"
#define	INS_MOVSW	"MOVSW"
#define	INS_MUL		"MUL"
#define	INS_NEG		"NEG"
#define	INS_NOP		"NOP"
#define	INS_NOT		"NOT"
#define	INS_OR		"OR"
#define	INS_OUT		"OUT"
#define	INS_POP		"POP"
#define	INS_POPF	"POPF"
#define	INS_PUSH	"PUSH"
#define	INS_PUSHF	"PUSHF"
#define	INS_RCL		"RCL"
#define	INS_RCR		"RCR"
#define	INS_RET		"RET"
#define	INS_ROL		"ROL"
#define	INS_ROR		"ROR"
#define	INS_SAHF	"SAHF"
#define	INS_SAL		"SAL"
#define	INS_SAR		"SAR"
#define	INS_SHL		"SHL"
#define	INS_SHR		"SHR"
#define	INS_SBB		"SBB"
#define	INS_SCANSB	"SCANSB"
#define	INS_SCANSW	"SCANSW"
#define	INS_STC		"STC"
#define	INS_STD		"STD"
#define	INS_STI		"STI"
#define	INS_STOSB	"STOSB"
#define	INS_STOSW	"STOSW"
#define	INS_SUB		"SUB"
#define	INS_TEST	"TEST"
#define	INS_WAIT	"WAIT"
#define	INS_XCHG	"XCHG"
#define	INS_XLAT	"XLAT"
#define	INS_XOR		"XOR"
#define	INS_CALL	"CALL"
/*Jcc*/

#define	INS_JXXXX	"JXXXX"
#define	INS_JO		"JO"
#define	INS_JNO		"JNO"
#define	INS_JC		"JC"
#define	INS_JB		"JB"
#define	INS_JNAE	"JNAE"
#define	INS_JNC		"JNC"
#define	INS_JAE		"JAE"
#define	INS_JNB		"JNB"
#define	INS_JE		"JE"
#define	INS_JZ		"JZ"
#define	INS_JNE		"JNE"
#define	INS_JNZ		"JNZ"
#define	INS_JBE		"JBE"
#define	INS_JNA		"JNA"
#define	INS_JA		"JA"
#define	INS_JNBE	"JNBE"
#define	INS_JS		"JS"
#define	INS_JNS		"JNS"
#define	INS_JP		"JP"
#define	INS_JPE		"JPE"
#define	INS_JNP		"JNP"
#define	INS_JPO		"JPO"
#define	INS_JL		"JL"
#define	INS_JNGE	"JNGE"
#define	INS_JGE		"JGE"
#define	INS_JNL		"JNL"
#define	INS_JLE		"JLE"
#define	INS_JNG		"JNG"
#define	INS_JG		"JG"
#define	INS_JNLE	"JNLE"



#define	INS_JCXZ	"JCXZ"
#define	INS_JMP		"JMP"
#define	INS_LOOP	"LOOP"
#define	INS_LOOPZ	"LOOPZ"
#define	INS_LOOPNZ	"LOOPNZ"
#define	INS_LOCK	"LOCK"
#define	INS_REP		"REP"
#define	INS_REPNZ	"REPNZ"
#define	INS_CS		"CS"
#define	INS_DS		"DS"
#define	INS_ES		"ES"
#define	INS_SS		"SS"


#define	OPT(x)		else if(StringCmp(Token, INS_##x INS_BYTE))	\
					{	\
						char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];	\
							\
						GETTOKEN(OPRD1);	\
						GETTOKEN(OPRD2);	\
						GETTOKEN(OPRD2);	\
							\
						if(IsReg(OPRD1) && IsReg(OPRD2))	\
							Encode##x##_Reg8_Reg8(GetReg(OPRD1), GetReg(OPRD2));	\
						else if(IsMem(OPRD1) && IsReg(OPRD2))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
								\
							GetMem(OPRD1, &Reg1, &Reg2, &Offset);	\
							Encode##x##_Mem8_Reg8(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));	\
						}	\
						else if(IsReg(OPRD1) && IsMem(OPRD2))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
								\
							GetMem(OPRD2, &Reg1, &Reg2, &Offset);	\
							Encode##x##_Reg8_Mem8(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);	\
						}	\
						else if(IsReg(OPRD1) && IsConstant(OPRD2))	\
						{	\
							if(StringCmp(OPRD1, INS_REG_PREFIX REGS_AL) && IsConstant(OPRD2))	\
								Encode##x##_Acc8_Imm8((uchar)GetConstant(OPRD2));		\
							else	\
								Encode##x##_Reg8_Imm8(GetReg(OPRD1), (uchar)GetConstant(OPRD2));	\
						}	\
						else if(IsMem(OPRD1) && IsConstant(OPRD2))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
								\
							GetMem(OPRD1, &Reg1, &Reg2, &Offset);	\
							Encode##x##_Mem8_Imm8(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));	\
						}	\
					}	\
					else if(StringCmp(Token, INS_##x INS_WORD))	\
					{	\
						char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];	\
							\
						GETTOKEN(OPRD1);	\
						GETTOKEN(OPRD2);	\
						GETTOKEN(OPRD2);	\
							\
						if(IsReg(OPRD1) && IsReg(OPRD2))	\
							Encode##x##_Reg16_Reg16(GetReg(OPRD1), GetReg(OPRD2));	\
						else if(IsMem(OPRD1) && IsReg(OPRD2))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
								\
							GetMem(OPRD1, &Reg1, &Reg2, &Offset);	\
							Encode##x##_Mem16_Reg16(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));	\
						}	\
						else if(IsReg(OPRD1) && IsMem(OPRD2))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
								\
							GetMem(OPRD2, &Reg1, &Reg2, &Offset);	\
							Encode##x##_Reg16_Mem16(GetReg(OPRD1), Reg1, Reg2, GetOffType(Offset), Offset);	\
						}	\
						else if(IsReg(OPRD1) && IsConstant(OPRD2))	\
						{	\
							if(StringCmp(OPRD1, INS_REG_PREFIX REGS_AX))	\
								Encode##x##_Acc16_Imm16(GetConstant(OPRD2));	\
							else	\
								Encode##x##_Reg16_Imm16(GetReg(OPRD1), GetConstant(OPRD2));	\
						}	\
						else if(IsMem(OPRD1) && IsConstant(OPRD2))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
								\
							GetMem(OPRD1, &Reg1, &Reg2, &Offset);	\
							Encode##x##_Mem16_Imm16(Reg1, Reg2, GetOffType(Offset), Offset, GetReg(OPRD2));	\
						}	\
					}

#define	JCC(cccc1, cccc2, cccc3)	else if(	StringCmp(Token, INS_J##cccc1)	||	\
												StringCmp(Token, INS_J##cccc2)	||	\
												StringCmp(Token, INS_J##cccc3))		\
									{	\
										char OPRD[OPRD_SIZE];	\
										uint Offset;	\
										uint CurrentPos = GetCurrentPos();	\
											\
										GETTOKEN(OPRD);	\
										Offset = GetConstant(OPRD);	\
										Offset = Offset - (CurrentPos + 2);	\
										EncodeJcc_SHORT((uchar)CCCC_##cccc1, Offset);	\
									}

#define	SHIFT(x)	else if(StringCmp(Token, INS_##x))	\
					{	\
						char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];	\
							\
						GETTOKEN(OPRD1);	\
						GETTOKEN(OPRD2);	\
						GETTOKEN(OPRD2);	\
							\
						if(IsReg(OPRD1) && IsConstant(OPRD2) && GetConstant(OPRD2) == 1)	\
							Encode##x##_Reg8_1(GetReg(OPRD1));	\
						else if(IsReg(OPRD1) && StringCmp(OPRD2, INS_REG_PREFIX REGS_CL))	\
							Encode##x##_Reg8_CL(GetReg(OPRD1));	\
						else if(IsMem(OPRD1))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
							GetMem(OPRD2, &Reg1, &Reg2, &Offset);	\
							\
							if(IsConstant(OPRD2) && GetConstant(OPRD2) == 1)	\
								Encode##x##_Mem8_1(Reg1, Reg2, GetOffType(Offset), Offset);	\
							else if(StringCmp(OPRD2, INS_REG_PREFIX REGS_CL))	\
								Encode##x##_Mem8_CL(Reg1, Reg2, GetOffType(Offset), Offset);	\
						}	\
					}	\
					else if(StringCmp(Token, INS_##x INS_WORD))	\
					{	\
						char OPRD1[OPRD_SIZE], OPRD2[OPRD_SIZE];	\
							\
						GETTOKEN(OPRD1);	\
						GETTOKEN(OPRD2);	\
						GETTOKEN(OPRD2);	\
							\
						if(IsReg(OPRD1) && IsConstant(OPRD2) && GetConstant(OPRD2) == 1)	\
							Encode##x##_Reg16_1(GetReg(OPRD1));	\
						else if(IsReg(OPRD1) && StringCmp(OPRD2, INS_REG_PREFIX REGS_CL))	\
							Encode##x##_Reg16_CL(GetReg(OPRD1));	\
						else if(IsMem(OPRD1))	\
						{	\
							uchar Reg1, Reg2;	\
							uint Offset;	\
							GetMem(OPRD2, &Reg1, &Reg2, &Offset);	\
							\
							if(IsConstant(OPRD2) && GetConstant(OPRD2) == 1)	\
								Encode##x##_Mem16_1(Reg1, Reg2, GetOffType(Offset), Offset);	\
							else if(StringCmp(OPRD2, INS_REG_PREFIX REGS_CL))	\
								Encode##x##_Mem16_CL(Reg1, Reg2, GetOffType(Offset), Offset);	\
						}	\
					}

extern void InitParser(void);
extern void DestroyParser(void);
extern void AddCharToParser(char Char);

#endif
