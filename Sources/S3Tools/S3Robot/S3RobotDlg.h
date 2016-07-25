// S3RobotDlg.h : header file
//

#if !defined(AFX_S3ROBOTDLG_H__7EFE14C3_D869_4410_ABD6_81B355BC4DFB__INCLUDED_)
#define AFX_S3ROBOTDLG_H__7EFE14C3_D869_4410_ABD6_81B355BC4DFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HyperLink.h"
#include <mmsystem.h>

#include "kprotocol.h"
#include "KTabFile.h"

/*
 * disable warning C4786: symbol greater than 255 character,
 * okay to ignore
 */
#pragma warning(disable: 4786)

#include <vector>
#include <map>
#include <string>

using namespace std;

typedef map< int, string > SERVER_MAP;
/////////////////////////////////////////////////////////////////////////////
// CS3RobotDlg dialog

class CS3RobotDlg : public CDialog
{
// Construction
public:
	CS3RobotDlg(CWnd* pParent = NULL);	// standard constructor
	
	void PreProcess();
	void Process();

	void InitFuntionEntry();

// Dialog Data
	//{{AFX_DATA(CS3RobotDlg)
	enum { IDD = IDD_S3ROBOT_DIALOG };
	CComboBox	m_ctrlSkillListComBox;
	CHyperLink	m_ctrlCopyRight;
	CListCtrl	m_ctlPlayerInfoList;
	CListCtrl	m_ctlCurrentMsgList;
	CComboBox	m_ctrlLoginServerComBox;
	CString	m_csAccountName;
	CString	m_csPassword;
	int		m_nPosition_x;
	int		m_nPosition_y;
	CString	m_csSpeakMessage;
	int		m_nPlayerAction;
	int		m_nPlayerLoginSelect;
	BOOL	m_bRecordCurrentTime;
	BOOL	m_bAutoControl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS3RobotDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

private:

	bool ( CS3RobotDlg::*ProcessArray[s2c_end])( const char *pMsg );

	void OnLogonSuccess();
	void OnWaitForLogon();

	inline void EnableCtrl( UINT nID, BOOL bEnable );
	void EnableInfoCtrl( BOOL bEnable = TRUE, BOOL bToAll = FALSE );
	
	void ShowInfomation( LPCTSTR pInfo, int nImage = -1 );
	void ShowPing( DWORD dwPingValue );

	void SpeakWord( const char *pText, size_t size = 0 );
	void ToMove( int x, int y );
	void TodoSkill( int nSkillID, int x, int y );

	void AutoMove();

	inline bool IsEquidistant( DWORD x1, DWORD y1, DWORD x2, DWORD y2, int nError );

	void ChangeServer( int nPos );

	CImageList		*m_pImgLst_InfoNormalIcon;
	CImageList		*m_pImgLst_InfoSmallIcon;

	UINT			m_unAutoCtrlTimer;

	UINT			m_unAppTimer;
	bool			m_bActiveLife;

	MMRESULT		m_wGameTimerID;

	SERVER_MAP		m_ServerAddrMap;	
	string			m_strServerAddr;
	
	struct tagPlayerInfo
	{
		DWORD dwID;
		char szName[32];
		
		WORD wHealthMax;
		WORD wManaMax;

		DWORD dwCurPixel_target_x;
		DWORD dwCurPixel_target_y;

		DWORD dwCurPixel_x;
		DWORD dwCurPixel_y;

	}m_thePlayerInfo;

	KTabFile	m_tabFile;
	BOOL		m_bSkillTabLoad;

	/*
	 * some funtion of processed protocol
	 */
	bool ProcessPing( const char *pMsg );
	bool Syncrolelist( const char *pMsg );
	bool SyncEnd( const char *pMsg );	
	bool SyncCurPlayerInfo( const char *pMsg );
	bool SyncCurPlayerNormalInfo( const char *pMsg );
	bool GetPlayerChar( const char *pMsg );
	bool SyncNpc( const char *pMsg );
	bool SyncNpcNormal( const char *pMsg );
	bool SyncCurPlayerSkill( const char *pMsg );

	enum
	{
		SI_INFO = 0,
		SI_COR,
		SI_WARN,
		SI_WORLD
	};
	
// Implementation
protected:
	HICON m_hIcon;
	
	NOTIFYICONDATA	m_nidIconData;

	bool ShowTrayIcon();
	bool HideTrayIcon();

	// Generated message map functions
	//{{AFX_MSG(CS3RobotDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnSendCommand();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRadioPlayerFirst();
	afx_msg void OnRadioPlayerSecond();
	afx_msg void OnRadioActionMove();
	afx_msg void OnRadioActionSkill();
	afx_msg void OnRadioActionSpeak();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeCtrlLoginserver();
	afx_msg void OnAutoControl();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnStExit();
	afx_msg void OnStMinimize();
	afx_msg void OnStRestore();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg HRESULT OnConnectCreate( WPARAM wParam, LPARAM lParam );
	afx_msg HRESULT OnConnectClose( WPARAM wParam, LPARAM lParam );
	afx_msg HRESULT OnRebootRobot( WPARAM wParam, LPARAM lParam );
	afx_msg HRESULT OnLoginGame( WPARAM wParam, LPARAM lParam );
	afx_msg HRESULT OnTrayNotifyMsg( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnTaskBarCreated( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()
};

inline void CS3RobotDlg::EnableCtrl( UINT nID, BOOL bEnable )
{
	CWnd *pWnd = NULL;

	pWnd = GetDlgItem( nID );

	if ( pWnd && ::IsWindow( pWnd->GetSafeHwnd() ) )
	{
		pWnd->EnableWindow( bEnable );
	}
}

inline bool CS3RobotDlg::IsEquidistant( DWORD x1, DWORD y1, DWORD x2, DWORD y2, int nError )
{
	if ( ( ( x1 - x2 ) < nError || ( x2 - x1 ) < nError ) &&
		( ( y1 - y2 ) < nError || ( y2 - y1 ) < nError ) )
	{
		return true;
	}

	return false;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S3ROBOTDLG_H__7EFE14C3_D869_4410_ABD6_81B355BC4DFB__INCLUDED_)
