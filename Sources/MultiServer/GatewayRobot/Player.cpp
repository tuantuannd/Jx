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
#pragma once

//#include <Windows.h>
#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include "Win32Exception.h"
//#include "KWin32.h"
// that fucking macro defined here
#include "Player.h"
//#include "..\..\Sword3PaySys\S3AccServer\AccountLoginDef.h"
#include "..\..\s3client\login\LoginDef.h"
//#include "..\..\Core\srcMsgGenreDef.h"
#include "KProtocolDef.h"
#include "KRelayProtocol.h"
#include "KProtocol.h"
#include "md5.h"

using namespace OnlineGameLib::Win32;

extern int g_nLoginLogoutFlag;
extern int g_nDeleteRoleFlag;

extern int g_nAddRoleFlag;
extern int g_nAddRoleGender;
extern int g_nAddRoleSeries;
extern int g_nAddRolePlaceID;
extern char g_szGMCommandList[MAX_GM_COUNT][MAX_GM_SIZE];
//#define _DETAIL_PROTOCOL_DEBUG_
//#define _GENERAL_DEBUG_

CCriticalSection		g_csPlayerList;
//extern int	g_nProtocolSize[MAX_PROTOCOL_NUM];
//#define g_nGameProtocolSize g_nProtocolSize
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

	-1,//sizeof(OBJ_ADD_SYNC),	// s2c_objadd,
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
	-1,//sizeof(CHAT_REFUSE_FRIEND_SYNC),	// s2c_chatrefusefriend
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
	sizeof(tagNewDelRoleResponse),			// s2c_rolenewdelresponse
	sizeof(ITEM_AUTO_MOVE_SYNC),			// s2c_ItemAutoMove
	sizeof(BYTE),							// s2c_itemexchangefinish
	sizeof(SYNC_WEATHER),					// s2c_changeweather
	sizeof(PK_NORMAL_FLAG_SYNC),			// s2c_pksyncnormalflag
	-1,//sizeof(PK_ENMITY_STATE_SYNC),		// s2c_pksyncenmitystate
	-1,//sizeof(PK_EXERCISE_STATE_SYNC),	// s2c_pksyncexercisestate
	sizeof(PK_VALUE_SYNC),					// s2c_pksyncpkvalue
	sizeof(NPC_SLEEP_SYNC),					// s2c_npcsleepmode
	sizeof(VIEW_EQUIP_SYNC),				// s2c_viewequip
	sizeof(LADDER_DATA),					// s2c_ladderresult
	-1,										// s2c_ladderlist
	sizeof(TONG_CREATE_SYNC),				// s2c_tongcreate
	sizeof(PING_COMMAND),					// s2c_replyclientping
	sizeof(NPC_GOLD_CHANGE_SYNC),			// s2c_npcgoldchange
	sizeof(ITEM_DURABILITY_CHANGE),			// s2c_itemdurabilitychange
};

using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::_tstring;

#if defined _USE_RAINBOW
CLibrary	CPlayer::s_theRainbowLib( "rainbow.dll" );
#else
//#include "IClient.h"
#endif
// 跟踪角色的名字
extern char *g_pRoleName;
// 跟踪的角色的ID
DWORD CPlayer::m_dwTargetID = ( DWORD )( -1 );

// The timer of chat.. 
// the value is 0.8 * 50
const int	g_nChatTimer = 40;

const char *g_szText[] = {
			"%s, who are you??",
			"Shut up your fucking mouth, %s!",
			"%s, I love you..",
			"Oh, %s, what a fucking day today, isn't it??",
			"you looks like a white, fat, stupid pig, %s, I swear.",
			"Michael Jackson is the king of POP",
			"Michael Jordan is the king of Ball",
			"who knows what dangdangdangdangdangdang, dangdangdangdangdangdang is?",
			"dangdangdangdangdangdang, dangdangdangdangdangdang is ...。。。oooOOO Only you, can get the bible with me.",
			"Only you, can kill the ghost.",
			"Only you, can protect me, prevent the fishes eatting me....",
			"%s, Do you have any brothers? Please speak, I just wanna get a friend before death..",
			"We are both Athena's saint. We should fight for the love and justice of the world.",
			"%s, What fucking you are doing??",
			"Oh, shit!! %s, I meet Michael Jordan today, ohhhhh!",
			"Note: Don't trust benchmarks more than you trust politicians. Trust my benchmarks even less.",
			"%s, I think it's time for sleeping.",
			"Linus Benedict Torvalds: Read The Fucking Source Code!" 
};

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
    m_pbyRoleMsg = NULL;

	m_ptCurPos.x = 0;
	m_ptCurPos.y = 0;

	m_ptTagPos.x = 0;
	m_ptTagPos.y = 0;

    m_RandValue = rand();

    m_uGameServerLoopCount = 0;

    m_nShutDownGatewayFlag = false;

	// The initialization of all flags goes here.
	// flying comment
	m_nMoveRange = 800;
	m_nFlgSetPos = 0;
	m_nFlgSetAttack = 0;
	m_nFlgSilence = 0;
	m_nFlgChatTimer = 0;
	m_nFlgGMDone = false;

	m_nChatTimer = 0;
	m_dwNpcID = 0;
	m_nFlgAI = 0;
	m_nDoing = 0;
	m_nTeamLeader = 0;
	memset(&m_ptTarget, 0, sizeof(POINT));
	memset(&m_ptBirth, 0, sizeof(POINT));	
	memset(&m_ptOffset, 0, sizeof(POINT));
	memset(&m_player, 0, sizeof(PLAYER_INFO));
	memset(szGMCommand, 0, 128);
	// 机器人运动偏移量
	m_ptOffset.x = 500;
	m_ptOffset.y = 300;
	m_nGMChannelID = 0;
	m_nCurChannel = 0;
	// ext size
	g_nGameProtocolSize[s2c_extend - s2c_clientbegin - 1] = -1;
	g_nGameProtocolSize[s2c_extendchat - s2c_clientbegin - 1] = -1;
	g_nGameProtocolSize[s2c_extendfriend - s2c_clientbegin - 1] = -1;
	g_nGameProtocolSize[s2c_extendtong - s2c_clientbegin - 1] = -1;
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

	// 退出工作线程
	m_hQuit.Set();

    if (m_pbyRoleMsg)
    {
        delete []m_pbyRoleMsg;
        m_pbyRoleMsg = NULL;
    }

    try
    {
	    if ( !Wait( 500000 ) )
	    {
		    Terminate();
	    }
    }
    catch (...) 
    {
    }
}

//******************************************************************************
/*! \fn     bool CPlayer::ConnectToGateway()
*   \brief  连接到网关
*******************************************************************************/
bool CPlayer::ConnectToGateway()
{
#if defined _USE_RAINBOW
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( s_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 256 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClientToGateWay ) );
		
		SAFE_RELEASE( pClientFactory );
	}
	else
		return false;
