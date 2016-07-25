// RobotManagerDlg.h : header file
//

#if !defined(AFX_ROBOTMANAGERDLG_H__798B4E17_CDAB_4983_BD18_7003BABF60B5__INCLUDED_)
#define AFX_ROBOTMANAGERDLG_H__798B4E17_CDAB_4983_BD18_7003BABF60B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IServer.h"
#include "HeavenInterface.h"

#include "Library.h"
#include "Event.h"
#include "CriticalSection.h"

#include <list>

/////////////////////////////////////////////////////////////////////////////
// CRobotManagerDlg dialog

class CRobotManagerDlg : public CDialog
{
// Construction
public:
	CRobotManagerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CRobotManagerDlg)
	enum { IDD = IDD_ROBOTMANAGER_DIALOG };
	CString	m_csAccNameBase;
	int		m_nAccPerUser;
	int		m_nVariableBegin;
	int		m_nVariableEnd;
	CString	m_csIP1;
	CString	m_csIP2;
	UINT	m_nPort;
	CString	m_csGameServerIP;
	UINT	m_nGameSvrPort;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRobotManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	static void __stdcall SvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType );
	
private:

	typedef HRESULT ( __stdcall * pfnCreateServerInterface )( REFIID riid, void **ppv );
	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	void _ServerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType );

	static DWORD WINAPI ThreadFunc( LPVOID lpParam );
	DWORD Working();

	IServer		*m_pManagerServer;
	
	OnlineGameLib::Win32::CCriticalSection	m_csUL;

	typedef std::list< size_t > stdList;

	stdList		m_theUserList;

	HANDLE		m_hThread;

	OnlineGameLib::Win32::CEvent			m_hQuitThreadEvent;

	static OnlineGameLib::Win32::CLibrary	m_theHeavenLib;

	bool StartupServer();
	void DestroyServer();
	
	void EnableGroupCtrl( BOOL bEnable = TRUE );

	inline void EnableCtrl( UINT nID, BOOL bEnable );
	inline void SetText( UINT nID, const char *pText );

// Implementation
protected:
	HICON	m_hIcon;

	OnlineGameLib::Win32::CCriticalSection	m_csCTBI;

	int		m_nCurrentTaskBaseIndex;

	void *GetAccInfo( size_t &datalength );

	// Generated message map functions
	//{{AFX_MSG(CRobotManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnLoginAllplayer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline void CRobotManagerDlg::EnableCtrl( UINT nID, BOOL bEnable )
{
	CWnd *pWnd = GetDlgItem( nID );

	if ( pWnd && ::IsWindow( pWnd->GetSafeHwnd() ) )
	{
		pWnd->EnableWindow( bEnable );
	}
}

inline void CRobotManagerDlg::SetText( UINT nID, const char *pText )
{
	CWnd *pWnd = GetDlgItem( nID );

	if ( pWnd && ::IsWindow( pWnd->GetSafeHwnd() ) )
	{
		pWnd->SetWindowText( pText );
	}
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROBOTMANAGERDLG_H__798B4E17_CDAB_4983_BD18_7003BABF60B5__INCLUDED_)
