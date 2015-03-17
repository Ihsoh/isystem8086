/*
	文件名:	gshell.c
	作者:	Ihsoh
	日期:	2013-7-23
*/

#include "gshell.h"

/* 鼠标状态 */
static uint MouseX = 0, MouseY = 0, MouseButton;

/*
	函数名:	KillShell
	描述:	杀死Shell
	参数:	无
	返回值:	无
*/
static void KillShell(void)
{
	char FileName[20];
	uchar TaskID;
	
	asm	PUSH	AX;
	for(TaskID = 0; TaskID < 6; TaskID++)
	{
		asm	PUSH	DI;
		asm	MOV		AL, TaskID;
		asm	LEA		DI, FileName;
		asm	MOV		AH, 4;
		asm	INT		23H;
		asm	POP		DI;
		if(StringCmp(FileName, "shell"))
		{
			asm	MOV		AL, TaskID;
			asm	MOV		AH, 2;
			asm	INT		23H;
			break;
		}
	}
}

/*
	函数名:	Init
	描述:	初始化
	参数:	无
	返回值:	无
*/
static void Init(void)
{
	uint ui, ui1, ui2;

	KillShell();
	asm	PUSH	AX;
	asm	MOV		AL, 1;
	asm	MOV		AH, 42;
	asm	INT		21H;
	asm	POP		AX;
	
	InitBuffer();
	GS_InitGraph(0x101);
	for(ui = 0; ui < 0x100; ui++)
		GS_SetPalette((uchar)ui, (uchar)(ui / 4), (uchar)(ui / 4), (uchar)(ui / 4));
	
	InitIFont("default.ifon");
	
	InitForm();
}

static char ShellName[] = "shell\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static char Arg[] = "noautoexec";

/*
	函数名:	ExitGShell
	描述:	退出
	参数:	无
	返回值:	无
*/
static void ExitGShell(void)
{
	InitVideo(VideoMode_System);
	DestroyBuffer();
	Console("cls");
	asm	MOV		SI, OFFSET ShellName;
	asm	MOV		DI, OFFSET Arg;
	asm	MOV		AH, 1;
	asm	INT		23H;
}

/*
	函数名:	InRectangle
	描述:	点是否在一个矩形内
	参数:	RectX=矩形X坐标
			RectY=矩形Y坐标
			RectWidth=矩形宽度
			RectHeight=矩形高度
			X=点的X坐标
			Y=点的Y坐标
	返回值:	0=否, 1=是
*/
static int InRectangle(uint RectX, uint RectY, uint RectWidth, uint RectHeight, uint X, uint Y)
{
	if(	X < RectX				||
		X > RectX + RectWidth	||
		Y < RectY				||
		Y > RectY + RectHeight) return 0;
	else return 1;
}

/*
	函数名:	DrawExitButton
	描述:	画GShell退出键
	参数:	无
	返回值:	无
*/
static void DrawExitButton(void)
{
	uint X, Y;

	for(X = 0; X < EXITIMAGE_WDITH; X++)
		for(Y = 0; Y < EXITIMAGE_HEIGHT; Y++)
		{
			uchar Pixel = GetPixel_Grayness(ExitImage, 
											X, 
											Y, 
											EXITIMAGE_WDITH, 
											EXITIMAGE_HEIGHT);
			if(Pixel == 0)
				SetPixelToBuffer(	X, 
									MaxHeight - EXITIMAGE_HEIGHT + Y, 
									0);
		}
}