#else
	// zcj's client goes here.
#endif
	if ( m_pClientToGateWay )
	{
		int nRetCode = 0;
		nRetCode = m_pClientToGateWay->Startup();
#if defined _USE_RAINBOW		
		m_pClientToGateWay->RegisterMsgFilter( ( void * )this, ClientEventNotify );
#else
		//typedef void (*CALLBACK_CLIENT_EVENT )(void *lpParam, const unsigned long &ulnEventType);
		m_pClientToGateWay->RegisterMsgFilter( ( void * )this, ClientEventNotify );
#endif
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
/*! \fn     int CPlayer::Run()
*   \brief  线程的运行函数
*******************************************************************************/

static long gs_lRandSeed = time(NULL);

int CPlayer::Run()
{
	// CCriticalSection::Owner locker( g_csPlayerList );
    srand(InterlockedIncrement(&gs_lRandSeed));
	while ( !m_hQuit.Wait( 0 ) )
	{
		m_nChatTimer++;
		ProcessGateway();
		ProcessGameSvr();
		// Keep the loop per-second to exactly 20 times.
		::Sleep( 50 );
		// hardware overflow? what a fucking thing!!
		if (m_nChatTimer > g_nChatTimer * 1000)
			m_nChatTimer = 0;
	}

	return 0;
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGateway()
*   \brief  处理和网关的通讯
*******************************************************************************/
void CPlayer::ProcessGateway()
{
	if (NULL == m_pClientToGateWay)
	{
		return;
	}

	ProcessGatewayMsg();
	ProcessGatewayLoop();
}

//******************************************************************************
/*! \fn     void ProcessGatewayMsg()
*   \brief  处理网关发来的消息
*******************************************************************************/
void CPlayer::ProcessGatewayMsg()
{
    if (GetStatus() == enumExitGame)
        return;

	size_t		datalength	= 0;
    bool        bRetCode    = false;
	const char *pData		= ( const char * )( m_pClientToGateWay->GetPackFromServer( datalength ) );
	const char *pMsg		= pData;
	int			nMsgLen		= 0;

    // for debug
//    if (datalength > 0) {
//        for (int i = 0; i < datalength; i++)
//            printf("%d_", pData[i]);
//        printf("\n");
//    }
//	printf("The length of fucking data: %d\n", datalength);
	while ( pMsg < ( pData + datalength ) )
    {
        if (GetStatus() == enumExitGame)
            return;

		BYTE cProtocol = CPackager::Peek( pMsg );
		switch ( cProtocol )
		{
		case s2c_login:
			 // 需要错误处理
			bRetCode =	OnAccountLoginNotify(pMsg);
            if (!bRetCode)
            {
                Sleep(rand() % 100);
                SetStatus(enumExitGame);
            }

			nMsgLen = *( unsigned short * )( pMsg + sizeof( BYTE ) );

			pMsg += sizeof( BYTE ) + nMsgLen;
			break;
		
		case s2c_roleserver_getrolelist_result:
        {
            printf("%s: Getrolelist Result ok!\n", m_sAccName.c_str());
            
            // Save RoleMsg for next process
            if (m_pbyRoleMsg)
            {
                delete []m_pbyRoleMsg;
                m_pbyRoleMsg = NULL;
            }

	        const S3DBI_RoleBaseInfo *pRoleList = NULL;
	        TProcessData *pPA = ( TProcessData * )pMsg;
	
        	nMsgLen = sizeof(TProcessData) + pPA->nDataLen - 1;

            m_pbyRoleMsg = new unsigned char [0x1000];
            if (!m_pbyRoleMsg)
            {
                Sleep(rand() % 100);
                SetStatus(enumExitGame);
                break;
            }

            memcpy(m_pbyRoleMsg, pMsg, nMsgLen);

			nMsgLen = OnGetRoleList((char *)m_pbyRoleMsg);
			pMsg += nMsgLen;


            //Sleep(rand() % 10);
            //SetStatus(enumExitGame);
			break;
        }

        //case s2c_logout:
        //    OnAccountLogoutNotify(pMsg);
        //    break;

		case s2c_rolenewdelresponse:
        {
			tagNewDelRoleResponse* pResponse = (tagNewDelRoleResponse*)pMsg;

            printf("%s: Rolenewdelresponse Result %d!\n", m_sAccName.c_str(), pResponse->bSucceeded);
            //if (!pResponse->bSucceeded)
            //{
            //  
            //}

            pMsg += sizeof(tagNewDelRoleResponse);

            if (g_nAddRoleFlag || g_nDeleteRoleFlag)
            {
	            const S3DBI_RoleBaseInfo *pRoleList = NULL;
	            TProcessData			*pPA		= ( TProcessData * )m_pbyRoleMsg;
	    
            	int nRoleCount = *( char * )( pPA->pDataBuffer );
               
                if (g_nDeleteRoleFlag)
                {
                    if (nRoleCount > 0)
                    {
                        OnGetRoleList((char *)m_pbyRoleMsg);
                        break;
                    }
                }
                else if (g_nAddRoleFlag)
                {
                    if (nRoleCount < 3)
                    {
                        OnGetRoleList((char *)m_pbyRoleMsg);
                        break;
                    }
                }
            }

            Sleep(rand() % 100);
            SetStatus(enumExitGame);

            break;
        }

		case s2c_notifyplayerlogin:
            
            // 暂时不测试登入服务器的动作
            //Sleep(rand() % 100);
            //SetStatus(enumExitGame);
            printf("%s: Notifyplayerlogin...\n", m_sAccName.c_str());
            			
            bRetCode = OnPlayerLoginNotify(pMsg);
            if (!bRetCode)
            {
                Sleep(rand() % 100);
                SetStatus(enumExitGame);
            }

            ShutdownGateway();

            if (GetStatus() == enumExitGame)
                return;

			pMsg += sizeof(tagNotifyPlayerLogin);
			break;
			
		default:
            Sleep(rand() % 100);
            SetStatus(enumExitGame);
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
	{
		return;
	}
	// Bug is in "ProcessGameSvrMsg"
	ProcessGameSvrMsg();
	ProcessGameSvrLoop();
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGameSvrMsg()
*   \brief  处理和游戏Server发来的消息
*******************************************************************************/
void CPlayer::ProcessGameSvrMsg()
{
	char *pData = NULL;
	char *pMsg = NULL;
	char* pGetData = NULL;
	unsigned int nGetLength;

	pData = (char *)( m_pClientToGameSvr->GetPackFromServer( nGetLength ) );
	pMsg = pData;
	while (nGetLength)
	{
		while ( pMsg < ( pData + nGetLength ) )
		{
#ifdef _GENERAL_DEBUG_
			printf("%d ==== %d\n", pMsg, pData + nGetLength);
#endif
			BYTE cProtocol = CPackager::Peek( pMsg );
			/*
				if ( !pData )
				{ 
					return 0; 
				} 
				return *( ( const BYTE * )pData + index ); 			
			*/

			switch ( cProtocol )
			{
			
			case s2c_syncend:
				SyncEnd();
            
				printf("%s: Enter Game Server SyncEnd!\n", m_sAccName.c_str());

				if (g_nLoginLogoutFlag)
				{
					Sleep(rand() % 600);

					ShutdownGameSvr();

					SetStatus(enumExitGame);
				}
				break;
			case s2c_syncworld:
			{
				WORLD_SYNC *pWorldSync = (WORLD_SYNC *)pMsg;
				//pWorldSync->SubWorld
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: SyncWorld\n");
#endif
			}
				break;
			
			case s2c_syncplayer:
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: SyncPlayer\n");
#endif
				break;

			case s2c_syncnpc:
				{
					NPC_SYNC *pPS = ( NPC_SYNC * )pMsg;
#ifdef _DETAIL_PROTOCOL_DEBUG_
					printf("Protocol: SyncNpc\n");
#endif
					printf("NPC ID: %d\n", pPS->ID);
					if ( pPS && ( '\0' != g_pRoleName[0] ) )
					{
						if ( 0 == memcmp( ( const char * )pPS->m_szName, g_pRoleName, sizeof( g_pRoleName ) ) )
						{
							m_dwTargetID = pPS->ID;
						}
						else
						{
							if (m_nFlgSetAttack)
							{
								ProcessNpcSync((NPC_NORMAL_SYNC*)pPS);
							}
						}
					}

				}
				break;
			case s2c_syncnpcmin:
//				{
//					NPC_NORMAL_SYNC *pNS = (NPC_NORMAL_SYNC *)pMsg;
//#ifdef _DETAIL_PROTOCOL_DEBUG_
//					printf("Protocol: SyncNpcMin [%d]\n");
//#endif
//					printf("NPC Found: ID [%d]\tCamp [%d]\tDoing [%d]\n",
//						pNS->ID, pNS->Camp, pNS->Doing);
//					if ( ( DWORD )( -1 ) != m_dwID )
//					{
//						if ( pNS && pNS->ID == m_dwID )
//						{
//							m_ptCurPos.x = pNS->MapX;
//							m_ptCurPos.y = pNS->MapY;
//						}
//						else if ( pNS && pNS->ID == m_dwTargetID )
//						{
//							m_ptTagPos.x = pNS->MapX;
//							m_ptTagPos.y = pNS->MapY;
//						}
//						else
//						{
//							// flying add this
//							// Auto attack against an NPC
//							if (m_nFlgSetAttack)
//							{
//								printf("XXXXXXXX A NPC will be attacked!!!!\n");
//								ProcessNpcSync(pNS);
//							}
//						}
//						if ( ( DWORD )( -1 ) == m_dwTargetID && '\0' != g_pRoleName[0] )
//						{
//							if (
//								(rand() % 200) < (rand() % 100)
//							)
//							{
//								NPC_REQUEST_COMMAND nrc;
//
//								nrc.ProtocolType = c2s_requestnpc;
//								nrc.ID = pNS->ID;
//
//								m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &nrc ), 
//									sizeof( NPC_REQUEST_COMMAND ) );
//							}
//						}
//					}
//				}
				break;
			case s2c_synccurplayer:
				{
					CURPLAYER_SYNC	*pCS = (CURPLAYER_SYNC *)pMsg;
#ifdef _DETAIL_PROTOCOL_DEBUG_
					printf("Protocol: SyncPlayer\n");
#endif
					if ( pCS )
					{
						m_dwID = pCS->m_dwID;
					}
				}				
				break;

			case s2c_syncnpcminplayer:
				{
#ifdef _DETAIL_PROTOCOL_DEBUG_
					printf("Protocol: SyncNpcMinPlayer\n");
#endif
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

			case s2c_skillcast:
			{
				NPC_SKILL_SYNC *pNpcSkillSync = (NPC_SKILL_SYNC *)pMsg;
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: SkillCast\n");
#endif
				if (pNpcSkillSync->ID == m_dwTargetID)  // 如果主角攻击，大家一起配合
				{
					NPC_SKILL_COMMAND NpcSkillCmd;

					NpcSkillCmd.ProtocolType = c2s_npcskill;
					NpcSkillCmd.nSkillID = pNpcSkillSync->nSkillID;

					NpcSkillCmd.nMpsX = pNpcSkillSync->nMpsX;
					NpcSkillCmd.nMpsY = pNpcSkillSync->nMpsY;
					ASSERT(m_pClientToGameSvr);
					m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &NpcSkillCmd ), 
						sizeof( NPC_SKILL_COMMAND ) );
				}
			}
			// flying add this branch
			// the robot has killed his "enemy NPC", oh yeah!!!! -_-|
			case s2c_npcdeath:
			{
				NPC_DEATH_SYNC* death = (NPC_DEATH_SYNC*) pMsg;
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: NpcDeath\n");
#endif
				if (death->ID == m_dwID)
				{
					// do revive
					//printf("ACTION: Die!! Revive at the local place!\n");
					NPC_REVIVE_COMMAND	ReviveCmd;

					ReviveCmd.ProtocolType = c2s_playerrevive;
					//ReviveCmd.ReviveType = LOCAL_REVIVE_TYPE;
					ASSERT(m_pClientToGameSvr);
					m_pClientToGameSvr->SendPackToServer((BYTE *)&ReviveCmd, sizeof(NPC_REVIVE_COMMAND));
				}
				else if (death->ID == m_dwNpcID)
				{
					//printf("ACTION: %d Die!!!\n", m_dwNpcID);
					m_nDoing = do_none;
					m_dwNpcID = 0;
				}
			}
				break;
			// flying add this branch
			// 组队相关
			// 入队邀请
			case s2c_teaminviteadd:
			{
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: TeamInviteAdd\n");
#endif
				TEAM_INVITE_ADD_SYNC* pSync = (TEAM_INVITE_ADD_SYNC *) pMsg;			
				TEAM_REPLY_INVITE_COMMAND	sReply;
				sReply.ProtocolType = c2s_teamreplyinvite;
				sReply.m_nIndex = pSync->m_nIdx;
				sReply.m_btResult = true;
				ASSERT(m_pClientToGameSvr);
				m_pClientToGameSvr->SendPackToServer(&sReply, sizeof(TEAM_REPLY_INVITE_COMMAND));
			}
				break;
			// 更换队长了
			case s2c_teamchangecaptain:
			{
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: TeamChangeCaptain\n");
#endif
				PLAYER_TEAM_CHANGE_CAPTAIN* pData = (PLAYER_TEAM_CHANGE_CAPTAIN*)pMsg;
				m_nTeamLeader = pData->m_dwCaptainID;
				// 反馈拒绝信息
			}
				break;
			// 离队邀请（叫你走人）
			case s2c_teamleave:
			{
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: TeamLeave\n");
#endif
				PLAYER_LEAVE_TEAM* pData = (PLAYER_LEAVE_TEAM*)pMsg;
				if (pData->m_dwNpcID == m_dwID)
					m_nTeamLeader = 0;
			}
				break;

			// 跨服务器
			case s2c_notifyplayerexchange:
			{
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Protocol: NotifyPlayerExchange\n");
#endif
				ProcessSwitchGameServer((void *)pMsg);
			}
				break;
			// linux服务器开始测试了
			case s2c_ping:
			{
				PING_COMMAND* pData = (PING_COMMAND*)pMsg;				
				if (pData == NULL)
					printf("Ping command data error!\n");
				else
				{
					SendPing(pData->m_dwTime);
				}
			}
				break;
			// 服务端同步装备
			case s2c_syncitem:
				break;
			// 获取玩家聊天信息
			case s2c_playersendchat:
			{
				PLAYER_SEND_CHAT_SYNC* pData = (PLAYER_SEND_CHAT_SYNC *)pMsg;
#ifdef _DETAIL_PROTOCOL_DEBUG_				
				printf("Protocol: SyncSendChat\n");
#endif
			}
				break;
			// 扩展协议
			case s2c_extend:
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Extend Protocol: [%d]\n", cProtocol);
#endif
				ProcessExtProtocol((char *)pMsg);
				break;
			default:
#ifdef _DETAIL_PROTOCOL_DEBUG_
				printf("Ignored Protocol: [%d]\n", cProtocol);
#endif
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
		} // while ( pMsg < ( pData + nGetLength ) )
		pData = (char *)( m_pClientToGameSvr->GetPackFromServer( nGetLength ) );
#ifdef _GENERAL_DEBUG_
		printf("%d bytes m_pClientToGameSvr->GetPackFromServer\n", nGetLength);
#endif
		pMsg = pData;
	} // while (nGetLength)
}

//******************************************************************************
/*! \fn     int CPlayer::ProcessExtProtocol(void* pMsg)
*   \brief  处理扩展协议
*******************************************************************************/
// flying add this
int CPlayer::ProcessExtProtocol(char* pMsg)
{
	int nResult = false;
	int nProtoType = 0;
//	int wLength = 0;
	char* pData = NULL;
	pData = pMsg + sizeof(tagExtendProtoHeader);

	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;
	ASSERT(pHeader->ProtocolFamily == pf_playercommunity);

	if (pMsg == NULL)
		goto Exit0;

	switch (pHeader->ProtocolID)
	{
	// query channel id result:
	case playercomm_s2c_notifychannelid:
	{
		// get the result first
		PLAYERCOMM_NOTIFYCHANNELID* pResult = (PLAYERCOMM_NOTIFYCHANNELID *)pData;
		m_nCurChannel = pResult->channelid;
		// subcribe the chat channel
		char szBuffer[sizeof(tagExtendProtoHeader) + sizeof(PLAYERCOMM_SUBSCRIBE)];
		tagExtendProtoHeader Header;
		PLAYERCOMM_SUBSCRIBE setChannel;
		Header.ProtocolType = c2s_extend;
		Header.wLength = sizeof(tagExtendProtoHeader) + sizeof(CHAT_SET_CHANNEL_COMMAND) - 1;
		setChannel.ProtocolFamily = pf_playercommunity;
		setChannel.ProtocolID = playercomm_c2s_subscribe;
		setChannel.channelid = m_nCurChannel;
		setChannel.subscribe = true;
		memcpy(szBuffer, &Header, sizeof(tagExtendProtoHeader));
		memcpy(szBuffer + sizeof(tagExtendProtoHeader), &setChannel, sizeof(PLAYERCOMM_SUBSCRIBE));
		m_pClientToGameSvr->SendPackToServer( szBuffer , 
			sizeof(tagExtendProtoHeader) + sizeof( PLAYERCOMM_SUBSCRIBE ) );
	}
		break;
	default:
		break;
	}
	nResult = true;
Exit0:
	return nResult;
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessSwitchGameServer()
*   \brief  切换游戏服务器
*******************************************************************************/
// flying add this
int CPlayer::ProcessSwitchGameServer(void* lpBuffer)
{
	int nResult = false;

	tagNotifyPlayerExchange* pInfo = (tagNotifyPlayerExchange*)lpBuffer;
	//_ASSERT(pInfo && pInfo->cProtocol == s2c_notifyplayerexchange);
	DWORD	dwIp = pInfo->nIPAddr;
	int	nPort = pInfo->nPort;
	GUID guid;
	guid = pInfo->guid;
	printf("Switch game server: %d : %d\n", dwIp, nPort);
	if (dwIp && nPort)
	{
		// the first step is to close the connection from gameserver
		m_pClientToGameSvr->Shutdown();
		// 开始与GameSvr进行连接
		if (ConnectToGameSvr((char *)&dwIp, nPort, guid))
			goto Exit1;
		else
			goto Exit0;
	}
	else
	{
		goto Exit0;
	}
Exit1:
	nResult = true;
Exit0:
	return nResult;
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessGameSvrLoop()
*   \brief  处理向游戏世界发送消息
*******************************************************************************/
void CPlayer::ProcessGameSvrLoop()
{
	int nRandom = 0;
//#define		SEED	65530
#define		SEED	1024
	if( NULL == m_pClientToGameSvr)
		return;
	nRandom = rand();
	//if ( rand() % 65530 < 5 )
	// flying use this instead.
	if (m_nFlgChatTimer)
	{
		// TODO: Timer-chat goes here.
		if (m_nChatTimer % 40 == 0)
		{
			char szSpeakText[128];
			int nIndex = rand() % g_TextNumber;
			printf("Chat timer\n");
			sprintf(szSpeakText, (const char *)g_szText[nIndex], g_pRoleName);
			SpeakWord(szSpeakText, strlen(szSpeakText));
		}
	}
	if (nRandom % SEED < 4)
	{
		if (m_nFlgSilence == 0 && m_nFlgChatTimer == 0)
		{
			char szSpeakText[128];
			int nIndex = rand() % g_TextNumber;

			sprintf(szSpeakText, (const char *)g_szText[nIndex], g_pRoleName);
			SpeakWord(szSpeakText, strlen(szSpeakText));
			//printf("Speaking\n");
		}
	}
	// cast...
	else if ( nRandom % SEED < 256)
	{
		CastSkill();
	}
	// flying add this branch, to walk randomly
	else if ( nRandom % SEED < 512)
	{
		NPC_WALK_COMMAND cmd;
		//cmd.CmdKind = do_walk;
		//printf("ACTION: Robot walk, haha!\n");
		cmd.ProtocolType = c2s_npcwalk;
		if (rand() % 2)
			cmd.nMpsX = m_ptCurPos.x + rand() % m_ptOffset.x;
		else
			cmd.nMpsX = m_ptCurPos.x - rand() % m_ptOffset.x;
		if (rand() % 2)
			cmd.nMpsY = m_ptCurPos.y + rand() % m_ptOffset.y;
		else
			cmd.nMpsY = m_ptCurPos.y - rand() % m_ptOffset.y;
		//printf("Walk...\n");
		m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &cmd ), 
			sizeof( NPC_WALK_COMMAND ) );
		//printf("Walk done\n");
	}
	else
	{
        m_uGameServerLoopCount++;
        if ((m_uGameServerLoopCount % 5) == 0)
        {
    		WalkTo();
        }
	}
	// 定时ping一把
	return;
}

//******************************************************************************
/*! \fn     void CPlayer::SendPing()
*   \brief  发送ping命令而已，所以不对外提供。
*******************************************************************************/
// flying add this
void CPlayer::SendPing(DWORD dTime)
{
	PING_CLIENTREPLY_COMMAND PingCmd;
	
	PingCmd.ProtocolType = c2s_ping;
	PingCmd.m_dwReplyServerTime = dTime;
	PingCmd.m_dwClientTime = GetTickCount();
	if (m_pClientToGameSvr)
	{
		m_pClientToGameSvr->SendPackToServer((BYTE*)&PingCmd, sizeof(PING_CLIENTREPLY_COMMAND));
		printf("Reply ping command: [%d]....\n", dTime);
	}
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessNpcSync(NPC_SYNC* pPS)
*   \brief  处理NPC同步
*******************************************************************************/
// flying add this
int CPlayer::ProcessNpcSync(NPC_NORMAL_SYNC* pPS)
{
	int nResult = false;
	// 自己
	if (pPS->ID == m_dwID)
	{
		m_player.nCamp = pPS->Camp;
		goto Exit1;
	}
	// 普通战斗NPC
	//if (pPS->m_btKind == kind_normal)
	if (pPS->Camp == 0)
	{
		if (m_nDoing == do_skill || m_nDoing == do_attack)
			goto Exit0;
		m_nDoing = do_skill;
		m_dwNpcID = pPS->ID;
        NPC_SKILL_COMMAND NpcSkillCmd;

        NpcSkillCmd.ProtocolType = c2s_npcskill;
        NpcSkillCmd.nSkillID = pPS->ID;

        NpcSkillCmd.nMpsX = pPS->MapX;
        NpcSkillCmd.nMpsY = pPS->MapY;

        m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &NpcSkillCmd ), 
			sizeof( NPC_SKILL_COMMAND ) );
		goto Exit1;
	}
	goto Exit0;
Exit1:
	nResult = true;
Exit0:
	return nResult;
}

//******************************************************************************
/*! \fn     void CPlayer::ProcessPlayerSync(NPC_PLAYER_TYPE_NORMAL_SYNC* pPS)
*   \brief  处理NPC同步
*******************************************************************************/
// flying add this
int CPlayer::ProcessPlayerSync(NPC_PLAYER_TYPE_NORMAL_SYNC* pPS)
{
	int nResult = false;
	m_nDoing = pPS->m_byDoing;
	
	nResult = true;
//Exit0:
	return nResult;
}

//******************************************************************************
/*! \fn     int CPlayer::SetSilence(int nSilence)
*   \brief  设定机器人沉默
*******************************************************************************/
// add by flying
int CPlayer::SetChatTimer(int nTimer)
{
	m_nFlgChatTimer = nTimer;
	return true;
}

//******************************************************************************
/*! \fn     int CPlayer::SetSilence(int nSilence)
*   \brief  设定机器人沉默
*******************************************************************************/
// add by flying
int CPlayer::SetSilence(int nSilence)
{
	m_nFlgSilence = nSilence;
	return true;
}

//******************************************************************************
/*! \fn     int CPlayer::SetAttack()
*   \brief  设定机器人自动攻击NPC
*******************************************************************************/
// add by flying
int CPlayer::SetAttack()
{
	m_nFlgSetAttack = 1;
	return true;
}

//******************************************************************************
/*! \fn     void CPlayer::SetPos()
*   \brief  通过GM指令移动机器人
*******************************************************************************/
// add by flying
int CPlayer::SetPos(int nX, int nY)
{
	int nResult = false;
	m_nFlgSetPos = 1;
	m_ptCurPos.x = nX;
	m_ptCurPos.y = nY;
	nResult = true;
//Exit0:
	return nResult;
}

//******************************************************************************
/*! \fn     void CPlayer::DoSetPos()
*   \brief  通过GM指令移动机器人
*******************************************************************************/
// add by flying
int CPlayer::DoSetPos()
{
	int nResult = false;
	char szGmCommand[128];
	// 判断XY的合法性，待定

	memset(szGmCommand, 0, 128);
	// 这里需要具体的Gm指令 ?gm ds SetPos(x, y)
	sprintf(szGmCommand, "?gm ds SetPos(%d, %d)", m_ptCurPos.x, m_ptCurPos.y);
	//printf("CM: %s\n", szGmCommand);
	SpeakWord(szGmCommand, strlen(szGmCommand));
	m_nFlgSetPos = 0;
	nResult = true;
	return nResult;
}

//******************************************************************************
/*! \fn     void CPlayer::LoginGateway()
*   \brief  向网关发出登陆请求
*******************************************************************************/
void CPlayer::LoginGateway()
{
	_tstring			buffer;
	KLoginAccountInfo	lai;
	char szTempPasswd[64];

	ZeroMemory( &lai, sizeof( KLoginAccountInfo ) );
	memset(szTempPasswd, 0, 64);
	// 填充帐号
	size_t used = sizeof( lai.Account );
	used = ( used > m_sAccName.length() ) ? m_sAccName.length() : used;
	memcpy( lai.Account, m_sAccName.c_str(), used );

	// 填充密码
	used = sizeof( lai.Password );
	used = ( used > m_sAccName.length() ) ? m_sAccName.length() : used;
	//memcpy( (void *)&(lai.Password), m_sAccName.c_str(), used );	
	memcpy(szTempPasswd, m_sAccName.c_str(), used);
	KSG_StringToMD5String(lai.Password.szPassword, szTempPasswd);

    #ifdef USE_KPROTOCOL_VERSION
    // Add By Freeway Chen in 2003.7.1
    lai.ProtocolVersion = KPROTOCOL_VERSION;
    #endif
	
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
	
//	if ( ( pLAI->Param | LOGIN_A_LOGIN ) &&
//		 0 == m_sAccName.compare( pLAI->Account ) &&
//		 0 == m_sAccName.compare( pLAI->Password ) )
	if ( ( pLAI->Param | LOGIN_A_LOGIN ) &&
		 0 == m_sAccName.compare( pLAI->Account ))
	{
		if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR )
		{
            printf("%s: An incorrect account or password!\n", m_sAccName.c_str());
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_FAILED )
		{
            printf("%s: Login failed!\n", m_sAccName.c_str());
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_ACCOUNT_EXIST )
		{
            printf("%s: Login failed! your accout was existed!\n", m_sAccName.c_str());
		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_TIMEOUT )
		{
            printf("%s: Login failed! no money!\n", m_sAccName.c_str());
		}
//		// comment by flying
//		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_INVALID_PROTOCOLVERSION )
//		{
//            printf("%s: Login failed! Invalid Protocol Version!\n", m_sAccName.c_str());
//		}
		else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_SUCCESS )
		{
            printf("%s: Login ok!\n", m_sAccName.c_str());
			bRet = true;
		}
	}
	return bRet;
}

// 
//******************************************************************************
/*! \fn     bool CPlayer::OnAccountLogoutNotify(const char* pMsg)
*   \brief  处理网关发回的帐号登出请求的结果
*******************************************************************************/
//bool CPlayer::OnAccountLogoutNotify(const char* pMsg)
//{
//
//    return true;
//}


//******************************************************************************
/*! \fn     int CPlayer::OnGetRoleListRole(const char* pMsg)
*   \brief  响应网关发来的消息，选择角色列表
*******************************************************************************/
int CPlayer::OnGetRoleList(const char* pMsg)
{
	int nRoleCount		= 0;
	int nSelRoleIndex	= 0;

	S3DBI_RoleBaseInfo *pRoleList = NULL;
	TProcessData			*pPA		= ( TProcessData * )pMsg;
	
    int nMsgLen = sizeof(TProcessData) + pPA->nDataLen - 1;
	
	nRoleCount = *( char * )( pPA->pDataBuffer );
	
	pRoleList = (S3DBI_RoleBaseInfo * )( ( const char * )( pPA->pDataBuffer ) + 1/* sizeof( char ) */ );
	const size_t datalength = sizeof( S3DBI_RoleBaseInfo ) * nRoleCount;
	
    int nAction = 0;

    if (g_nDeleteRoleFlag)
    {
        nAction = 2;
    }
    else if (g_nAddRoleFlag)
    {
        nAction = 1;
    }
    else
    {
        if (nRoleCount > 0)
            nAction = 0;
        else
            nAction = 1;
    }


    switch (nAction) 
    {
    case 0:     // 选择角色
    {
        if (nRoleCount > 0)
        {
            nSelRoleIndex = rand() % nRoleCount;
        }
        
        if (nSelRoleIndex < nRoleCount)
        {
            // Add by Freeway Chen for Old Version BiShop
            if (nSelRoleIndex >= 3)
                nSelRoleIndex = 2;

            printf("%s : Select Character %d/%d(%s)!\n", m_sAccName.c_str(), nSelRoleIndex, nRoleCount, pRoleList[nSelRoleIndex].szName);
        }
        else
        {
            printf("%s : Select Empty Character!\n", m_sAccName.c_str());
        }

	    tagDBSelPlayer	tDBSP;
	    tDBSP.cProtocol = c2s_dbplayerselect;

	    size_t nRoleNameLen = strlen( pRoleList[nSelRoleIndex].szName );
	    memcpy( tDBSP.szRoleName, pRoleList[nSelRoleIndex].szName, nRoleNameLen );
	    tDBSP.szRoleName[nRoleNameLen] = '\0';

	    m_pClientToGateWay->SendPackToServer( &tDBSP, sizeof( tagDBSelPlayer ) );
        break;
    } // case 0:     // 选择角色

    case 1:     // 增加角色
    {
        if (nRoleCount >= 3)
            break;

        // 创建角色
        const int cnAddressIDCount = 5;
        static int s_AddressIDTable[cnAddressIDCount] = 
        {
            1,      // 凤翔
            20,     // 江津村
            45,     // 天忍教
            53,     // 巴陵县
            81      // 武当
        };

        int nNameLen = 0;
	    char	Data[sizeof(TProcessData) + sizeof(NEW_PLAYER_COMMAND)];
	    TProcessData*	pNetCommand = (TProcessData*)&Data;
	    NEW_PLAYER_COMMAND* pInfo = (NEW_PLAYER_COMMAND*)pNetCommand->pDataBuffer;
	    if (g_nAddRoleGender == -1)
            pInfo->m_btRoleNo = rand() % 2;     // 男女
        else 
            pInfo->m_btRoleNo = g_nAddRoleGender;     // 男女
        
        if (g_nAddRoleSeries == -1)    
	        pInfo->m_btSeries = rand() % 5;     // 五行
        else
	        pInfo->m_btSeries = g_nAddRoleSeries;     // 五行

        if (g_nAddRolePlaceID == -1)
	        pInfo->m_NativePlaceId = s_AddressIDTable[rand() % cnAddressIDCount];
        else
            pInfo->m_NativePlaceId = g_nAddRolePlaceID;

        int nNameRand = rand() % 100;
	    sprintf(pInfo->m_szName, "%s_%d", m_sAccName.c_str(), nNameRand);
        nNameLen = strlen(pInfo->m_szName);
	    
	    pNetCommand->nProtoId = c2s_newplayer;
	    pNetCommand->nDataLen = sizeof(NEW_PLAYER_COMMAND) - sizeof(pInfo->m_szName) + nNameLen + 1/* sizeof( '\0' ) */;
	    pNetCommand->ulIdentity = 0;

        printf("%s : Add Character %d(%s)!\n", m_sAccName.c_str(), nRoleCount, pInfo->m_szName);

	    m_pClientToGateWay->SendPackToServer(&Data, sizeof(TProcessData) - sizeof(pNetCommand->pDataBuffer) + pNetCommand->nDataLen);

        if (nRoleCount < 3)
        {
            memcpy((void *)pRoleList[nRoleCount].szName, pInfo->m_szName, nNameLen + 1);

            (*(char *)(pPA->pDataBuffer))++;

            nRoleCount++;
        }

        break;    
    }   // case 1:     // 增加角色

    case 2:     // 删除角色
    {
        if (nRoleCount > 0)
        {
            nSelRoleIndex = rand() % nRoleCount;
        }
        
        if (nSelRoleIndex < nRoleCount)
        {
            printf("%s : Del Character %d/%d(%s)!\n", m_sAccName.c_str(), nSelRoleIndex, nRoleCount, pRoleList[nSelRoleIndex].szName);
	        tagDBDelPlayer tDBDP;
            tDBDP.cProtocol = c2s_roleserver_deleteplayer;
            strcpy(tDBDP.szAccountName, m_sAccName.c_str());
            //strcpy(tDBDP.szPassword,    m_sAccName.c_str());
            strcpy(tDBDP.szRoleName,    pRoleList[nSelRoleIndex].szName);

	        m_pClientToGateWay->SendPackToServer(&tDBDP, sizeof(tDBDP));

            while (nSelRoleIndex < (nRoleCount - 1))
            {
                pRoleList[nSelRoleIndex] = pRoleList[nSelRoleIndex + 1]; 
                
                nSelRoleIndex++;
            }

            (*(char *)(pPA->pDataBuffer))--;
        }
        else
        {
            printf("%s : Del Empry Character!\n", m_sAccName.c_str());
            
            Sleep(rand() % 10);
            SetStatus(enumExitGame);
        }

        break;    
    }   // case 2:     // 删除角色


    default :
        Sleep(rand() % 10);
        SetStatus(enumExitGame);
    

    }   // switch


	return nMsgLen;
}

//******************************************************************************
/*! \fn     bool CPlayer::OnPlayerLoginNotify(const char* pMsg)
*   \brief  处理网关发来的消息，响应角色进入游戏的通告
*******************************************************************************/
bool CPlayer::OnPlayerLoginNotify(const char* pMsg)
{
	tagNotifyPlayerLogin *pNPL = ( tagNotifyPlayerLogin * )pMsg;

    if (!pNPL->bPermit)
        return false;

	const char *pIPAddr = OnlineGameLib::Win32::net_ntoa( pNPL->nIPAddr );

	return ConnectToGameSvr( pIPAddr, pNPL->nPort, pNPL->guid );
}

//******************************************************************************
/*! \fn     bool CPlayer::ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid )
*   \brief  处理网关发来的消息，客户连接到网关
*******************************************************************************/
bool CPlayer::ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid )
{
    if (nPort == 0)
        return false;
#if defined _USE_RAINBOW
	pfnCreateClientInterface pFactroyFun = ( pfnCreateClientInterface )( s_theRainbowLib.GetProcAddress( "CreateInterface" ) );

	IClientFactory *pClientFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 256 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClientToGameSvr ) );
		
		pClientFactory->Release();
	}

	if ( !m_pClientToGameSvr )
	{
		//::MessageBox(NULL, "Initialization failed! Don't find a correct rainbow.dll", "Error", MB_OK);
        printf("Initialization failed! Don't find a correct rainbow.dll\n");

		return false;
	}
