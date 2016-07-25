// S3Relay.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <objbase.h>
#include <initguid.h>  // Include only once in your application
#include "../../Multiserver/Heaven/Interface/IServer.h"
#include "../../Multiserver/Rainbow/Interface/IClient.h"

#include "S3Relay.h"
#include "Global.h"

#include "time.h"

#include "LogFile.h"



static CLockSRSW gs_lockRecv;
static CLockSRSW gs_lockSend;
static size_t gs_recvdatasize = 0;
static size_t gs_senddatasize = 0;

static BOOL gs_initialized = FALSE;

static CLogFile gs_LogFile;

/////////////////////////////////////////////////


const char log_directory[] = "relay_log";
const size_t log_threshold = 1024 * 1024;


const UINT elapse_log = 3000;


/////////////////////////////////////////////////


void gTrackRecvData(const void* pData, size_t size)
{
	AUTOLOCKWRITE(gs_lockRecv);
	gs_recvdatasize += size;
}
void gTrackSendData(const void* pData, size_t size)
{
	AUTOLOCKWRITE(gs_lockSend);
	gs_senddatasize += size;
}
size_t gGetRecvDataSize()
{
	AUTOLOCKREAD(gs_lockRecv);
	return gs_recvdatasize;
}
size_t gGetSendDataSize()
{
	AUTOLOCKREAD(gs_lockSend);
	return gs_senddatasize;
}




#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Login(HWND, UINT, WPARAM, LPARAM);


std::_tstring g_rootloginname;
std::_tstring g_rootloginpswd;
BOOL g_interrupt = FALSE;

HWND g_mainwnd = NULL;
HWND g_listwnd = NULL;

//static inline sLogProgram(BOOL bEnter)

static int sLogProgram(BOOL bEnter)
{
	SYSTEMTIME systm;
	::GetLocalTime(&systm);
	TCHAR buff[256];
	int len = 0;

	if (bEnter)
		len = _stprintf(buff, "\r\n*\r\n***** S3Relay Start (@ %04d/%02d/%02d %02d:%02d:%02d.%03d) >>>>>\r\n*\r\n\r\n",
						systm.wYear, systm.wMonth, systm.wDay, systm.wHour, systm.wMinute, systm.wSecond, systm.wMilliseconds);
	else
		len = _stprintf(buff, "\r\n*\r\n***** S3Relay Stop (@ %04d/%02d/%02d %02d:%02d:%02d.%03d) <<<<<\r\n*\r\n\r\n",
						systm.wYear, systm.wMonth, systm.wDay, systm.wHour, systm.wMinute, systm.wSecond, systm.wMilliseconds);

	gs_LogFile.TraceLog(buff, len * sizeof(TCHAR));
return 0;//Fixed By MrChuCong@gmail.com
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	try
	{
		//user input root login name & pswd
		::DialogBox(hInstance, (LPCTSTR)IDD_DIALOG_ROOTLOGIN, NULL, (DLGPROC)Login);
		if (g_interrupt)
			return FALSE;

		std::_tstring strLogDir = gGetLocateDirectory() + log_directory;
		::CreateDirectory(strLogDir.c_str(), NULL);
		if (!gs_LogFile.Initialize(strLogDir, log_threshold))
		{
			::MessageBox(NULL, "Initializing LogFile fail", "error", MB_OK|MB_ICONERROR);
			return FALSE;
		}


		//the program enter
		sLogProgram(TRUE);


 		// TODO: Place code here.
		int retMain = 0;

		MSG msg;
		HACCEL hAccelTable;
		// Initialize global strings
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_S3RELAY, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		if (InitInstance (hInstance, nCmdShow))
		{
			// Perform application initialization:
			{
				hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_S3RELAY);

				// Main message loop:
				while (GetMessage(&msg, NULL, 0, 0)) 
				{
					if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}

			retMain = msg.wParam;

		}


		//the program exit
		sLogProgram(FALSE);


		gs_LogFile.Uninitialize();

		return retMain;
	}
	catch (...)
	{
		::MessageBox(NULL, "X state occur !", "error", MB_OK|MB_ICONERROR);

		return FALSE;
	}
}

class TwoFixBuffer
{
protected:
#define MAX_BUFFER_LEN 1024 * 1024
	struct CSBuffer
	{
		BYTE buff[MAX_BUFFER_LEN + sizeof(DWORD)];
		DWORD nInfoStart;
	};

