//******************************************************************************
/*! \file       Robot.cpp
*   \brief      简要描述
*
*				详细描述
*   \author		作者
*   \version	版本号
*   \date		2003-6-3 16:17:20
*   \sa			参考内容
*   \todo		需要完成退出时的帐号回收
				退出时角色的删除
				g_csPlayerList没有用
*******************************************************************************/


#include "stdafx.h"
#include "resource.h"
#include "Player.h"
#include "Robot.h"

#include <objbase.h>
#include <initguid.h>

#include <process.h>
#include <Shellapi.h>

#include "..\RobotManager\protocol.h"
#include "..\RobotManager\struct.h"

#include "RainbowInterface.h"

#include "library.h"
#include "Macro.h"
#include "Utils.h"
#include "tstring.h"
#include "Inifile.h"
#include "buffer.h"
#include "CriticalSection.h"
#include "Event.h"

using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CEvent;


PLAYER_LIST				g_thePlayers;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE	g_hInst;								// current instance
TCHAR		g_szBuffer[MAX_LOADSTRING];				// The title bar text
TCHAR		g_szWindowClass[MAX_LOADSTRING];		// The title bar text

// 要跟随的角色的名字
char g_szRoleName[32];
char *g_pRoleName = g_szRoleName;

NOTIFYICONDATA	g_nidIconData = { 0 };
const UINT		g_unTrayIconID = 500;

#define WM_TRAY_NOTIFY_MSG	WM_USER + 0x100

_tstring g_sAppPath;
CIniFile g_theIniFile;

HANDLE g_hWorkThread = INVALID_HANDLE_VALUE;
CEvent g_hQuitEvent( NULL, true, false, "QuitEvent" );

// 相对于RobotManager的客户
IClient *g_pClientToManager = NULL;

OnlineGameLib::Win32::CLibrary g_theRainbowLib( "rainbow.dll" );

typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

_tstring		g_pAddressToManager;
unsigned short	g_usPortToManager = 0;

bool StartupNetwork();
void DestroyNetwork();
void FreePlayerList();

unsigned int __stdcall HelperThread( void *pParam );
void OnDispatchTask(const void* pData);

// Foward declarations of functions included in this code module:
ATOM				RegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, g_szBuffer, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ROBOT, g_szWindowClass, MAX_LOADSTRING);
	RegisterClass(hInstance);

	g_sAppPath = GetAppFullPath( hInstance );

	char szIniFile[MAX_PATH];
	sprintf( szIniFile, "%srtcfg.ini", g_sAppPath.c_str() );

	g_theIniFile.SetFile( szIniFile );
	g_usPortToManager = g_theIniFile.ReadInteger( "Robot", "PORT", 5096 );

	g_pAddressToManager.resize( MAX_LOADSTRING );
	g_theIniFile.ReadString( "Robot", 
		"IPAddr", 
		const_cast< char * >( g_pAddressToManager.c_str() ), 
		MAX_LOADSTRING, 
		"..." );

	g_theIniFile.ReadString( "Role", "Main", g_szRoleName, 32, "" );

	if( !StartupNetwork())
		return FALSE;
	
	if (!InitInstance (hInstance, nCmdShow)) 
		return FALSE;

	::Shell_NotifyIcon( NIM_ADD, &g_nidIconData );

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ROBOT);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	FreePlayerList();
	::Shell_NotifyIcon( NIM_DELETE, &g_nidIconData );

	DestroyNetwork();

	return msg.wParam;
}

ATOM RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= g_szWindowClass;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd = NULL;

   g_hInst = hInstance; // Store instance handle in our global variable

   if ( NULL == ( hWnd = CreateWindow(g_szWindowClass, g_szBuffer, 0,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL) ) )
   {
      return FALSE;
   }

	g_nidIconData.cbSize = sizeof( NOTIFYICONDATA );

	g_nidIconData.hWnd = hWnd;
	g_nidIconData.uID = g_unTrayIconID;

	g_nidIconData.uCallbackMessage = WM_TRAY_NOTIFY_MSG;
	g_nidIconData.uFlags = NIF_MESSAGE;

	strcpy( g_nidIconData.szTip, "Robot" );	
	g_nidIconData.uFlags |= NIF_TIP;

	HICON hIcon = NULL;

	if ( NULL != ( hIcon = ::LoadIcon( hInstance, MAKEINTRESOURCE( IDI_TRAYICON ) ) ) )
	{
		g_nidIconData.hIcon = hIcon;

		g_nidIconData.uFlags |= NIF_ICON;
	} 

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

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
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_TRAY_NOTIFY_MSG:
			{
				UINT uID = (UINT)wParam;
				UINT uMsg = (UINT)lParam;

				POINT pt = { 0 };

				if ( g_unTrayIconID == uID )
				{
					switch ( uMsg )
					{
						case WM_LBUTTONDOWN:
						case WM_RBUTTONDOWN:
						case WM_CONTEXTMENU:
							{
								::GetCursorPos( &pt );
								
								HMENU hMenu = ::GetSubMenu( ::LoadMenu( g_hInst,
									MAKEINTRESOURCE( IDC_ROBOT ) ), 
									0 );
								
								::SetForegroundWindow( hWnd );
								
								::TrackPopupMenu( hMenu, 
											TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
											pt.x,
											pt.y,
											0,
											hWnd,
											NULL );
								
								::PostMessage( hWnd, WM_NULL, 0, 0 );
							}
							break;
					}
				}
			}
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

