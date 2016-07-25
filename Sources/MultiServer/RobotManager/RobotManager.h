// RobotManager.h : main header file for the ROBOTMANAGER application
//

#if !defined(AFX_ROBOTMANAGER_H__FB00B83E_6968_4950_B0B8_6E24C21C5AB1__INCLUDED_)
#define AFX_ROBOTMANAGER_H__FB00B83E_6968_4950_B0B8_6E24C21C5AB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRobotManagerApp:
// See RobotManager.cpp for the implementation of this class
//

class CRobotManagerApp : public CWinApp
{
public:
	CRobotManagerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRobotManagerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRobotManagerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROBOTMANAGER_H__FB00B83E_6968_4950_B0B8_6E24C21C5AB1__INCLUDED_)
