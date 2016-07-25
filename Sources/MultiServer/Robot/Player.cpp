//******************************************************************************
/*! \file       Player.cpp
*   \brief      简要描述
*
*				详细描述
*   \author		作者
*   \version	版本号
*   \date		2003-6-4 11:49:24
*   \sa			参考内容
*   \todo		将要做的工作
*******************************************************************************/

#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include "Player.h"
#include "..\..\Sword3PaySys\S3AccServer\AccountLoginDef.h"
#include "..\..\s3client\login\LoginDef.h"
#include "..\RobotManager\protocol.h"
#include "..\RobotManager\struct.h"

//#include "..\..\RoleDBManager\kroledbheader.h"
#include "S3DBInterface.h"
#include "KProtocolDef.h"
#include "KProtocol.h"
#include "Macro.h"
#include "Inifile.h"
#include "Utils.h"
#include "tstring.h"
#include "Library.h"
#include "Console.h"
#include "Robot.h"

CCriticalSection		g_csPlayerList;

int	g_nGameProtocolSize[MAX_PROTOCOL_NUM] = 
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
	-1,	//sizeof(NPC_SYNC),			// s2c_syncnpc,
	sizeof(NPC_NORMAL_SYNC),	// s2c_syncnpcmin,
	sizeof(NPC_PLAYER_TYPE_NORMAL_SYNC),	// s2c_syncnpcminplayer,

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

	sizeof(PLAYER_SEND_TEAM_INFO),			// s2c_teaminfo,
	sizeof(PLAYER_SEND_SELF_TEAM_INFO),		// s2c_teamselfinfo,
	sizeof(PLAYER_APPLY_TEAM_INFO_FALSE),	// s2c_teamapplyinfofalse,
	sizeof(PLAYER_SEND_CREATE_TEAM_SUCCESS),// s2c_teamcreatesuccess,
	sizeof(PLAYER_SEND_CREATE_TEAM_FALSE),	// s2c_teamcreatefalse,
	sizeof(PLAYER_TEAM_OPEN_CLOSE),			// s2c_teamopenclose,
	sizeof(PLAYER_APPLY_ADD_TEAM),			// s2c_teamgetapply,
	sizeof(PLAYER_TEAM_ADD_MEMBER),			// s2c_teamaddmember,
	sizeof(PLAYER_LEAVE_TEAM),				// s2c_teamleave,
	sizeof(PLAYER_TEAM_CHANGE_CAPTAIN),		// s2c_teamchangecaptain,
	sizeof(PLAYER_FACTION_DATA),			// s2c_playerfactiondata,
	sizeof(PLAYER_LEAVE_FACTION),			// s2c_playerleavefaction,
	sizeof(PLAYER_FACTION_SKILL_LEVEL),		// s2c_playerfactionskilllevel,
	-1,//sizeof(PLAYER_SEND_CHAT_SYNC),			// s2c_playersendchat,
	sizeof(PLAYER_LEAD_EXP_SYNC),			// s2c_playersyncleadexp
	sizeof(PLAYER_LEVEL_UP_SYNC),			// s2c_playerlevelup
	sizeof(PLAYER_TEAMMATE_LEVEL_SYNC),		// s2c_teammatelevel
	sizeof(PLAYER_ATTRIBUTE_SYNC),			// s2c_playersyncattribute
	sizeof(PLAYER_SKILL_LEVEL_SYNC),		// s2c_playerskilllevel
	sizeof(ITEM_SYNC),						// s2c_syncitem
	sizeof(ITEM_REMOVE_SYNC),				// s2c_removeitem
	sizeof(PLAYER_MONEY_SYNC),				// s2c_syncmoney
	sizeof(PLAYER_MOVE_ITEM_SYNC),			// s2c_playermoveitem
	-1,										// s2c_playershowui
	sizeof(CHAT_APPLY_ADD_FRIEND_SYNC),		// s2c_chatapplyaddfriend
	sizeof(CHAT_ADD_FRIEND_SYNC),			// s2c_chataddfriend
	-1,//sizeof(CHAT_REFUSE_FRIEND_SYNC),		// s2c_chatrefusefriend
	sizeof(CHAT_ADD_FRIEND_FAIL_SYNC),		// s2c_chataddfriendfail
	sizeof(CHAT_LOGIN_FRIEND_NONAME_SYNC),	// s2c_chatloginfriendnoname
	-1,//sizeof(CHAT_LOGIN_FRIEND_NAME_SYNC),	// s2c_chatloginfriendname
	sizeof(CHAT_ONE_FRIEND_DATA_SYNC),		// s2c_chatonefrienddata
	sizeof(CHAT_FRIEND_ONLINE_SYNC),		// s2c_chatfriendinline
	sizeof(CHAT_DELETE_FRIEND_SYNC),		// s2c_chatdeletefriend
	sizeof(CHAT_FRIEND_OFFLINE_SYNC),		// s2c_chatfriendoffline
	sizeof(ROLE_LIST_SYNC),					// s2c_syncrolelist
	sizeof(TRADE_CHANGE_STATE_SYNC),		// s2c_tradechangestate
	-1, // NPC_SET_MENU_STATE_SYNC			   s2c_npcsetmenustate
	sizeof(TRADE_MONEY_SYNC),				// s2c_trademoneysync
	sizeof(TRADE_DECISION_SYNC),			// s2c_tradedecision
	-1, // sizeof(CHAT_SCREENSINGLE_ERROR_SYNC)s2c_chatscreensingleerror
	sizeof(NPC_SYNC_STATEINFO),				// s2c_syncnpcstate,
	-1,	// sizeof(TEAM_INVITE_ADD_SYNC)		   s2c_teaminviteadd
	sizeof(TRADE_STATE_SYNC),				// s2c_tradepressoksync
	sizeof(PING_COMMAND),					// s2c_ping
	sizeof(NPC_SIT_SYNC),					// s2c_npcsit
	sizeof(SALE_BOX_SYNC),					// s2c_opensalebox
	sizeof(NPC_SKILL_SYNC),					// s2cDirectlyCastSkill
	-1,										// s2c_msgshow
	-1,										// s2c_syncstateeffect
	sizeof(BYTE),							// s2c_openstorebox
	sizeof(NPC_REVIVE_SYNC),				// s2c_playerrevive
	sizeof(NPC_REQUEST_FAIL),				// s2c_requestnpcfail
	sizeof(TRADE_APPLY_START_SYNC),			// s2c_tradeapplystart
	sizeof(tagNewDelRoleResponse),			// s3c_rolenewdelresponse
};

