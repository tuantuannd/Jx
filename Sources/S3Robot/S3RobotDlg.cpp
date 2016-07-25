// S3RobotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "S3Robot.h"
#include "S3RobotDlg.h"

#include "KEngine.h"
#include "KIniFile.h"

#include <process.h>    /* _beginthread, _endthread */

#include "jxclient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "winmm.lib")

const UINT g_unAppTimerEventID = 100;
const UINT g_unGameDelay = 50;

const char g_cszPlayerInfo[][MAX_PATH] = {
						"Player ID",
						"Current Position",
						"Current Mana",
						"Current Heath",
						"Server Info",
						"Ping Value",
						"Application Timer",
						""		// must insert this end of flag
						};

enum enumPlayerInfo
{
	enumPlayerID = 0,
	enumCurrentPosition,
	enumCurrentMana,
	enumCurrentHeath,
	enumServerAddress,
	enumPingValue,
	enumApplicationTimer
};

#define WM_CONNECT_CREATE	WM_USER + 0x100
#define WM_CONNECT_CLOSE	WM_USER + 0x110

void CALLBACK GameLoopFun(
			UINT uTimerID, 
			UINT uMsg, 
			DWORD_PTR dwUser, 
			DWORD_PTR dw1, 
			DWORD_PTR dw2 )
{
	CS3RobotDlg *pDlg = reinterpret_cast< CS3RobotDlg * >( dwUser );
	
	if ( pDlg )
	{
		pDlg->PreProcess();
		pDlg->Process();
	}
}

void CALLBACK EventNotify( DWORD dwEventType )
{
	HWND hWndMainDlg = AfxGetApp()->m_pMainWnd->GetSafeHwnd();

	if ( NULL == hWndMainDlg || FALSE == ::IsWindow( hWndMainDlg ) )
	{
		return;
	}

	switch ( dwEventType )
	{
	case SERVER_CONNECT_CREATE:
		PostMessage( hWndMainDlg, WM_CONNECT_CREATE, 0, 0 );
		break;

	case SERVER_CONNECT_CLOSE:
		PostMessage( hWndMainDlg, WM_CONNECT_CLOSE, 0, 0 );
		break;
	}
}

