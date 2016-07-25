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
const UINT g_unAutoCtrlTimer = 200;

const UINT g_unTrayIconID = 1;

const UINT g_unGameDelay = 50;

const char g_cszPlayerInfo[][MAX_PATH] = {
						"Player Name [ID]",
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
	enumPlayerNameID = 0,
	enumCurrentPosition,
	enumCurrentMana,
	enumCurrentHeath,
	enumServerAddress,
	enumPingValue,
	enumApplicationTimer
};

enum enumPlayerAction
{
	enumMoveAction = 0,
	enumUseSkill,
	enumSpeakWord
};

#define WM_CONNECT_CREATE	WM_USER + 0x100
#define WM_CONNECT_CLOSE	WM_USER + 0x110
#define WM_REBOOT_ROBOT		WM_USER + 0x200
#define WM_LOGIN_GAME		WM_USER + 0x210

#define WM_TRAY_NOTIFY_MSG	WM_USER + 0x500

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
	sizeof(NPC_JUMP_SYNC),		// s2c_npcjump,
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
	sizeof(TRADE_STATE_SYNC),			// s2c_tradepressoksync
	sizeof(PING_COMMAND),					// s2c_ping
	sizeof(NPC_SIT_SYNC),					// s2c_npcsit
	sizeof(SALE_BOX_SYNC),					// s2c_opensalebox
};

const UINT WM_TASKBARCREATED = ::RegisterWindowMessage( _T("TaskbarCreated") );

/////////////////////////////////////////////////////////////////////////////
// CS3RobotDlg dialog

CS3RobotDlg::CS3RobotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CS3RobotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CS3RobotDlg)
	m_csAccountName = _T("");
	m_csPassword = _T("");
	m_nPosition_x = 1;
	m_nPosition_y = 1;
	m_csSpeakMessage = _T("");
	m_nPlayerAction = 2;
	m_nPlayerLoginSelect = 0;
	m_bRecordCurrentTime = TRUE;
	m_bAutoControl = FALSE;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pImgLst_InfoSmallIcon = NULL;
	m_pImgLst_InfoNormalIcon = NULL;

	m_bActiveLife = false;

	m_wGameTimerID = 0;
	m_bSkillTabLoad = FALSE;

	memset( &m_thePlayerInfo, 0, sizeof(m_thePlayerInfo) );

	memset( &m_nidIconData, 0, sizeof(NOTIFYICONDATA) );

	InitFuntionEntry();
}

void CS3RobotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CS3RobotDlg)
	DDX_Control(pDX, IDC_CTRL_SKILLLIST, m_ctrlSkillListComBox);
	DDX_Control(pDX, IDC_COPYRIGHT, m_ctrlCopyRight);
	DDX_Control(pDX, IDC_LIST_PLAYER_INFO, m_ctlPlayerInfoList);
	DDX_Control(pDX, IDC_LIST_CURRENT_MSG, m_ctlCurrentMsgList);
	DDX_Control(pDX, IDC_CTRL_LOGINSERVER, m_ctrlLoginServerComBox);
	DDX_Text(pDX, IDC_EDIT_ACCOUNTNAME, m_csAccountName);
	DDV_MaxChars(pDX, m_csAccountName, 32);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_csPassword);
	DDV_MaxChars(pDX, m_csPassword, 32);
	DDX_Text(pDX, IDC_EDIT_POSITION_X, m_nPosition_x);
	DDX_Text(pDX, IDC_EDIT_POSITION_Y, m_nPosition_y);
	DDX_Text(pDX, IDC_EDIT_SPEAK_MESSAGE, m_csSpeakMessage);
	DDV_MaxChars(pDX, m_csSpeakMessage, 128);
	DDX_Radio(pDX, IDC_RADIO_ACTION_MOVE, m_nPlayerAction);
	DDX_Radio(pDX, IDC_RADIO_PLAYER_FIRST, m_nPlayerLoginSelect);
	DDX_Check(pDX, IDC_RECORD_CURRENTTIME, m_bRecordCurrentTime);
	DDX_Check(pDX, IDC_AUTO_CONTROL, m_bAutoControl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CS3RobotDlg, CDialog)
	//{{AFX_MSG_MAP(CS3RobotDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SEND_COMMAND, OnBtnSendCommand)
	ON_BN_CLICKED(IDC_RADIO_PLAYER_FIRST, OnRadioPlayerFirst)
	ON_BN_CLICKED(IDC_RADIO_PLAYER_SECOND, OnRadioPlayerSecond)
	ON_BN_CLICKED(IDC_RADIO_ACTION_MOVE, OnRadioActionMove)
	ON_BN_CLICKED(IDC_RADIO_ACTION_SKILL, OnRadioActionSkill)
	ON_BN_CLICKED(IDC_RADIO_ACTION_SPEAK, OnRadioActionSpeak)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_CTRL_LOGINSERVER, OnSelchangeCtrlLoginserver)
	ON_BN_CLICKED(IDC_AUTO_CONTROL, OnAutoControl)
	ON_WM_CREATE()
	ON_COMMAND(IDR_ST_EXIT, OnStExit)
	ON_COMMAND(IDR_ST_MINIMIZE, OnStMinimize)
	ON_COMMAND(IDR_ST_RESTORE, OnStRestore)
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CONNECT_CREATE, OnConnectCreate )
	ON_MESSAGE( WM_CONNECT_CLOSE, OnConnectClose )
	ON_MESSAGE( WM_REBOOT_ROBOT, OnRebootRobot )
	ON_MESSAGE( WM_LOGIN_GAME, OnLoginGame )
	ON_MESSAGE( WM_TRAY_NOTIFY_MSG, OnTrayNotifyMsg )
	ON_REGISTERED_MESSAGE( WM_TASKBARCREATED, OnTaskBarCreated )
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

}

