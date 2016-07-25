// S3DBClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "S3DBClientDlg.h"
#include "MainWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CS3DBClientApp

BEGIN_MESSAGE_MAP(CS3DBClientApp, CWinApp)
	//{{AFX_MSG_MAP(CS3DBClientApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CS3DBClientApp construction

CS3DBClientApp::CS3DBClientApp():
	m_bConnected( FALSE ),
	m_pClientSocket( NULL ),
	m_cstrCurUsername( _T("") ),
	m_cstrCurPassword( _T("") )
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CS3DBClientApp object

CS3DBClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CS3DBClientApp initialization

BOOL CS3DBClientApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	InitAccOperation();

	CMainWnd wnd( _T("《剑侠情缘Online》账户数据库客户端") );
	m_pMainWnd = &wnd;
	int nResponse = wnd.DoModal();
	if ( IDOK == nResponse )
	{
	}
	else if ( IDCANCEL == nResponse )
	{
	}
	/*
	CS3DBClientDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	*/
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CS3DBClientApp::InitAccOperation()
{
	char szSetupFilePath[MAX_PATH+1];
	KPIGetExePath( szSetupFilePath, MAX_PATH );
	strcat( szSetupFilePath, "setup.ini" );
	m_szDefaultAccPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
	KPIGetPrivateProfileString( _T("Account"), _T("password"), _T("888888"),
		m_szDefaultAccPassword, LOGIN_PASSWORD_MAX_LEN, szSetupFilePath );
	m_szDefaultAccRealName[LOGIN_REALNAME_MAX_LEN] = 0;
	KPIGetPrivateProfileString( _T("Account"), _T("realname"), _T("kingsoft"),
		m_szDefaultAccRealName, LOGIN_PASSWORD_MAX_LEN, szSetupFilePath );
	m_szLogPath[MAX_PATH] = 0;
	KPIGetPrivateProfileString( _T("Account"), _T("logpath"), _T("C:\\account.log"),
		m_szLogPath, MAX_PATH, szSetupFilePath );
}