int	g_nMsgProtocolSize[MAX_PROTOCOL_NUM] = 
{
	-1,							// s2c_login,
	-1,							// s2c_logout,
	sizeof(BYTE),				// s2c_syncend,
	sizeof(CURPLAYER_SYNC),		// s2c_synccurplayer,
	-1,							// s2c_synccurplayerskill
	sizeof(CURPLAYER_NORMAL_SYNC),// s2c_synccurplayernormal
	-1,							// s2c_newplayer,
	-1,							// s2c_removeplayer,
	sizeof(WORLD_SYNC),			// s2c_syncworld,
	sizeof(PLAYER_SYNC),		// s2c_syncplayer,
	sizeof(PLAYER_NORMAL_SYNC),	// s2c_syncplayermin,
	sizeof(NPC_SYNC),			// s2c_syncnpc,
	sizeof(NPC_NORMAL_SYNC),	// s2c_syncnpcmin,

	sizeof(OBJ_SYNC_ADD),		// s2c_objadd,
	sizeof(OBJ_SYNC_STATE),		// s2c_syncobjstate,
	sizeof(OBJ_SYNC_DIR),		// s2c_syncobjdir,
	sizeof(OBJ_SYNC_REMOVE),	// s2c_objremove,
	sizeof(OBJ_SYNC_TRAP_ACT),	// s2c_objTrapAct,

	sizeof(NPC_REMOVE_SYNC),	// s2c_npcremove,
	sizeof(NPC_WALK_SYNC),		// s2c_npcwalk,
	sizeof(NPC_RUN_SYNC),		// s2c_npcrun,
	-1,							// s2c_npcattack,
	-1,							// s2c_npcmagic,
	-1,							// s2c_npcjump,
	-1,							// s2c_npctalk,
	sizeof(NPC_HURT_SYNC),		// s2c_npchurt,
	sizeof(NPC_DEATH_SYNC),		// s2c_npcdeath,
	sizeof(NPC_CHGCURCAMP_SYNC),// s2c_npcchgcurcamp,
	sizeof(NPC_CHGCAMP_SYNC),	// s2c_npcchgcamp,
	sizeof(NPC_SKILL_SYNC),		// s2c_skillcast,
	-1,							// s2c_playertalk,
	sizeof(PLAYER_EXP_SYNC),	// s2c_playerexp,

	sizeof(PLAYER_SEND_TEAM_INFO),			// s2c_playersendteaminfo,
	sizeof(PLAYER_SEND_SELF_TEAM_INFO),		// s2c_playersendselfteaminfo,
	sizeof(PLAYER_APPLY_TEAM_INFO_FALSE),	// s2c_playerapplyteaminfofalse,
	sizeof(PLAYER_SEND_CREATE_TEAM_SUCCESS),// s2c_teamcreatesuccess,
	sizeof(PLAYER_SEND_CREATE_TEAM_FALSE),	// s2c_teamcreatefalse,
	sizeof(PLAYER_TEAM_OPEN_CLOSE),			// s2c_playersendteamopenclose,
	sizeof(PLAYER_APPLY_ADD_TEAM),			// s2c_teamgetapply,
	sizeof(PLAYER_TEAM_ADD_MEMBER),			// s2c_playersendteamaddmember,
	sizeof(PLAYER_LEAVE_TEAM),				// s2c_playerleaveteam,
	sizeof(PLAYER_TEAM_CHANGE_CAPTAIN),		// s2c_playerteamchangecaptain,
	sizeof(PLAYER_FACTION_DATA),			// s2c_playerfactiondata,
	sizeof(PLAYER_LEAVE_FACTION),			// s2c_playerleavefaction,
	sizeof(PLAYER_FACTION_SKILL_LEVEL),		// s2c_playerfactionskilllevel,
	-1,//sizeof(PLAYER_SEND_CHAT_SYNC),			// s2c_playersendchat,
	sizeof(PLAYER_LEAD_EXP_SYNC),			// s2c_playersyncleadexp
	sizeof(PLAYER_LEVEL_UP_SYNC),			// s2c_playerlevelup
	sizeof(PLAYER_TEAMMATE_LEVEL_SYNC),		// s2c_playerteammatelevel
	sizeof(PLAYER_ATTRIBUTE_SYNC),			// s2c_playersyncattribute
	sizeof(PLAYER_SKILL_LEVEL_SYNC),		// s2c_playerskilllevel
	sizeof(ITEM_SYNC),						// s2c_syncitem
	sizeof(ITEM_REMOVE_SYNC),				// s2c_removeitem
	sizeof(PLAYER_MONEY_SYNC),				// s2c_syncmoney
	sizeof(PLAYER_MOVE_ITEM_SYNC),			// s2c_playermoveitem
	-1,										// s2c_playershowui
	sizeof(CHAT_APPLY_ADD_FRIEND_SYNC),		// s2c_chatapplyaddfriend
	sizeof(CHAT_ADD_FRIEND_SYNC),			// s2c_chataddfriend
	sizeof(CHAT_REFUSE_FRIEND_SYNC),		// s2c_chatrefusefriend
	sizeof(CHAT_ADD_FRIEND_FAIL_SYNC),		// s2c_chataddfriendfail
	sizeof(CHAT_LOGIN_FRIEND_NONAME_SYNC),	// s2c_chatloginfriendnoname
	sizeof(CHAT_LOGIN_FRIEND_NAME_SYNC),	// s2c_chatloginfriendname
	sizeof(CHAT_ONE_FRIEND_DATA_SYNC),		// s2c_chatonefrienddata
	sizeof(CHAT_FRIEND_ONLINE_SYNC),		// s2c_chatfriendinline
	sizeof(CHAT_DELETE_FRIEND_SYNC),		// s2c_chatdeletefriend
	sizeof(CHAT_FRIEND_OFFLINE_SYNC),		// s2c_chatfriendoffline
	sizeof(ROLE_LIST_SYNC),					// s2c_syncrolelist
	sizeof(TRADE_CHANGE_STATE_SYNC),		// s2c_tradechangestate
	-1, // NPC_SET_MENU_STATE_SYNC	s2c_npcsetmenustate
	sizeof(TRADE_MONEY_SYNC),				// s2c_trademoneysync
	sizeof(TRADE_DECISION_SYNC),			// s2c_tradedecision
	-1, // sizeof(CHAT_SCREENSINGLE_ERROR_SYNC) s2c_chatscreensingleerror
	sizeof(NPC_SYNC_STATEINFO),				//	s2c_syncnpcstate,
	-1,	// sizeof(TEAM_INVITE_ADD_SYNC)	s2c_teaminviteadd
	sizeof(TRADE_PRESS_OK_SYNC),			// s2c_tradepressoksync
	sizeof(PING_COMMAND),					// s2c_ping
};
/////////////////////////////////////////////////////////////////////////////
// CS3RobotDlg dialog

