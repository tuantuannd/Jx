#include "stdlib.h"
#include "stdio.h"
#include "KStdAfx.h"
#include "S3PDBSocketPool.h"
#include "S3PDBConnectionPool.h"
#include "GlobalFun.h"

#include <process.h>

#include "../../../Headers/ServerPort.h"

#include "resource.h"

#include <initguid.h>  // Include only once in your application
#define _WIN32_DCOM
#include "../../Multiserver/Heaven/Interface/IServer.h"

#include "adoid.h"     // ADO GUID's
#include "adoint.h"
#include "stdafx.h"
#include "time.h"

#include "LogFile.h"


const char log_directory[] = "relayserver_log";
const size_t log_threshold = 1024 * 1024;

static CLogFile gs_LogFile;


typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
			REFIID	riid,
			void	**ppv
		);

void __stdcall ServerEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	S3PDBSocketPool *pSocketPool = (S3PDBSocketPool *)lpParam;
	switch( ulnEventType )
	{
	case enumClientConnectCreate:
		pSocketPool->AddUserClientID(ulnID);
		break;
	case enumClientConnectClose:
		pSocketPool->RemoveUserClientID(ulnID);
		break;
	}
}
//static inline sLogProgram(BOOL bEnter)

static int sLogProgram(BOOL bEnter)
{
	SYSTEMTIME systm;
	::GetLocalTime(&systm);
	TCHAR buff[256];
	int len = 0;

	if (bEnter)
		len = _stprintf(buff, "\r\n*\r\n***** S3RelayServer Start (@ %04d/%02d/%02d %02d:%02d:%02d.%03d) >>>>>\r\n*\r\n\r\n",
						systm.wYear, systm.wMonth, systm.wDay, systm.wHour, systm.wMinute, systm.wSecond, systm.wMilliseconds);
	else
		len = _stprintf(buff, "\r\n*\r\n***** S3RelayServer Stop (@ %04d/%02d/%02d %02d:%02d:%02d.%03d) <<<<<\r\n*\r\n\r\n",
						systm.wYear, systm.wMonth, systm.wDay, systm.wHour, systm.wMinute, systm.wSecond, systm.wMilliseconds);

	gs_LogFile.TraceLog(buff, len * sizeof(TCHAR));
return 0;
}


