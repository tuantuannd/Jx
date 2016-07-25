// S3Robot.h : main header file for the S3ROBOT application
//

#if !defined(AFX_S3ROBOT_H__C8C89DB4_646C_4821_BC18_A62C1979CFDA__INCLUDED_)
#define AFX_S3ROBOT_H__C8C89DB4_646C_4821_BC18_A62C1979CFDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CS3RobotApp:
// See S3Robot.cpp for the implementation of this class
//

class CS3RobotApp : public CWinApp
{
public:
	CS3RobotApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS3RobotApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CS3RobotApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S3ROBOT_H__C8C89DB4_646C_4821_BC18_A62C1979CFDA__INCLUDED_)