CS3RobotDlg::CS3RobotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CS3RobotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CS3RobotDlg)
	m_bCheckPostionRandom = FALSE;
	m_bCheckRepeatLastCmd = FALSE;
	m_csAccountName = _T("");
	m_csPassword = _T("");
	m_nPositionSkillID = 1;
	m_nPosition_x = 1;
	m_nPosition_y = 1;
	m_csSpeakMessage = _T("");
	m_nPlayerAction = 0;
	m_nPlayerLoginSelect = 0;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pImgLst_InfoSmallIcon = NULL;
	m_pImgLst_InfoNormalIcon = NULL;

	m_bActiveLife = false;

	m_wGameTimerID = 0;

	InitFuntionEntry();
}

void CS3RobotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CS3RobotDlg)
	DDX_Control(pDX, IDC_COPYRIGHT, m_ctrlCopyRight);
	DDX_Control(pDX, IDC_LIST_PLAYER_INFO, m_ctlPlayerInfoList);
	DDX_Control(pDX, IDC_LIST_CURRENT_MSG, m_ctlCurrentMsgList);
	DDX_Control(pDX, IDC_CTRL_LOGINSERVER, m_ctrlLoginServerComBox);
	DDX_Check(pDX, IDC_CHECK_POSITION_RANDOM, m_bCheckPostionRandom);
	DDX_Check(pDX, IDC_CHECK_REPEAT_LASTCOMMAND, m_bCheckRepeatLastCmd);
	DDX_Text(pDX, IDC_EDIT_ACCOUNTNAME, m_csAccountName);
	DDV_MaxChars(pDX, m_csAccountName, 32);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_csPassword);
	DDV_MaxChars(pDX, m_csPassword, 32);
	DDX_Text(pDX, IDC_EDIT_POSITION_SKILLID, m_nPositionSkillID);
	DDV_MinMaxInt(pDX, m_nPositionSkillID, 1, 500);
	DDX_Text(pDX, IDC_EDIT_POSITION_X, m_nPosition_x);
	DDX_Text(pDX, IDC_EDIT_POSITION_Y, m_nPosition_y);
	DDX_Text(pDX, IDC_EDIT_SPEAK_MESSAGE, m_csSpeakMessage);
	DDV_MaxChars(pDX, m_csSpeakMessage, 128);
	DDX_Radio(pDX, IDC_RADIO_ACTION_MOVE, m_nPlayerAction);
	DDX_Radio(pDX, IDC_RADIO_PLAYER_FIRST, m_nPlayerLoginSelect);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CS3RobotDlg, CDialog)
	//{{AFX_MSG_MAP(CS3RobotDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SEND_COMMAND, OnBtnSendCommand)
	ON_BN_CLICKED(IDC_BTN_USE_DEFAULT_SCTFILE, OnBtnUseDefaultSctfile)
	ON_BN_CLICKED(IDC_BTN_SCTIPTFILE_LOGIN, OnBtnSctiptfileLogin)
	ON_BN_CLICKED(IDC_RADIO_PLAYER_FIRST, OnRadioPlayerFirst)
	ON_BN_CLICKED(IDC_RADIO_PLAYER_SECOND, OnRadioPlayerSecond)
	ON_BN_CLICKED(IDC_RADIO_ACTION_MOVE, OnRadioActionMove)
	ON_BN_CLICKED(IDC_RADIO_ACTION_SKILL, OnRadioActionSkill)
	ON_BN_CLICKED(IDC_RADIO_ACTION_SPEAK, OnRadioActionSpeak)
	ON_BN_CLICKED(IDC_CHECK_POSITION_RANDOM, OnCheckPositionRandom)
	ON_BN_CLICKED(IDC_CHECK_REPEAT_LASTCOMMAND, OnCheckRepeatLastcommand)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_CTRL_LOGINSERVER, OnSelchangeCtrlLoginserver)
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CONNECT_CREATE, OnConnectCreate )
	ON_MESSAGE( WM_CONNECT_CLOSE, OnConnectClose )
