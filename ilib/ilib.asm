;ilib.asm
;Ihsoh
;2013-2-17

.MODEL TINY
OPTION CASEMAP:NONE
.DATA

	;*==================================================*
	;|以下代码来之BC31的WINLIB的FPINIT.asm文件, 当使用	|
	;|浮点数时需要!!!									|
	;*==================================================*
	public  		FIDRQQ          ; wait, esc
	public  		FIARQQ          ; wait, DS:
	public  		FICRQQ          ; wait, CS:
	public  		FIERQQ          ; wait, ES:
	public  		FISRQQ          ; wait, SS:
	public  		FIWRQQ          ; nop, wait
	public  		FJARQQ          ; Esc nn -> DS:nn
	public  		FJCRQQ          ; Esc nn -> CS:nn
	public  		FJSRQQ          ; Esc nn -> ES:nn

	FIDRQQ          equ     05C32h
	FIARQQ          equ     0FE32h
	FICRQQ          equ     00E32h
	FIERQQ          equ     01632h
	FISRQQ          equ     00632h
	FIWRQQ          equ     0A23Dh
	FJARQQ          equ     04000h
	FJCRQQ          equ     0C000h
	FJSRQQ          equ     08000h
	;====================================================

	;变量名:	FLAGS8087@
	;描述:		TCC链接需要. 用于储存FPU状态字.
	PUBLIC			FLAGS8087@
	FLAGS8087@		DW ?
.CODE
	EXTERN			_PrintString:FAR
	
	;过程名:	NotImplement
	;描述:		输出某个过程未实现的信息.
	;参数:		DS:SI=未实现的过程名段地址:偏移地址.
	NImplement_Msg	DB ' not implement!!!', 0
	NotImplement	PROC NEAR
		MOV			AH, 4
		INT			21H
		LEA			SI, NImplement_Msg
		INT			21H
		JMP			$	;死机
	NotImplement	ENDP

	;过程名:	FTOL@
	;描述:		TCC链接需要. 当把float类型强制转换为
	;			int类型时, 需要该过程实现.
	PUBLIC FTOL@
	FTOL@			PROC FAR
		Var_C		EQU WORD PTR -0CH
		Var_A		EQU WORD PTR -0AH
		Var_8		EQU QWORD PTR -8
		
		PUSH		BP
		MOV			BP, SP
		SUB			SP, 0CH
		FNSTCW		[BP + Var_C]
		WAIT
		MOV			AX, [BP + Var_C]
		OR			AX, 0C00H
		MOV			[BP + Var_A], AX
		FLDCW		[BP + Var_A]
		FISTP		[BP + Var_8]
		FLDCW		[BP + Var_C]
		MOV			DX, WORD PTR [BP + Var_8 + 2]
		MOV			AX, WORD PTR [BP + Var_8]
		MOV			SP, BP
		POP			BP
		RETF
	FTOL@			ENDP

	;过程名:	SCOPY@
	;描述:		TCC链接需要. 当使用一个初始化数组时,
	;			编译的程序会调用该过程初始化数组.
	PUBLIC SCOPY@
	SCOPY@			PROC FAR
		Arg0		EQU DWORD PTR 6
		Arg4		EQU DWORD PTR 0AH
		
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		DI
		PUSH		DS
		PUSHF
		LDS			SI, [BP + Arg0]
		LES			DI, [BP + Arg4]
		CLD
		SHR			CX, 1
		REP MOVSW
		ADC			CX, CX
		REP MOVSB
		POPF
		POP			DS
		POP			DI
		POP			SI
		POP			BP
		RETF		8
	SCOPY@			ENDP
	
	;过程名:	LUDIV@
	;描述:		TCC链接需要. 无符号长整形除法.
	PUBLIC	LUDIV@
	LUDIV@			PROC FAR
		; mov     cx, 1
		; jmp     loc_1068F
		arg_0           = word ptr  0Ah
		arg_2           = word ptr  0Ch
		arg_4           = word ptr  0Eh
		arg_6           = word ptr  10h
		
						mov     cx, 1
		
