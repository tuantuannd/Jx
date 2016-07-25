// UpdateClientDlg.h : header file
//

#if !defined(AFX_UPDATECLIENTDLG_H__A53C0157_CAD3_4A79_AEC5_F4D0A6951CB3__INCLUDED_)
#define AFX_UPDATECLIENTDLG_H__A53C0157_CAD3_4A79_AEC5_F4D0A6951CB3__INCLUDED_

#include "IClient.h"
#include "RainbowInterface.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CUpdateClientDlg dialog

class CUpdateClientDlg : public CDialog
{
// Construction
public:
	CUpdateClientDlg(CWnd* pParent = NULL);	// standard constructor

	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	static void __stdcall EventNotify( LPVOID lpParam, const unsigned long &ulnEventType );

// Dialog Data
	//{{AFX_DATA(CUpdateClientDlg)
	enum { IDD = IDD_UPDATECLIENT_DIALOG };
	CListBox	m_ctlTaskList;
	CIPAddressCtrl	m_ctlServerIP;
	UINT	m_nServerPort;
	CString	m_csExePath;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	void _EventNotify( const unsigned long &ulnEventType );

	IClient		*m_pClient;

	void Work();

	// Generated message map functions
	//{{AFX_MSG(CUpdateClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnBtnRemove();
	virtual void OnCancel();
	afx_msg void OnBtnFind();
	afx_msg void OnBtnInsert();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATECLIENTDLG_H__A53C0157_CAD3_4A79_AEC5_F4D0A6951CB3__INCLUDED_)