ATOM RegisterSysClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, int nLogTime);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char g_szWindowClass[MAX_PATH];

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	TCHAR szExePath[MAX_PATH + 1];
	KPIGetExePath( szExePath, MAX_PATH );

	TCHAR szLogDir[MAX_PATH + 1];
	_tcscpy(szLogDir, szExePath);
	_tcscat(szLogDir, log_directory);
	::CreateDirectory(szLogDir, NULL);
	if (!gs_LogFile.Initialize(szLogDir, log_threshold))
	{
		::MessageBox(NULL, "Initializing LogFile fail", "error", MB_OK|MB_ICONERROR);
		return FALSE;
	}
	sLogProgram(TRUE);


	TCHAR szINIPath[MAX_PATH + 1];
	_tcscpy(szINIPath, szExePath);
	_tcscat(szINIPath, "relay_setup.ini");
	TCHAR szPort[32];
	sprintf(szPort, "%d", RELAY_ROUTE_PORT);
	DWORD dwResult = KPIGetPrivateProfileString("Local",
		"port",
		szPort,
		szPort,
		31,
		szINIPath);
	short siPort = atoi(szPort);
	dwResult = KPIGetPrivateProfileString("Local",
		"maxconnect",
		"10",
		szPort,
		31,
		szINIPath);
	int snMaxPlayerCount = atoi(szPort);
	dwResult = KPIGetPrivateProfileString("Local",
		"precision",
		"1",
		szPort,
		31,
		szINIPath);
	int snPrecision = atoi(szPort);
	dwResult = KPIGetPrivateProfileString("Local",
		"maxbuffer",
		"15",
		szPort,
		31,
		szINIPath);
	int snMaxBuffer = atoi(szPort);
	dwResult = KPIGetPrivateProfileString("Local",
		"sizebuffer",
		"1048576",
		szPort,
		31,
		szINIPath);
	int snBufferSize = atoi(szPort);

	dwResult = KPIGetPrivateProfileString("Local",
		"timelog",
		"3000",
		szPort,
		31,
		szINIPath);
	int nTimelog = atoi(szPort);
	
	IServer *pServer = NULL;

	HMODULE hModule = ::LoadLibrary("heaven.dll");

	if (hModule)
	{
		pfnCreateServerInterface pFactroyFun = (pfnCreateServerInterface)GetProcAddress(hModule, "CreateInterface");

		IServerFactory *pServerFactory = NULL;

		if (SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >(&pServerFactory ))))
		{
			pServerFactory->SetEnvironment(snMaxPlayerCount, snPrecision, snMaxBuffer, snBufferSize);

			pServerFactory->CreateServerInterface(IID_IIOCPServer, reinterpret_cast< void ** >(&pServer));

			pServerFactory->Release();
		}
	}

	_tcscpy(szINIPath, szExePath);
	_tcscat(szINIPath, "DataBase.ini");

	dwResult = KPIGetPrivateProfileString("Local",
		"DBConnects",
		"5",
		szPort,
		31,
		szINIPath);
	int nDBConns = atoi(szPort);
	S3PDBConnectionPool * pConnectPool = S3PDBConnectionPool::Instance();
	S3PDBSocketPool *pSocketPool = S3PDBSocketPool::Instance();

	if (pServer == NULL)
	{
		::MessageBox(NULL, "heaven Error!", "error", MB_OK | MB_ICONERROR);
		goto Exit0;
	}
	if (!pConnectPool->Init(szINIPath, "account", nDBConns))
	{
		::MessageBox(NULL, "Connect DB Error!", "error", MB_OK | MB_ICONERROR);
		goto Exit0;
	}

	pSocketPool->Start(pServer);

	pServer->Startup();

	pServer->RegisterMsgFilter((LPVOID)pSocketPool, ServerEventNotify);
	pServer->OpenService(INADDR_ANY, siPort);

	MSG msg;
	LoadString(hInstance, IDC_S3RELAYSYS, g_szWindowClass, MAX_PATH);

	RegisterSysClass(hInstance);

		// Perform application initialization:
	if (InitInstance(hInstance, nCmdShow, nTimelog)) 
	{
		//hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_S3RELAY);

		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	pSocketPool->Stop();

Exit0:	
	S3PDBSocketPool::ReleaseInstance();
	S3PDBConnectionPool::ReleaseInstance();

	if (pServer)
	{
		pServer->CloseService();
		pServer->Cleanup();
		pServer->Release();
	}

	if (hModule)
		::FreeLibrary(hModule);

	sLogProgram(FALSE);
	gs_LogFile.Uninitialize();

	return 0;
}

//  FUNCTION: RegisterSysClass()
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
ATOM RegisterSysClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_MAIN);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= (LPCSTR)IDC_S3RELAYSYS;
	wcex.lpszClassName	= g_szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
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
HWND g_listwnd = NULL;
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
				n = 0;
			}

			if (::SendMessage(g_listwnd, LB_ADDSTRING, 0, (LPARAM)pBuff) >= 0)
				n++;

			gTraceLogFile(pBuff, (*pMem) - 1);	//去0写文件
		}
		else
			break;
	}
	if (n != n1)
	{
		::SendMessage(g_listwnd, LB_SETCURSEL, (WPARAM)n - 1, 0);
	}
	g_TraceBuffer.SwitchReadWrite();
}

HINSTANCE g_hInst = 0;
int g_nShowTime = 0;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, int nLogTime)
{
	HWND hWnd;
 
	hWnd = CreateWindow(g_szWindowClass, g_szWindowClass, WS_OVERLAPPEDWINDOW,
	  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
	  return FALSE;
	}

	RECT rc = {0};
	GetClientRect(hWnd, &rc);
	g_listwnd = ::CreateWindow("ListBox", "Out", WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_NOINTEGRALHEIGHT, 0, 0, rc.right, rc.bottom, hWnd, NULL, hInstance, NULL);
	if (!g_listwnd)
	   return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_nShowTime = SetTimer(hWnd, 1, nLogTime, NULL);
	g_hInst = hInstance;

	return TRUE;
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

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   SendMessage(hWnd, WM_CLOSE, 0, 0);
				   break;
				case IDM_CLEARLOG:
				   gClearTrace();
				   break;
				case IDM_SHOWINFO:
					{
						S3PDBSocketPool* p = S3PDBSocketPool::Instance();
						if (p)
							p->ShowAllClientInfo();
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
			if (g_nShowTime)
				KillTimer(hWnd, g_nShowTime);
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
			if (wParam == 1)
				gShowTrace();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