#else
	//m_pClientToGameSvr = 
#endif
	/*
	 * Create a connection
	 */
	m_pClientToGameSvr->Startup();

	//pGameSvrClient->RegisterMsgFilter( reinterpret_cast< void * >( pClient ), NULL );
#if defined _USE_RAINBOW	
    HRESULT h = m_pClientToGameSvr->RegisterMsgFilter( ( void * )this, GameServerToClientEventNotify );
#else
	m_pClientToGameSvr->RegisterMsgFilter( ( void * )this, GameServerToClientEventNotify );
#endif
	if ( FAILED( m_pClientToGameSvr->ConnectTo( pIPAddr, nPort ) ) )
	{
		//::MessageBox(NULL, "Connection failed! Don't connect to game server.", "Error", MB_OK);
        printf("Connection failed! Don't connect to game server.\n");

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
*           登入或者切换服务器的时候产生这个消息。
*			SyncWorld, SyncNpc, SyncPlayer, SyncCurPlayer, SyncCurPlayerSkill
*			SyncEnd, SyncNpcMinPlayer
*******************************************************************************/
void CPlayer::SyncEnd()
{
	SetStatus( enumPlayGame );

	BYTE cSyscEndFlag = c2s_syncend;

	m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &cSyscEndFlag ), sizeof( BYTE ) );
	// flying add the following code
	// Initialize the "birth position"
	if (m_nFlgSetPos)
		DoSetPos();
	// Query channel
	QueryDefaultChannel("\\S", 3);
	// Execute GM Commands
	SendGMCommand();
}