END_MESSAGE_MAP()

void CS3RobotDlg::PreProcess()
{
	const char *pData = NULL;
	size_t size = 0;
	
	do
	{
		pData = GetFromServer( size );
		
		if ( !pData || 0 == size )
		{
			break;
		}

		const char *pMsg = pData;

		while ( pMsg < ( pData + size ) )
		{
			PROTOCOL_MSG_TYPE pmt = *( PROTOCOL_MSG_TYPE * )pMsg;

			ASSERT( pmt > s2c_begin && pmt < s2c_end );

			/*
			 * Process protocol
			 */

			if ( ProcessArray[pmt] )
			{
				(this->*ProcessArray[pmt])( pMsg );
			}

			/*
			 * Get next protocol
			 */
			int nProtocolSize = g_nMsgProtocolSize[ pmt - s2c_begin - 1 ];

			if ( nProtocolSize >= 0 )
			{
				pMsg += nProtocolSize;
			}
			else
			{
				unsigned short usMsgSize = *( unsigned short * )( pMsg + PROTOCOL_MSG_SIZE );

				pMsg += PROTOCOL_MSG_SIZE + usMsgSize;
			}
		}		
		
	}while ( true );
}

void CS3RobotDlg::Process()
{
	PING_COMMAND	thePingCmd;

	thePingCmd.ProtocolType = c2s_ping;
	thePingCmd.m_dwTime = GetTickCount();
	
	SendToServer( reinterpret_cast< char * >( &thePingCmd ), sizeof( PING_COMMAND ) );
}

/////////////////////////////////////////////////////////////////////////////
// CS3RobotDlg message handlers

