// Goddess.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#include <objbase.h>
#include <initguid.h>
#include <winsock2.h>
#include <process.h>

#include "kprotocoldef.h"

#include "IDBRoleServer.h"

#include "console.h"
#include "Event.h"
#include "macro.h"
#include "Inifile.h"
#include "Utils.h"
#include "tstring.h"
#include "Library.h"
#include "Buffer.h"
#include "CriticalSection.h"

#include "IServer.h"
#include "HeavenInterface.h"

#include "ClientNode.h"

#include <iostream>
#include <strstream>
#include <string>
#include <list>

#include "FilterTextLib.h"
#include "RoleNameFilter.h"


using namespace std;

using OnlineGameLib::Win32::Console::clrscr;
using OnlineGameLib::Win32::Console::setcolor;
using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CCriticalSection;

/*
 * User data
 */

static CLibrary g_theHeavenLibrary( "heaven.dll" );

static CEvent g_hQuitEvent( NULL, true, false );
static CEvent g_hStartWorkEvent( NULL, true, false );

static size_t g_nMaxRoleCount = 3;

static unsigned short g_nServerPort = 5001;
static IServer *g_pServer = NULL;

static HANDLE g_hThread = NULL;

static CCriticalSection					g_csPlayer;

typedef map< size_t, CClientNode * >	stdPlayerMap;
static 	stdPlayerMap					g_thePlayer;

static CPackager						g_theRecv;
static CPackager						g_theSend;

UINT g_nDBEngineLoop = 0;
UINT g_nServiceLoop = 0;

extern HWND hListOutput;

#define WM_CREATE_ENGINE	WM_USER + 0x100
#define WM_ADD_CLIENT		WM_USER + 0x200
#define WM_DEL_CLIENT		WM_USER + 0x300


CFilterTextLib g_libFilterText;
CRoleNameFilter g_fltRoleName;

static size_t g_BackupSleepTime = 1;  //备份线程自动挂起时间
static bool g_IsBackupSuspend = false;  //备份线程是否手工挂起

/*
 * Helper function
 */
BOOL CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

bool CreateDatabaseEngine( HWND hwnd );
void DestroyDatabaseEngine();

void LoadSetting();
void SaveSetting();

//==========监视的线程
DWORD WINAPI MonitorProc(LPVOID lpParameter) {
	while(true) {
		Sleep(60 * 1000);			//1次
	}
	return 0;
}


/*
 * Callback function
 */
typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
			REFIID	riid,
			void	**ppv
		);

void __stdcall ServerEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	HWND hwnd = ( HWND )lpParam;

	switch( ulnEventType )
	{
	case enumClientConnectCreate:
		{
			CClientNode *pNode = CClientNode::AddNode( g_pServer, ulnID );
			
			if ( pNode )
			{
				CCriticalSection::Owner lock( g_csPlayer );

				g_thePlayer.insert( stdPlayerMap::value_type( ulnID, pNode ) );

				::PostMessage( hwnd, WM_ADD_CLIENT, ulnID, 0 );
			}
		}
		break;

	case enumClientConnectClose:
		{
			{
				CCriticalSection::Owner lock( g_csPlayer );

				stdPlayerMap::iterator it;
				if ( g_thePlayer.end() != ( it = g_thePlayer.find( ulnID ) ) )
				{
					g_thePlayer.erase( ulnID );

					::PostMessage( hwnd, WM_DEL_CLIENT, ulnID, 0 );
				}
			}
			
			CClientNode::DelNode( ulnID );
		}
		break;
	}
}