//******************************************************************************
/*! \fn     void CPlayer::QueryDefaultChannel()
*   \brief  查询默认频道
*******************************************************************************/
void CPlayer::QueryDefaultChannel(char* szName, int nSize)
{
	if (szName == NULL || strlen(szName)==0)
		return;
	char szBuffer[sizeof(tagExtendProtoHeader) + sizeof(PLAYERCOMM_QUERYCHANNELID)];
	tagExtendProtoHeader Header;
	PLAYERCOMM_QUERYCHANNELID setChannel;
	Header.ProtocolType = c2s_extend;
	Header.wLength = sizeof(tagExtendProtoHeader) + sizeof(PLAYERCOMM_QUERYCHANNELID) - 1;
	setChannel.ProtocolFamily = pf_playercommunity;
	setChannel.ProtocolID = playercomm_c2s_querychannelid;
	// Public Channel
	memcpy(setChannel.channel, szName, nSize);
	memcpy(szBuffer, &Header, sizeof(tagExtendProtoHeader));
	memcpy(szBuffer + sizeof(tagExtendProtoHeader), &setChannel, sizeof(PLAYERCOMM_QUERYCHANNELID));
	m_pClientToGameSvr->SendPackToServer( szBuffer , 
		sizeof(tagExtendProtoHeader) + sizeof( PLAYERCOMM_QUERYCHANNELID ) );
}

