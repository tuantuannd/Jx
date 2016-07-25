// Page1.cpp : implementation file
//

#include "stdafx.h"
#include "S3DBClient.h"
#include "Page1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CS3DBClientApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CPage1 property page

IMPLEMENT_DYNCREATE(CPage1, CPropertyPage)

CPage1::CPage1() : CPropertyPage(CPage1::IDD),
m_bHasLockDatabase( FALSE )
{
	//{{AFX_DATA_INIT(CPage1)
	m_cstrLogPath = _T("");
	m_dwDeleteFromID = 0;
	m_dwDeleteToID = 0;
	m_cstrDefaultPassword = _T("");
	m_cstrDefaultAccRealName = _T("");
	m_dwNumOfCreating = 1;
	m_cstrBaseAccount = _T("");
	//}}AFX_DATA_INIT
}

CPage1::~CPage1()
{
}

void CPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPage1)
	DDX_Control(pDX, IDC_CREATEPRO, m_createProCtrl);
	DDX_Control(pDX, IDC_CHANGELOGPATH, m_changeBTCtrl);
	DDX_Control(pDX, IDC_CREATEACCOUNT, m_createBTCtrl);
	DDX_Control(pDX, IDC_DELETEACCOUNT, m_deleteBTCtrl);
	DDX_Control(pDX, IDC_LOCKDATABASE, m_lockBTCtrl);
	DDX_Text(pDX, IDC_ACCOUNTLOGPATH, m_cstrLogPath);
	DDX_Text(pDX, IDC_DELETEFROMID, m_dwDeleteFromID);
	DDX_Text(pDX, IDC_DELETETOID, m_dwDeleteToID);
	DDX_Text(pDX, IDC_DEFAULTPASSWORD, m_cstrDefaultPassword);
	DDV_MaxChars(pDX, m_cstrDefaultPassword, 30);
	DDX_Text(pDX, IDC_DEFAULTREALNAME, m_cstrDefaultAccRealName);
	DDV_MaxChars(pDX, m_cstrDefaultAccRealName, 30);
	DDX_Text(pDX, IDC_NUMOFCREATING, m_dwNumOfCreating);
	DDV_MinMaxDWord(pDX, m_dwNumOfCreating, 1, 1000);
	DDX_Text(pDX, IDC_BASEACCOUNT, m_cstrBaseAccount);
	DDV_MaxChars(pDX, m_cstrBaseAccount, 29);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPage1, CPropertyPage)
	//{{AFX_MSG_MAP(CPage1)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHANGELOGPATH, OnChangelogpath)
	ON_BN_CLICKED(IDC_LOCKDATABASE, OnLockdatabase)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CREATEACCOUNT, OnCreateaccount)
	ON_BN_CLICKED(IDC_DELETEACCOUNT, OnDeleteaccount)
	ON_EN_CHANGE(IDC_NUMOFCREATING, OnChangeNumofcreating)
	ON_EN_CHANGE(IDC_DEFAULTPASSWORD, OnChangeDefaultpassword)
	ON_EN_CHANGE(IDC_DEFAULTREALNAME, OnChangeDefaultrealname)
	ON_EN_CHANGE(IDC_BASEACCOUNT, OnChangeBaseaccount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPage1 message handlers

BOOL CPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_cstrDefaultPassword = theApp.m_szDefaultAccPassword;
	m_cstrDefaultAccRealName = theApp.m_szDefaultAccRealName;
	m_cstrLogPath = theApp.m_szLogPath;
	UpdateData( FALSE );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPage1::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	UpdateUI();
}

void CPage1::UpdateUI()
{
	if ( NULL != GetSafeHwnd() )
	{
		if ( m_bHasLockDatabase )
		{
			CString cstrText;
			cstrText.LoadString( IDS_ACTIVATEBTTEXT );
			m_lockBTCtrl.SetWindowText( cstrText );
		}
		else
		{
			CString cstrText;
			cstrText.LoadString( IDS_LOCKBTTEXT );
			m_lockBTCtrl.SetWindowText( cstrText );
		}
		if ( theApp.m_bConnected )
		{
			m_lockBTCtrl.EnableWindow( TRUE );
			m_createBTCtrl.EnableWindow( TRUE );
			m_deleteBTCtrl.EnableWindow( TRUE );
		}
		else
		{
			m_lockBTCtrl.EnableWindow( FALSE );
			m_createBTCtrl.EnableWindow( FALSE );
			m_deleteBTCtrl.EnableWindow( FALSE );
		}
	}
}