loc_1068F:                              ; CODE XREF: sub_10682+3j
						push    bp
						push    si
						push    di
						mov     bp, sp
						mov     di, cx
						mov     ax, [bp+arg_0]
						mov     dx, [bp+arg_2]
						mov     bx, [bp+arg_4]
						mov     cx, [bp+arg_6]
						or      cx, cx
						jnz     short loc_106AE
						or      dx, dx
						jz      short loc_10713
						or      bx, bx
						jz      short loc_10713
		
		loc_106AE:                              ; CODE XREF: sub_1068C+18j
						test    di, 1
						jnz     short loc_106D0
						or      dx, dx
						jns     short loc_106C2
						neg     dx
						neg     ax
						sbb     dx, 0
						or      di, 0Ch
		
		loc_106C2:                              ; CODE XREF: sub_1068C+2Aj
						or      cx, cx
						jns     short loc_106D0
						neg     cx
						neg     bx
						sbb     cx, 0
						xor     di, 4
		
		loc_106D0:                              ; CODE XREF: sub_1068C+26j
												; sub_1068C+38j
						mov     bp, cx
						mov     cx, 20h ; ' '
						push    di
						xor     di, di
						xor     si, si
		
		loc_106DA:                              ; CODE XREF: sub_1068C:loc_106F1j
						shl     ax, 1
						rcl     dx, 1
						rcl     si, 1
						rcl     di, 1
						cmp     di, bp
						jb      short loc_106F1
						ja      short loc_106EC
						cmp     si, bx
						jb      short loc_106F1
		
		loc_106EC:                              ; CODE XREF: sub_1068C+5Aj
						sub     si, bx
						sbb     di, bp
						inc     ax
		
		loc_106F1:                              ; CODE XREF: sub_1068C+58j
												; sub_1068C+5Ej
						loop    loc_106DA
						pop     bx
						test    bx, 2
						jz      short loc_10700
						mov     ax, si
						mov     dx, di
						shr     bx, 1
		
		loc_10700:                              ; CODE XREF: sub_1068C+6Cj
						test    bx, 4
						jz      short loc_1070D
						neg     dx
						neg     ax
						sbb     dx, 0
		
		loc_1070D:                              ; CODE XREF: sub_1068C+78j
												; sub_1068C+93j
						pop     di
						pop     si
						pop     bp
						retf    8
		loc_10713:                              ; CODE XREF: sub_1068C+1Cj
												; sub_1068C+20j
						div     bx
						test    di, 2
						jz      short loc_1071D
						mov     ax, dx

		loc_1071D:                              ; CODE XREF: sub_1068C+8Dj
						xor     dx, dx
						jmp     short loc_1070D

	LUDIV@			ENDP
	
	;过程名:	LUMOD@
	;描述:		TCC链接需要. 无符号长整形求模.
	PUBLIC LUMOD@
	LUMOD@			PROC FAR
		
		arg_0           = word ptr  0Ah
		arg_2           = word ptr  0Ch
		arg_4           = word ptr  0Eh
		arg_6           = word ptr  10h
		
						mov     cx, 3
		
