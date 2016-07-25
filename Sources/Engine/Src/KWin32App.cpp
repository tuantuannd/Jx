//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KWin32App.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	KWin32App Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KDDraw.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KWin32Wnd.h"
#include "KWin32App.h"
#include "KIme.h"
//---------------------------------------------------------------------------
static KWin32App* m_pWin32App = NULL;
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ENGINE_API FILE *sout = NULL;
ENGINE_API FILE *serr = NULL;

#ifndef WM_MOUSEHOVER
	#define WM_MOUSEHOVER 0x02A1
#endif

#define	MOUSE_EVENT_NONE			0
#define	MOUSE_EVENT_HAPPEND			1
#define	MOUSE_HOVER_MSG_SENT		2
#define	MOUSE_HOVER_START_TIME_MIN	3
//m_uMouseHoverStartTime 含义
//MOUSE_EVENT_NONE		--> 未发生鼠标活动事件
//MOUSE_EVENT_HAPPEND	--> 有鼠标活动事件
//MOUSE_HOVER_MSG_SENT	--> 无鼠标活动事件的持续时间超过了设定的时间限制，已发送了WM_NCMOUSEHOVER消息
//其他值				--> 无鼠标活动事件的持续时间未超过设定的时间限制，此值表示无鼠标活动的开始时间

