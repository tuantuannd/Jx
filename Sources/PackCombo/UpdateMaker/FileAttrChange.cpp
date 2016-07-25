// FileAttrChange.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateMaker.h"
#include "FileAttrChange.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileAttrChange dialog


CFileAttrChange::CFileAttrChange(CWnd* pParent /*=NULL*/)
	: CDialog(CFileAttrChange::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileAttrChange)
	m_strFileName = _T("");
	m_strFilePath = _T("");
	m_strBelong = _T("");
	m_strCheckVersionMethod = _T("");
	m_strUpdateFileFlag = _T("");
	m_strLocalPath = _T("");
	m_strUpdateFileMethod = _T("");
	m_strRemotePath = _T("");
	m_strUpdateRelative = _T("");
	//}}AFX_DATA_INIT
}


void CFileAttrChange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileAttrChange)
	DDX_Text(pDX, IDC_TXT1, m_strFileName);
	DDX_Text(pDX, IDC_TXT2, m_strFilePath);
	DDX_Text(pDX, IDC_TXT3, m_strBelong);
	DDX_Text(pDX, IDC_TXT4, m_strCheckVersionMethod);
	DDX_Text(pDX, IDC_TXT5, m_strUpdateFileFlag);
	DDX_Text(pDX, IDC_TXT6, m_strLocalPath);
	DDX_Text(pDX, IDC_TXT7, m_strUpdateFileMethod);
	DDX_Text(pDX, IDC_TXT8, m_strRemotePath);
	DDX_Text(pDX, IDC_TXT9, m_strUpdateRelative);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileAttrChange, CDialog)
	//{{AFX_MSG_MAP(CFileAttrChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileAttrChange message handlers

void CFileAttrChange::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();
	CDialog::OnOK();
}