DWORD WINAPI ThreadProcess( void *pParam )
{
	IServer *pServer = ( IServer * )pParam;

	ASSERT( pServer );

	g_hStartWorkEvent.Wait();

	//try
	//{		
		while ( !g_hQuitEvent.Wait( 0 ) )
		{
			{
				CCriticalSection::Owner lock( g_csPlayer );
				
				stdPlayerMap::iterator it;
				for ( it = g_thePlayer.begin(); it != g_thePlayer.end(); it ++ )
				{
					UINT index = ( *it ).first;
					CClientNode *pClientNode = ( *it ).second;
					
					size_t datalength = 0;
					const char *pData = ( const char * )pServer->GetPackFromClient( index, datalength );
					
					while ( pClientNode && pData && datalength )
					{
						pClientNode->AppendData( pData, datalength );
						
						pData = ( const char * )pServer->GetPackFromClient( index, datalength );
					}
				}
			}
		
			if ( ++ g_nServiceLoop & 0x80000000 )
			{
				g_nServiceLoop = 0;
			}

			if ( g_nServiceLoop & 0x1 )
			{
				::Sleep( 1 );
			}
		}
	//}
	//catch(...)
	//{
	//	::MessageBox( NULL, "ThreadProcess was error!", "Warning", MB_OK );
	//}

	return 0L;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	//role name filter
	if (!g_libFilterText.Initialize()
		|| !g_fltRoleName.Initialize())
	{
		MessageBox(NULL, "text filter's initing has failed", "error", MB_OK|MB_ICONERROR);
		return -1;
	}



	HWND hwndMain = ::CreateDialog( hInstance, 
		MAKEINTRESOURCE( IDD_DLG_GODDESS ),
		NULL, 
		(DLGPROC)MainWndProc );

	ASSERT( hwndMain );

	::ShowWindow( hwndMain, SW_SHOWNORMAL );
	::UpdateWindow( hwndMain );
	
	hListOutput = ::GetDlgItem( hwndMain, IDC_OUTPUT );//输出list

	BOOL bRet;
	MSG msg;
	
	while ( ( bRet = ::GetMessage( &msg, NULL, 0, 0 ) ) != 0 )
	{
		if ( !IsWindow( hwndMain ) || !IsDialogMessage( hwndMain, &msg ) )
		{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
	}
	

	//rolename filter
	g_fltRoleName.Uninitialize();
	g_libFilterText.Uninitialize();
	

	return msg.wParam;
}

/*
 *
 * MainWndProc() - Main window callback procedure.
 *  
 */

BOOL CALLBACK MainWndProc( HWND hwnd, 
                       UINT msg,
                       WPARAM wParam,
                       LPARAM lParam )
{
	static const UINT g_nIDEvent = 0x100;
	static const UINT g_nElapse = 500;

	static const char g_szBaseInfo[] = "....................";
	static const int g_nDots = sizeof( g_szBaseInfo ) - 1;

	static UINT g_nLastServiceLoop = g_nServiceLoop;
	static int g_nServiceStep = g_nDots;
	static UINT g_nLastDBEngineLoop= g_nDBEngineLoop;
	static int g_nDBEngineStep = g_nDots;

	BOOL bTranslated = TRUE;

	switch ( msg )
	{
	case WM_TIMER:
		{
			if ( g_nLastServiceLoop != g_nServiceLoop )
			{
				g_nServiceStep = ( -- g_nServiceStep > 0 ) ? g_nServiceStep : g_nDots;

				::SetDlgItemText( hwnd, IDC_STATIC_SERVICE, ( const char * )g_szBaseInfo + g_nServiceStep );

				g_nLastServiceLoop = g_nServiceLoop;
			}

			if ( g_nLastDBEngineLoop != g_nDBEngineLoop )
			{
				g_nDBEngineStep = ( -- g_nDBEngineStep > 0 ) ? g_nDBEngineStep : g_nDots;

				::SetDlgItemText( hwnd, IDC_STATIC_DATABASE, ( const char * )g_szBaseInfo + g_nDBEngineStep );

				g_nLastDBEngineLoop = g_nDBEngineLoop;
			}

			if(IsBackupThreadWorking())
			{
				::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_SUS_RES ), TRUE );
				::SetWindowText(GetDlgItem( hwnd, IDC_LAB_BACKUP_STATUS ),
								"Backup thread status: Sleeping");
				::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_MANUAL ), TRUE );
				if(IsBackupWorking())
				{
					::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_SUS_RES ), FALSE );
					::SetWindowText(GetDlgItem( hwnd, IDC_LAB_BACKUP_STATUS ),
								"Backup thread status: Running");
					::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_MANUAL ), FALSE );
				}
				else if (g_IsBackupSuspend)
				{
					::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_SUS_RES ), TRUE );
					::SetWindowText(GetDlgItem( hwnd, IDC_LAB_BACKUP_STATUS ),
								"Backup thread status: Suspended");
					::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_MANUAL ), TRUE );
				}
						
			}
			else
			{
				::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_SUS_RES ), FALSE );
				::SetWindowText(GetDlgItem( hwnd, IDC_LAB_BACKUP_STATUS ),
								"Backup thread status: Stop");
				::EnableWindow( GetDlgItem( hwnd, IDC_BTN_BACKUP_MANUAL ), FALSE );
			}
		}
		break;

	case WM_INITDIALOG:

		LoadSetting();

		::SetDlgItemInt( hwnd, IDC_EDIT_PORT, g_nServerPort, FALSE );
		::SetDlgItemInt( hwnd, IDC_EDIT_MAXNUM_ROLE, g_nMaxRoleCount, FALSE );
		::SetDlgItemInt( hwnd, IDC_EDIT_BACKUP_SLEEP_TIME, g_BackupSleepTime, FALSE );
		
		::SetTimer( hwnd, g_nIDEvent, g_nElapse, NULL );
		
		return TRUE;
		break;

	case WM_CLOSE:
		{
			if ( IDYES == ::MessageBox( hwnd, "Are you sure quit?", "Info", MB_YESNO | MB_ICONQUESTION ) )
			{
				
				DestroyDatabaseEngine();
				
				::KillTimer( hwnd, g_nIDEvent );

				g_nServerPort = ::GetDlgItemInt( hwnd, IDC_EDIT_PORT, &bTranslated, FALSE );
				g_nMaxRoleCount = ::GetDlgItemInt( hwnd, IDC_EDIT_MAXNUM_ROLE, &bTranslated, FALSE );
				g_BackupSleepTime = ::GetDlgItemInt( hwnd, IDC_EDIT_BACKUP_SLEEP_TIME, &bTranslated, FALSE );

				SaveSetting();

				::DestroyWindow( hwnd );
				::PostQuitMessage( 0 );
			}
		}

		return TRUE;
		break;
	
	case WM_COMMAND:

		switch ( wParam )
		{
		case IDOK:

			::EnableWindow( GetDlgItem( hwnd, IDOK ), FALSE );
			::EnableWindow( GetDlgItem( hwnd, IDC_EDIT_PORT ), FALSE );
			::EnableWindow( GetDlgItem( hwnd, IDC_EDIT_MAXNUM_ROLE ), FALSE );
			::EnableWindow( GetDlgItem( hwnd, IDC_EDIT_BACKUP_SLEEP_TIME ), FALSE );

			::SetWindowText( hwnd, "Goddess - [Start up...]" );

			::PostMessage( hwnd, WM_CREATE_ENGINE, 0L, 0L );
			printf("here");
			break;

		case IDCANCEL:

			::PostMessage( hwnd, WM_CLOSE, 0L, 0L );
			break;

		case IDC_BTN_BACKUP_MANUAL:

			if(!IsBackupThreadWorking())
						MessageBox(hwnd, "Backup thread is not started yet.",
								"Information",MB_OK | MB_ICONEXCLAMATION);
			if(!DoManualBackup())
				MessageBox(hwnd, "Unable to start manual backup.\nPlease try later.",
								"Information",MB_OK | MB_ICONEXCLAMATION);

			break;
		case IDC_RESTART:
			CClientNode::End();
			ReleaseDBInterface();
			InitDBInterface(g_nMaxRoleCount);
			CClientNode::Start( g_pServer );
			break;
		case IDC_BTN_BACKUP_SUS_RES:

			if(!IsBackupThreadWorking())
						MessageBox(hwnd, "Backup thread is not started yet.",
								"Information",MB_OK | MB_ICONEXCLAMATION);
			else if(g_IsBackupSuspend)
			{
				if(ResumeBackupTimer())
				{
					g_IsBackupSuspend = false;
					::SetWindowText(GetDlgItem( hwnd, IDC_BTN_BACKUP_SUS_RES ),
								"Suspend");
				}
				else
					MessageBox(hwnd, "Unable to resume backup thread.\nPlease try later.",
								"Information",MB_OK | MB_ICONEXCLAMATION);
			}
			else
			{
				if(SuspendBackupTimer())
				{
					g_IsBackupSuspend = true;
					::SetWindowText(GetDlgItem( hwnd, IDC_BTN_BACKUP_SUS_RES ),
								"Resume");
				}
				else
					MessageBox(hwnd, "Unable to suspend backup thread.\nPlease try later.",
								"Information",MB_OK | MB_ICONEXCLAMATION);
			}
			break;
		}

		break;
	
	case WM_CREATE_ENGINE:
		{
		g_nServerPort = ::GetDlgItemInt( hwnd, IDC_EDIT_PORT, &bTranslated, FALSE );
		g_nMaxRoleCount = ::GetDlgItemInt( hwnd, IDC_EDIT_MAXNUM_ROLE, &bTranslated, FALSE );
		g_BackupSleepTime = ::GetDlgItemInt( hwnd, IDC_EDIT_BACKUP_SLEEP_TIME, &bTranslated, FALSE );

		CreateDatabaseEngine( hwnd );
/*		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hMonitorThread = CreateThread(
			NULL,				// no security attributes 
			0,					// use default stack size  
			MonitorProc,		// thread function 
			&dwThrdParam,		// argument to thread function 
			0,					// use default creation flags 
			&dwThreadId);		// returns the thread identifier 
*/
		::SetWindowText( hwnd, "Goddess - [Enable]" );
		}
		break;

	case WM_ADD_CLIENT:
		{
			const char *pText = g_pServer->GetClientInfo( wParam );

			if ( pText && pText[0] )
			{
				HWND hCtrl = ::GetDlgItem( hwnd, IDC_LIST_CLIENT );
				
				if ( hCtrl && ::IsWindow( hCtrl ) )
				{
					int nIndex = ::SendMessage( hCtrl, LB_ADDSTRING, 0, ( LPARAM )pText );
					
					if ( LB_ERR != nIndex )
					{
						::SendMessage( hCtrl, LB_SETITEMDATA, nIndex, wParam );
					}
				}
			}
		}		
		break;

	case WM_DEL_CLIENT:
		{
			HWND hCtrl = ::GetDlgItem( hwnd, IDC_LIST_CLIENT );
			
			if ( hCtrl && ::IsWindow( hCtrl ) )
			{
				int nCount = ::SendMessage( hCtrl, LB_GETCOUNT, 0, 0 );
				
				for ( int i=0; i<nCount, LB_ERR != nCount; i++ )
				{
					UINT nSearchID = 0;
					
					if ( wParam == ( nSearchID = ::SendMessage( hCtrl, LB_GETITEMDATA, i, 0 ) ) )
					{
						::SendMessage( hCtrl, LB_DELETESTRING, i, 0 );
						
						return TRUE;
					}
				}
			}
		}
		break;

	default:
		break;		
	}
	
	/*
	 * Clean up any unused messages by calling DefWindowProc
	 */
	return FALSE;
}

