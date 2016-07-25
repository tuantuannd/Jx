#include "StdAfx.h"
#include "resource.h"
#include "Environment.h"
#include "Utils.h"
#include "IniFile.h"
#include "Macro.h"

#include <Commdlg.h>

/*
 * macro define
 */
#define KEY_BUFFER	100

/*
 * const variable
 */
const int g_nDefaultPlayerMaxAt = 5000;
const int g_nDefaultPrecision = 10;
const LPCTSTR g_szDefaultSetupFile = "mgwcfg.ini";

/*
 * library functions
 */
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::GetFileVersion;
using OnlineGameLib::Win32::GetLocalName;
using OnlineGameLib::Win32::GetLocalAddress;
using OnlineGameLib::Win32::CIniFile;

static CIniFile	g_theConfigFile;

ENVIRONMENT	g_theEnviroment;

#define WSA_VERSION  MAKEWORD(2,0)

/*
 * Function helper
 */
bool InitSetupDlg( HWND hDlg );
void DestroySetupDlg( HWND hDlg );
void OpenSetupFile( HWND hDlg );

/*
 * Mesage handler for init-box.
 */
LRESULT CALLBACK DlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
	{
		case WM_INITDIALOG:

			return InitSetupDlg( hDlg );
			break;

		case WM_COMMAND:

			switch ( LOWORD( wParam ) )
			{
			case IDOK:
			case IDCANCEL:
				
				EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );
				EnableWindow( GetDlgItem( hDlg, IDCANCEL ), FALSE );

				EndDialog( hDlg, LOWORD( wParam ) );
				break;

			case IDC_BTN_SETUPFILEPATH:

				OpenSetupFile( hDlg );
				break;	

			default:
				return FALSE;
				break;
			}

			return TRUE;

			break;

		case WM_DESTROY:

			DestroySetupDlg( hDlg );
			return TRUE;

			break;
	}

    return FALSE;
}

bool LoadEnvironmentVariable( HINSTANCE hParentInst )
{
	WSADATA		WSAData = { 0 };
	
	if ( 0 != ::WSAStartup( WSA_VERSION, &WSAData ) )
	{
		/*
		 * Tell the user that we could not find a usable
		 * WinSock DLL.
		 */
		if ( LOBYTE( WSAData.wVersion ) != LOBYTE(WSA_VERSION) ||
			 HIBYTE( WSAData.wVersion ) != HIBYTE(WSA_VERSION) )
		{
			 ::MessageBox( NULL, _T(" Incorrect version of Winsock.dll found" ), _T( "Error" ), MB_OK );
		}

		::WSACleanup();

		return false;
	}	

	g_theEnviroment.sApplicationPath = GetAppFullPath( hParentInst );

	char szIniFile[MAX_PATH];

	sprintf( szIniFile, "%s%s", g_theEnviroment.sApplicationPath.c_str(), g_szDefaultSetupFile );

	g_theConfigFile.SetFile( szIniFile );
	
	/*
	 * Load infomation from ini-file
	 */
	g_theEnviroment.lnPlayerMaxCount = 
		g_theConfigFile.ReadInteger( "SYSTEM", "PlayerMaxAt", g_nDefaultPlayerMaxAt );
	
	g_theEnviroment.lnPrecision =
		g_theConfigFile.ReadInteger( "SYSTEM", "Precision", g_nDefaultPrecision );

	g_theConfigFile.ReadString( "SETUP",
		"SetupFile",
		g_theEnviroment.szConfigFileName, 
		sizeof( g_theEnviroment.szConfigFileName ),
		g_szDefaultSetupFile );

	g_theEnviroment.sAccountServerIP.resize( KEY_BUFFER );
	g_theConfigFile.ReadString( "NETWORK", "AccountSvrIP", 
		const_cast< LPTSTR >( g_theEnviroment.sAccountServerIP.c_str() ), KEY_BUFFER, NULL );

	g_theEnviroment.sRoleDBServerIP.resize( KEY_BUFFER );
	g_theConfigFile.ReadString( "NETWORK", "RoleDBSvrIP", 
		const_cast< LPTSTR >( g_theEnviroment.sRoleDBServerIP.c_str() ), KEY_BUFFER, NULL );

	g_theEnviroment.usAccSvrPort = g_theConfigFile.ReadInteger( "NETWORK", "AccountSvrPort", 5000 );
	g_theEnviroment.usRoleDBSvrPort = g_theConfigFile.ReadInteger( "NETWORK", "RoleDBSvrPort", 5000 );
	g_theEnviroment.usClientOpenPort = g_theConfigFile.ReadInteger( "NETWORK", "ClientOpenPort", 5000 );
	g_theEnviroment.usGameSvrOpenPort = g_theConfigFile.ReadInteger( "NETWORK", "GameSvrOpenPort", 5000 );
	
	/*
	 * Eject a dialog is used to initialize some variable.
	 */
	if ( IDCANCEL == DialogBox( hParentInst, (LPCTSTR)IDD_INITBOX, NULL, (DLGPROC)DlgProc ) )
	{
		return false;
	}
	
	/*
	 * Init information for a new player 
	 */
	ZeroMemory( g_theEnviroment.theRoleData, sizeof( TRoleData ) * NEW_PLAYER_KIND );

	for ( int i=0; i<10; i++ )
	{
		sprintf( szIniFile, "%ssetting\\newplayerini%2.2d.ini", g_theEnviroment.sApplicationPath.c_str(), i );

		CIniFile iniFile( szIniFile );

		TRoleData *pRoleData = &g_theEnviroment.theRoleData[i];

		ASSERT( pRoleData );

		//pRoleData->BaseInfo
	}

	return true;
}

