// RobotManagerDlg.cpp : implementation file
//

#include "stdafx.h"

#include <objbase.h>
#include <initguid.h>

#include "RobotManager.h"
#include "RobotManagerDlg.h"

#include "Macro.h"
#include "Buffer.h"

#include "protocol.h"
#include "struct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CPackager;

OnlineGameLib::Win32::CLibrary CRobotManagerDlg::m_theHeavenLib( "heaven.dll" );

/////////////////////////////////////////////////////////////////////////////
// CRobotManagerDlg dialog

CRobotManagerDlg::CRobotManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRobotManagerDlg::IDD, pParent)
	, m_pManagerServer( NULL )
	, m_hThread( NULL )
	, m_hQuitThreadEvent( NULL, true, false, "_QuitThreadEvent" )
	, m_nCurrentTaskBaseIndex( 0 )
{
	//{{AFX_DATA_INIT(CRobotManagerDlg)
	m_csAccNameBase = _T("Robot");
	m_nAccPerUser = 5;
	m_nVariableBegin = 0;
	m_nVariableEnd = 9999;
	m_csIP1 = _T("...");
	m_csIP2 = _T("...");
	m_nPort = 5096;
	m_csGameServerIP = _T("192.168.22.106");
	m_nGameSvrPort = 5622;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRobotManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRobotManagerDlg)
	DDX_Text(pDX, IDC_EDIT_ACCNAMEBASE, m_csAccNameBase);
	DDV_MaxChars(pDX, m_csAccNameBase, 5);
	DDX_Text(pDX, IDC_EDIT_ACCUSER, m_nAccPerUser);
	DDV_MinMaxInt(pDX, m_nAccPerUser, 0, 100);
	DDX_Text(pDX, IDC_EDIT_VARIABLE_BEGIN, m_nVariableBegin);
	DDV_MinMaxInt(pDX, m_nVariableBegin, 0, 9999);
	DDX_Text(pDX, IDC_EDIT_VARIABLE_END, m_nVariableEnd);
	DDV_MinMaxInt(pDX, m_nVariableEnd, 0, 9999);
	DDX_Text(pDX, IDC_EDIT_IP1, m_csIP1);
	DDX_Text(pDX, IDC_EDIT_IP2, m_csIP2);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDV_MinMaxUInt(pDX, m_nPort, 1, 9999);
	DDX_Text(pDX, IDC_EDIT_GAMESVR_IP, m_csGameServerIP);
	DDX_Text(pDX, IDC_EDIT_GAMESVR_PORT, m_nGameSvrPort);
	DDV_MinMaxUInt(pDX, m_nGameSvrPort, 1, 9999);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRobotManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CRobotManagerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOGIN_ALLPLAYER, OnBtnLoginAllplayer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRobotManagerDlg message handlers

BOOL CRobotManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRobotManagerDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRobotManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CRobotManagerDlg::OnOK() 
{
	EnableGroupCtrl( FALSE );

	UpdateData( TRUE );

	m_nCurrentTaskBaseIndex = m_nVariableBegin;

	if ( !StartupServer() )
	{
		EnableGroupCtrl( TRUE );
	}
}

void CRobotManagerDlg::OnCancel() 
{
	DestroyServer();

	UpdateData( FALSE );

	EnableGroupCtrl( TRUE );
	
	CDialog::OnCancel();
}

DWORD WINAPI CRobotManagerDlg::ThreadFunc( LPVOID lpParam )
{
	CRobotManagerDlg *pThis = ( CRobotManagerDlg * )lpParam;

	ASSERT( pThis );

	DWORD dwValue = 0;

	try
	{
		dwValue = pThis->Working();
	}
	catch(...)
	{}

	return dwValue;
}

DWORD CRobotManagerDlg::Working()
{
	if ( !m_pManagerServer )
	{
		return -1;
	}

	while ( !m_hQuitThreadEvent.Wait( 0 ) )
	{
		const void *pData = NULL;
		size_t datalength = 0;

		CCriticalSection::Owner locker( m_csUL );

		stdList::iterator it;

		for ( it = m_theUserList.begin(); it != m_theUserList.end(); it ++ )
		{
			size_t index = ( *it );

			pData = m_pManagerServer->GetPackFromClient( index, datalength );

			if ( !pData || 0 == datalength )
			{
				continue;
			}

			BYTE cProtocol = CPackager::Peek( pData );

			switch ( cProtocol )
			{
			case enumRequireTask:
				{
					size_t lenAccInfo = 0;
					char *pAccInfo = ( char * )GetAccInfo( lenAccInfo );
						
					if ( pAccInfo && lenAccInfo )
					{
						m_pManagerServer->SendData( index, pAccInfo, lenAccInfo );

						SAFE_DELETE( pAccInfo );
					}
				}				
				break;

			case enumRobotQuit:
				{
					tagRobotQuit*	RobotQuit = (tagRobotQuit*)pData;

					
				}

			default:				
				break;
			}
		}
		
		::Sleep( 1 );
	}

	return 0L;
}