bool CreateDatabaseEngine( HWND hwnd )
{
	if ( !InitDBInterface( g_nMaxRoleCount ) )
	{
		::MessageBox( NULL, "Setup dbserver is failed!", "Warning", MB_OK | MB_ICONSTOP );
		
		return false;
	}
	
	StartBackupTimer(g_BackupSleepTime); //开始运行备份线程
	::SetWindowText(GetDlgItem( hwnd, IDC_LAB_BACKUP_STATUS ),
								"Backup thread status: Running");
	/*
	 * Open this server to client
	 */
	pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )( g_theHeavenLibrary.GetProcAddress( "CreateInterface" ) );

	IServerFactory *pServerFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( 10, 10, 20, 4 * 1024 * 1024 );
		
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &g_pServer ) );
		
		pServerFactory->Release();
	}

	if ( !g_pServer )
	{
		::MessageBox( NULL, "Initialization failed! Don't find a correct heaven.dll", "Warning", MB_OK | MB_ICONSTOP );

		return false;
	}

	g_pServer->Startup();

	g_pServer->RegisterMsgFilter( ( void * )hwnd, ServerEventNotify );

	if ( FAILED( g_pServer->OpenService( INADDR_ANY, g_nServerPort ) ) )
	{
		return false;
	}

	DWORD dwThreadID = 0L;

	IServer *pCloneServer = NULL;
	g_pServer->QueryInterface( IID_IIOCPServer, ( void ** )&pCloneServer );

	g_hThread = ::CreateThread( NULL, 0, ThreadProcess, ( void * )pCloneServer, 0, &dwThreadID );

	if ( !g_hThread )
	{
		return false;
	}

	CClientNode::Start( g_pServer );

	g_hStartWorkEvent.Set();
	
	return true;
}