loc_1068F:                              ; CODE XREF: sub_10682+3j
						push    bp
						push    si
						push    di
						mov     bp, sp
						mov     di, cx
						mov     ax, [bp+arg_0]
						mov     dx, [bp+arg_2]
						mov     bx, [bp+arg_4]
						mov     cx, [bp+arg_6]
						or      cx, cx
						jnz     short loc_106AE
						or      dx, dx
						jz      short loc_10713
						or      bx, bx
						jz      short loc_10713
		
		loc_106AE:                              ; CODE XREF: sub_1068C+18j
						test    di, 1
						jnz     short loc_106D0
						or      dx, dx
						jns     short loc_106C2
						neg     dx
						neg     ax
						sbb     dx, 0
						or      di, 0Ch
		
		loc_106C2:                              ; CODE XREF: sub_1068C+2Aj
						or      cx, cx
						jns     short loc_106D0
						neg     cx
						neg     bx
						sbb     cx, 0
						xor     di, 4
		
		loc_106D0:                              ; CODE XREF: sub_1068C+26j
												; sub_1068C+38j
						mov     bp, cx
						mov     cx, 20h ; ' '
						push    di
						xor     di, di
						xor     si, si
		
		loc_106DA:                              ; CODE XREF: sub_1068C:loc_106F1j
						shl     ax, 1
						rcl     dx, 1
						rcl     si, 1
						rcl     di, 1
						cmp     di, bp
						jb      short loc_106F1
						ja      short loc_106EC
						cmp     si, bx
						jb      short loc_106F1
		
		loc_106EC:                              ; CODE XREF: sub_1068C+5Aj
						sub     si, bx
						sbb     di, bp
						inc     ax
		
		loc_106F1:                              ; CODE XREF: sub_1068C+58j
												; sub_1068C+5Ej
						loop    loc_106DA
						pop     bx
						test    bx, 2
						jz      short loc_10700
						mov     ax, si
						mov     dx, di
						shr     bx, 1
		
		loc_10700:                              ; CODE XREF: sub_1068C+6Cj
						test    bx, 4
						jz      short loc_1070D
						neg     dx
						neg     ax
						sbb     dx, 0
		
		loc_1070D:                              ; CODE XREF: sub_1068C+78j
												; sub_1068C+93j
						pop     di
						pop     si
						pop     bp
						retf    8
		loc_10713:                              ; CODE XREF: sub_1068C+1Cj
												; sub_1068C+20j
						div     bx
						test    di, 2
						jz      short loc_1071D
						mov     ax, dx

		loc_1071D:                              ; CODE XREF: sub_1068C+8Dj
						xor     dx, dx
						jmp     short loc_1070D

	LUMOD@			ENDP
	
	;过程名:	LXMUL@
	;描述:		TCC链接需要. 无符号长整形乘法.
	LXMUL@			PROC FAR
                push    si
                xchg    ax, si
                xchg    ax, dx
                test    ax, ax
                jz      short loc_10671
                mul     bx

loc_10671:                              ; CODE XREF: sub_10668+5j
                xchg    ax, cx
                test    ax, ax
                jz      short loc_1067A
                mul     si
                add     cx, ax

loc_1067A:                              ; CODE XREF: sub_10668+Cj
                xchg    ax, si
                mul     bx
                add     dx, cx
                pop     si
                retf

	LXMUL@			ENDP
	
	;过程名:	LXURSH@
	;描述:		TCC链接需要. 传递无符号长整形.
	LXURSH@		Proc FAR
                cmp     cl, 10h
                jnb     short loc_1066E
                mov     bx, dx
                shr     ax, cl
                shr     dx, cl
                neg     cl
                add     cl, 10h
                shl     bx, cl
                or      ax, bx
                retf
; ---------------------------------------------------------------------------

loc_1066E:                              ; CODE XREF: sub_10659+3j
                sub     cl, 10h
                mov     ax, dx
                xor     dx, dx
                shr     ax, cl
                retf

	LXURSH@		ENDP

	;过程名:	_Exit
	;描述:		ILib函数. 调用该函数会退出程序返回系统.
	;函数原型:	void Exit(void);
	PUBLIC _Exit
	_Exit			PROC NEAR
		MOV			AH, 0
		INT			21H
	_Exit			ENDP
	
	;过程名:	_GetChar
	;描述:		ILib函数. 调用该函数从键盘上获取一个字符.
	;函数原型:	char GetChar(void); 
	PUBLIC _GetChar
	_GetChar		PROC NEAR
		PUSH		BX
		PUSHF
		MOV			BH, AH
		MOV			AH, 7
		INT			21H
		MOV			AH, BH
		POPF
		POP			BX
		RET
	_GetChar		ENDP
	
	;过程名:	_CheckKey
	;描述:		ILib函数. 调用该函数确认按键队列里是否包含
	;			按键.
	;函数原型:	uint CheckKey(char * Has);
	PUBLIC _CheckKey
	_CheckKey		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		BX
		PUSHF
		MOV			AH, 1
		INT			16H
		MOV			BX, [BP + 4]
		MOV			BYTE PTR [BX], 1
		JNZ			_CheckKey_NoKey
		MOV			BYTE PTR [BX], 0
