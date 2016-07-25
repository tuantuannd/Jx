// UpdateServer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UpdateServer.h"
#include "UpdateServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerApp

BEGIN_MESSAGE_MAP(CUpdateServerApp, CWinApp)
	//{{AFX_MSG_MAP(CUpdateServerApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerApp construction

CUpdateServerApp::CUpdateServerApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUpdateServerApp object

CUpdateServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerApp initialization

BOOL CUpdateServerApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CUpdateServerDlg dlg;
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