void DestroyDatabaseEngine()
{
	CClientNode::End();

	g_hQuitEvent.Set();
	
	DWORD result = ::WaitForSingleObject( g_hThread, 50000 );

	if ( result == WAIT_TIMEOUT )
	{
		::TerminateThread( g_hThread, ( DWORD )( -1 ) );
	}

	SAFE_CLOSEHANDLE( g_hThread );

	if ( g_pServer )
	{
		g_pServer->CloseService();
		g_pServer = NULL;
	}

	SAFE_RELEASE( g_pServer );

	ReleaseDBInterface();
}

void LoadSetting()
{
	CIniFile theConfigFile;

	_tstring sAppPath = GetAppFullPath( NULL );

	_tstring sConfigFile;

	sConfigFile = sAppPath + "Goddess.cfg";

	theConfigFile.SetFile( sConfigFile.c_str() );

	g_nServerPort = theConfigFile.ReadInteger( "Setting", "Port", 5001 );

	g_nMaxRoleCount = theConfigFile.ReadInteger( "Setting", "MaxRoleCount", 3 );

	g_BackupSleepTime = theConfigFile.ReadInteger( "Setting", "BackupSleepTime", 3 );
}

void SaveSetting()
{
	CIniFile theConfigFile;

	_tstring sAppPath = GetAppFullPath( NULL );

	_tstring sConfigFile;

	sConfigFile = sAppPath + "Goddess.cfg";

	theConfigFile.SetFile( sConfigFile.c_str() );

	theConfigFile.WriteInteger( "Setting", "Port", g_nServerPort );

	theConfigFile.WriteInteger(  "Setting", "MaxRoleCount", g_nMaxRoleCount );

	theConfigFile.WriteInteger(  "Setting", "BackupSleepTime", g_BackupSleepTime );
}