static inline unsigned _RandValue(unsigned uHoldRand)
{
    uHoldRand = uHoldRand * 214013L + 2531011L;
     
    return uHoldRand;
}

//******************************************************************************
/*! \fn     void CPlayer::WalkTo()
*   \brief  机器人行走
*******************************************************************************/
void CPlayer::WalkTo()
{
	if ( ( DWORD )( -1 ) != m_dwTargetID && m_ptTagPos.x && m_ptTagPos.y )
	{
		if ( abs( m_ptCurPos.x - m_ptTagPos.x ) > 16 ||
			abs( m_ptCurPos.y - m_ptTagPos.y ) >  16 )
		{
            NPC_RUN_COMMAND     nrc;

            nrc.ProtocolType = c2s_npcrun;
            nrc.nMpsX = m_ptTagPos.x + ((_RandValue(m_RandValue)             % 9) - 5) * 32;
			nrc.nMpsY = m_ptTagPos.y + ((_RandValue(_RandValue(m_RandValue)) % 9) - 5) * 32;
			m_pClientToGameSvr->SendPackToServer( reinterpret_cast< BYTE * >( &nrc ), sizeof( nrc ) );
		}
	}
}

//******************************************************************************
/*! \fn     void CPlayer::CastSkill()
*   \brief  发送技能
*******************************************************************************/
void CPlayer::CastSkill()
{
	char* szGMAddMana = "?gm ds RestoreMana()";
	if (!m_nFlgGMDone)
		return;
	// skill casting implementation goes here
	NPC_SKILL_COMMAND NpcSkillCmd;

	NpcSkillCmd.ProtocolType = c2s_npcskill;
	NpcSkillCmd.nSkillID = 1;

	NpcSkillCmd.nMpsX = m_ptTagPos.x + ((_RandValue(m_RandValue) % 9) - 5) * 32;
	NpcSkillCmd.nMpsY = m_ptTagPos.y + ((_RandValue(_RandValue(m_RandValue)) % 9) - 5) * 32;;
	ASSERT(m_pClientToGameSvr);
	m_pClientToGameSvr->SendPackToServer( reinterpret_cast< char * >( &NpcSkillCmd ), 
		sizeof( NPC_SKILL_COMMAND ) );
	SpeakWord(szGMAddMana, strlen(szGMAddMana));
}

