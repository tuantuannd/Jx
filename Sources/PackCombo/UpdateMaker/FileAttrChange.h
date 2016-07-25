#if !defined(AFX_FILEATTRCHANGE_H__423CE7E6_DB61_45B1_8DF7_56A8C5C4AB3A__INCLUDED_)
#define AFX_FILEATTRCHANGE_H__423CE7E6_DB61_45B1_8DF7_56A8C5C4AB3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileAttrChange.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileAttrChange dialog

class CFileAttrChange : public CDialog
{
// Construction
public:
	CFileAttrChange(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFileAttrChange)
	enum { IDD = IDD_FILEATTRCHANGE_DIALOG };
	CString	m_strFileName;
	CString	m_strFilePath;
	CString	m_strBelong;
	CString	m_strCheckVersionMethod;
	CString	m_strUpdateFileFlag;
	CString	m_strLocalPath;
	CString	m_strUpdateFileMethod;
	CString	m_strRemotePath;
	CString	m_strUpdateRelative;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileAttrChange)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileAttrChange)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEATTRCHANGE_H__423CE7E6_DB61_45B1_8DF7_56A8C5C4AB3A__INCLUDED_)
