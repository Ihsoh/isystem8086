;*------------------------------------------------------*
;|�ļ���:	fe.inc										|
;|����:		Ihsoh										|
;|����ʱ��:	2013-3-20									|
;|														|
;|����:													|
;|������������Ĵ���źʹ�����Ϣ�ͽ��������������ĺ�.	|
;*------------------------------------------------------*

%IFNDEF	_FE_INC_
%DEFINE	_FE_INC_

%MACRO		FatalMsg 2
	DB 'Code: ', %1, '. ', 'Message: ', %2, '.', 0
%ENDMACRO

;δ֪����
FEC_Unknow							EQU 0000H
FEM_Unknow							FatalMsg '0000', 'Unknow fatal error'

;�ں˶�ջ���
FEC_KernelStackOverflow				EQU 0001H
FEM_KernelStackOverflow				FatalMsg '0001', 'Kernel stack overflow'

;�ں˶�ջ��
FEC_KernelStackBreakdown			EQU 0002H
FEM_KernelStackBreakdown			FatalMsg '0002', 'Kernel stack breakdown'

;δ֪��21H�жϹ��ܺ�
FEC_UnknowINT21HFuncNum				EQU 0003H
FEM_UnknowINT21HFuncNum				FatalMsg '0003', 'Unknow INT 21H function number'

;����
FatalErrorTitle						DB 'ISystem Fatal Error'
FatalErrorTitle_Len					EQU $ - FatalErrorTitle

;������:	FatalErrorP
;����:		���������Ϣ������.
;����:		AX=�����.
;����ֵ:	��
Procedure	FatalErrorP

	PUSH		AX
	MOV			AL, 3H
	MOV			AH, 0
	INT			10H
	POP			AX

	MOV			BP, AX
	
	MOV			BX, 0B800H
	MOV			ES, BX
	
	MOV			BX, CS
	MOV			DS, BX
	
	;����
	XOR			DI, DI
	MOV			AH, 17H
	MOV			AL, ' '
	MOV			CX, 80 * 25
	REP STOSW
	
	;��ӡ����
	MOV			SI, FatalErrorTitle
	XOR			DI, DI
	MOV			CX, FatalErrorTitle_Len
FatalErrorP_PrintTitle:
	LODSB
	STOSW
	LOOP		FatalErrorP_PrintTitle
	
	MOV			AX, BP
	
	CMP			AX, FEC_Unknow
	JNE			NotFE_Unknow
	MOV			SI, FEM_Unknow
	
NotFE_Unknow:
	CMP			AX, FEC_KernelStackOverflow
	JNE			NotFE_KernelStackOverflow
	MOV			SI, FEM_KernelStackOverflow
	
NotFE_KernelStackOverflow:
	CMP			AX, FEC_KernelStackBreakdown
	JNE			NotFE_KernelStackBreakdown
	MOV			SI, FEM_KernelStackBreakdown

NotFE_KernelStackBreakdown:
	CMP			AX, FEC_UnknowINT21HFuncNum
	JNE			NotFE_UnknowINT21HFuncNum
	MOV			SI, FEM_UnknowINT21HFuncNum
	
NotFE_UnknowINT21HFuncNum:
	
	MOV			AH, 17H
	MOV			DI, 1 * (80 * 2)
PrintFEMsg:
	LODSB
	OR			AL, AL
	JZ			$
	STOSW
	JMP			PrintFEMsg
EndProc		FatalErrorP
	
;����:		FatalError
;����:		���������Ϣ������.
;����:		����1=�����.
;����ֵ:	��.
%MACRO 		FatalError 1
	MOV		SP, 0
	MOV		AX, %1
	JMP		FatalErrorP
%ENDMACRO

%ENDIF