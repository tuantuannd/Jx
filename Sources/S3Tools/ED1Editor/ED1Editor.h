// ED1Editor.h : main header file for the ED1EDITOR application
//

#if !defined(AFX_ED1EDITOR_H__2BF70B37_D996_4B38_9077_93CDAB7BAECD__INCLUDED_)
#define AFX_ED1EDITOR_H__2BF70B37_D996_4B38_9077_93CDAB7BAECD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CED1EditorApp:
// See ED1Editor.cpp for the implementation of this class
//

class CED1EditorApp : public CWinApp
{
public:
	CED1EditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CED1EditorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CED1EditorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ED1EDITOR_H__2BF70B37_D996_4B38_9077_93CDAB7BAECD__INCLUDED_)