	CSBuffer m_RW_Buffer[2];
	int m_nRead;
	int m_nWrite;
	CRITICAL_SECTION m_switchsect;
	BOOL HasWriteInfo()
	{
		return m_RW_Buffer[m_nWrite].nInfoStart > 0;
	}
	BOOL HasReadInfo()
	{
		DWORD* pLen = (DWORD*)(m_RW_Buffer[m_nRead].buff + m_RW_Buffer[m_nRead].nInfoStart);
		return  *pLen != 0;
	}

public:
	TwoFixBuffer()
	{
		::InitializeCriticalSection(&m_switchsect);
		for (int n = 0; n < 2; n++)
		{
			memset(m_RW_Buffer[n].buff, 0, MAX_BUFFER_LEN + sizeof(DWORD));
			m_RW_Buffer[n].nInfoStart = 0;
		}
		m_nRead = 0;
		m_nWrite = 1;
	}
	~TwoFixBuffer()
	{
		::LeaveCriticalSection(&m_switchsect);
		::DeleteCriticalSection(&m_switchsect);
	}

	//WriteInfo可以多个线程使用
	//其中nLen指内存长度
	BOOL WriteInfo(BYTE* szInfo, DWORD nLen)
	{
		if (nLen <= 0 || szInfo == NULL)
			return FALSE;
		::EnterCriticalSection(&m_switchsect);
		BOOL bCanAdd = FALSE;
		DWORD nMemLen = nLen + sizeof(DWORD);
		DWORD nInfoStart = m_RW_Buffer[m_nWrite].nInfoStart;
		if (nInfoStart < MAX_BUFFER_LEN - nMemLen)
		{
			memcpy(m_RW_Buffer[m_nWrite].buff + nInfoStart, &nLen, sizeof(DWORD));
			memcpy(m_RW_Buffer[m_nWrite].buff + nInfoStart + sizeof(DWORD), szInfo, nLen);
			m_RW_Buffer[m_nWrite].nInfoStart += nMemLen;
			bCanAdd = TRUE;
			DWORD* pLen = (DWORD*)(m_RW_Buffer[m_nWrite].buff + nInfoStart + nMemLen);
			*pLen = 0;
		}
		::LeaveCriticalSection(&m_switchsect);
		return bCanAdd;
	}
	
	//ReadInfo只能一个线程访问
	BYTE* ReadInfo()
	{
		BYTE* pInfo = NULL;
		if (HasReadInfo())
		{
			pInfo = m_RW_Buffer[m_nRead].buff + m_RW_Buffer[m_nRead].nInfoStart;
			DWORD nLen = *((DWORD*)pInfo);
			m_RW_Buffer[m_nRead].nInfoStart += nLen + sizeof(DWORD);
		}
		return pInfo;
	}
	void SwitchReadWrite()
	{
		if (!HasReadInfo() && HasWriteInfo())
		{
			::EnterCriticalSection(&m_switchsect);
			//first clear read buffer
			m_RW_Buffer[m_nRead].nInfoStart = 0;
			DWORD* pLen = (DWORD*)m_RW_Buffer[m_nRead].buff;
			*pLen = 0;
			//switch read buffer and write buffer
			DWORD nTemp = m_nRead;
			m_nRead = m_nWrite;
			m_nWrite = nTemp;
			//read from head
			m_RW_Buffer[m_nRead].nInfoStart = 0;
			::LeaveCriticalSection(&m_switchsect);
		}
	}
};

TwoFixBuffer g_TraceBuffer;
long g_cnt = 0;

int gTrace(LPCSTR fmt, ...)
{
	if (g_listwnd == NULL)
		return -1;

	TCHAR tempbuff[256];

	g_cnt++;
	sprintf(tempbuff, "%d -- ", g_cnt);
	int n = strlen(tempbuff);

	va_list marker;
	va_start( marker, fmt );

	_vstprintf(tempbuff + n, fmt, marker);

	va_end( marker );

	n = strlen(tempbuff);

	g_TraceBuffer.WriteInfo((BYTE*)tempbuff, n + 1);	//带0写入
	return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_S3RELAY);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_S3RELAY;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

int gTimeStamp(char* sDest, char* pTail)
{
	assert(sDest);
	time_t curtm = ::time(NULL);
	struct tm* ptm = localtime(&curtm);
	return sprintf(sDest, "[%04d/%02d/%02d %02d:%02d:%02d]%s",
			ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, pTail);
}

void gTraceLogFile(LPCTSTR pBuffer, long nLen)
{
	if (!gs_LogFile.BeginBatch())
		return;

	char szTime[128];
	int nTime = gTimeStamp(szTime, " ");
	gs_LogFile.TraceLog(szTime, nTime);
	gs_LogFile.TraceLog(pBuffer, nLen);
	gs_LogFile.TraceLog(_T("\r\n"), 2 * sizeof(TCHAR));

	gs_LogFile.EndBatch();
}

