// UpdateServerDlg.cpp : implementation file
//

#include "stdafx.h"

#include <objbase.h>
#include <initguid.h>

#include "UpdateServer.h"
#include "UpdateServerDlg.h"

#include "Library.h"

#include "protocol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

OnlineGameLib::Win32::CLibrary	g_theHeavenLib( "heaven.dll" );

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerDlg dialog

CUpdateServerDlg::CUpdateServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateServerDlg)
	m_nServerPort = 7653;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pServer = NULL;
}

void CUpdateServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateServerDlg)
	DDX_Control(pDX, IDC_LIST_USEDNOD, m_ctlUsedNode);
	DDX_Control(pDX, IDC_LIST_FREENODE, m_ctlFreeNode);
	DDX_Text(pDX, IDC_EDIT_SERVERPORT, m_nServerPort);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdateServerDlg, CDialog)
	//{{AFX_MSG_MAP(CUpdateServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CREATESERVICE, OnBtnCreateservice)
	ON_BN_CLICKED(IDC_BTN_INSERT, OnBtnInsert)
	ON_BN_CLICKED(IDC_BTN_REMOVE, OnBtnRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateServerDlg message handlers

BOOL CUpdateServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUpdateServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUpdateServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CUpdateServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CUpdateServerDlg::OnBtnCreateservice() 
{
	UpdateData( TRUE );

	if ( !m_nServerPort )
	{
		AfxMessageBox( "Please input a valid port" );

		return;
	}

	/*
	 * We open the heaven to wait for the rainbow
	 */
	pfnCreateServerInterface pServerFactroyFun = 
		( pfnCreateServerInterface )( g_theHeavenLib.GetProcAddress( _T( "CreateInterface" ) ) );

	IServerFactory *pServerFactory = NULL;

	if ( pServerFactroyFun && 
		SUCCEEDED( pServerFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( 20, 0, 20, 1024 );

		/*
		 * For gameserver
		 */
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pServer ) );
		
		pServerFactory->Release();		
	}

	if ( m_pServer )
	{
		m_pServer->Startup();

		m_pServer->RegisterMsgFilter( reinterpret_cast< void * >( this ), EventNotify );

		if ( SUCCEEDED( m_pServer->OpenService( INADDR_ANY, m_nServerPort ) ) )
		{
			::EnableWindow( GetDlgItem( IDC_EDIT_SERVERPORT )->GetSafeHwnd(), FALSE );
			
			::EnableWindow( GetDlgItem( IDC_BTN_CREATESERVICE )->GetSafeHwnd(), FALSE );
		}
	}	
}

void CUpdateServerDlg::OnOK() 
{
	CString csInfo;
	int nLen;

	::EnableWindow( GetDlgItem( IDC_LIST_USEDNOD )->GetSafeHwnd(), FALSE );

	for ( int i=0; i < m_ctlUsedNode.GetCount(); i++ )
	{
		nLen = m_ctlUsedNode.GetTextLen( i );
		m_ctlUsedNode.GetText( i, csInfo.GetBuffer( nLen ) );

		csInfo.ReleaseBuffer();
		
		const char *pInfo = csInfo.GetBuffer( 0 );

		stdMap::iterator it;
		if ( m_theClient.end() != ( it = m_theClient.find( pInfo ) ) )
		{
			UINT nID = (* it).second;

			tagWork work;
			work.cProtocol	= enumWork;
			work.nAction	= 0;

			if ( m_pServer )
			{
				m_pServer->SendData( nID, &work, sizeof( tagWork ) );
			}
		}
	}
}

void CUpdateServerDlg::OnCancel() 
{
		
	CDialog::OnCancel();
}

void CUpdateServerDlg::OnBtnInsert() 
{
	m_ctlFreeNode.GetCurSel();

	int nIndex = m_ctlFreeNode.GetCurSel();
	int nCount = m_ctlFreeNode.GetCount();

	if ( ( nIndex != LB_ERR ) && ( nCount > 0 ))
	{
		CString csInfo;
		int nLen = 0;

		nLen = m_ctlFreeNode.GetTextLen( nLen );
		m_ctlFreeNode.GetText( nIndex, csInfo.GetBuffer( nLen ) );
		csInfo.ReleaseBuffer();

		m_ctlFreeNode.DeleteString( nIndex );

		m_ctlUsedNode.AddString( csInfo.GetBuffer( 0 ) );
	}
}

void CUpdateServerDlg::OnBtnRemove() 
{
	m_ctlUsedNode.GetCurSel();

	int nIndex = m_ctlUsedNode.GetCurSel();
	int nCount = m_ctlUsedNode.GetCount();

	if ( ( nIndex != LB_ERR ) && ( nCount > 0 ))
	{
		CString csInfo;
		int nLen = 0;

		nLen = m_ctlUsedNode.GetTextLen( nLen );
		m_ctlUsedNode.GetText( nIndex, csInfo.GetBuffer( nLen ) );
		csInfo.ReleaseBuffer();

		m_ctlUsedNode.DeleteString( nIndex );

		m_ctlFreeNode.AddString( csInfo.GetBuffer( 0 ) );
	}
}

void __stdcall CUpdateServerDlg::EventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CUpdateServerDlg *pThis = reinterpret_cast< CUpdateServerDlg * >( lpParam );

	ASSERT( pThis );

	try
	{
		pThis->_EventNotify( ulnID, ulnEventType );
	}
	catch(...)
	{		
	}
}

void CUpdateServerDlg::_EventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	const char *pInfo = m_pServer->GetClientInfo( ulnID );

	if ( !pInfo )
	{
		return;
	}

	switch ( ulnEventType )
	{
	case enumClientConnectCreate:

		m_ctlFreeNode.AddString( pInfo );

		m_theClient[pInfo] = ulnID;

		break;

	case enumClientConnectClose:
		{
			int nIndex = 0;

			if ( LB_ERR != ( nIndex = m_ctlFreeNode.FindString( 0, pInfo ) ) )
			{
				m_ctlFreeNode.DeleteString( nIndex );
			}

			if ( LB_ERR != ( nIndex = m_ctlUsedNode.FindString( 0, pInfo ) ) )
			{
				m_ctlUsedNode.DeleteString( nIndex );
			}

			m_theClient.erase( pInfo );
		}
		break;
	}
}