_CheckKey_NoKey:
		POPF
		POP			BX
		MOV			SP, BP
		POP			BP
		RET
	_CheckKey		ENDP
	
	;过程名:	_GetCharNP
	;描述:		ILib函数. 获取一个字符但不回显.
	;函数原型:	uint GetCharNP(void);
	PUBLIC _GetCharNP
	_GetCharNP		PROC NEAR
		PUSHF
		MOV			AH, 0
		INT			16H
		POPF
		RET
	_GetCharNP		ENDP
	
	;过程名:	_PrintChar
	;描述:		ILib函数. 显示一个字符.
	;函数原型:	void PrintChar(char Char);
	PUBLIC _PrintChar
	_PrintChar		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSHF
		MOV			AL, BYTE PTR [BP + 4]
		MOV			AH, 1
		INT			21H
		POPF
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_PrintChar		ENDP
	
	;过程名:	_PrintCharP
	;描述:		ILib函数. 打印一个字符附带属性.
	;函数原型:	void PrintCharP(char Char, uchar Property);
	PUBLIC _PrintCharP
	_PrintCharP		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSH		BX
		PUSHF
		MOV			AL, BYTE PTR [BP + 4]
		MOV			BL, BYTE PTR [Bp + 6]
		MOV			AH, 16
		INT			21H
		POPF
		POP			BX
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_PrintCharP		ENDP
	
	;过程名:	_CR
	;描述:		ILib函数. 回车.
	;函数原型:	void CR(void);
	PUBLIC _CR
	_CR				PROC NEAR
		PUSH		AX
		PUSHF
		MOV			AH, 2
		INT			21H
		POPF
		POP			AX
		RET
	_CR				ENDP
	
	;过程名:	_LF
	;描述:		ILib函数. 换行.
	;函数原型:	void LF(void);
	PUBLIC _LF
	_LF				PROC NEAR
		PUSH		AX
		PUSHF
		MOV			AH, 3
		INT			21H
		POPF
		POP			AX
		RET
	_LF				ENDP
	
	;过程名:	_ClearScreen
	;描述:		ILib函数. 清屏.
	;函数原型:	void ClearScreen();
	PUBLIC _ClearScreen
	_ClearScreen	PROC NEAR
		PUSH		AX
		PUSHF
		MOV			AH, 5
		INT			21H
		POPF
		POP			AX
		RET
	_ClearScreen	ENDP
	
	;过程名:	__NewFile
	;描述:		ILib函数. 新建一个文件.
	;函数原型:	int _NewFile(char * FileName);
	PUBLIC __NewFile
	__NewFile		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		DS
		PUSH		SI
		PUSHF
		PUSH		CS
		POP			DS
		MOV			SI, [BP + 4]
		MOV			AH, 10
		INT			21H
		POPF
		POP			SI
		POP			DS
		XOR			AH, AH
		MOV			SP, BP
		POP			BP
		RET
	__NewFile		ENDP
	
	;过程名:	__DelFile
	;描述:		ILib函数. 删除一个文件.
	;函数原型:	int _DelFile(char * FileName);
	PUBLIC __DelFile
	__DelFile		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		DS
		PUSH		SI
		PUSHF
		PUSH		CS
		POP			DS
		MOV			SI, [BP + 4]
		MOV			AH, 11
		INT			21H
		POPF
		POP			SI
		POP			DS
		XOR			AH, AH
		MOV			SP, BP
		POP			BP
		RET
	__DelFile		ENDP
	
	;过程名:	__WriteFile
	;描述:		ILib函数. 写文件.
	;函数原型:	int _WriteFile(char * FileName, uchar * Data, uint Len);
	PUBLIC __WriteFile
	__WriteFile		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		DI
		PUSH		SI
		PUSH		CX
		PUSHF
		
		;FileName
		MOV			SI, [BP + 4]
		
		;Data
		MOV			DI, [BP + 6]
		
		;Len
		MOV			CX, [BP + 8]
		
		MOV			AH, 12
		INT			21H
		
		POPF
		POP			CX
		POP			SI
		POP			DI
		XOR			AH, AH
		MOV			SP, BP
		POP			BP
		RET
	__WriteFile		ENDP
	
	;过程名:	__ReadFile
	;描述:		ILib函数. 读文件.
	;函数原型:	int _ReadFile(char * FileName, uchar * Data);
	PUBLIC __ReadFile
	__ReadFile		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		DI
		PUSH		SI
		PUSH		CX
		PUSHF
		
		;FileName
		MOV			SI, [BP + 4]
		
		;Data
		MOV			DI, [BP + 6]
		
		MOV			AH, 13
		INT			21H
		
		CMP			AL, 1
		JE			__ReadFile_Label1
		MOV			AX, CX
		JMP			__ReadFile_Label2
