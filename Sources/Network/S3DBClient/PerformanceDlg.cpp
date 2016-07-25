// PerformanceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "PerformanceDlg.h"
#include "vector"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg dialog


CPerformanceDlg::CPerformanceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPerformanceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPerformanceDlg)
	m_siDBServerPort = 0;
	m_siLocalPort = 0;
	m_dwWaitTime = 0;
	//}}AFX_DATA_INIT
	memset( m_byField, 0, sizeof( BYTE ) );
}


void CPerformanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPerformanceDlg)
	DDX_Control(pDX, IDC_DBSERVER_ADDR, m_DBServerIPCtrl);
	DDX_Text(pDX, IDC_DBSERVER_PORT, m_siDBServerPort);
	DDV_MinMaxInt(pDX, m_siDBServerPort, 0, 10000);
	DDX_Text(pDX, IDC_LOCAL_PORT, m_siLocalPort);
	DDV_MinMaxInt(pDX, m_siLocalPort, 0, 10000);
	DDX_Text(pDX, IDC_WAITTIME, m_dwWaitTime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPerformanceDlg, CDialog)
	//{{AFX_MSG_MAP(CPerformanceDlg)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_DBSERVER_ADDR, OnFieldchangedDbserverAddr)
	ON_EN_CHANGE(IDC_DBSERVER_PORT, OnChangeDbserverPort)
	ON_EN_CHANGE(IDC_LOCAL_PORT, OnChangeLocalPort)
	ON_EN_CHANGE(IDC_WAITTIME, OnChangeWaittime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg message handlers

void CPerformanceDlg::OnOK() 
{
	// TODO: Add extra validation here
	m_DBServerIPCtrl.GetAddress( m_byField[0], m_byField[1],
		m_byField[2], m_byField[3] );
	CDialog::OnOK();
}

void CPerformanceDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CPerformanceDlg::SetAddress( LPCTSTR lpszAddress )
{
	if ( NULL != lpszAddress )
	{
		TCHAR* pAt = ( TCHAR* )lpszAddress;
		TCHAR* pAtTemp = pAt;
		size_t length = _tcslen( lpszAddress );
		int iLoop = 0;
		for ( size_t i = 0; i < length; i++ )
		{
			if ( _T('.') == pAt[i] )
			{
				pAt[i] = 0;
				m_byField[iLoop] = atoi( pAtTemp );
				pAt[i] = _T('.');
				pAtTemp = &( pAt[i+1] );
				iLoop++;
			}
			else if ( ( length - 1 ) == i )
			{
				m_byField[iLoop] = atoi( pAtTemp );
			}
		}
	}
}

BOOL CPerformanceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_DBServerIPCtrl.SetAddress( m_byField[0], m_byField[1],
		m_byField[2], m_byField[3] );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPerformanceDlg::GetAddress(BYTE &byField0, BYTE &byField1, BYTE &byField2, BYTE &byField3)
{
	byField0 = m_byField[0];
	byField1 = m_byField[1];
	byField2 = m_byField[2];
	byField3 = m_byField[3];
}

void CPerformanceDlg::OnFieldchangedDbserverAddr(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	*pResult = 0;
}

void CPerformanceDlg::OnChangeDbserverPort() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CPerformanceDlg::OnChangeLocalPort() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CPerformanceDlg::OnChangeWaittime() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