using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::_tstring;

CLibrary	CPlayer::s_theRainbowLib( "rainbow.dll" );
// 跟踪角色的名字
extern char *g_pRoleName;
// 跟踪的角色的ID
DWORD CPlayer::m_dwTargetID = ( DWORD )( -1 );

const char *g_szText[] = {
			"%s, 你是谁?",
			"我们坚决拥护%s!",
			"%s, 我爱你! *^_^*",
			"%s, 我帅吧?",
			"我们去打猪吧? %s",
			"%s, 我以后就跟定你了!",
			"%s, 我们这是去哪?",
			"跟着%s走~~~",
			"今天的天气不错嘛! %s",
			"%s, 你还好吧?",
			"看气色不错啊! %s",
			"%s, 我和你去野外转转吧",
			"你们怎么这么多废话, 跟着%s走就行了",
			"%s, 你带我去哪?",
			"呵呵, 这么多人, 好爽啊! %s, 你好!",
			"%s, 我这是在哪? -_-b",
			"%s, 你觉得我漂亮吗?",
			"%s" };

const int g_TextNumber = 17;

struct tagRGB
{
	char r;
	char g;
	char b;
};

#pragma warning( disable : 4305 4309 )

const tagRGB g_rgbText[] = {
		{ 140, 255, 206 },
		{ 255, 227, 33 },
		{ 255, 130, 41 },
		{ 255, 0, 0 },
		{ 255, 113, 206 },
		{ 0, 134, 255 },
		{ 255, 251, 181 },
		{ 255, 255, 255 },
		{ 0, 211, 33 }
		};

const int g_rgbNumber = 9;