__ReadFile_Label1:
		MOV			AX, 0FFFFH
__ReadFile_Label2:
		
		POPF
		POP			CX
		POP			SI
		POP			DI
		MOV			SP, BP
		POP			BP
		RET
	__ReadFile		ENDP
	
	;过程名:	__FileExists
	;描述:		ILib函数. 确认文件是否存在.
	;函数原型:	int _FileExists(char * FileName);
	PUBLIC __FileExists
	__FileExists	PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		DS
		PUSH		SI
		PUSHF
		PUSH		CS
		POP			DS
		MOV			SI, [BP + 4]
		MOV			AH, 14
		INT			21H
		POPF
		POP			SI
		POP			DS
		XOR			AH, AH
		MOV			SP, BP
		POP			BP
		RET
	__FileExists	ENDP
	
	;过程名:	_MoveCaret
	;描述:		ILib函数. 移动光标.
	;函数原型:	void MoveCaret(uchar X, uchar Y);
	PUBLIC _MoveCaret
	_MoveCaret		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSH		BX
		PUSH		DX
		PUSHF
		MOV			DL, [BP + 4]
		MOV			DH, [BP + 6]
		XOR			BH, BH
		MOV			AH, 2H
		INT			10H
		POPF
		POP			DX
		POP			BX
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_MoveCaret		ENDP
	
	;过程名:	_GetCaretPos
	;描述:		ILib函数. 获取光标位置.
	;函数原型:	void GetCaretPos(uchar * X, uchar * Y);
	PUBLIC _GetCaretPos
	_GetCaretPos	PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSH		BX
		PUSH		CX
		PUSH		DX
		PUSHF
		XOR			BH, BH
		MOV			AH, 3H
		INT			10H
		MOV			BX, [BP + 4]
		MOV			[BX], DL
		MOV			BYTE PTR [BX + 1], 0
		MOV			BX, [BP + 6]
		MOV			[BX], DH
		MOV			BYTE PTR [BX + 1], 0
		POPF
		POP			DX
		POP			CX
		POP			BX
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_GetCaretPos	ENDP
	
	;过程名:	_Format
	;描述:		ILib函数. 删除所有文件.
	;函数原型:	void Format(void);
	PUBLIC _Format
	_Format			PROC NEAR
		PUSH		AX
		PUSHF
		MOV			AH, 15
		INT			21H
		POPF
		POP			AX
		RET
	_Format			ENDP
	
	;过程名:	_ClearBg
	;描述:		ILib函数. 修改背景色.
	;函数原型:	void ClearBg(Color Clr);
	PUBLIC _ClearBg
	_ClearBg		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSH		BX
		PUSHF
		XOR			BH, BH
		MOV			BL, [BP + 4]
		MOV			AH, 0BH
		INT			10H
		POPF
		POP			BX
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_ClearBg		ENDP
	
	;过程名:	_DrawPixel
	;描述:		ILib函数. 画像素.
	;函数原型:	void DrawPixel(uint X, uint Y, uchar Color);
	PUBLIC _DrawPixel
	_DrawPixel		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSH		BX
		PUSH		CX
		PUSH		DX
		PUSHF
		XOR			BH, BH
		MOV			CX, [BP + 4]
		MOV			DX, [BP + 6]
		MOV			AL, [BP + 8]
		MOV			AH, 0CH
		INT			10H
		POPF
		POP			DX
		POP			CX
		POP			BX
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_DrawPixel		ENDP
	
	;过程名:	_GetPixel
	;描述:		ILib函数. 获取像素.
	;函数原型:	uchar GetPixel(uint X, uint Y);
	PUBLIC _GetPixel
	_GetPixel		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		BX
		PUSH		CX
		PUSH		DX
		PUSHF
		MOV			CX, [BP + 4]
		MOV			DX, [BP + 6]
		MOV			AH, 0DH
		INT			10H
		POPF
		POP			DX
		POP			CX
		POP			BX
		XOR			AH, AH
		MOV			SP, BP
		POP			BP
		RET
	_GetPixel		ENDP
	
	;过程名:	__Rename
	;描述:		ILib函数. 重命名文件.
	;函数原型:	int _Rename(char * SrcFileName, char * DstFileName);
	PUBLIC __Rename
	__Rename			PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		DI
		MOV			DI, [BP + 6]
		MOV			SI, [BP + 4]
		MOV			AH, 19
		INT			21H
		XOR			AH, AH
		POP			DI
		POP			SI
		MOV			SP, BP
		POP			BP
		RET
	__Rename			ENDP
	
	;过程名:	_Console
	;描述:		ILib函数. 运行命令.
	;函数原型:	void Console(char * Command);
	PUBLIC _Console
	_Console		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		AX
		MOV			SI, [BP + 4]
		MOV			AH, 20
		INT			21H
		POP			AX
		POP			SI
		MOV			SP, BP
		POP			BP
		RET
	_Console		ENDP
	
	;过程名:	_GetFileLength
	;描述:		ILib函数. 获取文件长度
	;函数原型:	int _GetFileLength(char * FileName);
	PUBLIC __GetFileLength
	__GetFileLength	PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		CX
		MOV			SI, [BP + 4]
		MOV			AH, 23
		INT			21H
		MOV			AX, CX
		JNC			__GetFileLength_NoErr
		MOV			AX, 0FFFFH	;返回-1