//******************************************************************************
/*! \fn     bool StartupNetwork()
*   \brief   启动与RobotManager的连接，robot是RobotManager的客户，RobotManager向每个Robot
				分配帐号,密码和角色的名字
*******************************************************************************/
bool StartupNetwork()
{
	// 启动与RobotManager的连接
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 50, 1024 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &g_pClientToManager ) );
		
		SAFE_RELEASE( pClientFactory );
	}

	if ( g_pClientToManager )
	{
		g_pClientToManager->Startup();
		
		g_pClientToManager->RegisterMsgFilter( 0, NULL );

		if ( FAILED( g_pClientToManager->ConnectTo( g_pAddressToManager.c_str(), g_usPortToManager ) ) )
		{
			g_pClientToManager->Cleanup();

			SAFE_RELEASE( g_pClientToManager );

			return false;
		}
	}

	// 启动工作线程
	IClient *pCloneClient = NULL;
	g_pClientToManager->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pCloneClient ) );

	unsigned int threadID = 0;
	
	g_hWorkThread = (HANDLE)::_beginthreadex(0, 
		0, 
		HelperThread,
		( void * )pCloneClient, 
		0, 
		&threadID );
	
	if ( g_hWorkThread == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	// 向RobotManager发送消息，请求登陆帐号，和服务器地址
	tagReqT_CMD rt;

	rt.cProtocol = enumRequireTask;

	g_pClientToManager->SendPackToServer( &rt, sizeof( rt ) );

	return true;
}

void DestroyNetwork()
{
	// 关闭网络连接
	if ( g_pClientToManager )
	{
		g_pClientToManager->Cleanup();

		SAFE_RELEASE( g_pClientToManager );
	}

	// 中止工作线程
	g_hQuitEvent.Set();

	if ( WAIT_TIMEOUT == ::WaitForSingleObject( g_hWorkThread, 2000 ) )
	{
		::TerminateThread( g_hWorkThread, 0 );
	}

	if ( INVALID_HANDLE_VALUE != g_hWorkThread )
	{
		SAFE_CLOSEHANDLE( g_hWorkThread );
	}
}

// Robot 与 RobotManager 通讯的线程
unsigned int __stdcall HelperThread( void *pParam )
{
	IClient *pCloneClient = ( IClient * )pParam;

	ASSERT( pCloneClient );

	while ( !g_hQuitEvent.Wait( 0 ) )
	{
		size_t datalength = 0;
		const void *pData = pCloneClient->GetPackFromServer( datalength );

		if ( pData && datalength )
		{
			BYTE cProtocol = CPackager::Peek( pData );

			switch ( cProtocol )
			{
			case enumDispatchTask:
				OnDispatchTask(pData);
				break;

			default:
				break;
			}
		}

		::Sleep( 40 );
	}

	SAFE_RELEASE( pCloneClient );

	return 0L;
}

//******************************************************************************
/*! \fn     void OnDispatchTask(const void* pData)
*   \brief  从RobotManager接受任务
*******************************************************************************/
void OnDispatchTask(const void* pData)
{
	tagDspT_INFO *pDTI = ( tagDspT_INFO * )pData;

	if ( pDTI->dwVersion != 1 )
	{
		::MessageBox( NULL, "Version is old, Please update this application", "Info", MB_ICONEXCLAMATION );
		return;
	}

	size_t	nAccCount	= pDTI->nAccCount;
	size_t	nNameLen	= pDTI->nNameLen;
	char*	pAccName	= pDTI->szAccName; 

	for ( int i = 0; i < nAccCount; i ++ )
	{
		memcpy( g_szBuffer, pAccName + i*nNameLen, nNameLen );

		IPlayer *pPlayer = new CPlayer( pDTI->theGameSvrInfo.szIP, 
			pDTI->theGameSvrInfo.nPort,
			g_szBuffer );

		pPlayer->ConnectToGateway();
		g_thePlayers.push_back( pPlayer );
	}	
}

//******************************************************************************
/*! \fn     void FreePlayerList()
*   \brief  释放所有的Player
*******************************************************************************/
void FreePlayerList()
{
	CCriticalSection::Owner locker( g_csPlayerList);

	PLAYER_LIST::iterator it;
	for ( it = g_thePlayers.begin(); it != g_thePlayers.end(); it ++ )
	{
		IPlayer *pPlayer = ( *it );

		if ( pPlayer )
		{
			delete pPlayer;
		}
	}
}