void CPage1::OnChangelogpath() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg( TRUE );
	if ( IDOK == dlg.DoModal() )
	{
		m_cstrLogPath = dlg.GetPathName();
		UpdateData( FALSE );
	}
}

static DWORD WINAPI ActivateDBCallbackFun( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_ACTIVATEDATABASE | DBLOGIN_R_SUCCESS ) == dwMsgParam )
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

static DWORD WINAPI LockDBCallbackFun( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_LOCKDATABASE | DBLOGIN_R_SUCCESS ) == dwMsgParam )
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
void CPage1::OnLockdatabase() 
{
	// TODO: Add your control notification handler code here
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
		if ( !m_bHasLockDatabase )
		{
			dbLoginStruct.Param = DBLOGIN_A_LOCKDATABASE;
		}
		else
		{
			dbLoginStruct.Param = DBLOGIN_A_ACTIVATEDATABASE;
		}
		
		int iResult = 0;
		BOOL bResult;
		if ( !m_bHasLockDatabase )
		{
			bResult =
				theApp.m_pClientSocket->RemoteQuery( &dbLoginStruct,
				LockDBCallbackFun, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
		}
		else
		{
			bResult =
				theApp.m_pClientSocket->RemoteQuery( &dbLoginStruct,
				ActivateDBCallbackFun, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
		}
		if ( bResult )
		{
			CString cstrMsg;
			if ( 1 == iResult )			// Log in successfully
			{
				if ( !m_bHasLockDatabase )
				{
					m_bHasLockDatabase = TRUE;
				}
				else
				{
					m_bHasLockDatabase = FALSE;
				}
				UpdateUI();
			}
			else
			{
				if ( !m_bHasLockDatabase )
				{
					cstrMsg =
						_T("Failed to lock database.\r\nMaybe the net is busy, nplease try again later.");
				}
				else
				{
					cstrMsg =
						_T("Failed to activate database.\r\nMaybe the net is busy, please try again later.");
				}
				MessageBox( cstrMsg );
			}
		}
		else
		{
			MessageBox( _T("Failed to access database.\r\nMaybe the net is busy, please try again later."), NULL, MB_OK | MB_ICONASTERISK );
		}
	}
	else
	{
		MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
	}
}

static DWORD CALLBACK CreateAccountCallBack( DWORD dwPara1, DWORD dwPara2 )
{
	DWORD dwRet = 0;
	IBYTE* pRecvMsg = ( IBYTE* )dwPara1;
	int* piRet = ( int* )dwPara2;
	if ( ( NULL != pRecvMsg )
		|| ( NULL != piRet ) )
	{
		DWORD dwMsgParam;
		memcpy( &dwMsgParam, &( pRecvMsg[1+sizeof( DWORD )] ), sizeof( DWORD ) );
		if ( ( DBLOGIN_R_CREATEACCOUNT | DBLOGIN_R_SUCCESS ) == dwMsgParam )
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
void CPage1::OnCreateaccount() 
{
	// TODO: Add your control notification handler code here
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
		dbLoginStruct.Size = sizeof( _DBLOGINSTRUCT )
			+ LOGIN_ACCOUNT_MAX_LEN
			+ LOGIN_REALNAME_MAX_LEN
			+ LOGIN_PASSWORD_MAX_LEN;
		dbLoginStruct.Param = DBLOGIN_A_CREATEACCOUNT;
		
		IBYTE* pSendBuf = new IBYTE[dbLoginStruct.Size];
		FILE* pLogFile = fopen( theApp.m_szLogPath, "r" );
		if ( NULL == pLogFile )
		{
			pLogFile = fopen( theApp.m_szLogPath, "wb" );
		}
		else
		{
			fclose( pLogFile );
			pLogFile = fopen( theApp.m_szLogPath, "r+b" );
		}
		if ( NULL == pLogFile )
		{
			CString cstrMsg;
			cstrMsg.Format( _T("Failed to create account because\r\ncan not open log file \"%s\""), m_cstrLogPath );
			MessageBox( cstrMsg );
		}
		if ( NULL != pSendBuf )
		{
			fseek( pLogFile, 0, SEEK_END );

			memset( pSendBuf, 0, dbLoginStruct.Size );
			DWORD dwOffset = 0;
			memcpy( pSendBuf, &dbLoginStruct, sizeof( _DBLOGINSTRUCT ) );
			dwOffset += sizeof( _DBLOGINSTRUCT );
			memcpy( pSendBuf + dwOffset, m_cstrDefaultAccRealName, m_cstrDefaultAccRealName.GetLength() );
			dwOffset += LOGIN_REALNAME_MAX_LEN;
			memcpy( pSendBuf + dwOffset, m_cstrDefaultPassword, m_cstrDefaultPassword.GetLength() );
			dwOffset += LOGIN_PASSWORD_MAX_LEN;
			char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
			BOOL bHasPrintTitle = FALSE;
			m_createProCtrl.SetPos( 0 );
			for ( DWORD i = 0; i < m_dwNumOfCreating; i++ )
			{
				memset( szAccount, 0, LOGIN_ACCOUNT_MAX_LEN + 2 );
				sprintf( szAccount, "%s%d", m_cstrBaseAccount, i );
				szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
				memcpy( pSendBuf + dwOffset, szAccount, LOGIN_ACCOUNT_MAX_LEN );
				int iResult = 0;
				BOOL bResult =
					theApp.m_pClientSocket->RemoteQuery( ( _PDBLOGINSTRUCT )pSendBuf,
					CreateAccountCallBack, ( DWORD )( &iResult ), theApp.m_serverInfo.dwWaitTime );
				if ( bResult )
				{
					CString cstrMsg;
					if ( 1 == iResult )
					{
						if ( FALSE == bHasPrintTitle )
						{
							CTime time = CTime::GetCurrentTime();
							KPIPrintToFile( pLogFile,
								_T("[Begin created account at %d:%d, %d-%d-%d]\r\n"),
								time.GetHour(), time.GetMinute(), time.GetYear(), time.GetMonth(), time.GetDay() );
							bHasPrintTitle = TRUE;
						}
						KPIPrintToFile( pLogFile, _T("%s\t%s\t%s\r\n"),
							szAccount, m_cstrDefaultPassword, m_cstrDefaultAccRealName );
						double dwProPos = 100.0 * ( i + 1.0 ) / ( ( double )m_dwNumOfCreating );
						m_createProCtrl.SetPos( ( int )dwProPos );
					}
					else
					{
						break;
					}
				}
			}
int i;
			if ( 0 < i )
			{
				CTime time = CTime::GetCurrentTime();
				KPIPrintToFile( pLogFile,
					_T("[Completed to created %d account(s) at %d:%d, %d-%d-%d]\r\n"), i,
					time.GetHour(), time.GetMinute(), time.GetYear(), time.GetMonth(), time.GetDay() );
			}
			CString cstrMsg;
			cstrMsg.Format( _T("Created %d account(s)"), i );
			MessageBox( cstrMsg );
			m_createProCtrl.SetPos( 0 );
			delete []pSendBuf;
			pSendBuf = NULL;
		}
		else
		{
			MessageBox( _T("Failed to allocate memory.") );
		}
		if ( NULL != pLogFile )
		{
			fclose( pLogFile );
		}
	}
	else
	{
		MessageBox( _T("No availd socket"), NULL, MB_OK | MB_ICONASTERISK );
	}
}

void CPage1::OnDeleteaccount() 
{
	// TODO: Add your control notification handler code here
	
}

void CPage1::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	
	// TODO: Add your message handler code here
	_tcscpy( theApp.m_szDefaultAccPassword, m_cstrDefaultPassword );
	_tcscpy( theApp.m_szDefaultAccRealName, m_cstrDefaultAccRealName );
	_tcscpy( theApp.m_szLogPath, m_cstrLogPath );

	char szSetupFilePath[MAX_PATH+1];
	KPIGetExePath( szSetupFilePath, MAX_PATH );
	strcat( szSetupFilePath, "setup.ini" );
	KPIWritePrivateProfileString( _T("Account"), _T("realname"),
		theApp.m_szDefaultAccRealName, szSetupFilePath );
	KPIWritePrivateProfileString( _T("Account"), _T("password"),
		theApp.m_szDefaultAccPassword, szSetupFilePath );
	KPIWritePrivateProfileString( _T("Account"), _T("logpath"),
		theApp.m_szLogPath, szSetupFilePath );
}

void CPage1::OnChangeNumofcreating() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CPage1::OnChangeDefaultpassword() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CPage1::OnChangeDefaultrealname() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CPage1::OnChangeBaseaccount() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}
