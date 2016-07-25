// MGateWay.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
//#include "resource.h"
#include "MGateWay.h"
#include "PlayerManager.h"

#include "Macro.h"

#define MAX_LOADSTRING 100

/*
 * Global Variables
 */
HINSTANCE g_hInst;								// current instance

static HWND	g_hwndAnnounceBox = NULL;

static HANDLE g_hOneInstanceEvent = INVALID_HANDLE_VALUE;
static CPlayerManager	*g_pPlayerManager = NULL;

TCHAR g_szTitle[MAX_LOADSTRING];				// The title bar text
TCHAR g_szWindowClass[MAX_LOADSTRING];			// The title bar text
TCHAR g_szAppInfo[MAX_LOADSTRING];

/*
 * Forward declarations of functions included in this code module
 */
ATOM				AppRegisterClass( HINSTANCE hInstance );
BOOL				InitInstance( HINSTANCE, int );
void				ExitInstance();
LRESULT CALLBACK	WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK	About( HWND, UINT, WPARAM, LPARAM );

BOOL				CheckStartupCondition();
void				CleanStartupInfo();

void				ShowAnnounceBox( HWND hwndParent, int nCmdShow );
BOOL CALLBACK		AnnounceBoxProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );

/*
 * Implement
 */
void ShowAnnounceBox( HWND hwndParent, int nCmdShow )
{
	if ( NULL == g_hwndAnnounceBox && !IsWindow( g_hwndAnnounceBox ) )
	{ 
		g_hwndAnnounceBox = ::CreateDialog( g_hInst, 
			MAKEINTRESOURCE( IDD_ANNOUNCEBOX ), 
			hwndParent, 
			(DLGPROC) AnnounceBoxProc );
		
		RECT rect;

		::GetWindowRect( g_hwndAnnounceBox, &rect );

		::SetWindowPos( g_hwndAnnounceBox, 
			NULL, 
			rect.left, 
			rect.top / 2,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER );
	}

	::ShowWindow( g_hwndAnnounceBox, nCmdShow );

	::UpdateWindow( g_hwndAnnounceBox );
}

BOOL CALLBACK AnnounceBoxProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{	
    switch (message) 
    { 
	case WM_INITDIALOG: 

		return TRUE; 
		
	case WM_COMMAND: 

		switch ( LOWORD( wParam ) ) 
		{ 
		case IDCANCEL: 

			DestroyWindow( hwndDlg );
			g_hwndAnnounceBox = NULL;

			return TRUE; 
		} 
    }

    return FALSE; 
}

int APIENTRY WinMain( HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR     lpCmdLine,
                      int       nCmdShow )
{
	MSG msg;
	HACCEL hAccelTable;

	CHAR szInfo[MAX_LOADSTRING];

	LoadString( hInstance, IDS_APP_TITLE, g_szAppInfo, MAX_LOADSTRING );
	
#ifdef	TRUE

	bool bOpenTracer = false;

    while( lpCmdLine[0] == '-' || lpCmdLine[0] == '/' )
    {
        lpCmdLine++;
		
        switch ( *lpCmdLine++ )
        {
		case 'c':
        case 'C':
            bOpenTracer = true;
            break;
        }
		
        while( IS_SPACE( *lpCmdLine ) )
        {
            lpCmdLine++;
        }
    }
	
	if ( bOpenTracer ) 
	{
		AllocConsole();
	}

#endif // End of this function

	/*
	 * Is it one instance?
	 */
	if ( !CheckStartupCondition() )
	{
		LoadString( hInstance, IDS_STARTUP_ERROR, szInfo, MAX_LOADSTRING );
		
		::MessageBox( NULL, szInfo, g_szAppInfo, MB_OK | MB_ICONASTERISK );

		return FALSE;
	}

	/*
	 * Load and setup environment
	 */
	if ( !LoadEnvironmentVariable( hInstance ) )
	{
		return FALSE;
	}

	/*
	 * Initialize global strings
	 */
	LoadString( hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING );
	LoadString( hInstance, IDC_MGATEWAY, g_szWindowClass, MAX_LOADSTRING );
	
	AppRegisterClass( hInstance );

	/*
	 * Perform application initialization
	 */
	if ( !InitInstance ( hInstance, nCmdShow ) )
	{
		LoadString( hInstance, IDS_LOADENVIRONMENT_ERROR, szInfo, MAX_LOADSTRING );

		::MessageBox( NULL, szInfo, g_szAppInfo, MB_OK | MB_ICONEXCLAMATION );

		return FALSE;
	}

	hAccelTable = LoadAccelerators( hInstance, (LPCTSTR)IDC_MGATEWAY );

	/*
	 * Main message loop
	 */
	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) &&
			( !IsWindow( g_hwndAnnounceBox ) ||
			  !IsDialogMessage( g_hwndAnnounceBox, &msg ) ) ) 
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	ExitInstance();
	
	/*
	 * Update and save environment
	 */
	SaveEnvironmentVariable();

	CleanStartupInfo();

#ifdef TRUE

	if ( bOpenTracer )
	{
		FreeConsole();
	}

#endif

	return msg.wParam;
}

