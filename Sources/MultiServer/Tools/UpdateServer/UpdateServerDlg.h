// UpdateServerDlg.h : header file
//

#if !defined(AFX_UPDATESERVERDLG_H__12CAA4AD_4990_47F0_8498_4A15F80F83D8__INCLUDED_)
#define AFX_UPDATESERVERDLG_H__12CAA4AD_4990_47F0_8498_4A15F80F83D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CriticalSection.h"
#include "tstring.h"
#include "Buffer.h"

#include "IServer.h"
#include "HeavenInterface.h"

#include <map>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerDlg dialog

class CUpdateServerDlg : public CDialog
{
// Construction
public:
	CUpdateServerDlg(CWnd* pParent = NULL);	// standard constructor

	typedef HRESULT ( __stdcall * pfnCreateServerInterface )( REFIID riid, void **ppv );

	static void __stdcall EventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType );

// Dialog Data
	//{{AFX_DATA(CUpdateServerDlg)
	enum { IDD = IDD_UPDATESERVER_DIALOG };
	CListBox	m_ctlUsedNode;
	CListBox	m_ctlFreeNode;
	UINT	m_nServerPort;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON	m_hIcon;

	IServer	*m_pServer;
	
	typedef map< string, UINT >	stdMap;
	stdMap	m_theClient;	

	void _EventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType );

	// Generated message map functions
	//{{AFX_MSG(CUpdateServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnCreateservice();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnInsert();
	afx_msg void OnBtnRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATESERVERDLG_H__12CAA4AD_4990_47F0_8498_4A15F80F83D8__INCLUDED_)