//---------------------------------------------------------------------------
// 函数:	WndProc
// 功能:	窗口回调函数
// 参数:	hWnd		窗口句柄
//			uMsg		消息类型
//			wParam		参数1(32bit)
//			lParam		参数2(32bit)
// 返回:	LRESULT
//---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(
						 HWND	hWnd,		// handle of window
						 UINT	uMsg,		// type of message
						 WPARAM	wParam, 	// first message parameter 32bit
						 LPARAM	lParam) 	// second message parameter 32bit
{
	return m_pWin32App->MsgProc(hWnd, uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------
// 函数:	KWin32App
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KWin32App::KWin32App()
{
	m_bActive = TRUE;
	m_bMultiGame = TRUE;
	m_bShowMouse = TRUE;
	m_uMouseHoverTimeSetting = 0;
	m_uMouseHoverStartTime = MOUSE_EVENT_NONE;
	m_uLastMouseStatus = 0;
	m_nLastMousePos = 0;
	m_uLastMouseStatus = 0;
	m_pWin32App = this;
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	初始化
// 参数:	hInstance	实例句柄
// 返回:	void
//---------------------------------------------------------------------------
BOOL KWin32App::Init(HINSTANCE hInstance,char *AppName)
{
	g_StrCpy(m_szClass, AppName);
	strcat(m_szClass," Class");
	g_StrCpy(m_szTitle, AppName);
	strcat(m_szTitle," Title");
//#ifdef _DEBUG
	{
//		sout = freopen( "c:\\stdout1.txt", "a", stdout );
//		serr = freopen( "c:\\stderr1.txt", "a", stderr );
//		if (sout == NULL || serr == NULL)
//			return 0;
	}
//#endif
	if (!InitClass(hInstance))
		return FALSE;

	if (!InitWindow(hInstance))
		return FALSE;

	return GameInit();
}
//---------------------------------------------------------------------------
// 函数:	InitClass
// 功能:	初始化窗口程序类
// 参数:	hInstance	实例句柄
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KWin32App::InitClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	
	wc.style			= CS_DBLCLKS;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTATOM(SWORD_ICON));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName 	= NULL;
	wc.lpszClassName	= m_szClass;

	return RegisterClass(&wc);
}
//---------------------------------------------------------------------------
// 函数:	InitWindow
// 功能:	初始化窗口
// 参数:	hInstance	实例句柄
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KWin32App::InitWindow(HINSTANCE hInstance)
{
	HWND hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		m_szClass,
		m_szTitle,
		//WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX,
        WS_VISIBLE | WS_CAPTION | WS_EX_TOOLWINDOW | WS_SYSMENU | WS_MINIMIZEBOX, 
		0,
		0,
		WND_INIT_WIDTH + 6,
		WND_INIT_HEIGHT + 25,
		NULL,
		NULL,
		hInstance,		NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	g_SetMainHWnd(hWnd);
	g_SetDrawHWnd(hWnd);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Run
// 功能:	主消息循环
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KWin32App::Run()
{
	MSG	Msg;

	while (TRUE)
	{
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			//if (!GetMessage(&Msg, NULL, 0, 0))
			//	break;
            if (Msg.message == WM_QUIT)
                break;

			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else if (m_bActive || m_bMultiGame)
		{
			GenerateMsgHoverMsg();
			if (!GameLoop())
			{
				PostMessage(g_GetMainHWnd(), WM_CLOSE, 0, 0);
			}
		}
		else
		{
			WaitMessage();
		}
	}
	GameExit();
}
//---------------------------------------------------------------------------
// 函数:	GameInit
// 功能:	游戏初始化
// 参数:	void
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KWin32App::GameInit()
{
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GameLoop
// 功能:	游戏主循环
// 参数:	void
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KWin32App::GameLoop()
{
	WaitMessage();
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GameExit
// 功能:	游戏退出
// 参数:	void
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KWin32App::GameExit()
{
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	MsgProc
// 功能:	窗口回调函数
// 参数:	hWnd		窗口句柄
//			uMsg		消息类型
//			wParam		参数1(32bit)
//			lParam		参数2(32bit)
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
LRESULT KWin32App::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		if (HandleInput(uMsg, wParam, lParam))
			return 0;
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SETCURSOR:
		if (m_bShowMouse == FALSE && m_bActive &&
			LOWORD(lParam) == HTCLIENT)
		{
			SetCursor(NULL);
			return TRUE;
		}
        else
        {
            // 如果外部使用我们自己的鼠标，如果没有返回TRUE，
            // 那么就会导致鼠标被设置回来
            return TRUE;
        }
		break;
//	case WM_SOCKET_READ:
//        if(g_pNetClient)   
//          g_pNetClient->RecvMsg(); 
//		break;

	case WM_IME_CHAR:
		 return HandleInput(uMsg, wParam, lParam);
	case WM_INPUTLANGCHANGEREQUEST:
	case WM_IME_NOTIFY:
		if (g_pIme && 
			g_pIme->WndMsg(hWnd,uMsg,wParam,lParam))
			return 0;
		break;

	case WM_ACTIVATEAPP:
		m_bActive = (BOOL)wParam;
		if (m_bActive && g_pDirectDraw)
			g_pDirectDraw->RestoreSurface();

   	    HandleInput(uMsg, wParam, lParam);
		break;

    case WM_SYSCOMMAND:
        if (wParam == SC_KEYMENU)
        {
            return 0;
        }
	case WM_COPYDATA:
		HandleInput(uMsg, wParam, lParam);
		break;
	default:
		if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
		{
			HandleInput(uMsg, wParam, lParam);
		}
		else if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
		{
			m_uLastMouseStatus = wParam;
			m_nLastMousePos = lParam;
			m_uMouseHoverStartTime = MOUSE_EVENT_HAPPEND;
			HandleInput(uMsg, wParam, lParam);
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//---------------------------------------------------------------------------
// 函数:	ShowMouse
// 功能:	设置鼠标可见
// 参数:	bShow		BOOL
// 返回:	void
//---------------------------------------------------------------------------
void KWin32App::ShowMouse(BOOL bShow)
{
	m_bShowMouse = bShow;
}
//---------------------------------------------------------------------------
// 函数:	SetMultiGame
// 功能:	设置是否是后台继续运行的多人游戏
// 参数:	bMulti		BOOL
// 返回:	void
//---------------------------------------------------------------------------
void KWin32App::SetMultiGame(BOOL bMulti)
{
	m_bMultiGame = bMulti;
}

void KWin32App::SetMouseHoverTime(unsigned int nHoverTime)
{
	m_uMouseHoverTimeSetting = nHoverTime;
}

void KWin32App::GenerateMsgHoverMsg()
{
	if (m_uMouseHoverStartTime == MOUSE_HOVER_MSG_SENT)
	{
		static char cCounter = 0;
		if ((++cCounter) == 7)
		{
			cCounter = 0;
			HandleInput(WM_MOUSEHOVER, m_uLastMouseStatus, m_nLastMousePos);
		}
	}
	else if (m_uMouseHoverStartTime >= MOUSE_HOVER_START_TIME_MIN)
	{
		unsigned int	nCurrentTime = timeGetTime();
		if ((nCurrentTime - m_uMouseHoverStartTime) >= m_uMouseHoverTimeSetting)
		{
			HandleInput(WM_MOUSEHOVER, m_uLastMouseStatus, m_nLastMousePos);
			m_uMouseHoverStartTime = MOUSE_HOVER_MSG_SENT;
		}
	}
	else if (m_uMouseHoverStartTime == MOUSE_EVENT_HAPPEND)
	{
		m_uMouseHoverStartTime = MOUSE_EVENT_NONE;
	}
	else if (m_uMouseHoverTimeSetting)
	{
		//此时(m_uMouseHoverStartTime == MOUSE_EVENT_NONE)
		m_uMouseHoverStartTime = timeGetTime();
		if (m_uMouseHoverStartTime < MOUSE_HOVER_START_TIME_MIN)
			m_uMouseHoverStartTime = MOUSE_HOVER_START_TIME_MIN;
	}
}