__GetFileLength_NoErr:
		
		POP			CX
		POP			SI
		MOV			SP, BP
		POP			BP
		RET
	__GetFileLength	ENDP
	
	;过程名:	__GetFileCD
	;描述:		ILib函数. 获取文件修改时间
	;函数原型:	int _GetFileCD(char * FileName, uchar * ChangedDate)
	PUBLIC __GetFileCD
	__GetFileCD		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		DI
		
		MOV			SI, [BP + 4]
		MOV			DI, [BP + 6]
		MOV			AH, 24
		INT			21H
		
		POP			DI
		POP			SI
		MOV			SP, BP
		POP			BP
		RET		
	__GetFileCD		ENDP
	
	;过程名:	__GetFileCrD
	;描述:		ILib函数. 获取文件创建时间
	;函数原型:	int _GetFileCrD(char * FileName, uchar * CreatedDate);
	PUBLIC __GetFileCrD
	__GetFileCrD	PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		DI
		
		MOV			SI, [BP + 4]
		MOV			DI, [BP + 6]
		MOV			AH, 25
		INT			21H
		
		POP			DI
		POP			SI
		MOV			SP, BP
		POP			BP
		RET		
	__GetFileCrD	ENDP
	
	;过程名:	__ISVer
	;描述:		ILib函数. 获取ISystem版本
	;函数原型:	void _ISVer(uchar * Ver);
	PUBLIC __ISVer
	__ISVer			PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		AX
		MOV			SI, [BP + 4]
		MOV			AH, 26
		INT			21H
		MOV			[SI], AX
		POP			AX
		POP			SI
		MOV			SP, BP
		POP			BP
		RET	
	__ISVer			ENDP
	
	Registers		DB 24 DUP(0)
	R_Flags			EQU 0
	R_AX			EQU 2
	R_BX			EQU 4
	R_CX			EQU 6
	R_DX			EQU 8
	R_SI			EQU 10
	R_DI			EQU 12
	R_BP			EQU 14
	R_SP			EQU 16
	R_DS			EQU 18
	R_SS			EQU 20
	R_ES			EQU 22
	
	;过程名:	__GetRegisters
	;描述:		获取部分寄存器的值
	;函数原型:	void _GetRegisters(uchar * Regs);
	PUBLIC __GetRegisters
	__GetRegisters	PROC NEAR
		MOV			WORD PTR [Registers + R_AX], AX
		MOV			WORD PTR [Registers + R_BX], BX
		MOV			WORD PTR [Registers + R_CX], CX
		MOV			WORD PTR [Registers + R_DX], DX
		MOV			WORD PTR [Registers + R_SI], SI
		MOV			WORD PTR [Registers + R_DI], DI
		MOV			WORD PTR [Registers + R_BP], BP
		MOV			WORD PTR [Registers + R_SP], SP
		PUSHF
		;获取的SP的值是调用该函数之前的值
		SUB			WORD PTR [Registers + R_SP], 6
		POPF
		MOV			WORD PTR [Registers + R_DS], DS
		MOV			WORD PTR [Registers + R_SS], SS
		MOV			WORD PTR [Registers + R_ES], ES
		PUSHF
		POP			AX
		MOV			WORD PTR [Registers + R_Flags], AX
		MOV			AX, WORD PTR [Registers + R_AX]
		
		PUSH		BP
		MOV			BP, SP
		PUSH		SI
		PUSH		DI
		PUSH		AX
		PUSH		CX
		
		LEA			SI, Registers
		MOV			DI, [BP + 4]
		MOV			CX, 24