/////////////////////////////////////////////////////////////////////////////
// CS3RobotDlg message handlers

BOOL CS3RobotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_ctrlCopyRight.SetURL( "mailto:liupeng@bj.kingsoft.net" );
	
	/*
	 * m_ctrlLoginServerComBox
	 */

	int nServerCount = 0;

	KIniFile iniFile;
	if ( iniFile.Load( "\\Settings\\ServerList.ini" ) )
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

	int nServerSelIndex = 0;

	if ( iniFile.Load( "\\RobotRecorder.ini" ) )
	{
		szContent[0] = 0;
		iniFile.GetString( "Logininfo", "AccountName", NULL, szContent, MAX_PATH );
		m_csAccountName = szContent;

		szContent[0] = 0;
		iniFile.GetString( "Logininfo", "Password", NULL, szContent, MAX_PATH );
		m_csPassword = szContent;

		iniFile.GetInteger( "Settings", "RecordCurTime", TRUE, &m_bRecordCurrentTime );		

		iniFile.GetInteger( "Settings", "ServerSelIndex", 0, &nServerSelIndex );		
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

	m_ctlCurrentMsgList.InsertColumn (0, "CurrentMessage", LVCFMT_LEFT, rect.Width() - 20 );

	m_ctlCurrentMsgList.InsertItem( LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, 0, "CurrentMessage", 0, LVIS_SELECTED, 0, 0 );
	m_ctlCurrentMsgList.SetItemText( 0, 0, "Welcome to swordman world!" );

	/*
	 * Update some ctrl status
	 */
	if ( nServerSelIndex >= nAddressCount )
	{
		nServerSelIndex = 0;
	}

	m_ctrlLoginServerComBox.SetCurSel( nServerSelIndex );
	ChangeServer( nServerSelIndex );

	EnableInfoCtrl( FALSE );

	UpdateData( FALSE );

	/*
	 * Init skill file
	 */
	
	m_bSkillTabLoad = m_tabFile.Load( "\\settings\\skills.txt" );

	/*
	 * Start up network engine
	 */
	ClientStartup();

	/*
	 * Prepare to show status
	 */
	srand( ( unsigned )time( NULL ) );

	m_unAppTimer = SetTimer( g_unAppTimerEventID, 1000, NULL );

	m_unAutoCtrlTimer = SetTimer( g_unAutoCtrlTimer, 3000, NULL );

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

void CS3RobotDlg::SpeakWord( const char *pText, size_t size /*0*/ )
{
	if ( NULL == pText || 0 == size )
	{
		return;
	}

	size_t dataLength = size;

	if ( 0 == dataLength )
	{
		dataLength = strlen( pText );
	}

	PLAYER_SEND_CHAT_COMMAND pscc;

	pscc.ProtocolType = c2s_playersendchat;
	pscc.m_btCurChannel = 1; // To all
	pscc.m_btType = 0; // MSG_G_CHAT

	pscc.m_dwTargetID = 0;
	pscc.m_nTargetIdx = 0;
	pscc.m_btChatPrefixLen = 0;

	strcpy( pscc.m_szSentence, pText );
	pscc.m_wSentenceLen = dataLength;

	pscc.m_wLength = 
		dataLength +
		sizeof( PLAYER_SEND_CHAT_COMMAND ) - 
		sizeof( pscc.m_szSentence ) - 
		sizeof( BYTE );

	SendToServer( &pscc, pscc.m_wLength + sizeof( BYTE ) );
}

void CS3RobotDlg::ToMove( int x, int y )
{
	NPC_WALK_COMMAND	nwc;
	
	nwc.ProtocolType = c2s_npcwalk;
	nwc.nMpsX = x;
	nwc.nMpsY = y;
	
	SendToServer( reinterpret_cast< BYTE * >( &nwc ), sizeof( nwc ) );
}

void CS3RobotDlg::TodoSkill( int nSkillID, int x, int y )
{
	NPC_SKILL_COMMAND	nsc;
	
	nsc.ProtocolType = c2s_npcskill;
	nsc.nSkillID = nSkillID;
	nsc.nMpsX = x;
	nsc.nMpsY = y;

	SendToServer( reinterpret_cast< BYTE * >( &nsc ), sizeof( NPC_SKILL_COMMAND ) );
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
	UpdateData( TRUE );

	switch ( m_nPlayerAction )
	{
	case enumMoveAction:
		ToMove( m_nPosition_x, m_nPosition_y );
		break;
	case enumUseSkill:
		{
			int nSelIndex = m_ctrlSkillListComBox.GetCurSel();
			int nSkillID = m_ctrlSkillListComBox.GetItemData( nSelIndex );

			TodoSkill( nSkillID, m_nPosition_x + 5, m_nPosition_y + 10 );
		}
		break;
	case enumSpeakWord:
		SpeakWord( m_csSpeakMessage, m_csSpeakMessage.GetLength() );
		break;
	}
}

void CS3RobotDlg::OnOK() 
{
	PostMessage( WM_LOGIN_GAME, 0, 0 );
}

void CS3RobotDlg::OnCancel() 
{
	Shutdown();
	
	CDialog::OnCancel();
}

void CS3RobotDlg::OnRadioPlayerFirst() 
{
	
}

void CS3RobotDlg::OnRadioPlayerSecond() 
{
	
}

void CS3RobotDlg::OnRadioActionMove() 
{
	UpdateData( TRUE );

	m_nPosition_x = m_thePlayerInfo.dwCurPixel_x;
	m_nPosition_y = m_thePlayerInfo.dwCurPixel_y;

	UpdateData( FALSE );
}

void CS3RobotDlg::OnAutoControl() 
{
	UpdateData( TRUE );
}

void CS3RobotDlg::OnRadioActionSkill() 
{
	
}

void CS3RobotDlg::OnRadioActionSpeak() 
{
	
}

void CS3RobotDlg::OnLogonSuccess()
{

}

void CS3RobotDlg::OnWaitForLogon()
{

}

void CS3RobotDlg::EnableInfoCtrl( BOOL bEnable /*TRUE*/,  BOOL bToAll /*FALSE*/ )
{
	m_bActiveLife = (bEnable) ? true : false;

	/*
	 * when TRUE == bEnable
	 */
	m_ctlPlayerInfoList.EnableWindow( bEnable );
//	m_ctlCurrentMsgList.EnableWindow( bEnable );	
	
	EnableCtrl( IDC_CTRL_SKILLLIST, bEnable );
	EnableCtrl( IDC_RADIO_ACTION_MOVE, bEnable );
	EnableCtrl( IDC_RADIO_ACTION_SKILL, bEnable );
	EnableCtrl( IDC_RADIO_ACTION_SPEAK, bEnable );
	EnableCtrl( IDC_EDIT_POSITION_X, bEnable );
	EnableCtrl( IDC_EDIT_POSITION_Y, bEnable );
	EnableCtrl( IDC_SPIN1, bEnable );
	EnableCtrl( IDC_SPIN2, bEnable );
	EnableCtrl( IDC_EDIT_SPEAK_MESSAGE, bEnable );
	EnableCtrl( IDC_AUTO_CONTROL, bEnable );	
	
	EnableCtrl( IDC_BTN_SEND_COMMAND, bEnable );
	EnableCtrl( IDC_BTN_USE_DEFAULT_SCTFILE, bEnable );

	/*
	 * FALSE == bEnable
	 */
	BOOL bDisable = ( bToAll ) ? bEnable : ( !bEnable );

	m_ctrlLoginServerComBox.EnableWindow( bDisable );

	EnableCtrl( IDC_EDIT_ACCOUNTNAME, bDisable );
	EnableCtrl( IDC_EDIT_PASSWORD, bDisable );
	EnableCtrl( IDC_RADIO_PLAYER_FIRST, bDisable );
	EnableCtrl( IDC_RADIO_PLAYER_SECOND, bDisable );
	EnableCtrl( IDC_BTN_SCTIPTFILE_LOGIN, bDisable );
	EnableCtrl( IDOK, bDisable );
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
	   
	   /*
	    * Send a ping command
	    */
	   PING_COMMAND	thePingCmd;
	   
	   thePingCmd.ProtocolType = c2s_ping;
	   thePingCmd.m_dwTime = GetTickCount();
	   
	   SendToServer( reinterpret_cast< char * >( &thePingCmd ), sizeof( PING_COMMAND ) );
	}

	if ( m_bAutoControl && g_unAutoCtrlTimer == nIDEvent )
	{
		switch ( m_nPlayerAction )
		{
		case enumMoveAction:
			AutoMove();
			break;
		case enumSpeakWord:
			break;
		case enumUseSkill:
			break;
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CS3RobotDlg::AutoMove()
{	
	static int nMoveConvertFlag = 0;
	static bool bSchedule_x = ( ( rand() % 10  ) > 4 ) ? true : false;
	static bool bSchedule_y = ( ( rand() % 10  ) > 4 ) ? true : false;	
	
	static int nBlockTimeCount = 0; 
	
	if ( IsEquidistant( m_thePlayerInfo.dwCurPixel_target_x, 
				m_thePlayerInfo.dwCurPixel_target_y,
				m_thePlayerInfo.dwCurPixel_x,					
				m_thePlayerInfo.dwCurPixel_y,
				10 ) )
	{
		int x = rand() % 100;
		int y = rand() % 100;
		
		int xmv = 0;
		int ymv = 0;
		
		if ( nMoveConvertFlag > 0 )
		{
			nMoveConvertFlag = 0;

			bSchedule_x = ( ( rand() % 10  ) > 4 ) ? true : false;
			bSchedule_y = ( ( rand() % 10  ) > 4 ) ? true : false;					
		}					
		
		if ( bSchedule_x )
		{
			xmv = ( x > 5 ) ? ( -x ) : x;
		}
		else
		{
			xmv = ( x > 5 ) ? x : ( -x );
		}
		
		if ( bSchedule_y )
		{
			ymv = ( x > 5 ) ? y : ( -y );
		}
		else
		{						
			ymv = ( x > 5 ) ? ( -y ) : y;
		}
		
		int nTarget_x = m_thePlayerInfo.dwCurPixel_x + xmv;
		int nTarget_y = m_thePlayerInfo.dwCurPixel_y + ymv;
		
		m_thePlayerInfo.dwCurPixel_target_x = nTarget_x;
		m_thePlayerInfo.dwCurPixel_target_y = nTarget_y;
		
		ToMove( nTarget_x, nTarget_y );
	}
	else
	{
		if ( nBlockTimeCount ++ > 3 )
		{
			m_thePlayerInfo.dwCurPixel_target_x = m_thePlayerInfo.dwCurPixel_x;
			m_thePlayerInfo.dwCurPixel_target_y = m_thePlayerInfo.dwCurPixel_y;

			nBlockTimeCount = 0;
		}

		nMoveConvertFlag ++;
	}
}

HRESULT CS3RobotDlg::OnConnectCreate( WPARAM wParam, LPARAM lParam )
{
	ShowInfomation( "Successful connect to server and wait for it confirm", SI_WORLD );
	
	::MessageBeep( -1 );

	return S_OK;
}

HRESULT CS3RobotDlg::OnConnectClose( WPARAM wParam, LPARAM lParam )
{
	ShowInfomation( "Connection was breaked", SI_WARN );
	
	EnableInfoCtrl( FALSE );
	
	if ( !IsWindowVisible() )
	{
		OnStRestore();
	}

	::MessageBeep( -1 );

	return S_OK;
}

HRESULT CS3RobotDlg::OnRebootRobot( WPARAM wParam, LPARAM lParam )
{
	EnableInfoCtrl( FALSE );

	return S_OK;
}

HRESULT CS3RobotDlg::OnLoginGame( WPARAM wParam, LPARAM lParam )
{
	UpdateData( TRUE );

	EnableInfoCtrl( FALSE, TRUE );

	if ( m_csAccountName.IsEmpty() )
	{
		AfxMessageBox( "Are you forget your name, aha?" );
		return S_FALSE;
	}
	
	static unsigned short s_uServerPort = 0;
	if ( 0 == s_uServerPort )
	{
		KIniFile iniFile;
		
		if ( iniFile.Load( "\\config.ini" ) )
		{
			iniFile.GetInteger( "Server", "GameServPort", 0, reinterpret_cast< int * >( &s_uServerPort ) );
		}
	}

	/*
	 * Connect to S3server
	 */
	InstallCallBack( EventNotify );

	if ( !ConnectTo( m_strServerAddr.c_str(), s_uServerPort ) )
	{
		ShowInfomation( "Don't find server", SI_WARN );
		
		PostMessage( WM_REBOOT_ROBOT, 0, 0 );

		return FALSE;
	}

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

	size_t size = m_csAccountName.GetLength();
	size = ( size > 31 ) ? 31 : size;

	memcpy( &szLoginInfo[1], m_csAccountName, size );
	szLoginInfo[size + 1] = 0;

	SendToServer( reinterpret_cast< char * >( szLoginInfo ), 33 );

	return S_OK;
}

void CS3RobotDlg::ShowInfomation( LPCTSTR pInfo, int nImage /*-1*/ )
{
	static CString s_csInfo;

	if ( nImage > 0 )
	{
		m_ctlCurrentMsgList.InsertItem( LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, 0, "CurrentMessage", 0, LVIS_SELECTED, nImage, 0 );
	}
	else
	{
		m_ctlCurrentMsgList.InsertItem( LVIF_TEXT | LVIF_STATE, 0, "CurrentMessage", 0, LVIS_SELECTED, 0, 0 );
	}

	if ( m_bRecordCurrentTime )
	{
		SYSTEMTIME systime;
		GetLocalTime( &systime );

		s_csInfo.Format( _T("[%02d:%02d:%02d] %s"), systime.wHour, systime.wMinute, systime.wSecond, pInfo );

		m_ctlCurrentMsgList.SetItemText( 0, 0, s_csInfo );
	}
	else
	{
		m_ctlCurrentMsgList.SetItemText( 0, 0, pInfo );
	}
}

void CS3RobotDlg::ShowPing( DWORD dwPingValue )
{
	CString csPingValue;

	csPingValue.Format( "%dms", dwPingValue );

	m_ctlPlayerInfoList.SetItemText( enumPingValue, 1, csPingValue );
}

bool CS3RobotDlg::ProcessPing( const char *pMsg )
{
	DWORD	dwTimer = GetTickCount();
	PING_COMMAND* pPC = (PING_COMMAND *)pMsg;

	dwTimer -= pPC->m_dwTime;
	dwTimer >>= 1;

	ShowPing( dwTimer );

	return true;
}

bool CS3RobotDlg::Syncrolelist( const char *pMsg )
{
	ROLE_LIST_SYNC *pRLS = (ROLE_LIST_SYNC *)pMsg;

	bool bOpt = false;

	if ( pRLS )
	{
		for ( int i=0; i<2; i++ )
		{
			if ( 0 != pRLS->m_RoleList[i].szName[0] )
			{
				if ( i == m_nPlayerLoginSelect )
				{
					DB_PLAYERSELECT_COMMAND dpc;

					dpc.ProtocolType = c2s_dbplayerselect;
					dpc.m_nSelect = m_nPlayerLoginSelect;

					SendToServer( reinterpret_cast< char * >( &dpc ), sizeof( DB_PLAYERSELECT_COMMAND ) );
					
					strcpy( m_thePlayerInfo.szName, pRLS->m_RoleList[i].szName );
					
					bOpt = true;

					break;
				}
			}
		}
	}

	if ( !bOpt )
	{
		ShowInfomation( "Get role-list failed", SI_COR );

		Shutdown();

		EnableInfoCtrl( FALSE );
	}

	return true;
}

bool CS3RobotDlg::SyncEnd( const char *pMsg )
{
	BYTE cSyscEndFlag = c2s_syncend;

	SendToServer( reinterpret_cast< char * >( &cSyscEndFlag ), sizeof( BYTE ) );

	ShowInfomation( "Login successed", SI_INFO );

	EnableInfoCtrl( TRUE );

	return true;
}

bool CS3RobotDlg::SyncCurPlayerInfo( const char *pMsg )
{
	CURPLAYER_SYNC	*pCS = (CURPLAYER_SYNC *)pMsg;

	if ( pCS )
	{
		CString csInfo;

		m_thePlayerInfo.wHealthMax	= pCS->m_wLifeMax;
		m_thePlayerInfo.wManaMax	= pCS->m_wManaMax;

		m_thePlayerInfo.dwID = pCS->m_dwID;
		csInfo.Format( "%s [%d]", m_thePlayerInfo.szName, pCS->m_dwID );

		m_ctlPlayerInfoList.SetItemText( enumPlayerNameID, 1, csInfo );
	}

	return true;
}

bool CS3RobotDlg::SyncCurPlayerNormalInfo( const char *pMsg )
{
	CURPLAYER_NORMAL_SYNC *pCNS = (CURPLAYER_NORMAL_SYNC *)pMsg;

	if ( pCNS )
	{
		CString csInfo;

		csInfo.Format( "%d / %d", pCNS->m_wLife, m_thePlayerInfo.wHealthMax );
		m_ctlPlayerInfoList.SetItemText( enumCurrentHeath, 1, csInfo );

		csInfo.Format( "%d / %d", pCNS->m_wMana, m_thePlayerInfo.wManaMax );
		m_ctlPlayerInfoList.SetItemText( enumCurrentMana, 1, csInfo );
	}

	return true;
}

bool CS3RobotDlg::GetPlayerChar( const char *pMsg )
{
	PLAYER_SEND_CHAT_SYNC *pPSCS = (PLAYER_SEND_CHAT_SYNC *)pMsg;

	static char szName[MAX_PATH];
	static char szText[MAX_PATH];

	if ( pPSCS )
	{
		CString csContent;
		
		memcpy( szName, pPSCS->m_szSentence, pPSCS->m_btNameLen );
		szName[pPSCS->m_btNameLen] = 0;

		size_t sizeInfo = pPSCS->m_btNameLen + pPSCS->m_btChatPrefixLen;

		size_t tTextLength =
				pPSCS->m_wLength - 
				sizeInfo -
				( sizeof( PLAYER_SEND_CHAT_SYNC ) - sizeof( BYTE ) - sizeof( pPSCS->m_szSentence ) );

		memcpy( szText, pPSCS->m_szSentence + sizeInfo, tTextLength );
		szText[tTextLength] = 0;

		if ( 0 == strcmp( m_thePlayerInfo.szName, szName ) )
		{
			csContent.Format( "I say : %s", szText );
		}
		else
		{
			csContent.Format( "%s say : %s", szName, szText );
		}

		ShowInfomation( csContent );
	}

	return true;
}

bool CS3RobotDlg::SyncNpc( const char *pMsg )
{
	NPC_SYNC *pNS = (NPC_SYNC *)pMsg;

	static char szInfo[32];

	if ( pNS && pNS->ID == m_thePlayerInfo.dwID )
	{
		m_thePlayerInfo.dwCurPixel_x = ( ( ( pNS->RegionID >> 16 ) & 0xFFFF ) << 9 ) + pNS->MapX;
		m_thePlayerInfo.dwCurPixel_y = ( ( pNS->RegionID & 0xFFFF ) << 10 ) + pNS->MapY;

		memset( szInfo, 0, 32 );
		sprintf( szInfo, "x : %d - y : %d", m_thePlayerInfo.dwCurPixel_x, m_thePlayerInfo.dwCurPixel_y );

		m_ctlPlayerInfoList.SetItemText( enumCurrentPosition, 1, szInfo );
	}

	return true;
}

bool CS3RobotDlg::SyncNpcNormal( const char *pMsg )
{
	NPC_NORMAL_SYNC *pNNS = (NPC_NORMAL_SYNC *)pMsg;

	static char szInfo[32];

	if ( pNNS && pNNS->ID == m_thePlayerInfo.dwID )
	{
		m_thePlayerInfo.dwCurPixel_x = ( ( ( pNNS->RegionID >> 16 ) & 0xFFFF ) << 9 ) + pNNS->MapX;
		m_thePlayerInfo.dwCurPixel_y = ( ( pNNS->RegionID & 0xFFFF ) << 10 ) + pNNS->MapY;

		if ( 0 == m_thePlayerInfo.dwCurPixel_target_x || 0 == m_thePlayerInfo.dwCurPixel_target_y )
		{
			m_thePlayerInfo.dwCurPixel_target_x = m_thePlayerInfo.dwCurPixel_x;
			m_thePlayerInfo.dwCurPixel_target_y = m_thePlayerInfo.dwCurPixel_y;
		}

		memset( szInfo, 0, 32 );
		sprintf( szInfo, "x : %d - y : %d", m_thePlayerInfo.dwCurPixel_x, m_thePlayerInfo.dwCurPixel_y );

		m_ctlPlayerInfoList.SetItemText( enumCurrentPosition, 1, szInfo );
	}

	return true;
}

bool CS3RobotDlg::SyncCurPlayerSkill( const char *pMsg )
{
	SKILL_SEND_ALL_SYNC *pSSAS = (SKILL_SEND_ALL_SYNC *)pMsg;

	static char szSkillInfo[32];
	static char szSkillName[20];

	static int nPosIndex = 0;

	if ( pSSAS && m_bSkillTabLoad )
	{
		WORD wDataLen =
			pSSAS->m_wProtocolLong +
			sizeof( BYTE ) +
			sizeof( pSSAS->m_sAllSkill ) - 
			sizeof( SKILL_SEND_ALL_SYNC );

		for ( int i=0; i<( wDataLen / sizeof( SKILL_SEND_ALL_SYNC_DATA ) ); i++ )
		{
			WORD wID = pSSAS->m_sAllSkill[i].SkillId;

			m_tabFile.GetString( wID + 1, "SkillName", "", szSkillName, sizeof( szSkillName ) );

			if ( 0 != szSkillName[0] )
			{
				sprintf( szSkillInfo, "%2.2d. %s [%d]", ++nPosIndex, szSkillName, wID );

				int nIndex = m_ctrlSkillListComBox.AddString( szSkillInfo );
				m_ctrlSkillListComBox.SetItemData( nIndex, wID );
			}
		}
	}

	m_ctrlSkillListComBox.SetCurSel( 0 );

	return true;
}

void CS3RobotDlg::InitFuntionEntry()
{
	ZeroMemory( ProcessArray, sizeof(ProcessArray) );
// Multi Fixed By MrchuCong@gmail.com
	ProcessArray[s2c_ping] = &CS3RobotDlg::ProcessPing;
	ProcessArray[s2c_syncrolelist] = &CS3RobotDlg::Syncrolelist;
	ProcessArray[s2c_syncend] = &CS3RobotDlg::SyncEnd;
	ProcessArray[s2c_synccurplayer] = &CS3RobotDlg::SyncCurPlayerInfo;
	ProcessArray[s2c_synccurplayernormal] = &CS3RobotDlg::SyncCurPlayerNormalInfo;
	ProcessArray[s2c_playersendchat] = &CS3RobotDlg::GetPlayerChar;
	ProcessArray[s2c_syncnpc] = &CS3RobotDlg::SyncNpc;
	ProcessArray[s2c_syncnpcmin] = &CS3RobotDlg::SyncNpcNormal;
	ProcessArray[s2c_synccurplayerskill] = &CS3RobotDlg::SyncCurPlayerSkill;
}

bool CS3RobotDlg::ShowTrayIcon()
{
	::Shell_NotifyIcon( NIM_ADD, &m_nidIconData );

	return true;
}

bool CS3RobotDlg::HideTrayIcon()
{
	::Shell_NotifyIcon( NIM_DELETE, &m_nidIconData );

	return true;
}

HRESULT CS3RobotDlg::OnTrayNotifyMsg( WPARAM wParam, LPARAM lParam )
{
    UINT uID = (UINT)wParam;
    UINT uMsg = (UINT)lParam;

	if ( uID != g_unTrayIconID )
	{
		return S_FALSE;
	}

	CPoint pt;

	switch ( uMsg )
	{
	case WM_LBUTTONDOWN:
		{
			if ( IsWindowVisible() )
			{
				OnStMinimize();
			}
			else
			{
				OnStRestore();
			}
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
		{
			GetCursorPos( &pt );

			CMenu menu;
			if ( menu.LoadMenu( IDR_SYSTRAY_MENU ) )
			{
				CMenu *pSubMenu = menu.GetSubMenu( 0 );

				if ( pSubMenu )
				{
					SetForegroundWindow();
					
					if ( IsWindowVisible() )
					{
						pSubMenu->EnableMenuItem( IDR_ST_RESTORE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED  );
					}
					else
					{
						pSubMenu->EnableMenuItem( IDR_ST_MINIMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED  );
					}

					pSubMenu->TrackPopupMenu( 
								TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
								pt.x,
								pt.y,
								this );
					pSubMenu->SetDefaultItem( 0, TRUE );
				}
			}			
		}
		break;
	}	

	return S_OK;
}

LRESULT CS3RobotDlg::OnTaskBarCreated( WPARAM wParam, LPARAM lParam )
{
	ShowTrayIcon();

	return S_OK;
}

int CS3RobotDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_nidIconData.cbSize = sizeof(NOTIFYICONDATA);

	m_nidIconData.hWnd = this->m_hWnd;
	m_nidIconData.uID = g_unTrayIconID;

	m_nidIconData.uCallbackMessage = WM_TRAY_NOTIFY_MSG;
	m_nidIconData.uFlags = NIF_MESSAGE;

	CString csInfo;
	
	if ( csInfo.LoadString( IDS_TRAYICON_TOOLTIP ) )
	{
		strcpy( m_nidIconData.szTip, csInfo );
	
		m_nidIconData.uFlags |= NIF_TIP;
	}

	HICON hIcon = NULL;

	if ( NULL != ( hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME ) ) )
	{
		m_nidIconData.hIcon = hIcon;

		m_nidIconData.uFlags |= NIF_ICON;
	}

	ShowTrayIcon();
	
	return 0;
}

void CS3RobotDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	if ( m_wGameTimerID )
	{
		timeKillEvent( m_wGameTimerID );
		m_wGameTimerID = 0;
	}

	KillTimer( m_unAppTimer );
	KillTimer( m_unAutoCtrlTimer );

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

	HideTrayIcon();

	KIniFile iniFile;

	iniFile.WriteString( "Logininfo", "AccountName", m_csAccountName );
	iniFile.WriteString( "Logininfo", "Password", m_csPassword );
	iniFile.WriteInteger( "Settings", "RecordCurTime", m_bRecordCurrentTime );
	
	int nIndex = m_ctrlLoginServerComBox.GetCurSel();

	iniFile.WriteInteger( "Settings", "ServerSelIndex", nIndex );
	
	iniFile.Save( "RobotRecorder.ini" );
}

void CS3RobotDlg::OnStExit() 
{
	PostMessage( WM_CLOSE, 0, 0 );	
}

void CS3RobotDlg::OnStMinimize() 
{
	ShowWindow( SW_MINIMIZE );
}

void CS3RobotDlg::OnStRestore() 
{
	ShowWindow( SW_SHOW );
	ShowWindow( SW_RESTORE );

	BringWindowToTop();
	SetForegroundWindow();
}

void CS3RobotDlg::OnSysCommand(UINT nID, LPARAM lParam) 
{
//	if ( ( nID & 0xFFF0) == SC_MINIMIZE )

	CDialog::OnSysCommand(nID, lParam);
}


void CS3RobotDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	switch ( nType )
	{
	case SIZE_MINIMIZED:
		{
			if ( IsWindowVisible() )
			{
				ShowWindow( SW_HIDE );
			}
		}
		break;
	}
}

BOOL CS3RobotDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg && WM_KEYDOWN == pMsg->message )
	{
		if ( VK_ESCAPE == pMsg->wParam )
		{
			return FALSE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