BOOL CS3RobotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_theLoginInfo.szAccountName[0] = 0;
	m_theLoginInfo.szPassword[0] = 0;

	m_theLoginInfo.nPlayerIndex = 0;

	m_ctrlCopyRight.SetURL( "mailto:liupeng@bj.kingsoft.net" );
	
	/*
	 * m_ctrlLoginServerComBox
	 */

	int nServerCount = 0;

	KIniFile iniFile;
	if ( iniFile.Load( "Settings\\ServerList.ini" ) )
	{
		iniFile.GetInteger( "List", "Count", 0, &nServerCount );
	}

	CString csSection;
	char szTitle[MAX_PATH];
	char szAddress[MAX_PATH];

	int nAddressCount = 0;

	for ( int i=nServerCount-1; i>=0; i-- )
	{
		csSection.Format( "%d", i );

		szTitle[0] = 0;		
		iniFile.GetString( csSection, "Title", NULL, szTitle, MAX_PATH );

		szAddress[0] = 0;
		iniFile.GetString( csSection, "Address", NULL, szAddress, MAX_PATH );

		if ( 0 != szTitle[0] )
		{
			int nPos = m_ctrlLoginServerComBox.InsertString( 0, szTitle );

			if ( LB_ERR != nPos && 0 != szAddress[0] )
			{
				m_ServerAddrMap.insert( SERVER_MAP::value_type( nAddressCount, szAddress ) );

				m_ctrlLoginServerComBox.SetItemData( nPos, nAddressCount );

				nAddressCount ++;
			}
		}
	}

	/*
	 * m_ctlPlayerInfoList
	 */
	m_ctlPlayerInfoList.SetExtendedStyle(
		m_ctlPlayerInfoList.GetExtendedStyle() | 
		LVS_EX_FULLROWSELECT | 
		LVS_EX_GRIDLINES );

	m_ctlPlayerInfoList.InsertColumn( 0, "Status", LVCFMT_LEFT, 100, 0 );
	m_ctlPlayerInfoList.InsertColumn( 1, "Value", LVCFMT_LEFT, 153, 1 );

	int nIndex = 0;

	while ( 0 != g_cszPlayerInfo[nIndex][0] )
	{
		m_ctlPlayerInfoList.InsertItem( nIndex, g_cszPlayerInfo[nIndex] );

		nIndex ++;
	}

	char szContent[MAX_PATH];

	if ( iniFile.Load( "RobotRecorder.ini" ) )
	{
		szContent[0] = 0;
		iniFile.GetString( "Logininfo", "AccountName", NULL, szContent, MAX_PATH );
		m_csAccountName = szContent;

		szContent[0] = 0;
		iniFile.GetString( "Logininfo", "Password", NULL, szContent, MAX_PATH );
		m_csPassword = szContent;
	}
	
	/*
	 * m_ctlCurrentMsgList
	 */
	m_ctlCurrentMsgList.SetExtendedStyle(
			m_ctlCurrentMsgList.GetExtendedStyle() | 
			LVS_EX_FULLROWSELECT );

	m_ctlCurrentMsgList.ModifyStyle( 0, LVS_NOCOLUMNHEADER, 0 );	

	m_pImgLst_InfoSmallIcon = new CImageList();
	VERIFY( m_pImgLst_InfoSmallIcon->Create( 16, 16, TRUE, 0, 1 ) );	

	m_pImgLst_InfoSmallIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_INFO) ) );
	m_pImgLst_InfoSmallIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_COR) ) );
	m_pImgLst_InfoSmallIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_WARN) ) );
	m_pImgLst_InfoSmallIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_WORLD) ) );

	m_pImgLst_InfoNormalIcon = new CImageList();
	VERIFY( m_pImgLst_InfoNormalIcon->Create( 32, 32, TRUE, 0, 1 ) );

	m_pImgLst_InfoNormalIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_INFO) ) );
	m_pImgLst_InfoNormalIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_COR) ) );
	m_pImgLst_InfoNormalIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_WARN) ) );
	m_pImgLst_InfoNormalIcon->Add( LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_WORLD) ) );

	m_ctlCurrentMsgList.SetImageList( m_pImgLst_InfoSmallIcon, LVSIL_SMALL );
	m_ctlCurrentMsgList.SetImageList( m_pImgLst_InfoNormalIcon, LVSIL_NORMAL );

	CRect rect;
	m_ctlCurrentMsgList.GetWindowRect( &rect );

	m_ctlCurrentMsgList.InsertColumn (0, "CurrentMessage", LVCFMT_LEFT, rect.Width() - 4 );

	m_ctlCurrentMsgList.InsertItem( LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, 0, "CurrentMessage", 0, LVIS_SELECTED, 0, 0 );
	m_ctlCurrentMsgList.SetItemText( 0, 0, "Welcome to sworldman world!" );

	/*
	 * Update some ctrl status
	 */

	m_ctrlLoginServerComBox.SetCurSel( 0 );
	ChangeServer( 0 );

	EnableInfoCtrl( FALSE );

	UpdateData( FALSE );

	/*
	 * Start up network engine
	 */
	ClientStartup();

	/*
	 * Prepare to show status
	 */
	m_unAppTimer = SetTimer( g_unAppTimerEventID, 1000, NULL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CS3RobotDlg::OnPaint() 
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

HCURSOR CS3RobotDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CS3RobotDlg::OnSelchangeCtrlLoginserver() 
{
	int nPos = m_ctrlLoginServerComBox.GetCurSel();

	ChangeServer( nPos );
}

void CS3RobotDlg::ChangeServer( int nPos )
{
	if ( LB_ERR != nPos )
	{
		int nAddrIndex = m_ctrlLoginServerComBox.GetItemData( nPos );

		SERVER_MAP::iterator info = m_ServerAddrMap.find( nAddrIndex );
		if ( info != m_ServerAddrMap.end() )
		{
			const string &addr = (*info).second;

			CString csServerInfo;
			csServerInfo.Format( "@ %s , %d", addr.c_str(), 8888 );

			m_ctlPlayerInfoList.SetItemText( enumServerAddress, 1, csServerInfo );

			m_strServerAddr = addr;
		}
	}
}

void CS3RobotDlg::OnBtnSendCommand() 
{
	
}

void CS3RobotDlg::OnBtnUseDefaultSctfile() 
{
	
}

void CS3RobotDlg::OnOK() 
{
	UpdateData( TRUE );

	if ( m_csAccountName.IsEmpty() )
	{
		AfxMessageBox( "Are you forget your name, aha?" );
		return;
	}

	EnableInfoCtrl( TRUE );

	strcpy( m_theLoginInfo.szAccountName, m_csAccountName );
	strcpy( m_theLoginInfo.szPassword, m_csPassword );

	m_theLoginInfo.nPlayerIndex = m_nPlayerLoginSelect;

	/*
	 * Connect to S3server
	 */
	InstallCallBack( EventNotify );

	ConnectTo( m_strServerAddr.c_str(), 8888 );

	/*
	 * Start game loop
	 */
	if ( m_wGameTimerID )
	{
		timeKillEvent( m_wGameTimerID );
		m_wGameTimerID = 0;
	}

	m_wGameTimerID = timeSetEvent( 
		g_unGameDelay, 
		10,
		GameLoopFun, 
		reinterpret_cast< DWORD >( this ), 
		TIME_PERIODIC | TIME_CALLBACK_FUNCTION );

	/*
	 * Require to login
	 */
	BYTE szLoginInfo[33];

	szLoginInfo[0] = c2s_login;
	memcpy( &szLoginInfo[1], m_csAccountName, 31 );

	szLoginInfo[32] = 0;

	SendToServer( reinterpret_cast< char * >( szLoginInfo ), 33 );
}

void CS3RobotDlg::OnCancel() 
{
	Shutdown();
	
	CDialog::OnCancel();
}

void CS3RobotDlg::OnBtnSctiptfileLogin() 
{
	
}

void CS3RobotDlg::OnRadioPlayerFirst() 
{
	
}

void CS3RobotDlg::OnRadioPlayerSecond() 
{
	
}

void CS3RobotDlg::OnRadioActionMove() 
{
	
}

void CS3RobotDlg::OnRadioActionSkill() 
{
	
}

void CS3RobotDlg::OnRadioActionSpeak() 
{
	
}

void CS3RobotDlg::OnCheckPositionRandom() 
{
	
}

void CS3RobotDlg::OnCheckRepeatLastcommand() 
{
	
}

void CS3RobotDlg::OnLogonSuccess()
{

}

void CS3RobotDlg::OnWaitForLogon()
{

}

void CS3RobotDlg::EnableInfoCtrl( BOOL bEnable /*TRUE*/ )
{
	m_bActiveLife = (bEnable) ? true : false;

	/*
	 * when TRUE == bEnable
	 */
	m_ctlPlayerInfoList.EnableWindow( bEnable );
	m_ctlCurrentMsgList.EnableWindow( bEnable );	

	EnableCtrl( IDC_RADIO_ACTION_MOVE, bEnable );		
	EnableCtrl( IDC_RADIO_ACTION_SKILL, bEnable );
	EnableCtrl( IDC_RADIO_ACTION_SPEAK, bEnable );
	EnableCtrl( IDC_EDIT_POSITION_X, bEnable );
	EnableCtrl( IDC_EDIT_POSITION_Y, bEnable );
	EnableCtrl( IDC_SPIN1, bEnable );
	EnableCtrl( IDC_SPIN2, bEnable );
	EnableCtrl( IDC_SPIN3, bEnable );
	EnableCtrl( IDC_CHECK_POSITION_RANDOM, bEnable );
	EnableCtrl( IDC_EDIT_SPEAK_MESSAGE, bEnable );
	EnableCtrl( IDC_EDIT_POSITION_SKILLID, bEnable );
	
	EnableCtrl( IDC_BTN_SEND_COMMAND, bEnable );
	EnableCtrl( IDC_CHECK_REPEAT_LASTCOMMAND, bEnable );
	EnableCtrl( IDC_BTN_USE_DEFAULT_SCTFILE, bEnable );

	/*
	 * FALSE == bEnable
	 */
	BOOL bDisable = !bEnable;

	m_ctrlLoginServerComBox.EnableWindow( bDisable );

	EnableCtrl( IDC_EDIT_ACCOUNTNAME, bDisable );
	EnableCtrl( IDC_EDIT_PASSWORD, bDisable );
	EnableCtrl( IDC_RADIO_PLAYER_FIRST, bDisable );
	EnableCtrl( IDC_RADIO_PLAYER_SECOND, bDisable );
	EnableCtrl( IDC_BTN_SCTIPTFILE_LOGIN, bDisable );
	EnableCtrl( IDOK, bDisable );
}

void CS3RobotDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	if ( m_wGameTimerID )
	{
		timeKillEvent( m_wGameTimerID );
		m_wGameTimerID = 0;
	}

	KillTimer( g_unAppTimerEventID );

	ClientCleanup();

	if ( m_pImgLst_InfoSmallIcon )
	{
		delete m_pImgLst_InfoSmallIcon;
		m_pImgLst_InfoSmallIcon = NULL;
	}

	if ( m_pImgLst_InfoNormalIcon )
	{
		delete m_pImgLst_InfoNormalIcon;
		m_pImgLst_InfoNormalIcon = NULL;
	}
	
	UpdateData( TRUE );

	KIniFile iniFile;

	iniFile.WriteString( "Logininfo", "AccountName", m_csAccountName );
	iniFile.WriteString( "Logininfo", "Password", m_csPassword );
	
	iniFile.Save( "RobotRecorder.ini" );
}

