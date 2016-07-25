#if !defined(AFX_PAGE1_H__1329B889_0105_4796_BC9E_BA67A6DC7193__INCLUDED_)
#define AFX_PAGE1_H__1329B889_0105_4796_BC9E_BA67A6DC7193__INCLUDED_

#include "ClientSocket.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Page1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPage1 dialog

class CPage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPage1)

// Construction
public:
	CPage1();
	~CPage1();
	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CPage1)
	enum { IDD = IDD_PAGE1 };
	CProgressCtrl	m_createProCtrl;
	CButton	m_changeBTCtrl;
	CButton	m_createBTCtrl;
	CButton	m_deleteBTCtrl;
	CButton	m_lockBTCtrl;
	CString	m_cstrLogPath;
	DWORD	m_dwDeleteFromID;
	DWORD	m_dwDeleteToID;
	CString	m_cstrDefaultPassword;
	CString	m_cstrDefaultAccRealName;
	DWORD	m_dwNumOfCreating;
	CString	m_cstrBaseAccount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bHasLockDatabase;
	// Generated message map functions
	//{{AFX_MSG(CPage1)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnChangelogpath();
	afx_msg void OnLockdatabase();
	afx_msg void OnDestroy();
	afx_msg void OnCreateaccount();
	afx_msg void OnDeleteaccount();
	afx_msg void OnChangeNumofcreating();
	afx_msg void OnChangeDefaultpassword();
	afx_msg void OnChangeDefaultrealname();
	afx_msg void OnChangeBaseaccount();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGE1_H__1329B889_0105_4796_BC9E_BA67A6DC7193__INCLUDED_)