//******************************************************************************
/*! \fn     void CPlayer::SendGMCommand()
*   \brief  发送GM指令
*******************************************************************************/
void CPlayer::SendGMCommand()
{
	if (m_nFlgGMDone)
		return;
	printf("Try to send GM command_S_.........\n");
	for (int i = 0; i < MAX_GM_COUNT; i++)
	{
		if (g_szGMCommandList[i][0] == 0)
			break;
		SpeakWord((char *)g_szGMCommandList[i], strlen((char *)szGMCommand));
		printf("--> %s\n", g_szGMCommandList[i]);
		Sleep(20);

	}
	printf("GM Completed...............\n");
	m_nFlgGMDone = true;
	return;
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
		dataLength = strlen(pText);
	}
	// New protocol
	int nPackSize = sizeof(tagExtendProtoHeader) + sizeof(CHAT_CHANNELCHAT_CMD) + dataLength;
	char *ProtocolBuffer = (char*)malloc(nPackSize);
	int nProtocolOffset = 0;

	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)ProtocolBuffer;
	pExHeader->ProtocolType = c2s_extendchat;
	pExHeader->wLength = nPackSize - 1;
	nProtocolOffset += sizeof(tagExtendProtoHeader);

	CHAT_CHANNELCHAT_CMD* pCccCmd = (CHAT_CHANNELCHAT_CMD*)(ProtocolBuffer + nProtocolOffset);
	pCccCmd->ProtocolType = chat_channelchat;
	pCccCmd->wSize = dataLength - 1;
	pCccCmd->packageID = -1;
	pCccCmd->filter = 1;
	if (strncmp(pText, "?gm ds", 6) == 0)
		pCccCmd->channelid = m_nGMChannelID;
	else
		pCccCmd->channelid = m_nCurChannel;
	pCccCmd->cost = 0;
	pCccCmd->sentlen = dataLength;
	nProtocolOffset += sizeof(CHAT_CHANNELCHAT_CMD);
	memcpy(pCccCmd + 1, pText, dataLength);
	nProtocolOffset += dataLength;

	if (m_pClientToGameSvr)
		m_pClientToGameSvr->SendPackToServer(ProtocolBuffer, nProtocolOffset);
	free(ProtocolBuffer);
}

