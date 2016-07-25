// Intercessor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "Network.h"
#include "PlayerManager.h"

#include "KProtocolDef.h"
#include "../../Network/S3AccServer/AccountLoginDef.h"

#include "macro.h"
#include "Event.h"
#include "string.h"
#include "Buffer.h"

using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CPackager;

#define WM_ENTER_AFFIRM				( WM_USER + 0x100 )
#define WM_LEAVE_AFFIRM				( WM_USER + 0x200 )
#define WM_SERVER_LOGIN_SUCCESSFUL	( WM_USER + 0x300 )
#define WM_SERVER_LOGIN_FAILED		( WM_USER + 0x400 )

#define BUFFER_LENGTH	100

enum enumServerLoginErrorCode
{
	enumConnectFailed	= 0xA1,
	enumUsrNamePswdErr,
	enumIPPortErr,
	enumException
};

const DWORD g_dwServerIdentify = 0xAEFC07B5;

/*
 * Global variable
 */

HINSTANCE	g_hInst;

CEvent		g_theWorkingTutorEvent( NULL, false, false, _T("Working_Tutor_Event") );

CNetwork	g_theNetwork;

CPlayerManager	*g_pPlayerManager = NULL;

/*
 * Helper function
 */
bool LoginSystem( HINSTANCE hInstance );

BOOL CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

UINT MainDialog();
void InitMainDlg( HWND hDlg );
void CloseMainDlg( HWND hDlg );

BOOL CALLBACK LoginDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * WinMain
 */
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	int nRet = 0;

	g_hInst = hInstance;

	g_theNetwork.Create();

	/*
	 * To affirm account that it is used to check the application
	 */
	if ( LoginSystem( hInstance ) )
	{
		g_pPlayerManager = new CPlayerManager( 50, g_theNetwork );

		ASSERT( g_pPlayerManager );

		MainDialog();
	}

	g_theNetwork.Destroy();
	return nRet;
}

/*
 * Log on to system
 */
bool LoginSystem( HINSTANCE hInstance )
{
	if ( WM_SERVER_LOGIN_SUCCESSFUL == ::DialogBox( hInstance,
					MAKEINTRESOURCE( IDD_DLG_LOGIN ),
					NULL,
					( DLGPROC )LoginDlgProc ) )
	{
		return true;
	}

	return false;
}

UINT MainDialog()
{
	/*
	 * Create the main window as dialog. 
	 */
	HWND hwndMain = ::CreateDialog( g_hInst, 
		MAKEINTRESOURCE( IDD_DLG_INTERCESSOR ),
		NULL, 
		(DLGPROC) MainWndProc );

	ASSERT( hwndMain );

	::ShowWindow( hwndMain, SW_SHOWNORMAL );
	::UpdateWindow( hwndMain );

	
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
	
	return msg.wParam;
}

void InitMainDlg( HWND hDlg )
{
	::SetDlgItemText( hDlg, IDC_EDIT_ACCSVRIP, g_theNetwork.GetAccSvrIP() );
	::SetDlgItemInt( hDlg, IDC_EDIT_ACCSVRPORT, g_theNetwork.GetAccSvrPort(), FALSE );

	::SetDlgItemText( hDlg, IDC_EDIT_ROLESVRIP, g_theNetwork.GetRoleSvrIP() );
	::SetDlgItemInt( hDlg, IDC_EDIT_ROLESVRPORT, g_theNetwork.GetRoleSvrPort(), FALSE );

	::SetDlgItemInt( hDlg, IDC_EDIT_CLIENT_PORT, g_theNetwork.GetClientOpenPort(), FALSE );

	::SetDlgItemInt( hDlg, IDC_EDIT_GAMESVR_PORT, g_theNetwork.GetGameSvrOpenPort(), FALSE );
}

void CloseMainDlg( HWND hDlg )
{
	_tstring sBuffer;

	sBuffer.resize( BUFFER_LENGTH );

	::GetDlgItemText( hDlg, IDC_EDIT_ROLESVRIP, const_cast< char * >( sBuffer.c_str() ), BUFFER_LENGTH );
	g_theNetwork.SetRoleSvrIP( sBuffer.c_str() );

	UINT nValue = 0;
	BOOL bTranslated = TRUE;

	nValue = ::GetDlgItemInt( hDlg, IDC_EDIT_ROLESVRPORT, &bTranslated, FALSE );
	g_theNetwork.SetRoleSvrPort( nValue );

	nValue = ::GetDlgItemInt( hDlg, IDC_EDIT_CLIENT_PORT, &bTranslated, FALSE );
	g_theNetwork.SetClientOpenPort( nValue );

	nValue = ::GetDlgItemInt( hDlg, IDC_EDIT_GAMESVR_PORT, &bTranslated, FALSE );
	g_theNetwork.SetGameSvrOpenPort( nValue );
}

