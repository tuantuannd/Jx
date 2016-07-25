// S3AccClientSample.h : main header file for the S3ACCCLIENTSAMPLE application
//

#if !defined(AFX_S3ACCCLIENTSAMPLE_H__94920028_958A_4DA2_B102_28FCCE9D84CF__INCLUDED_)
#define AFX_S3ACCCLIENTSAMPLE_H__94920028_958A_4DA2_B102_28FCCE9D84CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CS3AccClientSampleApp:
// See S3AccClientSample.cpp for the implementation of this class
//

class CS3AccClientSampleApp : public CWinApp
{
public:
	CS3AccClientSampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS3AccClientSampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CS3AccClientSampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S3ACCCLIENTSAMPLE_H__94920028_958A_4DA2_B102_28FCCE9D84CF__INCLUDED_)