bool SaveEnvironmentVariable()
{
	g_theConfigFile.WriteInteger( "SYSTEM", "PlayerMaxAt", g_theEnviroment.lnPlayerMaxCount );
	g_theConfigFile.WriteInteger( "SYSTEM", "Precision", g_theEnviroment.lnPrecision );

	g_theConfigFile.WriteString( "SETUP", "SetupFile", ( LPCTSTR )g_theEnviroment.szConfigFileName );

	g_theConfigFile.WriteString( "NETWORK", "AccountSvrIP", g_theEnviroment.sAccountServerIP.c_str() );
	g_theConfigFile.WriteString( "NETWORK", "RoleDBSvrIP", g_theEnviroment.sRoleDBServerIP.c_str() );

	g_theConfigFile.WriteInteger( "NETWORK", "AccountSvrPort", g_theEnviroment.usAccSvrPort );
	g_theConfigFile.WriteInteger( "NETWORK", "RoleDBSvrPort", g_theEnviroment.usRoleDBSvrPort );
	g_theConfigFile.WriteInteger( "NETWORK", "ClientOpenPort", g_theEnviroment.usClientOpenPort );
	g_theConfigFile.WriteInteger( "NETWORK", "GameSvrOpenPort", g_theEnviroment.usGameSvrOpenPort );

	::WSACleanup();

	return true;
}