DWORD WINAPI ServerLoginRoutine( LPVOID lpParam ) 
{
	HWND hwndDlg = ( HWND )lpParam;

	ASSERT( hwndDlg );

	/*
	 * Ask for log on to the account server
	 */

	UINT		nPort = 0;
	_tstring	sBuffer, sUsername, sPassword;

	sBuffer.resize( MAX_PATH + 1 );

	::GetDlgItemText( hwndDlg, IDC_EDIT_LOGONTO_IP, const_cast< char * >( sBuffer.c_str() ), MAX_PATH );

	BOOL bTranslated = TRUE;

	nPort = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LOGONTO_PORT, &bTranslated, TRUE );

	IClient *pAccSvrClient = g_theNetwork.CreateAccSvrClient( sBuffer.c_str(),  nPort );

	if ( !pAccSvrClient )
	{
		::PostMessage( hwndDlg, WM_SERVER_LOGIN_FAILED, enumConnectFailed, 0L );

		SAFE_RELEASE( pAccSvrClient );
		return 1L;
	}

	sUsername.resize( BUFFER_LENGTH );
	::GetDlgItemText( hwndDlg, IDC_EDIT_USERNAME, const_cast< char * >( sUsername.c_str() ), BUFFER_LENGTH );

	sPassword.resize( BUFFER_LENGTH );
	::GetDlgItemText( hwndDlg, IDC_EDIT_PASSWORD, const_cast< char * >( sPassword.c_str() ), BUFFER_LENGTH );
	
	/*
	 * Send a login command
	 */
	CBuffer *pBuffer = CNetwork::m_theGlobalAllocator.Allocate();

	BYTE *pData = const_cast< BYTE * >( pBuffer->GetBuffer() );

	const size_t datalength = sizeof( KAccountUserLoginInfo ) + 1;

	KAccountUserLoginInfo serlogin;
	serlogin.Size = sizeof( KAccountUserLoginInfo );

	serlogin.Type = AccountUserLoginInfo;
	serlogin.Version = ACCOUNT_CURRENT_VERSION;
	serlogin.Operate = g_dwServerIdentify;

	strcpy( serlogin.Account, sUsername.c_str() );
	strcpy( serlogin.Password, sPassword.c_str() );

	*pData = c2s_gatewayverify;
	memcpy( pData + 1, &serlogin, sizeof( KAccountUserLoginInfo ) );

	pBuffer->Use( datalength );

	pAccSvrClient->SendPackToServer( ( const void * )pData, datalength );

	SAFE_RELEASE( pBuffer );

	/*
	 * Wait for write back
	 */
	while ( pAccSvrClient )
	{
		size_t nLen = 0;
		
		const void *pData = pAccSvrClient->GetPackFromServer( nLen );

		if ( !pData || 0 == nLen )
		{
			::Sleep( 1 );

			continue;
		}

		BYTE cProtocol = CPackager::Peek( pData );

		switch ( cProtocol )
		{
		case s2c_gatewayverify:
			{
				KAccountUserReturn* pReturn = ( KAccountUserReturn * )( ( ( char * )pData ) + 1 );

				nLen--;
				if ( pReturn &&
					( g_dwServerIdentify == pReturn->Operate ) && 
					( nLen == sizeof( KAccountUserReturn ) ) )
				{
					if ( ACTION_SUCCESS == pReturn->nReturn )
					{
						::PostMessage( hwndDlg, WM_SERVER_LOGIN_SUCCESSFUL, 0L, 0L );
					}
					else if ( E_ACCOUNT_OR_PASSWORD == pReturn->nReturn )
					{
						::PostMessage( hwndDlg, WM_SERVER_LOGIN_FAILED, enumUsrNamePswdErr, 0L );
					}
					else if ( E_ADDRESS_OR_PORT == pReturn->nReturn )
					{
						::PostMessage( hwndDlg, WM_SERVER_LOGIN_FAILED, enumIPPortErr, 0L );
					}
					else
					{
						::PostMessage( hwndDlg, WM_SERVER_LOGIN_FAILED, enumException, 0L );
					}

					SAFE_RELEASE( pAccSvrClient );
					return 1L;
				}
			}
			break;

		default:

			ASSERT( FALSE );

			break;
		}
	}	

	SAFE_RELEASE( pAccSvrClient );

	return 0L;
}

void EnterToAffirm( HWND hwndDlg )
{
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_USERNAME ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_PASSWORD ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LOGONTO_IP ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LOGONTO_PORT ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDOK ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDCANCEL ), FALSE );

	/*
	 * Start up a listening thread to wait a required confirm from the account server
	 */
	DWORD dwThreadID = 0;

	HANDLE hThread = ::CreateThread( NULL, 0, ServerLoginRoutine, ( void * )hwndDlg, 0, &dwThreadID );
	
	SAFE_CLOSEHANDLE( hThread );
}

void LeaveToAffirm( HWND hwndDlg )
{
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_USERNAME ), TRUE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_PASSWORD ), TRUE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LOGONTO_IP ), TRUE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LOGONTO_PORT ), TRUE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDOK ), TRUE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDCANCEL ), TRUE );
}