__GetRegisters_Loop:
		LODSB
		STOSB
		LOOP		__GetRegisters_Loop
		
		POP			CX
		POP			AX
		POP			DI
		POP			SI
		MOV			SP, BP
		POP			BP
		RET
	__GetRegisters	ENDP
	
	;过程名:	_GetAX
	;描述:		获取AX
	;原型:		uint GetAX(void);
	PUBLIC _GetAX
	_GetAX			PROC NEAR
		RET
	_GetAX			ENDP
	
	;过程名:	_SetAX
	;描述:		设置AX
	;原型:		void SetAX(uint Value);
	PUBLIC _SetAX
	_SetAX			PROC NEAR
		PUSH		BP
		MOV			BP, SP
		MOV			AX, [BP + 4]
		MOV			SP, BP
		POP			BP
		RET
	_SetAX			ENDP
	
	;过程名:	_GetLine
	;描述:		获取一行字符串
	;原型:		void GetLine(char * Line)
	PUBLIC	_GetLine
	_GetLine		PROC NEAR
		PUSH		BP
		MOV			BP, SP
		PUSH		AX
		PUSH		DI
		MOV			DI, [BP + 4]
		MOV			AH, 8
		INT			21H
		POP			DI
		POP			AX
		MOV			SP, BP
		POP			BP
		RET
	_GetLine		ENDP
	
	;过程名:	__CopyFile
	;描述:		复制文件
	;原型:		int _CopyFile(char * Src, char * Dst, uchar DstDiskNumber);
	PUBLIC	__CopyFile
	__CopyFile	PROC NEAR
		PUSH	BP
		MOV		BP, SP
		PUSH	SI
		PUSH	DI
		MOV		SI, [BP + 4]
		MOV		DI, [BP + 6]
		MOV		AL, [BP + 8]
		MOV		AH, 36
		INT		21H
		XOR		AH, AH
		POP		DI
		POP		SI
		MOV		SP, BP
		POP		BP
		RET
	__CopyFile	ENDP
	
	;过程名:	__CutFile
	;描述:		剪切文件
	;原型:		int _CutFile(char * Src, char * Dst, uchar DstDiskNumber);
	PUBLIC	__CutFile
	__CutFile	PROC
		PUSH	BP
		MOV		BP, SP
		PUSH	SI
		PUSH	DI
		MOV		SI, [BP + 4]
		MOV		DI, [BP + 6]
		MOV		AL, [BP + 8]
		MOV		AH, 37
		INT		21H
		XOR		AH, AH
		POP		DI
		POP		SI
		MOV		SP, BP
		POP		BP
		RET
	__CutFile	ENDP
	
	;过程名:	_ChangeDisk
	;描述:		修改当前使用的磁盘
	;原型:		int ChangeDisk(uchar DiskNumber);
	PUBLIC	_ChangeDisk
	_ChangeDisk	PROC NEAR
		PUSH	BP
		MOV		BP, SP
		MOV		AL, [BP + 4]
		MOV		AH, 35
		INT		21H
		XOR		AH, AH
		MOV		SP, BP
		POP		BP
		RET
	_ChangeDisk	ENDP
	
	;过程名:	_GetCurrentDisk
	;描述:		获取当前磁盘号
	;函数原型:	uchar GetCurrentDisk(void);
	PUBLIC	_GetCurrentDisk
	_GetCurrentDisk	PROC NEAR
		PUSH	BX
		MOV		BH, AH
		MOV		AH, 34
		INT		21H
		MOV		AH, BH
		POP		BX
		RET
	_GetCurrentDisk	ENDP
	
END
