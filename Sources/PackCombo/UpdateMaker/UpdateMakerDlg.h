// UpdateMakerDlg.h : header file
//

#if !defined(AFX_UPDATEMAKERDLG_H__CC071647_D40E_4D97_91A9_6819CA5F86B6__INCLUDED_)
#define AFX_UPDATEMAKERDLG_H__CC071647_D40E_4D97_91A9_6819CA5F86B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CUpdateMakerDlg dialog
#include <STRING>

using namespace std;
class CUpdateMakerDlg : public CDialog
{
// Construction
public:
	CUpdateMakerDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	//{{AFX_DATA(CUpdateMakerDlg)
	enum { IDD = IDD_UPDATEMAKER_DIALOG };
	CListCtrl	m_UpFile;
	CString	m_SaveFileDir;
	//}}AFX_DATA
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateMakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUpdateMakerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnUpFile();
	afx_msg void OnBtnSaveFile();
	afx_msg void OnListUpFileDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRun();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void RunProcess(char * CommandLine);
	void RunAutoBat(const string& DataPath,const string&  ToolsPath,const string& InfoPath,const string& IndexPath);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEMAKERDLG_H__CC071647_D40E_4D97_91A9_6819CA5F86B6__INCLUDED_)

