// S3Robot.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "S3Robot.h"
#include "S3RobotDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CS3RobotApp

BEGIN_MESSAGE_MAP(CS3RobotApp, CWinApp)
	//{{AFX_MSG_MAP(CS3RobotApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CS3RobotApp construction

CS3RobotApp::CS3RobotApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CS3RobotApp object

CS3RobotApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CS3RobotApp initialization

BOOL CS3RobotApp::InitInstance()
{
	AfxEnableControlContainer();

	g_SetRootPath( NULL );

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CS3RobotDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