bool InitSetupDlg( HWND hDlg )
{
	::SetDlgItemInt( hDlg, IDC_EDIT_MAXAT, g_theEnviroment.lnPlayerMaxCount, FALSE );
	::SetDlgItemInt( hDlg, IDC_EDIT_PRECISION, g_theEnviroment.lnPrecision, FALSE );
	
	::SetDlgItemText( hDlg, IDC_SETUPFILEPATH, g_theEnviroment.szConfigFileName );
	
	::SetDlgItemText( hDlg, IDC_EDIT_PRODUCTVERSION, GetFileVersion().c_str() );
	
	g_theEnviroment.sLocalName.resize( KEY_BUFFER );
	if ( GetLocalName( const_cast< LPTSTR >( g_theEnviroment.sLocalName.c_str() ), KEY_BUFFER ) )
	{
		::SetDlgItemText( hDlg, IDC_EDIT_USERNAME, g_theEnviroment.sLocalName.c_str() );
	}
	
	g_theEnviroment.sLocalAddress0.resize( KEY_BUFFER );
	if ( GetLocalAddress( const_cast< LPTSTR >( g_theEnviroment.sLocalAddress0.c_str() ), KEY_BUFFER, 0 ) )
	{
		::SetDlgItemText( hDlg, IDC_EDIT_IPADDRESS0, g_theEnviroment.sLocalAddress0.c_str() );
	}
	
	g_theEnviroment.sLocalAddress1.resize( KEY_BUFFER );
	if ( GetLocalAddress( const_cast< LPTSTR >( g_theEnviroment.sLocalAddress1.c_str() ), KEY_BUFFER, 1 ) )
	{
		::SetDlgItemText( hDlg, IDC_EDIT_IPADDRESS1, g_theEnviroment.sLocalAddress1.c_str() );
	}
	
	::SetDlgItemInt( hDlg, IDC_PORT_ACCOUNTSVR, g_theEnviroment.usAccSvrPort, FALSE );
	::SetDlgItemInt( hDlg, IDC_PORT_ROLEDBSVR, g_theEnviroment.usRoleDBSvrPort, FALSE );
	::SetDlgItemInt( hDlg, IDC_PORT_CLIENTSVR, g_theEnviroment.usClientOpenPort, FALSE );
	::SetDlgItemInt( hDlg, IDC_PORT_GAMESVR, g_theEnviroment.usGameSvrOpenPort, FALSE );
	
	::SetDlgItemText( hDlg, IDC_IP_ROLEDBSVR, g_theEnviroment.sRoleDBServerIP.c_str() );
	::SetDlgItemText( hDlg, IDC_IP_ACCOUNTSVR, g_theEnviroment.sAccountServerIP.c_str() );
	
	::SendDlgItemMessage( hDlg,
		IDC_ICON_INIFILE, 
		WM_SETICON, 
		ICON_SMALL, 
		(LPARAM)( ::LoadIcon( NULL, MAKEINTRESOURCE( IDI_INIFILE ) ) ) );
				
	return true;
}

void DestroySetupDlg( HWND hDlg )
{
	g_theEnviroment.lnPlayerMaxCount = ::GetDlgItemInt( hDlg, IDC_EDIT_MAXAT, NULL, FALSE );
	g_theEnviroment.lnPrecision = ::GetDlgItemInt( hDlg, IDC_EDIT_PRECISION, NULL, FALSE );

	g_theEnviroment.usAccSvrPort = ::GetDlgItemInt( hDlg, IDC_PORT_ACCOUNTSVR, NULL, FALSE );
	g_theEnviroment.usRoleDBSvrPort = ::GetDlgItemInt( hDlg, IDC_PORT_ROLEDBSVR, NULL, FALSE );
	g_theEnviroment.usClientOpenPort = ::GetDlgItemInt( hDlg, IDC_PORT_CLIENTSVR, NULL, FALSE );
	g_theEnviroment.usGameSvrOpenPort = ::GetDlgItemInt( hDlg, IDC_PORT_GAMESVR, NULL, FALSE );

	g_theEnviroment.sRoleDBServerIP.resize( KEY_BUFFER );
	::GetDlgItemText( hDlg, IDC_IP_ROLEDBSVR, 
		const_cast< LPSTR >( g_theEnviroment.sRoleDBServerIP.c_str() ), KEY_BUFFER );

	g_theEnviroment.sAccountServerIP.resize( KEY_BUFFER );
	::GetDlgItemText( hDlg, IDC_IP_ACCOUNTSVR,
		const_cast< LPSTR >( g_theEnviroment.sAccountServerIP.c_str() ), KEY_BUFFER );
}

void OpenSetupFile( HWND hDlg )
{
	OPENFILENAME ofn;       // common dialog box structure
	
	if ( 0 == g_theEnviroment.szConfigFileName[0] )
	{
		strcpy( g_theEnviroment.szConfigFileName, g_szDefaultSetupFile );
	}
	
	/*
	 * Initialize OPENFILENAME
	 */
	ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
	
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = g_theEnviroment.szConfigFileName;
	ofn.nMaxFile = sizeof( g_theEnviroment.szConfigFileName );
	ofn.lpstrFilter = "Config Files (*.ini)\0*.ini\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrTitle = "Open config File";
	ofn.lpstrInitialDir = ( LPCTSTR )( g_theEnviroment.sApplicationPath.c_str() );
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	/*
	 * Display the Open dialog box. 
	 */					
	if ( ::GetOpenFileName( &ofn ) )
	{
		::SetDlgItemText( hDlg, IDC_SETUPFILEPATH, ofn.lpstrFile );
	}
}