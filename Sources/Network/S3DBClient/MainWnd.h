#if !defined(AFX_MAINWND_H__A607E8B4_561C_4F60_BAFA_1428DDFD6626__INCLUDED_)
#define AFX_MAINWND_H__A607E8B4_561C_4F60_BAFA_1428DDFD6626__INCLUDED_

#include "ClientSocket.h"
#include "Page1.h"	// Added by ClassView
#include "Page2.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMainWnd

class CMainWnd : public CPropertySheet
{
	DECLARE_DYNAMIC(CMainWnd)
protected:
// Construction
public:
	CMainWnd(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMainWnd(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainWnd)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainWnd();

	// Generated message map functions
protected:
	BOOL QueryUserlist( const char* lpszUserName, const char* lpszPassword );
	HICON m_hIcon;
	CPage2 m_page2;
	CPage1 m_page1;
	CMenu m_menu;
	
	void InitClientSocket();
	void UpdateMenuState( BOOL bEnable = FALSE );

	//{{AFX_MSG(CMainWnd)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnAbout();
	afx_msg void OnLogin();
	afx_msg void OnLogout();
	afx_msg void OnAdduser();
	afx_msg void OnDeleteuser();
	afx_msg void OnPreferences();
	afx_msg void OnExit();
	afx_msg void OnDestroy();
	afx_msg void OnMainface();
	afx_msg void OnDatabaseuser();
	afx_msg void OnRefresh();
	//}}AFX_MSG
	virtual void OnIMEInputLangChangeRequest( WPARAM wParam, LPARAM lParam );
	virtual void OnTurntoPage2( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINWND_H__A607E8B4_561C_4F60_BAFA_1428DDFD6626__INCLUDED_)
