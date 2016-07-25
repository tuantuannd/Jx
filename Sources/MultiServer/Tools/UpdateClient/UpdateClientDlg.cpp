// UpdateClientDlg.cpp : implementation file
//

#include "stdafx.h"

#include <objbase.h>
#include <initguid.h>

#include "UpdateClient.h"
#include "UpdateClientDlg.h"

#include "Library.h"
#include "Macro.h"
#include "Buffer.h"

#include "..\\UpdateServer\\protocol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

OnlineGameLib::Win32::CLibrary	g_theRainbowLib( "rainbow.dll" );

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
// CUpdateClientDlg dialog

CUpdateClientDlg::CUpdateClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateClientDlg)
	m_nServerPort = 7653;
	m_csExePath = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pClient = NULL;
}

void CUpdateClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateClientDlg)
	DDX_Control(pDX, IDC_LIST_TASK, m_ctlTaskList);
	DDX_Control(pDX, IDC_IPADDRESS_SERVER, m_ctlServerIP);
	DDX_Text(pDX, IDC_EDIT_SERVER_PORT, m_nServerPort);
	DDX_Text(pDX, IDC_EDIT_EXE_PATH, m_csExePath);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdateClientDlg, CDialog)
	//{{AFX_MSG_MAP(CUpdateClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_REMOVE, OnBtnRemove)
	ON_BN_CLICKED(IDC_BTN_FIND, OnBtnFind)
	ON_BN_CLICKED(IDC_BTN_INSERT, OnBtnInsert)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateClientDlg message handlers

BOOL CUpdateClientDlg::OnInitDialog()
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
	
	m_ctlServerIP.SetAddress( 0x7F, 0, 0, 1 );

	::SetTimer( *this, 1000, 50, NULL );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUpdateClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CUpdateClientDlg::OnPaint() 
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

HCURSOR CUpdateClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CUpdateClientDlg::OnOK() 
{
	UpdateData( TRUE );

	BYTE cIP[4];
	m_ctlServerIP.GetAddress( cIP[0], cIP[1], cIP[2], cIP[3] );

	if ( !m_nServerPort )
	{
		AfxMessageBox( "Please input a valid IPAddress and port" );

		return;
	}

	CString csIP;
	csIP.Format( "%d.%d.%d.%d", cIP[0], cIP[1], cIP[2], cIP[3] );

	/*
	 * For the db-role server
	 */

	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClient ) );
		
		SAFE_RELEASE( pClientFactory );
	}

	if ( m_pClient )
	{
		m_pClient->Startup();

		m_pClient->RegisterMsgFilter( reinterpret_cast< void * >( this ), EventNotify );

		if ( SUCCEEDED( m_pClient->ConnectTo( csIP, m_nServerPort ) ) )
		{
			::EnableWindow( GetDlgItem( IDOK )->GetSafeHwnd(), FALSE );
			
			::EnableWindow( GetDlgItem( IDC_IPADDRESS_SERVER )->GetSafeHwnd(), FALSE );
			::EnableWindow( GetDlgItem( IDC_EDIT_SERVER_PORT )->GetSafeHwnd(), FALSE );
		}
	}
}

void CUpdateClientDlg::OnBtnInsert() 
{
	if ( !m_csExePath.IsEmpty() )
	{
		m_ctlTaskList.AddString( m_csExePath );
	}
}

void CUpdateClientDlg::OnBtnRemove() 
{
	m_ctlTaskList.GetCurSel();

	int nIndex = m_ctlTaskList.GetCurSel();
	int nCount = m_ctlTaskList.GetCount();

	if ( ( nIndex != LB_ERR ) && ( nCount > 0 ))
	{
		m_ctlTaskList.DeleteString( nIndex );
	}
}

void CUpdateClientDlg::OnCancel() 
{
	::KillTimer( *this, 1000 );
	
	CDialog::OnCancel();
}

void CUpdateClientDlg::OnBtnFind() 
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name

	szFile[0] = '\0';
	
	// Initialize OPENFILENAME
	ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
	
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = *this;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "Bat file\0*.bat\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if ( GetOpenFileName( &ofn ) == TRUE ) 
	{
		m_csExePath = ofn.lpstrFile;

		UpdateData( FALSE );
	}
}

void __stdcall CUpdateClientDlg::EventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType )
{
	CUpdateClientDlg *pThis = reinterpret_cast< CUpdateClientDlg * >( lpParam );

	ASSERT( pThis );

	try
	{
		pThis->_EventNotify( ulnEventType );
	}
	catch(...)
	{
	}
}

void CUpdateClientDlg::_EventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		break;

	case enumServerConnectClose:

		break;
	}
}

void CUpdateClientDlg::OnTimer(UINT nIDEvent) 
{
	if ( m_pClient )
	{
		size_t datalength = 0;
		const void *pData = m_pClient->GetPackFromServer( datalength );

		if ( pData && datalength )
		{
			BYTE cProtocol = *( const char *)pData;

			switch( cProtocol )
			{
			case enumWork:

				Work();				
				break;

			default:
				break;
			}
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CUpdateClientDlg::Work()
{
	CString str, str2;
	int nLen;
	
	for ( int i=0; i < m_ctlTaskList.GetCount(); i++)
	{
		nLen = m_ctlTaskList.GetTextLen( i );

		m_ctlTaskList.GetText( i, str.GetBuffer( nLen ) );
		str.ReleaseBuffer();
		
		char *pData = str.GetBuffer(0);

		if ( !pData )
		{
			continue;
		}
		
		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		
		// Set up the start up info struct.
		ZeroMemory(&si,sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;
		// Use this if you want to hide the child:
		//     si.wShowWindow = SW_HIDE;
		// Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
		// use the wShowWindow flags.
		
		
		CreateProcess(NULL,pData,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
		
		// Set global child process handle to cause threads to exit.
		
		// Close any unnecessary handles.
		CloseHandle(pi.hThread);		
	}
}