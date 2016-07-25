#if !defined(AFX_PERFORMANCEDLG_H__B329E7E2_1429_4CB4_9D57_4F6880624BA6__INCLUDED_)
#define AFX_PERFORMANCEDLG_H__B329E7E2_1429_4CB4_9D57_4F6880624BA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PerformanceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg dialog

class CPerformanceDlg : public CDialog
{
// Construction
public:
	void GetAddress( BYTE& byField0, BYTE& byField1, BYTE& byField2, BYTE& byField3 );
	void SetAddress( LPCTSTR lpszAddress );
	CPerformanceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPerformanceDlg)
	enum { IDD = IDD_PREFERENCES };
	CIPAddressCtrl	m_DBServerIPCtrl;
	short	m_siDBServerPort;
	short	m_siLocalPort;
	DWORD	m_dwWaitTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPerformanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BYTE m_byField[4];

	// Generated message map functions
	//{{AFX_MSG(CPerformanceDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnFieldchangedDbserverAddr(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDbserverPort();
	afx_msg void OnChangeLocalPort();
	afx_msg void OnChangeWaittime();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERFORMANCEDLG_H__B329E7E2_1429_4CB4_9D57_4F6880624BA6__INCLUDED_)