CPlayer::CPlayer( const char *pSrvIP, unsigned short nPort, const char *pAccName )
			: m_hQuit( NULL, true, false, pAccName )
			, m_pClientToGateWay( NULL )
			, m_pClientToGameSvr(NULL)
			, m_sGatewayIP( pSrvIP )
			, m_nGatewayPort( nPort )
			, m_sAccName( pAccName )
			, m_nStatus( enumStartup ) 
			, m_dwID( ( DWORD )( -1 ) )
{

	srand( (unsigned)time( NULL ) );

	m_ptCurPos.x = 0;
	m_ptCurPos.y = 0;

	m_ptTagPos.x = 0;
	m_ptTagPos.y = 0;
}

CPlayer::~CPlayer()
{
	Free();	
}

//******************************************************************************
/*! \fn     void CPlayer::Free()
*   \brief  释放和Server的连接
*******************************************************************************/
void CPlayer::Free()
{
	// 关闭到网关的连接
	ShutdownGateway();
	
	// 关闭到游戏服务器的连接
	ShutdownGameSvr();

	NotifyRobotManager();

	// 退出工作线程
	m_hQuit.Set();

	if ( !Wait( 500000 ) )
	{
		Terminate();
	}
}

//******************************************************************************
/*! \fn     void CPlayer::NotifyRobotManager()
*   \brief  通知Robot Manager，Robot做帐号回收
*******************************************************************************/
void CPlayer::NotifyRobotManager()
{
	tagRobotQuit	RobotQuit;

	if(NULL == g_pClientToManager)
		return;

	RobotQuit.cProtocol		= enumRobotQuit;
	strcpy(RobotQuit.szAccName, m_sAccName.c_str());
	

	g_pClientToManager->SendPackToServer( &RobotQuit, sizeof( RobotQuit ) );
}

//******************************************************************************
/*! \fn     bool CPlayer::ConnectToGateway()
*   \brief  连接到网关
*******************************************************************************/
bool CPlayer::ConnectToGateway()
{
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( s_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 5, 1024 * 256 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClientToGateWay ) );
		
		SAFE_RELEASE( pClientFactory );
	}
	else
		return false;

	if ( m_pClientToGateWay )
	{
		m_pClientToGateWay->Startup();
		
		m_pClientToGateWay->RegisterMsgFilter( ( void * )this, ClientEventNotify );

		if ( FAILED( m_pClientToGateWay->ConnectTo( m_sGatewayIP.c_str(), m_nGatewayPort ) ) )
		{
			m_pClientToGateWay->Cleanup();

			SAFE_RELEASE( m_pClientToGateWay );

			return false;
		}
	}
	else
		return false;

	// 启动线程
	Start();

	SetStatus(enumStartup);
	
	return true;
}

//******************************************************************************
/*! \fn     bool CPlayer::ReConnectGateway()
*   \brief  断开与网关和游戏服务器的连接，并再次建立连接
*******************************************************************************/
bool CPlayer::ReConnectGateway()
{
	// 关闭到网关的连接
	ShutdownGateway();
	
	// 关闭到游戏服务器的连接
	ShutdownGameSvr();


	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( s_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 5, 1024 * 256 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClientToGateWay ) );
		
		SAFE_RELEASE( pClientFactory );
	}
	else
		return false;

	if ( m_pClientToGateWay )
	{
		m_pClientToGateWay->Startup();
		
		m_pClientToGateWay->RegisterMsgFilter( ( void * )this, ClientEventNotify );

		if ( FAILED( m_pClientToGateWay->ConnectTo( m_sGatewayIP.c_str(), m_nGatewayPort ) ) )
		{
			m_pClientToGateWay->Cleanup();

			SAFE_RELEASE( m_pClientToGateWay );

			return false;
		}
	}
	else
		return false;

	SetStatus(enumStartup);

	m_dwID			= ( ( DWORD )( -1 ) );

	m_ptCurPos.x	= 0;
	m_ptCurPos.y	= 0;

	m_ptTagPos.x	= 0;
	m_ptTagPos.y	= 0;

	return true;
}