//
//  FUNCTION: AppRegisterClass()
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
ATOM AppRegisterClass( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon( hInstance, (LPCTSTR)IDI_MGATEWAY );
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH)( COLOR_WINDOW+1 );
	wcex.lpszMenuName	= (LPCSTR)IDC_MGATEWAY;
	wcex.lpszClassName	= g_szWindowClass;
	wcex.hIconSm		= LoadIcon( wcex.hInstance, (LPCTSTR)IDI_SMALL );

	return RegisterClassEx( &wcex );
}

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
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HWND hWnd;

	g_hInst = hInstance; // Store instance handle in our global variable

	/*
     * Create window
	 */
	hWnd = CreateWindow( g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL );

	if ( !hWnd )
	{
		return FALSE;
	}

	EnableWindow( hWnd, FALSE );
  
	/*
	 * Show window
	 */
	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	ShowAnnounceBox( NULL, SW_SHOW );

	/*
	 * Init player manager
	 */
	g_pPlayerManager = new CPlayerManager( g_theEnviroment );
   
	if ( !g_pPlayerManager->Create() )
	{
		return FALSE;
	}

	ShowAnnounceBox( NULL, SW_HIDE );
   
	EnableWindow( hWnd, TRUE );
	::InvalidateRect( hWnd, NULL, FALSE );

	return TRUE;
}

void SetShowInfo( _tstring sTitle, _tstring sContent, int nFormat /*= SIF_NONE*/ )
{
	static const _tstring sSpace = _T("    ");
	static const _tstring sColon = _T(" : ");

	switch ( nFormat )
	{
	case SIF_SPACE:

		g_theEnviroment.mShowInfo[sTitle] = sTitle + sSpace + sContent;

		break;

	case SIF_COLON:

		g_theEnviroment.mShowInfo[sTitle] = sTitle + sColon + sContent;

		break;

	case SIF_NONE:
	default:

		g_theEnviroment.mShowInfo[sTitle] = sTitle + sContent;
		
		break;
	}
}

void ExitInstance()
{
	ShowAnnounceBox( NULL, SW_SHOW );

	g_pPlayerManager->Destroy();

	SAFE_DELETE( g_pPlayerManager );

	ShowAnnounceBox( NULL, SW_HIDE );
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
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int wmId, wmEvent;

	PAINTSTRUCT ps;
	HDC hdc;
	HBRUSH hBkBrush;

	RECT rect;

/*	static UINT s_unTimerID = 0;
	static const UINT s_unEvent = 1;
	static const UINT s_unElapse = 1000;
*/
//	static UINT s_unAppTimer = 0;
	static char szInfo[100];

	static const int s_nCornerCoord = 8;
	static const int s_nTextSpace = 30;

	switch (message) 
	{
	case WM_CREATE:

//		s_unTimerID = ::SetTimer( hWnd, s_unEvent, s_unElapse, NULL );

		break;
		
	case WM_COMMAND:
		{
			wmId    = LOWORD( wParam ); 
			wmEvent = HIWORD( wParam ); 
			
			/*
			* Parse the menu selections:
			*/
			switch ( wmId )
			{
			case IDM_ABOUT:

				DialogBox( g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About );
				break;

			case IDM_EXIT:

				DestroyWindow( hWnd );
				break;

			default:
				return DefWindowProc( hWnd, message, wParam, lParam );
			}
		}
		break;

/*	case WM_TIMER:
		if ( wParam == s_unEvent )
		{			
			UINT s_unHour = s_unAppTimer / 3600;
			UINT s_unMinute = ( s_unAppTimer % 3600 ) / 60;
			UINT s_unSecond = ( s_unAppTimer % 3600 ) % 60;

			sprintf( szInfo, "%d : %2.2d : %2.2d", s_unHour, s_unMinute, s_unSecond );

			SetShowInfo( "Application Timer", szInfo, SIF_SPACE );

			s_unAppTimer ++;

			::InvalidateRect( hWnd, NULL, FALSE );
		}
		break;
*/		
	case WM_PAINT:
		{
			hdc = BeginPaint( hWnd, &ps );
			hBkBrush = CreateSolidBrush( RGB( 255, 255, 255 ) );
			
			GetClientRect( hWnd, &rect );
			FillRect( hdc, &rect, hBkBrush );

			rect.left = rect.top = s_nCornerCoord;			
			
			for ( _tssmap_it i = g_theEnviroment.mShowInfo.begin();
				i != g_theEnviroment.mShowInfo.end();
				i ++ )
			{
				DrawText( hdc, (*i).second.c_str(), strlen( (*i).second.c_str() ), &rect, DT_LEFT );

				rect.top += s_nTextSpace;
			}
			
			DeleteObject( hBkBrush );
			EndPaint( hWnd, &ps );
		}
		break;
		
	case WM_ERASEBKGND:

		return FALSE;
		break;

	case WM_DESTROY:

//		::KillTimer( hWnd, s_unTimerID );

		PostQuitMessage(0);
		break;

	default:

		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	
	return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL )
			{
				EndDialog( hDlg, LOWORD( wParam ) );

				return TRUE;
			}
			break;
	}
    return FALSE;
}

BOOL CheckStartupCondition()
{
	g_hOneInstanceEvent = ::CreateMutex( NULL, FALSE, "MGW_ONEINST" );

	if ( ERROR_ALREADY_EXISTS == ::GetLastError() )
	{
		return false;
	}

	return true;
}

void CleanStartupInfo()
{
	SAFE_CLOSEHANDLE( g_hOneInstanceEvent );
}