void gClearTrace()
{
	if (g_listwnd)
		::SendMessage(g_listwnd, LB_RESETCONTENT, 0, 0);
}

void gShowTrace()
{
	if (!g_listwnd)
		return;

	static BOOL bLastRead = FALSE;
	char* pBuff = NULL;
	DWORD* pMem = NULL;
	int nPercent = 0;
	int n1 = ::SendMessage(g_listwnd, LB_GETCOUNT, 0, 0);
	int cursel = ::SendMessage(g_listwnd, LB_GETCURSEL, 0, 0);
	BOOL isTrack = cursel < 0 || cursel + 1 >= n1;

	int n = n1;
	while (1)
	{
		pMem = (DWORD*)g_TraceBuffer.ReadInfo();
		if (pMem)
		{
			pBuff = (char*)(pMem + 1);
			
			if (n > 0 && n % 8100 == 0)
			{
				::SendMessage(g_listwnd, LB_DELETESTRING, 0, NULL);
			}

			if (::SendMessage(g_listwnd, LB_ADDSTRING, 0, (LPARAM)pBuff) >= 0)
				n++;

			gTraceLogFile(pBuff, (*pMem) - 1);	//去0写文件
		}
		else
			break;
	}
	if (isTrack && n > n1)
	{
		::SendMessage(g_listwnd, LB_SETCURSEL, (WPARAM)n - 1, 0);
	}
	g_TraceBuffer.SwitchReadWrite();
}