//******************************************************************************
/*! \fn     int CPlayer::Run()
*   \brief  线程的运行函数
*******************************************************************************/
int CPlayer::Run()
{
	// CCriticalSection::Owner locker( g_csPlayerList );
	
	while ( !m_hQuit.Wait( 0 ) )
	{
		ProcessGateway();
		ProcessGameSvr();

		::Sleep( 1 );
	}

	return 0;
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGateway()
*   \brief  处理和网关的通讯
*******************************************************************************/
void CPlayer::ProcessGateway()
{
	if(NULL == m_pClientToGateWay)
		return;

	ProcessGatewayMsg();
	ProcessGatewayLoop();
}

//******************************************************************************
/*! \fn     void ProcessGatewayMsg()
*   \brief  处理网关发来的消息
*******************************************************************************/
void CPlayer::ProcessGatewayMsg()
{
	size_t		datalength	= 0;
	const char *pData		= ( const char * )( m_pClientToGateWay->GetPackFromServer( datalength ) );
	const char *pMsg		= pData;
	int			nMsgLen		= 0;

	while ( pMsg < ( pData + datalength ) )
	{
		BYTE cProtocol = CPackager::Peek( pMsg );
		
		switch ( cProtocol )
		{
		case s2c_login:
			 // 需要错误处理
			/*
			if(!OnAccountLoginNotify(pMsg))
			{
				Free();
				return;
			}
			*/
			OnAccountLoginNotify(pMsg);
			nMsgLen = *( unsigned short * )( pMsg + sizeof( BYTE ) );

			pMsg += sizeof( BYTE ) + nMsgLen;
			break;

		case s2c_roleserver_getrolelist_result:
			nMsgLen = OnGetRoleList(pMsg);
			pMsg += nMsgLen;
			break;

		case s2c_notifyplayerlogin:
			OnPlayerLoginNotify(pMsg);

			pMsg += sizeof(tagNotifyPlayerLogin);

			//ReConnectGateway();
			break;
			
		default:
			return;
		}
	}
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGatewayLoop()
*   \brief  向网关发送的命令
*******************************************************************************/
void CPlayer::ProcessGatewayLoop()
{
	if ( m_nStatus == enumStartup )
	{
		LoginGateway();
		SetStatus(enumConnectToGateway);
	}		
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGameSvr()
*   \brief  处理和GameSvr的通讯
*******************************************************************************/
void CPlayer::ProcessGameSvr()
{
	if (NULL == m_pClientToGameSvr)
		return;
	
	ProcessGameSvrMsg();
	ProcessGameSvrLoop();
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGameSvrMsg()
*   \brief  处理和游戏Server发来的消息
*******************************************************************************/
void CPlayer::ProcessGameSvrMsg()
{
	size_t datalength = 0;
	const char *pData = ( const char * )( m_pClientToGameSvr->GetPackFromServer( datalength ) );

	const char *pMsg = pData;

	while ( pMsg < ( pData + datalength ) )
	{
		BYTE cProtocol = CPackager::Peek( pMsg );
		
		switch ( cProtocol )
		{
		
		case s2c_syncend:		
			SyncEnd();	
			
			//ReConnectGateway();
			break;
			
		case s2c_syncplayer:
			break;

		case s2c_syncnpc:
			{
				NPC_SYNC *pPS = ( NPC_SYNC * )pMsg;

				if ( pPS && ( '\0' != g_pRoleName[0] ) )
				{
					if ( 0 == memcmp( ( const char * )pPS->m_szName, g_pRoleName, sizeof( g_pRoleName ) ) )
					{
						m_dwTargetID = pPS->ID;
					}
				}
			}				
			break;

		case s2c_synccurplayer:
			{
				/*
				CURPLAYER_SYNC	*pCS = (CURPLAYER_SYNC *)pMsg;
				
				if ( pCS )
				{
					m_dwID = pCS->m_dwID;
				}
				*/
				//ReConnectGateway();
			}				
			break;

		case s2c_syncnpcmin:
			{
				if ( ( DWORD )( -1 ) != m_dwID )
				{
					NPC_NORMAL_SYNC *pNS = (NPC_NORMAL_SYNC *)pMsg;
					
					if ( pNS && pNS->ID == m_dwID )
					{
						m_ptCurPos.x = pNS->MapX;
						m_ptCurPos.y = pNS->MapY;
					}
					else if ( pNS && pNS->ID == m_dwTargetID )
					{
						m_ptTagPos.x = pNS->MapX;
						m_ptTagPos.y = pNS->MapY;
					}

					if ( ( DWORD )( -1 ) == m_dwTargetID && '\0' != g_pRoleName[0] )
					{
						NPC_REQUEST_COMMAND nrc;

						nrc.ProtocolType = c2s_requestnpc;
						nrc.ID = pNS->ID;

						m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &nrc ), 
							sizeof( NPC_REQUEST_COMMAND ) );
					}
				}
			}
			break;

		case s2c_syncnpcminplayer:
			{
				if ( ( DWORD )( -1 ) != m_dwID )
				{
					NPC_PLAYER_TYPE_NORMAL_SYNC *pNPTNS = (NPC_PLAYER_TYPE_NORMAL_SYNC *)pMsg;
					
					if ( pNPTNS && pNPTNS->m_dwNpcID == m_dwID )
					{
						m_ptCurPos.x = pNPTNS->m_dwMapX;
						m_ptCurPos.y = pNPTNS->m_dwMapY;
					}
					else if ( pNPTNS && pNPTNS->m_dwNpcID == m_dwTargetID )
					{
						m_ptTagPos.x = pNPTNS->m_dwMapX;
						m_ptTagPos.y = pNPTNS->m_dwMapY;
					}
				}
			}
			break;

		case s2c_playersendchat:
			
			break;
			
		default:
			break;
		}

		/*
		 * Get next protocol
		 */
		int nProtocolSize = g_nGameProtocolSize[ cProtocol - s2c_clientbegin - 1 ];

		if ( nProtocolSize >= 0 )
		{
			pMsg += nProtocolSize;
		}
		else
		{
			unsigned short usMsgSize = *( unsigned short * )( pMsg + sizeof( BYTE ) );

			pMsg += sizeof( BYTE ) + usMsgSize;
		}
	}
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGameSvrLoop()
*   \brief  处理向游戏世界发送消息
*******************************************************************************/
void CPlayer::ProcessGameSvrLoop()
{
	if( NULL == m_pClientToGameSvr)
		return;

	if ( rand() % 100 < 5 )
	{
		char szSpeakText[128];

		int nIndex = rand() % g_TextNumber;

		sprintf( szSpeakText, ( const char * )g_szText[nIndex], g_pRoleName );

		SpeakWord( szSpeakText, strlen( szSpeakText ) );

		//ReConnectGateway();
	}
	else
	{
		WalkTo();
	}
	
	return;
}

//******************************************************************************
/*! \fn     void CPlayer::LoginGateway()
*   \brief  向网关发出登陆请求
*******************************************************************************/
void CPlayer::LoginGateway()
{
	_tstring			buffer;
	KLoginAccountInfo	lai;

	ZeroMemory( &lai, sizeof( KLoginAccountInfo ) );

	// 填充帐号
	size_t used = sizeof( lai.Account );
	used = ( used > m_sAccName.length() ) ? m_sAccName.length() : used;
	memcpy( lai.Account, m_sAccName.c_str(), used );

	// 填充密码
	used = sizeof( lai.Password );
	used = ( used > m_sAccName.length() ) ? m_sAccName.length() : used;
	memcpy( lai.Password, m_sAccName.c_str(), used );

	// 其他信息
	lai.Param = LOGIN_A_LOGIN | LOGIN_R_REQUEST;
	lai.Size = sizeof( lai );

	buffer.resize( lai.Size + 1/* protocol id */ );

	BYTE bProtocol = c2s_login;
	memcpy( const_cast< char * >( buffer.c_str() ), &bProtocol, sizeof( BYTE ) );
	memcpy( const_cast< char * >( buffer.c_str() + 1 ), &lai, sizeof( lai ) );

	// 向网关发送请求
	m_pClientToGateWay->SendPackToServer( buffer.c_str(), buffer.size() );

	return;
}

// 
//******************************************************************************
/*! \fn     bool CPlayer::OnAccountLoginNotify(const char* pMsg)
*   \brief  处理网关发回的帐号登陆请求的结果
*******************************************************************************/
bool CPlayer::OnAccountLoginNotify(const char* pMsg)
{
	KLoginAccountInfo *pLAI = ( KLoginAccountInfo * )( ( const BYTE * )pMsg + 1 );
	bool	bRet	= false;
	
	if ( ( pLAI->Param | LOGIN_A_LOGIN ) &&
		 0 == m_sAccName.compare( pLAI->Account ) &&
		 0 == m_sAccName.compare( pLAI->Password ) )
	{
		if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR )
		{
			bRet	= false;
			//::MessageBox(NULL, "An incorrect account or password!", "Error", MB_OK);
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_FAILED )
		{
			bRet	= false;
			//::MessageBox(NULL, "Login failed!", "Error", MB_OK);
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_ACCOUNT_EXIST )
		{
			bRet	= false;
			//::MessageBox(NULL, "Login failed! your accout was existed!", m_sAccName.c_str(), MB_OK);
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_TIMEOUT )
		{
			bRet	= false;
			//::MessageBox(NULL, "Login failed! no money!", "Error", MB_OK);
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_SUCCESS )
		{
			bRet = true;
		}
	}
	return bRet;
}

//******************************************************************************
/*! \fn     int CPlayer::OnGetRoleListRole(const char* pMsg)
*   \brief  响应网关发来的消息，选择角色列表
*******************************************************************************/
int CPlayer::OnGetRoleList(const char* pMsg)
{
	int nRoleCount		= 0;
	int nSelRoleIndex	= 0;

	const S3DBI_RoleBaseInfo *pRoleList = NULL;
	TProcessData			*pPA		= ( TProcessData * )pMsg;
	
	nRoleCount = *( char * )( pPA->pDataBuffer );
	
	if( nRoleCount > 0)
	{
		pRoleList = ( const S3DBI_RoleBaseInfo * )( ( const char * )( pPA->pDataBuffer ) + 1/* sizeof( char ) */ );
		const size_t datalength = sizeof( S3DBI_RoleBaseInfo ) * nRoleCount;
					
		/*
		 * Send this info to gateway
		 */
		tagDBSelPlayer	tDBSP;
		tDBSP.cProtocol = c2s_dbplayerselect;

		size_t nRoleNameLen = strlen( pRoleList[nSelRoleIndex].szName );
		memcpy( tDBSP.szRoleName, pRoleList[nSelRoleIndex].szName, nRoleNameLen );
		tDBSP.szRoleName[nRoleNameLen] = '\0';

		m_pClientToGateWay->SendPackToServer( &tDBSP, sizeof( tagDBSelPlayer ) );
	}
	
	int nMsgLen = sizeof(TProcessData) + pPA->nDataLen - 1;
	return nMsgLen;
}

//******************************************************************************
/*! \fn     bool CPlayer::OnPlayerLoginNotify(const char* pMsg)
*   \brief  处理网关发来的消息，响应角色进入游戏的通告
*******************************************************************************/
bool CPlayer::OnPlayerLoginNotify(const char* pMsg)
{
	tagNotifyPlayerLogin *pNPL = ( tagNotifyPlayerLogin * )pMsg;

	const char *pIPAddr = OnlineGameLib::Win32::net_ntoa( pNPL->nIPAddr );

	return ConnectToGameSvr( pIPAddr, pNPL->nPort, pNPL->guid );
}

//******************************************************************************
/*! \fn     bool CPlayer::ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid )
*   \brief  处理网关发来的消息，客户连接到网关
*******************************************************************************/
bool CPlayer::ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid )
{
	pfnCreateClientInterface pFactroyFun = ( pfnCreateClientInterface )( s_theRainbowLib.GetProcAddress( "CreateInterface" ) );

	IClientFactory *pClientFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 50, 1024 * 16 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClientToGameSvr ) );
		
		pClientFactory->Release();
	}

	if ( !m_pClientToGameSvr )
	{
		::MessageBox(NULL, "Initialization failed! Don't find a correct rainbow.dll", "Error", MB_OK);

		return false;
	}

	/*
	 * Create a connection
	 */
	m_pClientToGameSvr->Startup();

	//pGameSvrClient->RegisterMsgFilter( reinterpret_cast< void * >( pClient ), NULL );

	if ( FAILED( m_pClientToGameSvr->ConnectTo( pIPAddr, nPort ) ) )
	{
		//::MessageBox(NULL, "Connection failed! Don't connect to game server.", "Error", MB_OK);

		return false;
	}

	tagLogicLogin ll;

	ll.cProtocol = c2s_logiclogin;
	memcpy( &ll.guid, &guid, sizeof( GUID ) );

	m_pClientToGameSvr->SendPackToServer( &ll, sizeof( tagLogicLogin ) );

	ShutdownGateway();

	return true;
}

