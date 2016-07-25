// UpdateServer.h : main header file for the UPDATESERVER application
//

#if !defined(AFX_UPDATESERVER_H__A9581534_0BD9_4380_A273_4A8D83D4C944__INCLUDED_)
#define AFX_UPDATESERVER_H__A9581534_0BD9_4380_A273_4A8D83D4C944__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerApp:
// See UpdateServer.cpp for the implementation of this class
//

class CUpdateServerApp : public CWinApp
{
public:
	CUpdateServerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateServerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CUpdateServerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATESERVER_H__A9581534_0BD9_4380_A273_4A8D83D4C944__INCLUDED_)