//******************************************************************************
/*! \fn     void ShutdownGateway()
*   \brief  切断与网关的连接
*******************************************************************************/
void CPlayer::ShutdownGateway()
{

    m_nShutDownGatewayFlag = true;

	// 关闭到网关的连接
	if ( m_pClientToGateWay )
	{
		m_pClientToGateWay->Cleanup();

		SAFE_RELEASE( m_pClientToGateWay );
	}
    //SetStatus(enumExitGame);
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

#if defined _USE_RAINBOW
void __stdcall CPlayer::ClientEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnEventType )
#else
void CPlayer::ClientEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnEventType )
#endif
{
	CPlayer *pPlayer = ( CPlayer * )lpParam;

	ASSERT( pPlayer );

	try
	{
		pPlayer->_ClientEventNotify( ulnEventType );
	}
	catch(...)
	{
		//::MessageBeep( -1 );
	}
}

void CPlayer::_ClientEventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		//::MessageBeep( -1 );

		break;

	case enumServerConnectClose:
        if (!m_nShutDownGatewayFlag)
        {
            // 如果不是主动断开网络的连接，说明是被网关踢掉
            SetStatus(enumExitGame);
        }

		//::MessageBeep( -1 );

		break;
	}
}

#if defined _USE_RAINBOW
void __stdcall CPlayer::GameServerToClientEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnEventType )
#else
void CPlayer::GameServerToClientEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnEventType )
#endif
{
	CPlayer *pPlayer = ( CPlayer * )lpParam;

	ASSERT( pPlayer );

	try
	{
		pPlayer->_GameServerToClientEventNotify( ulnEventType );
	}
	catch(...)
	{
		//::MessageBeep( -1 );
	}
}