//******************************************************************************
/*! \fn     void CPlayer::SyncEnd()
*   \brief  响应游戏服务器发来的消息，初始同步结束
*			SyncWorld, SyncNpc, SyncPlayer, SyncCurPlayer, SyncCurPlayerSkill
*			SyncEnd, SyncNpcMinPlayer
*******************************************************************************/
void CPlayer::SyncEnd()
{
	SetStatus( enumPlayGame );

	BYTE cSyscEndFlag = c2s_syncend;
	
	m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &cSyscEndFlag ), sizeof( BYTE ) );
}

//******************************************************************************
/*! \fn     void CPlayer::WalkTo()
*   \brief  机器人行走
*******************************************************************************/
void CPlayer::WalkTo()
{
	if ( ( DWORD )( -1 ) != m_dwTargetID && m_ptTagPos.x && m_ptTagPos.y )
	{
		if ( abs( m_ptCurPos.x - m_ptTagPos.x ) > 5 * 32 ||
			abs( m_ptCurPos.y - m_ptTagPos.y ) > 6 * 32 )
		{
			NPC_WALK_COMMAND	nwc;
			
			nwc.ProtocolType = c2s_npcwalk;
			nwc.nMpsX = m_ptTagPos.x;
			nwc.nMpsY = m_ptTagPos.y;
			
			m_pClientToGameSvr->SendPackToServer( reinterpret_cast< BYTE * >( &nwc ), sizeof( nwc ) );
		}
	}
}

