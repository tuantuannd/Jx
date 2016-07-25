// MainWnd.cpp : implementation file
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "PerformanceDlg.h"
#include "LoginDlg.h"
#include "MainWnd.h"
#include "AddUserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CS3DBClientApp theApp;

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
// CMainWnd

IMPLEMENT_DYNAMIC(CMainWnd, CPropertySheet)

CMainWnd::CMainWnd(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	AddPage( &m_page1 );
	AddPage( &m_page2 );
}

CMainWnd::CMainWnd(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	AddPage( &m_page1 );
	AddPage( &m_page2 );
}

CMainWnd::~CMainWnd()
{
}


BEGIN_MESSAGE_MAP(CMainWnd, CPropertySheet)
	//{{AFX_MSG_MAP(CMainWnd)
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_ABOUT, OnAbout)
	ON_COMMAND(ID_LOGIN, OnLogin)
	ON_COMMAND(ID_LOGOUT, OnLogout)
	ON_COMMAND(ID_ADDUSER, OnAdduser)
	ON_COMMAND(ID_DELETEUSER, OnDeleteuser)
	ON_COMMAND(ID_PREFERENCES, OnPreferences)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_WM_DESTROY()
	ON_COMMAND(ID_MAINFACE, OnMainface)
	ON_COMMAND(ID_DATABASEUSER, OnDatabaseuser)
	ON_COMMAND(ID_REFRESH, OnRefresh)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INPUTLANGCHANGEREQUEST,OnIMEInputLangChangeRequest)
	ON_MESSAGE(WM_TURNTOPAGE2,OnTurntoPage2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWnd message handlers

BOOL CMainWnd::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();

	InitClientSocket();

	CTabCtrl* pTabCtrl = GetTabControl();
	CRect oldTabRc;
	CRect newTabRc;
	CRect rc;
	GetWindowRect( &rc );
	if ( NULL != pTabCtrl )
	{
		pTabCtrl->GetWindowRect( &oldTabRc );
	}
	m_menu.LoadMenu( IDR_MAINMENU );
	SetMenu( &m_menu );
	if ( NULL != pTabCtrl )
	{
		pTabCtrl->GetWindowRect( &newTabRc );
		rc.InflateRect( 0, abs( newTabRc.top - oldTabRc.top ) / 2 );
		MoveWindow( &rc );
	}
	// TODO: Add extra initialization here
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetActivePage( &m_page1 );

	UpdateMenuState();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainWnd::OnPaint() 
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
		CPropertySheet::OnPaint();
	}
}

void CMainWnd::OnAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CMainWnd::OnSysCommand(UINT nID, LPARAM lParam) 
{
	// TODO: Add your message handler code here and/or call default
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CPropertySheet::OnSysCommand(nID, lParam);
	}
}

HCURSOR CMainWnd::OnQueryDragIcon()
{
	return ( HCURSOR )m_hIcon;
}