/*
	函数名:	DrawForms
	描述:	画所有窗体
	参数:	无
	返回值:	无
*/
static void DrawForms(void)
{
	int i, i1, i2;
	uchar CharImage[32];

	for(i = MAX_FORM_COUNT - 1; i >= 0; i--)
		if(Forms[i].Used)
		{
			uint TitleX = Forms[i].X, TitleY = Forms[i].Y + 1;
			int Point;
			uint X, Y;
			
			/* 清空窗体背景 */
			for(X = Forms[i].X; X < Forms[i].X + Forms[i].Width; X++)
				for(Y = Forms[i].Y; Y < Forms[i].Y + Forms[i].Height; Y++)
					SetPixelToBuffer(X, Y, 0xFF);
			
			for(i1 = 0; i1 < GetStringLen(Forms[i].Title) && i1 < Forms[i].Width / 16 - 1; i1++, TitleX += 16)
			{
				GetCharImage(Forms[i].Title[i1], CharImage);
				DrawGraynessImageToBuffer(TitleX, TitleY, CharImage, 16, 16);
			}	
			
			Point = !Forms[i].Top;
			DrawHLineToBuffer(	Forms[i].X,
								Forms[i].Y,
								Forms[i].Width,
								0,
								Point);
			DrawHLineToBuffer(	Forms[i].X,
								Forms[i].Y + TITLEBAR_HEIGHT,
								Forms[i].Width,
								0,
								Point);
			DrawHLineToBuffer(	Forms[i].X,
								Forms[i].Y + Forms[i].Height - 1,
								Forms[i].Width,
								0,
								Point);
			DrawVLineToBuffer(	Forms[i].X,
								Forms[i].Y,
								Forms[i].Height,	
								0,
								Point);
			DrawVLineToBuffer(	Forms[i].X + Forms[i].Width - 1,
								Forms[i].Y,
								Forms[i].Height,
								0,
								Point);
			DrawGraynessImageToBuffer(	Forms[i].X + Forms[i].Width - 16, 
										Forms[i].Y + 1,
										CloseImage,
										CLOSEIMAGE_WIDTH,
										CLOSEIMAGE_HEIGHT);
								
			/* 画窗体内容 */
			for(i1 = 0; i1 < Forms[i].TextCount; i1++)
			{
				uint TextX, TextY;
				
				TextX = Forms[i].Texts[i1].X + Forms[i].X + 1;
				TextY = Forms[i].Texts[i1].Y + Forms[i].Y + 18;
				
				for(i2 = 0; i2 < GetStringLen(Forms[i].Texts[i1].Text); i2++, TextX += 16)
				{
					GetCharImage(Forms[i].Texts[i1].Text[i2], CharImage);
					DrawGraynessImageToBuffer(	TextX,
												TextY,
												CharImage,
												16,
												16);
					
				}
			}
			Forms[i].TextCount = 0;
		}
}

/*
	函数名:	DrawMouse
	描述:	画鼠标
	参数:	无
	返回值:	无
*/
static void DrawMouse(void)
{
	uint X, Y;

	for(X = 0; X < MOUSEIMAGE_WIDTH; X++)
		for(Y = 0; Y < MOUSEIMAGE_HEIGHT; Y++)
		{
			uchar Pixel = GetPixel_Grayness(MouseImage, 
											X, 
											Y, 
											MOUSEIMAGE_WIDTH, 
											MOUSEIMAGE_HEIGHT);
			if(Pixel == 0)
				SetPixelToBuffer(MouseX + X, MouseY + Y, 0);
		}
}

/*
	函数名:	GetMouseState
	描述:	获取鼠标状态
	参数:	无
	返回值:	MouseX=鼠标X坐标
			MouseY=鼠标Y坐标
*/
static void GetMouseState(void)
{
	asm	CLI;
	asm	PUSH	AX;
	asm	PUSH	BX;
	asm	PUSH	CX;
	asm	PUSH	DX;
	asm	MOV		AH, 40;
	asm	INT		21H;
	MouseX = _BX;
	MouseY = _CX;
	MouseButton = _DL;
	asm	POP		DX;
	asm	POP		CX;
	asm	POP		BX;
	asm	POP		AX;
	if(MouseX >= MaxWidth && MouseX < 65535 - 255 + 1)
	{
		uint NewX = MaxWidth - 1;
	
		asm	MOV		BX, NewX;
		asm	MOV		CX, 0FFFFH;
		asm	MOV		AH, 41;
		asm	INT		21H;
	}
	if(MouseX >= 65535 - 255 + 1)
	{
		asm	MOV		BX, 0;
		asm	MOV		CX, 0FFFFH;
		asm	MOV		AH, 41;
		asm	INT		21H;
	}
	if(MouseY >= MaxHeight && MouseY < 65535 - 255 + 1)
	{
		uint NewY = MaxHeight - 1;
	
		asm	MOV		BX, 0FFFFH;
		asm	MOV		CX, NewY;
		asm	MOV		AH, 41;
		asm	INT		21H;
	}
	if(MouseY >= 65535 - 255 + 1)
	{
		asm	MOV		BX, 0FFFFH;
		asm	MOV		CX, 0;
		asm	MOV		AH, 41;
		asm	INT		21H;
	}
	asm	STI;
}