bool CheckUserInfo( HWND hwndDlg )
{
	UINT		nLen = 0;
	_tstring	sBuffer;

	sBuffer.resize( MAX_PATH + 1 );
	
	if ( 0 == ( nLen = ::GetDlgItemText( hwndDlg, IDC_EDIT_USERNAME, const_cast< char * >( sBuffer.c_str() ), MAX_PATH ) ) )
	{
		::SetFocus( ::GetDlgItem( hwndDlg, IDC_EDIT_USERNAME ) );

		::MessageBox( hwndDlg, "Please input a valid username!", "Warning", MB_ICONASTERISK );

		return false;
	}

	if ( 0 == ( nLen = ::GetDlgItemText( hwndDlg, IDC_EDIT_LOGONTO_IP, const_cast< char * >( sBuffer.c_str() ), MAX_PATH ) ) )
	{
		::SetFocus( ::GetDlgItem( hwndDlg, IDC_EDIT_LOGONTO_IP ) );

		::MessageBox( hwndDlg, "Please input a valid IP!", "Warning", MB_ICONASTERISK );

		return false;
	}
	
	BOOL bTranslated = FALSE;

	if ( 0 == ( nLen = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LOGONTO_PORT, &bTranslated, TRUE ) ) )
	{
		::SetFocus( ::GetDlgItem( hwndDlg, IDC_EDIT_LOGONTO_PORT ) );

		::MessageBox( hwndDlg, "Please input a valid port!", "Warning", MB_ICONASTERISK );

		return false;
	}

	return true;
}

BOOL CALLBACK LoginDlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	_tstring sBuffer;

    switch (message)
    {
	case WM_SERVER_LOGIN_SUCCESSFUL:

		::EndDialog( hwndDlg, WM_SERVER_LOGIN_SUCCESSFUL );
		
		break;

	case WM_SERVER_LOGIN_FAILED:

		switch( wParam )
		{
		case enumUsrNamePswdErr:

			sBuffer.resize( MAX_PATH );

			::LoadString( g_hInst, IDS_NAMEPSWD_ERROR, const_cast< char * >( sBuffer.c_str() ), MAX_PATH );

			::MessageBox( hwndDlg, sBuffer.c_str(), NULL, MB_ICONEXCLAMATION );

			::PostMessage( hwndDlg, WM_LEAVE_AFFIRM, 0L, 0L );

			break;

		case enumConnectFailed:
		case enumIPPortErr:

			sBuffer.resize( MAX_PATH );

			::LoadString( g_hInst, IDS_NETWORK_ERROR, const_cast< char * >( sBuffer.c_str() ), MAX_PATH );

			::MessageBox( hwndDlg, sBuffer.c_str(), NULL, MB_ICONEXCLAMATION );

			::PostMessage( hwndDlg, WM_LEAVE_AFFIRM, 0L, 0L );

			break;

		case enumException:
		default:

			::MessageBox( hwndDlg, "Exception!", NULL, MB_ICONEXCLAMATION );

			::EndDialog( hwndDlg, WM_SERVER_LOGIN_FAILED );

			break;
		}
		
		break;

	case WM_ENTER_AFFIRM:

		EnterToAffirm( hwndDlg );
		
		break;

	case WM_LEAVE_AFFIRM:

		LeaveToAffirm( hwndDlg );

		break;

	case WM_INITDIALOG:

		::SetFocus( ::GetDlgItem( hwndDlg, IDC_EDIT_USERNAME ) );
		
		// TEMPORAL
		::SetDlgItemText( hwndDlg, IDC_EDIT_LOGONTO_IP, "192.168.26.1" );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LOGONTO_PORT, 5002, FALSE );
		
		break;

	case WM_COMMAND:

		switch ( LOWORD( wParam ) )
		{
		case IDOK:

			if ( CheckUserInfo( hwndDlg ) )
			{
				::PostMessage( hwndDlg, WM_ENTER_AFFIRM, 0L, 0L );
			}

			break;

		case IDCANCEL:
			
			::EndDialog( hwndDlg, WM_SERVER_LOGIN_FAILED );

			return TRUE; 
		} 
    }

    return FALSE;
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
	switch ( msg )
	{
	case WM_INITDIALOG:
		
		/*
		 * Initialize
		 */
		InitMainDlg( hwnd );
		
		return TRUE;

		break;

	case WM_CLOSE:

		CloseMainDlg( hwnd );

		if ( g_pPlayerManager )
		{
			g_pPlayerManager->Destroy();
		}

		::DestroyWindow( hwnd );
		::PostQuitMessage( 0 );

		return TRUE;

		break;
	
	case WM_COMMAND:

		switch (wParam)
		{
		case IDOK:

			::EnableWindow( GetDlgItem( hwnd, IDOK ), FALSE );

			if ( !g_pPlayerManager->Create() )
			{
				::EnableWindow( GetDlgItem( hwnd, IDOK ), TRUE );
				
				g_pPlayerManager->Destroy();
			}

			break;

		case IDCANCEL:

			::PostMessage( hwnd, WM_CLOSE, 0L, 0L );

			break;
		}

		break;

	default:

		break;
		
	}
	
	/*
	 * Clean up any unused messages by calling DefWindowProc
	 */
	return FALSE;
	//return ::DefWindowProc( hwnd, msg, wParam, lParam );
}