void CS3RobotDlg::OnTimer(UINT nIDEvent) 
{
	static TCHAR s_szBuffer[9];
	static unsigned long s_unTotalLifeTime = 1;

	static UINT unHour = 0;
	static UINT unMinute = 0;
	static UINT unSecond = 0;

	if ( m_bActiveLife && g_unAppTimerEventID == nIDEvent )
	{
		unHour = (UINT)( s_unTotalLifeTime / 3600 );
		unMinute = (UINT)( ( s_unTotalLifeTime % 3600 ) / 60 );
		unSecond = (UINT)( ( s_unTotalLifeTime % 3600 ) % 60 );

	   _stprintf( s_szBuffer, _T("%02d:%02d:%02d"),
							 unHour,
							 unMinute,
							 unSecond );

	   m_ctlPlayerInfoList.SetItemText( enumApplicationTimer, 1, s_szBuffer );

	   s_unTotalLifeTime ++;
	}
	
	CDialog::OnTimer(nIDEvent);
}

HRESULT CS3RobotDlg::OnConnectCreate( WPARAM wParam, LPARAM lParam )
{
	m_ctlCurrentMsgList.InsertItem( LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, 0, "CurrentMessage", 0, LVIS_SELECTED, 3, 0 );
	m_ctlCurrentMsgList.SetItemText( 0, 0, "Successful connect to server and wait for it confirm" );
	
	::MessageBeep( -1 );

	return S_OK;
}