/*
	函数名:	Welcome
	描述:	欢迎窗体
	参数:	New=是否创建新窗体
	返回值:	无
*/
void Welcome(int New)
{
	static int FormID;
	static int Closed = 0;
	
	if(!Closed)
	{
		if(New) FormID = AllocForm(MaxWidth / 2 - 320 / 2, MaxHeight / 2 - 200 / 2, 320, 200, "Welcome");
		else
			if(HasForm(FormID) == -1) Closed = 1;
			else 
			{
				DrawStringToForm(FormID, 0, 0, "Welcome to");
				DrawStringToForm(FormID, 160, 0, " ISystem!");
			}
	}
}

/*
	函数名:	Timer
	描述:	时钟Form
	参数:	New = 是否创建新窗体
	返回值:	无
*/
static void Timer(int New)
{
	static uint TimerFID = 0;
	char Buffer[11];
	uchar Hour, Minute, Second;
	uchar YearH, YearL, Month, Day;
	
	if(New == 1 && HasForm(TimerFID) == -1) 
		TimerFID = AllocForm(MaxWidth - 170, MaxHeight - 60, 170, 60, "Timer");
	if(HasForm(TimerFID) != -1)
	{
		asm	PUSH	AX;
		asm	PUSH	CX;
		asm	PUSH	DX;
		asm	MOV		AH, 2;
		asm	INT		1AH;
		Hour = _CH;
		Minute = _CL;
		Second = _DH;
		asm	POP		DX;
		asm	POP		CX;
		asm	POP		AX;
		Buffer[0] = (char)(Hour >> 4) + '0';
		Buffer[1] = (char)(Hour & 0x0F) + '0';
		Buffer[2] = ':';
		Buffer[3] = (char)(Minute >> 4) + '0';
		Buffer[4] = (char)(Minute & 0x0F) + '0';
		Buffer[5] = ':';
		Buffer[6] = (char)(Second >> 4) + '0';
		Buffer[7] = (char)(Second & 0x0F) + '0';
		Buffer[8] = 0;
		DrawStringToForm(TimerFID, 0, 0, Buffer);
		asm	PUSH	AX;
		asm	PUSH	CX;
		asm	PUSH	DX;
		asm	MOV		AH, 4;
		asm	INT		1AH;
		YearH = _CH;
		YearL = _CL;
		Month = _DH;
		Day = _DL;
		asm	POP		DX;
		asm	POP		CX;
		asm	POP		AX;
		Buffer[0] = (char)(YearH >> 4) + '0';
		Buffer[1] = (char)(YearH & 0x0F) + '0';
		Buffer[2] = (char)(YearL >> 4) + '0';
		Buffer[3] = (char)(YearL & 0x0F) + '0';
		Buffer[4] = '-';
		Buffer[5] = (char)(Month >> 4) + '0';
		Buffer[6] = (char)(Month & 0x0F) + '0';
		Buffer[7] = '-';
		Buffer[8] = (char)(Day >> 4) + '0';
		Buffer[9] = (char)(Day & 0x0F) + '0';
		Buffer[10] = 0;
		DrawStringToForm(TimerFID, 0, 17, Buffer);
	}
}