void CPlayer::_GameServerToClientEventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		//::MessageBeep( -1 );

		break;

	case enumServerConnectClose:		
        SetStatus(enumExitGame);

		//::MessageBeep( -1 );

		break;
	}
}

int CPlayer::KSG_StringToMD5String(char szDestMD5String[64], const char cszSrcString[])
{
    int nResult = false;
    md5_state_t md5_state;
    unsigned char MD5Value[16];
    int nSrcStringLen = 0;

    if (!szDestMD5String)
        goto Exit0;

    szDestMD5String[0] = '\0';

    if  (!cszSrcString)
       goto Exit0;

    nSrcStringLen = strlen(cszSrcString);

    if (!nSrcStringLen)
        goto Exit0;

    md5_init(&md5_state);

    md5_append(&md5_state, (unsigned char *)cszSrcString, nSrcStringLen);

    md5_finish(&md5_state, MD5Value);


    sprintf(
        szDestMD5String,
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
        MD5Value[ 0], MD5Value[ 1], MD5Value[ 2], MD5Value[ 3],
        MD5Value[ 4], MD5Value[ 5], MD5Value[ 6], MD5Value[ 7],
        MD5Value[ 8], MD5Value[ 9], MD5Value[10], MD5Value[11],
        MD5Value[12], MD5Value[13], MD5Value[14], MD5Value[15]
    );

    nResult = true;
Exit0:
    return nResult;
}