void *CRobotManagerDlg::GetAccInfo( size_t &datalength )
{
	CCriticalSection::Owner locker( m_csCTBI );

	if ( m_nAccPerUser > 0 &&
		m_nCurrentTaskBaseIndex >= m_nVariableBegin &&
		( m_nCurrentTaskBaseIndex + m_nAccPerUser ) < m_nVariableEnd )
	{
		CString csAccName;

		int nAccBeginIndex = m_nCurrentTaskBaseIndex;
		int nAccEndIndex = m_nCurrentTaskBaseIndex + m_nAccPerUser;

		size_t nAccCount = nAccEndIndex - nAccBeginIndex;

		csAccName.Format( "%s%4.4d", m_csAccNameBase, 0 );

		size_t nNameLen = strlen( csAccName );

		datalength = sizeof( tagDspT_INFO ) + nAccCount * nNameLen;

		tagDspT_INFO *pDTI = ( tagDspT_INFO * )new char[ datalength ];

		ASSERT( pDTI );
		
		ZeroMemory( ( char * )pDTI, datalength );

		pDTI->cProtocol = enumDispatchTask;

		strcpy( pDTI->theGameSvrInfo.szIP, m_csGameServerIP );
		pDTI->theGameSvrInfo.nPort = m_nGameSvrPort;

		pDTI->nAccCount = nAccCount;
		pDTI->nNameLen	= nNameLen;
		pDTI->dwVersion = 1;

		char *pAccName = pDTI->szAccName;
		int nIndex = 0;

		for ( int i=nAccBeginIndex; i< nAccEndIndex; i ++ )
		{
			csAccName.Format( "%s%4.4d", m_csAccNameBase, i );

			memcpy( pAccName, csAccName, nNameLen );

			pAccName += nNameLen;
		}
		
		m_nCurrentTaskBaseIndex = nAccEndIndex;

		return ( void * )pDTI;
	}
		
	datalength = 0;
	
	return NULL;
}

bool CRobotManagerDlg::StartupServer()
{
	if ( m_pManagerServer )
	{
		return true;
	}

	bool ok = false;

	/*
	 * We open the heaven to wait for the rainbow
	 */
	pfnCreateServerInterface pServerFactroyFun = 
		( pfnCreateServerInterface )( m_theHeavenLib.GetProcAddress( _T( "CreateInterface" ) ) );

	IServerFactory *pServerFactory = NULL;

	if ( pServerFactroyFun && 
		SUCCEEDED( pServerFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( 1000, 10, 500, 1024 * 8 );

		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pManagerServer ) );

		pServerFactory->Release();		
	}

	if ( m_pManagerServer )
	{
		m_pManagerServer->Startup();

		m_pManagerServer->RegisterMsgFilter( reinterpret_cast< void * >( this ), SvrEventNotify );

		if ( SUCCEEDED( m_pManagerServer->OpenService( INADDR_ANY, m_nPort ) ) )
		{
			ok = true;
		}
	}

	DWORD dwThreadID = 0;

	HANDLE hThread = ::CreateThread( NULL, 0, ThreadFunc, ( void * )this, 0, &dwThreadID );

	SAFE_CLOSEHANDLE( hThread );

	return ok;
}

void CRobotManagerDlg::DestroyServer()
{
	m_hQuitThreadEvent.Set();

	/*
	 * Disconnect network and relase this resource
	 */
	if ( m_pManagerServer )
	{
		m_pManagerServer->CloseService();

		m_pManagerServer->Cleanup();
		
		SAFE_RELEASE( m_pManagerServer );
	}
}

void __stdcall CRobotManagerDlg::SvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CRobotManagerDlg *pThis = reinterpret_cast< CRobotManagerDlg * >( lpParam );

	ASSERT( pThis );

	try
	{
		pThis->_ServerEventNotify( ulnID, ulnEventType );
	}
	catch(...)
	{}
}

void CRobotManagerDlg::_ServerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CCriticalSection::Owner locker( m_csUL );

	switch ( ulnEventType )
	{
	case enumClientConnectCreate:

		m_theUserList.push_back( ulnID );
		
		break;

	case enumClientConnectClose:

		m_theUserList.remove( ulnID );
		
		break;
	}
}

void CRobotManagerDlg::EnableGroupCtrl( BOOL bEnable /*= TRUE*/ )
{
	EnableCtrl( IDC_EDIT_PORT, bEnable );
	EnableCtrl( IDC_EDIT_ACCNAMEBASE, bEnable );
	EnableCtrl( IDC_EDIT_VARIABLE_BEGIN, bEnable );
	EnableCtrl( IDC_EDIT_VARIABLE_END, bEnable );
	EnableCtrl( IDC_EDIT_ACCUSER, bEnable );

	EnableCtrl( IDOK, bEnable );

	EnableCtrl( IDC_EDIT_GAMESVR_IP, bEnable );
	EnableCtrl( IDC_EDIT_GAMESVR_PORT, bEnable );

	EnableCtrl( IDC_BTN_LOGIN_ALLPLAYER, !bEnable );
}

void CRobotManagerDlg::OnBtnLoginAllplayer() 
{
	static bool bLogin = true;

	EnableCtrl( IDC_BTN_LOGIN_ALLPLAYER, FALSE );
	
	{
		CCriticalSection::Owner locker( m_csUL );
		
		size_t datalength = 0;
		
		stdList::iterator it;
		
		for ( it = m_theUserList.begin(); it != m_theUserList.end(); it ++ )
		{
			size_t index = ( *it );
			
			//m_pManagerServer->SendData( index, "test", 4 );			
		}
		
	}

	if ( bLogin )
	{
		SetText( IDC_BTN_LOGIN_ALLPLAYER, "Let's all player to logout server" );
	}
	else
	{
		SetText( IDC_BTN_LOGIN_ALLPLAYER, "Let's all player to login server" );
	}

	bLogin = !bLogin;
		
	EnableCtrl( IDC_BTN_LOGIN_ALLPLAYER, TRUE );		
}
