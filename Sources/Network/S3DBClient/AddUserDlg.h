#if !defined(AFX_ADDUSERDLG_H__E8789E47_1840_429A_B6DB_F45B7170E071__INCLUDED_)
#define AFX_ADDUSERDLG_H__E8789E47_1840_429A_B6DB_F45B7170E071__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddUserDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddUserDlg dialog

class CAddUserDlg : public CDialog
{
// Construction
public:
	short int m_siPriority;
	CAddUserDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddUserDlg)
	enum { IDD = IDD_ADDUSER };
	CComboBox	m_priorityCtrl;
	CEdit	m_passwordCheckCtrl;
	CEdit	m_passwordCtrl;
	CEdit	m_nameCtrl;
	CString	m_cstrName;
	CString	m_cstrPassword;
	CString	m_cstrPasswordCheck;
	CString	m_cstrPriority;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddUserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddUserDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeName();
	afx_msg void OnChangePassword();
	afx_msg void OnChangePasswordcheck();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDUSERDLG_H__E8789E47_1840_429A_B6DB_F45B7170E071__INCLUDED_)