/*
	函数名:	Console
	描述:	控制台Form
	参数:	New = 是否创建新窗体
	返回值:	无
*/
/*static void Console(int New)
{
	
}*/

/*
	函数名:	main
	描述:	主过程
	参数:	无
	返回值:	无
*/
void main(void)
{
	int i, i1, i2, i3;
	uint ui, ui1;
	int SelectedForm = -1;
	uint FID;
	uchar PixelBuffer[1024];
	
	Init();
	Welcome(1);
	Timer(1);
	Console(1);
	while(1)
	{
		uint X, Y;
		
		Welcome(0);
		Timer(0);
		Console(0);
		
		ScanKeyboard();
		
		if((MouseButton & 0x2) != 0)
		{
			/* 取消选择窗体 */
			SelectedForm = -1;
		}
		
		if((MouseButton & 0x1) != 0)
		{
			/* 检测退出GShell键是否被单击 */
			if(InRectangle(	0, 
							MaxHeight - EXITIMAGE_HEIGHT, 
							EXITIMAGE_WDITH, 
							EXITIMAGE_HEIGHT, 
							MouseX, 
							MouseY))
			{
				ExitGShell();
				return;
			}
			
			/* 检测最前窗口的关闭键是否被单击 */
			for(i = 0; i < MAX_FORM_COUNT; i++)
				if(Forms[i].Top)
				{
					if(InRectangle(	Forms[i].X + Forms[i].Width - 16,
									Forms[i].Y + 1,
									CLOSEIMAGE_WIDTH,
									CLOSEIMAGE_HEIGHT,
									MouseX,
									MouseY))
					{
						Forms[i].Used = 0;
						Forms[i].ID = 0xFFFF;
					}
					break;
				}
			
			/* 选择窗体 */
			if(SelectedForm == -1)
				for(i = 0; i < MAX_FORM_COUNT; i++)
					if(	InRectangle(Forms[i].X,
									Forms[i].Y,
									Forms[i].Width - 9,
									TITLEBAR_HEIGHT + 2,
									MouseX,
									MouseY)	&&
						Forms[i].Used)
					{
						Form Temp;
						uint FormX, FormY;
						
						CopyForm(&Temp, &Forms[i]);
						CopyForm(&Forms[i], &Forms[0]);
						CopyForm(&Forms[0], &Temp);
						Forms[i].Top = 0;
						Forms[0].Top = 1;
						SelectedForm = Forms[0].ID;
						FormX = Forms[0].X;
						FormY = Forms[0].Y;
						asm	PUSH	AX;
						asm	PUSH	BX;
						asm	PUSH	CX;
						asm	MOV		BX, FormX;
						asm	MOV		CX, FormY;
						asm	MOV		AH, 41;
						asm	INT		21H;
						asm	POP		CX;
						asm	POP		BX;
						asm	POP		AX;
						break;
					}
		}
		
		if(HasKey(KEY_F2))
			Timer(1);
		
		if(HasKey(KEY_F10))
		{
			ExitGShell();
			return;
		}
		
		GetMouseState();
		
		if(SelectedForm != -1)
		{
			uint Index = FormIDToIndex(SelectedForm);
		
			Forms[Index].X = MouseX;
			Forms[Index].Y = MouseY;
		}
		
		DrawExitButton();
		DrawForms();
		DrawMouse();
			
		/* 把视频缓冲区的数据送到显存 */	
		GS_NextFrame();
		for(ui = 0; ui < MaxHeight; ui++)
		{
			GetPixelsFromBuffer(0, ui, MaxWidth, PixelBuffer);
			for(ui1 = 0; ui1 < MaxWidth; ui1++)
				GS_FillPixel(PixelBuffer[ui1]);
		}
		
										
		/* 清空视频缓冲区 */
		for(ui = 0; ui < MaxWidth; ui++)
			for(ui1 = 0; ui1 < MaxHeight; ui1++)
				SetPixelToBuffer(ui, ui1, 0xFF);
			
		asm	INT		24H;
	}
}
