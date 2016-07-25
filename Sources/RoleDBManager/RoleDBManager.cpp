// RoleDBManager.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "kdbprocessthread.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text
char g_SaveMarkString[200] = "";
char g_RemoveMarkString[200] = "";
char g_LoadMarkString[200] = "";
// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
bool StartServer() ;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ROLEDBMANAGER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	if (!StartServer()) 
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ROLEDBMANAGER);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_ROLEDBMANAGER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_ROLEDBMANAGER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
	TCHAR szHello[MAX_MARK_COUNT + 400];
//	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_CREATE:
			::SetTimer( hWnd, 1,  100, NULL );
		break;
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
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
	
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
		

			sprintf(szHello, "Recv%d, Sent%d", g_dwRecvLen , g_dwSendLen);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_LEFT);
		
			char Marks[MAX_MARK_COUNT +3];
			rt.top += 50;
			memset(Marks, MARK, g_MainLoopMark);
			Marks[g_MainLoopMark] = 0;
			sprintf(szHello, "MainLoop  %s", Marks);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_LEFT);
			
			rt.top += 50;
			memset(Marks, MARK, g_NetServerMark);
			Marks[g_NetServerMark] = 0;
			sprintf(szHello, "NetServer %s", Marks);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_LEFT);
			
			rt.top += 50;
			if (g_LoadDBThreadMark == MAX_MARK_COUNT + LOADMARK_NO)
			{
				strcpy(szHello, g_LoadMarkString);
			}
			else
			{
				memset(Marks, MARK, g_LoadDBThreadMark);
				Marks[g_LoadDBThreadMark] = 0;
				sprintf(szHello, "DBLoad    %s", Marks);
			}
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_LEFT);

			rt.top += 50;
			if (g_SaveDBThreadMark == MAX_MARK_COUNT + SAVEMARK_NO)
			{
				strcpy(szHello, g_SaveMarkString);
			}
			else if (g_SaveDBThreadMark == MAX_MARK_COUNT + REMOVEMARK_NO)
			{
				strcpy(szHello, g_RemoveMarkString);
			}
			else
			{
				memset(Marks, MARK, g_SaveDBThreadMark);
				Marks[g_SaveDBThreadMark] = 0;
				sprintf(szHello, "DBSave    %s",Marks);
			}
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_LEFT);

			rt.top = rt.bottom - 50;
			sprintf(szHello, "Mistake   %d", g_nMistakeCount);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_RIGHT);
			EndPaint(hWnd, &ps);
			
			break;
		case WM_DESTROY:
			ReleaseServer();
			PostQuitMessage(0);
			break;

		case WM_TIMER:
			::InvalidateRect( hWnd, NULL, TRUE);
		
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

extern TThreadData g_ThreadData[4];


bool StartServer() 
{
	// TODO: Add your control notification handler code here
	if (InitServer() == 0)	return false;

    g_ThreadData[0].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        RoleServerMainThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[0].dwThreadId);                // returns the thread identifier 
	
	g_ThreadData[1].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        DatabaseLoadThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[1].dwThreadId);                // returns the thread identifier 
	
	g_ThreadData[2].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        DatabaseSaveThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[2].dwThreadId);                // returns the thread identifier 
	
	g_ThreadData[3].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        RoleNetWorkServiceThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[3].dwThreadId);                // returns the thread identifier 
	return true;

}