//******************************************************************************
/*! \fn     void CPlayer::SpeakWord( const char *pText, size_t size  )
*   \brief  随机聊天
*******************************************************************************/
void CPlayer::SpeakWord( const char *pText, size_t size /*0*/ )
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
	pscc.m_btCurChannel = 1;	// To all
	pscc.m_btType = 0;			// MSG_G_CHAT

	pscc.m_dwTargetID = 0;
	pscc.m_nTargetIdx = 0;
	pscc.m_btChatPrefixLen = 4;

	pscc.m_szSentence[0] = 0x2;

	int nIndex = rand() % g_rgbNumber;

	pscc.m_szSentence[1] = g_rgbText[nIndex].r;
	pscc.m_szSentence[2] = g_rgbText[nIndex].g;
	pscc.m_szSentence[3] = g_rgbText[nIndex].b;

	memcpy( &( pscc.m_szSentence[4] ), pText, strlen( pText ) );
	pscc.m_wSentenceLen = dataLength;

	pscc.m_wLength = 
		dataLength + 4 +
		sizeof( PLAYER_SEND_CHAT_COMMAND ) - 
		sizeof( pscc.m_szSentence ) - 
		sizeof( BYTE );

	m_pClientToGameSvr->SendPackToServer( &pscc, pscc.m_wLength + sizeof( BYTE ) );
}

//******************************************************************************
/*! \fn     void ShutdownGateway()
*   \brief  切断与网关的连接
*******************************************************************************/
void CPlayer::ShutdownGateway()
{
	// 关闭到网关的连接
	if ( m_pClientToGateWay )
	{
		m_pClientToGateWay->Cleanup();

		SAFE_RELEASE( m_pClientToGateWay );
	}
}

//******************************************************************************
/*! \fn     void ShutdownGateway()
*   \brief  切断与网关的连接
*******************************************************************************/
void CPlayer::ShutdownGameSvr()
{
	// 关闭到网关的连接
	if ( m_pClientToGameSvr )
	{
		m_pClientToGameSvr->Cleanup();

		SAFE_RELEASE( m_pClientToGameSvr );
	}
}

void __stdcall CPlayer::ClientEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnEventType )
{
	CPlayer *pPlayer = ( CPlayer * )lpParam;

	ASSERT( pPlayer );

	try
	{
		pPlayer->_ClientEventNotify( ulnEventType );
	}
	catch(...)
	{
		::MessageBeep( -1 );
	}
}

void CPlayer::_ClientEventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		::MessageBeep( -1 );

		break;

	case enumServerConnectClose:

		::MessageBeep( -1 );

		break;
	}
}