int g_nShowTime = 0;

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   g_mainwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_mainwnd)
   {
      return FALSE;
   }

   RECT rc = {0};
   GetClientRect(g_mainwnd, &rc);
   g_listwnd = ::CreateWindow("ListBox", "Out", WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_NOINTEGRALHEIGHT, 0, 0, rc.right, rc.bottom, g_mainwnd, NULL, hInst, NULL);
   if (!g_listwnd)
	   return FALSE;

   ShowWindow(g_mainwnd, nCmdShow);
   UpdateWindow(g_mainwnd);

	//gTraceLogFile(tempbuff, strlen(tempbuff));

	g_nShowTime = SetTimer(g_mainwnd, timer_log, elapse_log, NULL);

	//auto startup
	::SendMessage(g_mainwnd, WM_COMMAND, MAKEWPARAM(IDM_STARTUP, 0), NULL);
	gShowTrace();
	gShowTrace();

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   SendMessage(hWnd, WM_CLOSE, 0, 0);
				   break;
				case IDM_CLEAR_TRACE:
					::SendMessage(g_listwnd, LB_RESETCONTENT, 0, 0);
					break;
				case IDM_UPDATE_TRACE:
					gShowTrace();
					gShowTrace();
					break;

				case IDM_TRACE_HOSTSERVER: g_HostServer.TraceInfo(); break;
				case IDM_TRACE_CHATSERVER: g_ChatServer.TraceInfo(); break;
				case IDM_TRACE_TONGSERVER: g_TongServer.TraceInfo(); break;
				case IDM_TRACE_RELAYSERVER: g_RelayServer.TraceInfo(); break;

				case IDM_TRACE_ROOTCENTER: g_RootCenter.TraceInfo(); break;
				case IDM_TRACE_GATEWAYCENTER: g_GatewayCenter.TraceInfo(); break;
				case IDM_TRACE_DBROLECENTER: g_DBCenter.TraceInfo(); break;
				case IDM_TRACE_RELAYCENTER: g_RelayCenter.TraceInfo(); break;

				case IDM_TRACE_PLAYER_DETAIL: g_HostServer.TracePlayer(); break;
				case IDM_TRACE_PLAYER:
					rTRACE("message: online player total: %d", g_HostServer.GetPlayerCount());
					break;

				case IDM_TRACE_FRIENDPLAYER:
					rTRACE("message: friend player total: %d", g_FriendMgr.GetPlayerCount());
					break;

				case IDM_TRACE_FRIENDAFQS:
					rTRACE("message: friend AFQS: %d", g_FriendMgr.GetAFQSize());
					break;

				case IDM_TRACE_STATUS:
					rTRACE("message: [0] RootClient: %d, GatewayClient: %d, DBRoleClient: %d",
						g_RootCenter.GetClientCount(), g_GatewayCenter.GetClientCount(), g_DBCenter.GetClientCount());
					rTRACE("message: [1] RelayClient: %d, RelayConnect: %d",
						g_RelayCenter.GetClientCount(), g_RelayServer.GetConnectCount());
					rTRACE("message: [2] HostConnect: %d, ChatConnect: %d, TongConnect: %d",
						g_HostServer.GetConnectCount(), g_ChatServer.GetConnectCount(), g_TongServer.GetConnectCount());
					break;
				case IDM_TRACE_SOCKTHREAD:
					g_threadSock.TraceInfo();
					break;
				case IDM_TRACE_DATASIZE:
					dTRACE("message: recvsum: %ld B, sendsum: %ld B", gGetRecvDataSize(), gGetSendDataSize());
					break;
				case IDM_TRACE_MAC:
					{{
						std::_tstring descLocal;
						{{
						size_t lenLocal = 0;
						const UCHAR* macLocal = gGetHostMac(local_adapt, &lenLocal);
						for (int i = 0; i < lenLocal; i++)
						{
							char buff[3];
							_stprintf(buff, "%02X", macLocal[i]);
							if (i > 0)
								descLocal += '-';
							descLocal += buff;
						}
						}}
						std::_tstring descGlobal;
						{{
						size_t lenGlobal = 0;
						const UCHAR* macGlobal = gGetHostMac(global_adapt, &lenGlobal);
						for (int i = 0; i < lenGlobal; i++)
						{
							char buff[3];
							_stprintf(buff, "%02X", macGlobal[i]);
							if (i > 0)
								descGlobal += '-';
							descGlobal += buff;
						}
						}}

						rTRACE("message: localMAC: %s, globalMAC: %s", descLocal.c_str(), descGlobal.c_str());
					}}
					break;
				case IDM_TRACE_IP:
					rTRACE("message: localIP: 0x%08X (%s), globalIP: 0x%08X (%s)", gGetHostIP(local_adapt), std::_tstring(_ip2a(gGetHostIP(local_adapt))).c_str(), gGetHostIP(global_adapt), std::_tstring(_ip2a(gGetHostIP(global_adapt))).c_str());
					break;
				case IDM_STARTUP:
					if (gs_initialized)
						rTRACE("waring: already Initialized");
					else
					{
						rTRACE("Initializing ...");
						if (gInitialize())
						{
							gs_initialized = TRUE;
							rTRACE("Initialize OK");
						}
						else
						{
							gUninitialize();
							rTRACE("error: Initialize FAIL");
						}
					}
					break;
				case IDM_SHUTDOWN:
					if (!gs_initialized)
					{
						rTRACE("waring: already Uninitialized");
						gUninitialize();
					}
					else
					{
						rTRACE("Uninitializing ...");
						gUninitialize();
						gs_initialized = FALSE;
						rTRACE("Uninitialize OK");
					}
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			//auto shutdown
			::SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_SHUTDOWN, 0), NULL);
			if (g_nShowTime)
				KillTimer(hWnd, g_nShowTime);
			gShowTrace();
			gShowTrace();

			PostQuitMessage(0);
			break;
		case WM_CLOSE:
			if (::MessageBox(hWnd, "Are you sure ?", "warning", MB_YESNO) == IDYES)
			{
				DestroyWindow(hWnd);
			}
			break;
		case WM_SIZE:
			{
				RECT rc;
				GetClientRect(hWnd, &rc);
				MoveWindow(g_listwnd, 0, 0, rc.right, rc.bottom, TRUE);
			}
			break;
		case WM_TIMER:
			switch (wParam)
			{
			case timer_log: gShowTrace(); break;
			case timer_rootclient:		gReOneClient(oneclient_root); break;
			case timer_gatewayclient:	gReOneClient(oneclient_gateway); break;
			case timer_dbclient:		gReOneClient(oneclient_db); break;
			case timer_friendudtdb: gFriendTimingUpdateDB(); break;
			default: break;
			}
			break;
		case UM_CONSIGNCLIENTSD:
			{{
			IClient* pClient = (IClient*)wParam;
			if (pClient)
			{
				//pClient->Shutdown();
				pClient->Cleanup();
				pClient->Release();
			}
			}}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

// Mesage handler for login box.
LRESULT CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			{{
			TCHAR buffer[_MAX_PATH];
			switch (wParam)
			{
			case IDOK:
				::GetDlgItemText(hDlg, IDC_EDIT_NAME, buffer, _MAX_PATH);
				g_rootloginname = buffer;
				::GetDlgItemText(hDlg, IDC_EDIT_PSWD, buffer, _MAX_PATH);
				g_rootloginpswd = buffer;
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			case IDCANCEL:
				g_interrupt = TRUE;
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			}}
			break;
	}
    return FALSE;
}

HWND gGetMainWnd()
{
	return g_mainwnd;
}

HWND gGetListBox()
{
	return g_listwnd;
}