static DWORD CALLBACK LoginCallback( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_LOGIN | DBLOGIN_R_SUCCESS ) == dwMsgParam )
		{
			*piRet = 1;
		}
		else if ( ( DBLOGIN_R_LOGIN | DBLOGIN_R_HASLOGGEDIN ) == dwMsgParam )
		{
			*piRet = -1;
		}
		else
		{
			*piRet = 0;
		}
	}
	return dwRet;
}
void CMainWnd::OnLogin() 
{
	// TODO: Add your command handler code here
	if ( theApp.m_bConnected )
	{
		return;
	}
	if ( NULL != theApp.m_pClientSocket )
	{
		CLoginDlg dlg;
		if ( IDOK == dlg.DoModal() )
		{
			_DBLOGINSTRUCT dbLoginStruct;
			memset( &dbLoginStruct, 0, sizeof( _DBLOGINSTRUCT ) );
			memcpy( dbLoginStruct.szUserName,
				dlg.m_cstrUserName,
				dlg.m_cstrUserName.GetLength() );
			theApp.m_cstrCurUsername = dlg.m_cstrUserName;
			memcpy( dbLoginStruct.szPassword,
				dlg.m_cstrPassword,
				dlg.m_cstrPassword.GetLength() );
			theApp.m_cstrCurPassword = dlg.m_cstrPassword;
			dbLoginStruct.Size = sizeof( _DBLOGINSTRUCT );
			dbLoginStruct.Param = DBLOGIN_A_LOGIN;

			int iResult = 0;
			BOOL bResult =
				theApp.m_pClientSocket->RemoteQuery( &dbLoginStruct,
				LoginCallback, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
			if ( bResult )
			{
				CString cstrMsg;
				if ( 1 == iResult )			// Log in successfully
				{
					cstrMsg = _T("Successful to log in.");
					UpdateMenuState( TRUE );
					OnRefresh();
				}
				else if ( -1 == iResult )	// Has logged in
				{
					//cstrMsg = _T("The same user have connected the database server from other machine.");
					//UpdateMenuState( FALSE );
					cstrMsg = _T("Successful to log in.");
					UpdateMenuState( TRUE );
					OnRefresh();
				}
				else
				{
					cstrMsg = _T("Failed to log in.\r\nMaybe the username or password you input is wrong.");
				}
				MessageBox( cstrMsg );
			}
			else
			{
				MessageBox( _T("Failed to log in"), NULL, MB_OK | MB_ICONASTERISK );
			}
		}
	}
	else
	{
		MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
	}
}

static DWORD CALLBACK LogoutCallback( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_LOGOUT | DBLOGIN_R_SUCCESS ) == dwMsgParam )
		{
			*piRet = 1;
		}
		else
		{
			*piRet = -1;
		}
	}
	return dwRet;
}
void CMainWnd::OnLogout() 
{
	// TODO: Add your command handler code here
	if ( !theApp.m_bConnected )
	{
		return;
	}
	if ( NULL != theApp.m_pClientSocket )
	{
		_DBLOGINSTRUCT dbLoginStruct;
		memset( &dbLoginStruct, 0, sizeof( _DBLOGINSTRUCT ) );
		memcpy( dbLoginStruct.szUserName,
			theApp.m_cstrCurUsername,
			theApp.m_cstrCurUsername.GetLength() );
		memcpy( dbLoginStruct.szPassword,
			theApp.m_cstrCurPassword,
			theApp.m_cstrCurPassword.GetLength() );
		dbLoginStruct.Size = sizeof( _DBLOGINSTRUCT );
		dbLoginStruct.Param = DBLOGIN_A_LOGOUT;
		
		int iResult = 0;
		BOOL bResult =
			theApp.m_pClientSocket->RemoteQuery( &dbLoginStruct,
			LogoutCallback, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
		if ( bResult )
		{
			CString cstrMsg;
			if ( 1 == iResult )			// Log in successfully
			{
				cstrMsg = _T("Successful to log out.");
				UpdateMenuState( FALSE );
				OnRefresh();
			}
			else
			{
				cstrMsg = _T("Failed to log out.\r\nMaybe the net is busy, please try again later.");
				MessageBox( cstrMsg );
			}
		}
		else
		{
			MessageBox( _T("Failed to log out"), NULL, MB_OK | MB_ICONASTERISK );
		}
	}
	else
	{
		MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
	}
}

static DWORD CALLBACK AddUserCallback( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_ADDDBUSER | DBLOGIN_R_SUCCESS ) == dwMsgParam )
		{
			*piRet = 1;
		}
		else if ( ( DBLOGIN_R_ADDDBUSER | DBLOGIN_R_HASNOENOUGHPRIORITY ) == dwMsgParam )
		{
			*piRet = -1;
		}
		else if ( ( DBLOGIN_R_ADDDBUSER | DBLOGIN_R_USERHASEXISTED ) == dwMsgParam )
		{
			*piRet = -2;
		}
		else
		{
			*piRet = 0;
		}
	}
	return dwRet;
}
void CMainWnd::OnAdduser() 
{
	// TODO: Add your command handler code here
	if ( !theApp.m_bConnected )
	{
		return;
	}
	if ( NULL != theApp.m_pClientSocket )
	{
		CAddUserDlg dlg;
		if ( IDOK == dlg.DoModal() )
		{
			_DBLOGINSTRUCT dbLoginStruct;
			memset( &dbLoginStruct, 0, sizeof( _DBLOGINSTRUCT ) );
			memcpy( dbLoginStruct.szUserName,
				theApp.m_cstrCurUsername,
				theApp.m_cstrCurUsername.GetLength() );
			memcpy( dbLoginStruct.szPassword,
				theApp.m_cstrCurPassword,
				theApp.m_cstrCurPassword.GetLength() );
			dbLoginStruct.Size =
				sizeof( _DBLOGINSTRUCT )
				+ def_DBUSERNAME_MAX_LEN
				+ def_DBPASSWORD_MAX_LEN
				+ sizeof( short int );
			dbLoginStruct.Param = DBLOGIN_A_ADDDBUSER;
			IBYTE* pSendData = new IBYTE[dbLoginStruct.Size];
			if ( NULL != pSendData )
			{
				memset( pSendData, 0, dbLoginStruct.Size );
				DWORD dwOffset = 0;
				memcpy( pSendData, &dbLoginStruct, sizeof( _DBLOGINSTRUCT ) );
				dwOffset += sizeof( _DBLOGINSTRUCT );
				memcpy( pSendData + dwOffset, dlg.m_cstrName, dlg.m_cstrName.GetLength() );
				dwOffset += def_DBUSERNAME_MAX_LEN;
				memcpy( pSendData + dwOffset, dlg.m_cstrPassword, dlg.m_cstrPassword.GetLength() );
				dwOffset += def_DBPASSWORD_MAX_LEN;
				memcpy( pSendData + dwOffset, &( dlg.m_siPriority ), sizeof( short int ) );
				dwOffset += sizeof( short int );
				int iResult = 0;
				BOOL bResult =
					theApp.m_pClientSocket->RemoteQuery( ( _PDBLOGINSTRUCT )pSendData,//&dbLoginStruct,
					AddUserCallback, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
				if ( bResult )
				{
					CString cstrMsg;
					if ( 1 == iResult )			// Log in successfully
					{
						OnRefresh();
					}
					else if ( -1 == iResult )
					{
						cstrMsg =
							_T("Has no enough priority to carry out this operation.");
						MessageBox( cstrMsg );
					}
					else if ( -2 == iResult )
					{
						cstrMsg.Format( _T("User \"%s\" has existed."), dlg.m_cstrName );
					}
					else
					{
						cstrMsg =
							_T("Failed to access database.\r\nMaybe the net is busy, please try again later.");
						MessageBox( cstrMsg );
					}
				}
				else
				{
					MessageBox( _T("Failed to access database.\r\nMaybe the net is busy, please try again later."), NULL, MB_OK | MB_ICONASTERISK );
				}
				delete []pSendData;
				pSendData = NULL;
			}
			else
			{
				MessageBox( _T("Failed to allocate memory.") );
			}
		}
	}
	else
	{
		MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
	}
}

static DWORD CALLBACK DeleteUserCallBack( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;//DBLOGIN_R_HASNOENOUGHPRIORITY
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_DELDBUSER | DBLOGIN_R_SUCCESS ) == dwMsgParam )
		{
			*piRet = 1;
		}
		else if ( ( DBLOGIN_R_DELDBUSER | DBLOGIN_R_HASNOENOUGHPRIORITY ) == dwMsgParam )
		{
			*piRet = -1;
		}
		else
		{
			*piRet = 0;
		}
	}
	return dwRet;
}
void CMainWnd::OnDeleteuser() 
{
	// TODO: Add your command handler code here
	if ( ( &m_page2 == GetActivePage() )
		&& theApp.m_bConnected )
	{
		UINT uSelectedCount = m_page2.m_dbUserListCtrl.GetSelectedCount();
		int  nItem = -1;
		
		if ( 1 == uSelectedCount )
		{
			nItem = m_page2.m_dbUserListCtrl.GetNextItem( nItem, LVNI_SELECTED );
			ASSERT(nItem != -1);
			CString cstrSelName = m_page2.m_dbUserListCtrl.GetItemText( nItem, 0 );

			if ( NULL != theApp.m_pClientSocket )
			{
				_DBLOGINSTRUCT dbLoginStruct;
				memset( &dbLoginStruct, 0, sizeof( _DBLOGINSTRUCT ) );
				memcpy( dbLoginStruct.szUserName,
					theApp.m_cstrCurUsername,
					theApp.m_cstrCurUsername.GetLength() );
				memcpy( dbLoginStruct.szPassword,
					theApp.m_cstrCurPassword,
					theApp.m_cstrCurPassword.GetLength() );
				dbLoginStruct.Size = sizeof( _DBLOGINSTRUCT ) + def_DBUSERNAME_MAX_LEN;
				dbLoginStruct.Param = DBLOGIN_A_DELDBUSER;
				IBYTE* pSendBuf = new IBYTE[dbLoginStruct.Size];
				memset( pSendBuf, 0, dbLoginStruct.Size );
				if ( NULL != pSendBuf )
				{
					memcpy( pSendBuf, &dbLoginStruct, sizeof( _DBLOGINSTRUCT ) );
					memcpy( pSendBuf + sizeof( _DBLOGINSTRUCT ), cstrSelName, cstrSelName.GetLength() );
					CString cstrMsg;
					int iResult = 0;
					BOOL bResult =
						theApp.m_pClientSocket->RemoteQuery( ( _PDBLOGINSTRUCT )pSendBuf,
						DeleteUserCallBack, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
					if ( bResult )
					{
						if ( 1 == iResult )
						{
							if ( theApp.m_cstrCurUsername == cstrSelName )
							{
								UpdateMenuState( FALSE );
							}
							OnRefresh();
						}
						else if ( -1 == iResult )
						{
							cstrMsg = ( _T("You have no enough priority to carry out this operation.") );
							MessageBox( cstrMsg );
						}
						else
						{
							cstrMsg.Format( _T("Failed to delete \"%s\".\r\nMaybe the net is busy, please try again later."), cstrSelName );
							MessageBox( cstrMsg );
						}
					}
					else
					{
						cstrMsg.Format( _T("Failed to delete \"%s\".\r\nMaybe the net is busy, please try again later."), cstrSelName );
						MessageBox( cstrMsg );
					}
					delete []pSendBuf;
					pSendBuf = NULL;
				}
				else
				{
					MessageBox( _T("Failed to allocate memory.") );
				}
			}
			else
			{
				MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
			}
		}
	}
}

void CMainWnd::OnPreferences() 
{
	// TODO: Add your command handler code here
	CPerformanceDlg dlg;
	dlg.SetAddress( theApp.m_serverInfo.szDBServerIP );
	dlg.m_siDBServerPort = theApp.m_serverInfo.siDBServerPort;
	dlg.m_siLocalPort = theApp.m_serverInfo.siLocalPort;
	dlg.m_dwWaitTime = theApp.m_serverInfo.dwWaitTime;
	if ( IDOK == dlg.DoModal() )
	{
		theApp.m_serverInfo.siDBServerPort = dlg.m_siDBServerPort;
		BYTE byField[4];
		dlg.GetAddress( byField[0], byField[1],
			byField[2], byField[3] );
		sprintf( theApp.m_serverInfo.szDBServerIP, "%d.%d.%d.%d",
			byField[0], byField[1], byField[2], byField[3] );
		theApp.m_serverInfo.siLocalPort = dlg.m_siLocalPort;
		theApp.m_serverInfo.dwWaitTime = dlg.m_dwWaitTime;
	}
}

void CMainWnd::OnExit() 
{
	// TODO: Add your command handler code here
	m_page1.OnCancel();
	m_page2.OnCancel();
	EndDialog( IDCANCEL );
}

void CMainWnd::OnDestroy() 
{
	CPropertySheet::OnDestroy();
	
	// TODO: Add your message handler code here
	char szSetupFilePath[MAX_PATH+1];
	KPIGetExePath( szSetupFilePath, MAX_PATH );
	strcat( szSetupFilePath, "setup.ini" );
	char szPort[32];
	sprintf( szPort, "%d", theApp.m_serverInfo.siLocalPort );
	KPIWritePrivateProfileString( _T("Local"), _T("port"), szPort, szSetupFilePath );
	sprintf( szPort, "%d", theApp.m_serverInfo.dwWaitTime );
	KPIWritePrivateProfileString( _T("Local"), _T("waittime"), szPort, szSetupFilePath );
	KPIWritePrivateProfileString( _T("DBServer"), _T("addr"), theApp.m_serverInfo.szDBServerIP, szSetupFilePath );
	sprintf( szPort, "%d", theApp.m_serverInfo.siDBServerPort );
	KPIWritePrivateProfileString( _T("DBServer"), _T("port"), szPort, szSetupFilePath );

	OnLogout();
	
	CClientSocket::ReleaseInstance();
}

void CMainWnd::UpdateMenuState( BOOL bEnable /* = FALSE  */ )
{
	theApp.m_bConnected = bEnable;
	if ( m_menu.GetSafeHmenu() )
	{
		m_menu.EnableMenuItem( ID_LOGIN, theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_LOGOUT, !theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_NEWACCOUNT, !theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_DELETEACCOUNT, !theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_ADDUSER, !theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_DELETEUSER, !theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_REFRESH, !theApp.m_bConnected );
		m_menu.EnableMenuItem( ID_PREFERENCES, theApp.m_bConnected );
	}
}

void CMainWnd::InitClientSocket()
{
	char szSetupFilePath[MAX_PATH+1];
	KPIGetExePath( szSetupFilePath, MAX_PATH );
	strcat( szSetupFilePath, "setup.ini" );
	char szPort[32];
	KPIGetPrivateProfileString( _T("Local"), _T("port"), _T("8000"),
		szPort, 32, szSetupFilePath );
	theApp.m_serverInfo.siLocalPort = atoi( szPort );
	KPIGetPrivateProfileString( _T("Local"), _T("waittime"), _T("6000"),
		szPort, 32, szSetupFilePath );
	theApp.m_serverInfo.dwWaitTime = atoi( szPort );
	KPIGetPrivateProfileString( _T("DBServer"), _T("addr"), _T(""),
		theApp.m_serverInfo.szDBServerIP, 16 * sizeof( TCHAR ), szSetupFilePath );
	KPIGetPrivateProfileString( _T("DBServer"), _T("port"), _T(""),
		szPort, 32, szSetupFilePath );
	theApp.m_serverInfo.siDBServerPort = atoi( szPort );
	
	theApp.m_pClientSocket = CClientSocket::Instance( &( theApp.m_serverInfo.siLocalPort ) );
	if ( NULL != theApp.m_pClientSocket )
	{
		theApp.m_pClientSocket->SetServerAddr( theApp.m_serverInfo.szDBServerIP,
			theApp.m_serverInfo.siDBServerPort );
		theApp.m_pClientSocket->Connect( theApp.m_serverInfo.szDBServerIP,
			theApp.m_serverInfo.siDBServerPort );
		theApp.m_pClientSocket->Start();
	}
}

void CMainWnd::OnIMEInputLangChangeRequest( WPARAM wParam, LPARAM lParam )
{
	ActivateKeyboardLayout( HKL( lParam ), wParam );
}

void CMainWnd::OnMainface() 
{
	// TODO: Add your command handler code here
	SetActivePage( &m_page1 );
}

void CMainWnd::OnDatabaseuser() 
{
	// TODO: Add your command handler code here
	SetActivePage( &m_page2 );
	if ( theApp.m_bConnected )
	{
		OnRefresh();
	}
}

void CMainWnd::OnRefresh() 
{
	// TODO: Add your command handler code here
	if ( m_page2.m_dbUserListCtrl.GetSafeHwnd() )
	//if ( GetActivePage() == &m_page2 )
	{
		if ( theApp.m_bConnected )
			QueryUserlist( theApp.m_cstrCurUsername, theApp.m_cstrCurPassword );
		else
			m_page2.m_dbUserListCtrl.DeleteAllItems();
	}
	if ( GetActivePage() == &m_page1 )
	{
		m_page1.UpdateUI();
	}
}

static DWORD CALLBACK QueryUserlistCallback( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvBuf = ( IBYTE* )dwPara1;
	CListCtrl* pListCtrl = ( CListCtrl* )dwPara2;
	if ( ( NULL != pRecvBuf )
		&& ( NULL != pListCtrl ) )
	{
		if ( pListCtrl->GetSafeHwnd() )
		{
			CClientSocket::_USERINFO item;
			DWORD dwSize = 0;
			DWORD dwUserListSize = 0;
			memcpy( &dwSize, &pRecvBuf[1], sizeof( DWORD ) );
			dwUserListSize = dwSize - sizeof( _DBLOGINSTRUCT ) - sizeof( GUID );
			if ( 0 == dwUserListSize % sizeof( CClientSocket::_USERINFO ) )
			{
				int iItemNum =
					dwUserListSize / sizeof( CClientSocket::_USERINFO );
				pListCtrl->DeleteAllItems();
				for ( int i = 0; i < iItemNum; i++ )
				{
					memset( &item, 0, sizeof( CClientSocket::_USERINFO ) );
					memcpy( &item,
						&pRecvBuf[1+sizeof( _DBLOGINSTRUCT )+sizeof( GUID )+i*sizeof( CClientSocket::_USERINFO )],
						sizeof( CClientSocket::_USERINFO ) );
					pListCtrl->InsertItem( i, item.szUserName );
					pListCtrl->SetItemText( i, 1, item.szHostAddr );
					CString cstrPriority;
					if ( 0 == item.siPriority )
					{
						cstrPriority = _T("Read");
					}
					else if ( 1 == item.siPriority )
					{
						cstrPriority = _T("Write");
					}
					else if ( 2 == item.siPriority )
					{
						cstrPriority = _T("Read-Write");
					}
					else if ( 3 == item.siPriority )
					{
						cstrPriority = _T("Administrator");
					}
					pListCtrl->SetItemText( i, 2, cstrPriority );
					CString cstrState;
					if ( TRUE == item.bLoggedin )
					{
						cstrState = _T("Connected");
					}
					else
					{
						cstrState = _T("Disconnected");
					}
					pListCtrl->SetItemText( i, 3, cstrState );
					pListCtrl->SetItemText( i, 4, item.szLastLoginTime );
					pListCtrl->SetItemText( i, 5, item.szLastLogoutTime );
				}
			}
		}
	}
	return dwRet;
}
BOOL CMainWnd::QueryUserlist(const char *lpszUserName, const char *lpszPassword)
{
	BOOL bRet = FALSE;
	if ( ( NULL == lpszUserName )
		|| ( NULL == lpszPassword ) )
	{
		bRet = FALSE;
		return bRet;
	}
	if ( ( 0 == lpszUserName[0] )
		|| ( 0 == lpszPassword[0] ) )
	{
		bRet = FALSE;
		return bRet;
	}
	if ( NULL != theApp.m_pClientSocket )
	{
		_DBLOGINSTRUCT dbLoginStruct;
		memset( &dbLoginStruct, 0, sizeof( _DBLOGINSTRUCT ) );
		memcpy( dbLoginStruct.szUserName,
			theApp.m_cstrCurUsername,
			theApp.m_cstrCurUsername.GetLength() );
		memcpy( dbLoginStruct.szPassword,
			theApp.m_cstrCurPassword,
			theApp.m_cstrCurPassword.GetLength() );
		dbLoginStruct.Size = sizeof( _DBLOGINSTRUCT );
		dbLoginStruct.Param = DBLOGIN_A_QUERYUSERLIST;
		
		CListCtrl* pListCtrl = &( m_page2.m_dbUserListCtrl );
		BOOL bResult =
			theApp.m_pClientSocket->RemoteQuery( &dbLoginStruct,
			QueryUserlistCallback, ( DWORD )( pListCtrl ), theApp.m_serverInfo.dwWaitTime );
		if ( bResult )
		{
			bRet = TRUE;
		}
		else
		{
			MessageBox( _T("Failed to query user list.\r\nMaybe the net is busy, please try again later."),
				NULL, MB_OK | MB_ICONASTERISK );
		}
	}
	else
	{
		MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
	}
	return bRet;
}

void CMainWnd::OnTurntoPage2( WPARAM wParam, LPARAM lParam )
{
	OnRefresh();
}
