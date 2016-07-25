// S3AccClientSampleDlg.h : header file
//

#if !defined(AFX_S3ACCCLIENTSAMPLEDLG_H__418103D6_C5F1_43C9_858C_87D9C79A3BA1__INCLUDED_)
#define AFX_S3ACCCLIENTSAMPLEDLG_H__418103D6_C5F1_43C9_858C_87D9C79A3BA1__INCLUDED_

#include "Sample.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CS3AccClientSampleDlg dialog

class CS3AccClientSampleDlg : public CDialog
{
// Construction
public:
	CS3AccClientSampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CS3AccClientSampleDlg)
	enum { IDD = IDD_S3ACCCLIENTSAMPLE_DIALOG };
	CComboBox	m_paramCtrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS3AccClientSampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CSample* m_pSample;

	// Generated message map functions
	//{{AFX_MSG(CS3AccClientSampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditchangeParam();
	afx_msg void OnSend();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S3ACCCLIENTSAMPLEDLG_H__418103D6_C5F1_43C9_858C_87D9C79A3BA1__INCLUDED_)
