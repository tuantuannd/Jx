// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog


CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_cstrUserName = _T("");
	m_cstrPassword = _T("");
	//}}AFX_DATA_INIT
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Text(pDX, IDC_USERNAME, m_cstrUserName);
	DDV_MaxChars(pDX, m_cstrUserName, 16);
	DDX_Text(pDX, IDC_PASSWORD, m_cstrPassword);
	DDV_MaxChars(pDX, m_cstrPassword, 16);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePassword)
	ON_EN_CHANGE(IDC_USERNAME, OnChangeUsername)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers

void CLoginDlg::OnOK() 
{
	// TODO: Add extra validation here
	if ( m_cstrUserName.IsEmpty() )
	{
		MessageBox( _T("Please input user name"), NULL, MB_OK | MB_ICONASTERISK );
		CWnd* pWnd = GetDlgItem( IDC_USERNAME );
		if ( NULL != pWnd )
		{
			pWnd->SetFocus();
		}
		return;
	}
	if ( m_cstrPassword.IsEmpty() )
	{
		MessageBox( _T("Please input password"), NULL, MB_OK | MB_ICONASTERISK );
		CWnd* pWnd = GetDlgItem( IDC_PASSWORD );
		if ( NULL != pWnd )
		{
			pWnd->SetFocus();
		}
		return;
	}
	CDialog::OnOK();
}

void CLoginDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CLoginDlg::OnChangePassword() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CLoginDlg::OnChangeUsername() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}
