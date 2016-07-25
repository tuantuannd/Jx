// AddUserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "AddUserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddUserDlg dialog


CAddUserDlg::CAddUserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddUserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddUserDlg)
	m_cstrName = _T("");
	m_cstrPassword = _T("");
	m_cstrPasswordCheck = _T("");
	m_cstrPriority = _T("");
	//}}AFX_DATA_INIT
	m_siPriority = 0;
}


void CAddUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddUserDlg)
	DDX_Control(pDX, IDC_PRIORITY, m_priorityCtrl);
	DDX_Control(pDX, IDC_PASSWORDCHECK, m_passwordCheckCtrl);
	DDX_Control(pDX, IDC_PASSWORD, m_passwordCtrl);
	DDX_Control(pDX, IDC_NAME, m_nameCtrl);
	DDX_Text(pDX, IDC_NAME, m_cstrName);
	DDV_MaxChars(pDX, m_cstrName, 16);
	DDX_Text(pDX, IDC_PASSWORD, m_cstrPassword);
	DDV_MaxChars(pDX, m_cstrPassword, 16);
	DDX_Text(pDX, IDC_PASSWORDCHECK, m_cstrPasswordCheck);
	DDV_MaxChars(pDX, m_cstrPasswordCheck, 16);
	DDX_CBString(pDX, IDC_PRIORITY, m_cstrPriority);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddUserDlg, CDialog)
	//{{AFX_MSG_MAP(CAddUserDlg)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePassword)
	ON_EN_CHANGE(IDC_PASSWORDCHECK, OnChangePasswordcheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddUserDlg message handlers

void CAddUserDlg::OnOK() 
{
	// TODO: Add extra validation here
	if ( def_DBUSERNAME_MIN_LEN > m_cstrName.GetLength() )
	{
		MessageBox( "The length of name must be shorter than 16 and longer than 6" );
		m_nameCtrl.SetFocus();
		m_nameCtrl.SetSel( 0, -1 );
		return;
	}
	if ( def_DBPASSWORD_MIN_LEN > m_cstrPassword.GetLength() )
	{
		MessageBox( "The length of password must be shorter than 16 and longer than 6" );
		m_passwordCtrl.SetFocus();
		m_passwordCtrl.SetSel( 0, -1 );
		return;
	}
	if ( m_cstrPassword != m_cstrPasswordCheck )
	{
		MessageBox( "Checking password error." );
		m_passwordCheckCtrl.SetFocus();
		m_passwordCheckCtrl.SetSel( 0, -1 );
		return;
	}

	m_siPriority = m_priorityCtrl.GetCurSel();
	if ( ( m_siPriority < 0 )
		|| ( m_siPriority > 3 ) )
	{
		m_siPriority = 0;
		m_priorityCtrl.SetFocus();
		m_priorityCtrl.SetCurSel( m_siPriority );
		return;
	}

	CDialog::OnOK();
}

void CAddUserDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

void CAddUserDlg::OnChangeName() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CAddUserDlg::OnChangePassword() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CAddUserDlg::OnChangePasswordcheck() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

BOOL CAddUserDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_priorityCtrl.SetCurSel( m_siPriority );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