HRESULT CS3RobotDlg::OnConnectClose( WPARAM wParam, LPARAM lParam )
{
	m_ctlCurrentMsgList.InsertItem( LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, 0, "CurrentMessage", 0, LVIS_SELECTED, 2, 0 );
	m_ctlCurrentMsgList.SetItemText( 0, 0, "Connection was breaked" );

	::MessageBeep( -1 );

	return S_OK;
}

bool CS3RobotDlg::ProcessPing( const char *pMsg )
{

	return true;
}

bool CS3RobotDlg::Syncrolelist( const char *pMsg )
{
	ROLE_LIST_SYNC *pRLS = (ROLE_LIST_SYNC *)pMsg;

	if ( pRLS )
	{
		for ( int i=0; i<2; i++ )
		{
			if ( 0 != pRLS->m_RoleList[i].szName[0] )
			{
				AfxMessageBox( pRLS->m_RoleList[i].szName );
			}
			else
			{
				AfxMessageBox( "not find!" );
			}
		}
	}

	return true;
}

void CS3RobotDlg::InitFuntionEntry()
{
	ZeroMemory( ProcessArray, sizeof(ProcessArray) );

	ProcessArray[s2c_ping] = ProcessPing;
	ProcessArray[s2c_syncrolelist] = Syncrolelist;
}

