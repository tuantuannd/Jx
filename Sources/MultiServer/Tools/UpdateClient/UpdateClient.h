// UpdateClient.h : main header file for the UPDATECLIENT application
//

#if !defined(AFX_UPDATECLIENT_H__95656FF4_CAA6_4564_A5F7_706063D05085__INCLUDED_)
#define AFX_UPDATECLIENT_H__95656FF4_CAA6_4564_A5F7_706063D05085__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CUpdateClientApp:
// See UpdateClient.cpp for the implementation of this class
//

class CUpdateClientApp : public CWinApp
{
public:
	CUpdateClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CUpdateClientApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATECLIENT_H__95656FF4_CAA6_4564_A5F7_706063D05085__INCLUDED_)
