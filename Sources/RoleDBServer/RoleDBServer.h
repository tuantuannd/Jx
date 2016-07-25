// RoleDBServer.h : main header file for the ROLEDBSERVER application
//

#if !defined(AFX_ROLEDBSERVER_H__75F3DE15_81FD_4E76_816E_5B2A29B160B2__INCLUDED_)
#define AFX_ROLEDBSERVER_H__75F3DE15_81FD_4E76_816E_5B2A29B160B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerApp:
// See RoleDBServer.cpp for the implementation of this class
//

class CRoleDBServerApp : public CWinApp
{
public:
	CRoleDBServerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoleDBServerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CRoleDBServerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLEDBSERVER_H__75F3DE15_81FD_4E76_816E_5B2A29B160B2__INCLUDED_)
