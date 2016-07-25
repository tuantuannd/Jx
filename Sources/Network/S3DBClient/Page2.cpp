// Page2.cpp : implementation file
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "Page2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPage2 property page

IMPLEMENT_DYNCREATE(CPage2, CPropertyPage)

CPage2::CPage2() : CPropertyPage(CPage2::IDD)
{
	//{{AFX_DATA_INIT(CPage2)
	//}}AFX_DATA_INIT
}

CPage2::~CPage2()
{
}

void CPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPage2)
	DDX_Control(pDX, IDC_DBUSERLIST, m_dbUserListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CPage2)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPage2 message handlers

BOOL CPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	static const CString itemNameArray[] =
	{
		_T("User name"),
		_T("Host addr"),
		_T("Priority"),
		_T("State"),
		_T("Last login time"),
		_T("Last logout time")
	};
	static const int itemWidthArray[] =
	{
		70, 75, 60, 55, 120, 120
	};
	for ( int i = 0; i < 6; i++ )
	{
		m_dbUserListCtrl.InsertColumn( i,
			itemNameArray[i], LVCFMT_LEFT, itemWidthArray[i] );
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPage2::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	if ( TRUE == bShow )
	{
		CWnd* pParent = GetParent();
		if ( NULL != pParent )
		{
			::SendMessage( pParent->m_hWnd, WM_TURNTOPAGE2, 0, 0 );
		}
	}
}
