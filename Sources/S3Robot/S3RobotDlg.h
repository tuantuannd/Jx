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
	CHyperLink	m_ctrlCopyRight;
	CListCtrl	m_ctlPlayerInfoList;
	CListCtrl	m_ctlCurrentMsgList;
	CComboBox	m_ctrlLoginServerComBox;
	BOOL	m_bCheckPostionRandom;
	BOOL	m_bCheckRepeatLastCmd;
	CString	m_csAccountName;
	CString	m_csPassword;
	int		m_nPositionSkillID;
	int		m_nPosition_x;
	int		m_nPosition_y;
	CString	m_csSpeakMessage;
	int		m_nPlayerAction;
	int		m_nPlayerLoginSelect;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS3RobotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

private:

	bool ( CS3RobotDlg::*ProcessArray[s2c_end])( const char *pMsg );

	void OnLogonSuccess();
	void OnWaitForLogon();

	inline void EnableCtrl( UINT nID, BOOL bEnable );
	void EnableInfoCtrl( BOOL bEnable = TRUE );

	void ChangeServer( int nPos );

	struct tagLoginInfo
	{
		char szAccountName[MAX_PATH];
		char szPassword[MAX_PATH];

		int	nPlayerIndex;

	}m_theLoginInfo;
	
	CImageList		*m_pImgLst_InfoNormalIcon;
	CImageList		*m_pImgLst_InfoSmallIcon;

	UINT			m_unAppTimer;
	bool			m_bActiveLife;

	MMRESULT		m_wGameTimerID;

	SERVER_MAP		m_ServerAddrMap;	
	string			m_strServerAddr;

	/*
	 * some funtion of processed protocol
	 */
	bool ProcessPing( const char *pMsg );
	bool Syncrolelist( const char *pMsg );
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CS3RobotDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnSendCommand();
	afx_msg void OnBtnUseDefaultSctfile();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnSctiptfileLogin();
	afx_msg void OnRadioPlayerFirst();
	afx_msg void OnRadioPlayerSecond();
	afx_msg void OnRadioActionMove();
	afx_msg void OnRadioActionSkill();
	afx_msg void OnRadioActionSpeak();
	afx_msg void OnCheckPositionRandom();
	afx_msg void OnCheckRepeatLastcommand();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeCtrlLoginserver();
	//}}AFX_MSG
	afx_msg HRESULT OnConnectCreate( WPARAM wParam, LPARAM lParam );
	afx_msg HRESULT OnConnectClose( WPARAM wParam, LPARAM lParam );
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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S3ROBOTDLG_H__7EFE14C3_D869_4410_ABD6_81B355BC4DFB__INCLUDED_)
