// RootClient.cpp: implementation of the CRootClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "RootClient.h"
#include "RootCenter.h"
#include "S3Relay.h"
#include "DealRelay.h"
#include "../../../Headers/KGmProtocol.h"
#include "malloc.h"
#include "crtdbg.h"
#include "../../Engine/Src/KWin32.h"
#include "../../Engine/Src/KSG_MD5_String.h"
#include "../../Network/S3AccServer/AccountLoginDef.h"
#include "DoScript.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRootClient::CRootClient(CRootCenter* pRootCenter, BOOL bAutoFree)
	: CNetClient(pRootCenter, bAutoFree)
{
	m_LastRevPingTime = 0;
	m_LastSendPingTime = 0;
	m_isStartAutoTime = FALSE;
}

CRootClient::~CRootClient()
{
}


void CRootClient::OnServerEventCreate()
{
	gOnOneClientCreate(oneclient_root);

	extern std::_tstring g_rootloginname;
	extern std::_tstring g_rootloginpswd;

	rTRACE("root client create: %s", _ip2a(GetSvrIP()));

	KServerAccountUserLoginInfo serlogin;
	serlogin.Size = sizeof(KServerAccountUserLoginInfo);
	serlogin.Type = ServerAccountUserLoginInfo;
	serlogin.Version = ACCOUNT_CURRENT_VERSION;
	strncpy(serlogin.Account, g_rootloginname.c_str(), LOGIN_USER_ACCOUNT_MAX_LEN);
    serlogin.Account[LOGIN_USER_ACCOUNT_MAX_LEN - 1] = '\0';

    //#ifdef SWORDONLINE_USE_MD5_PASSWORD

    KSG_StringToMD5String(serlogin.Password, g_rootloginpswd.c_str());

    //#else
    //
    //#pragma message (KSG_ATTENTION("Add Password to MD5 string"))
	//strncpy(serlogin.Password, g_rootloginpswd.c_str(), LOGIN_USER_PASSWORD_MAX_LEN);
    //serlogin.Password[LOGIN_USER_PASSWORD_MAX_LEN - 1] = '\0';
    //#endif
    


	memcpy(serlogin.MacAddress, gGetHostMac(global_adapt), 6);
	char send[MAX_PATH + 1];
	send[0] = pf_normal;
	send[1] = c2s_gatewayverify;
	memcpy(send + 2, &serlogin, sizeof(serlogin));

	SendPackage(send, sizeof(serlogin) + 2);
}

void CRootClient::OnServerEventClose()
{
	StopAutoTime();

	rTRACE("root client close: %s", _ip2a(GetSvrIP()));

	gOnOneClientClose(oneclient_root);
}

BOOL CRootClient::StartAutoTime()
{
	if (!m_isStartAutoTime)
	{
		m_LastRevPingTime = m_LastSendPingTime = GetTickCount();
		m_isStartAutoTime = TRUE;

		rTRACE("start autotimer for Ping: %s", _ip2a(GetSvrIP()));
	}

	return TRUE;
}

BOOL CRootClient::StopAutoTime()
{
	if (m_isStartAutoTime)
	{
		m_isStartAutoTime = FALSE;
		m_LastRevPingTime = m_LastSendPingTime = 0;

		rTRACE("stop autotimer for Ping: %s", _ip2a(GetSvrIP()));
	}

	return TRUE;
}

BOOL CRootClient::IsStartAutoTime()
{
	return m_isStartAutoTime;
}

void CRootClient::AutoTime()
{
	if (!IsStartAutoTime())
		return;


	DWORD tick = GetTickCount();

	if (tick - m_LastRevPingTime >= 600000)	//十分钟没有收到Ping
	{
		rTRACE("warning: root client timeout: %s", _ip2a(GetSvrIP()));

		InterShutdown();
		return;
	}
	if (tick - m_LastSendPingTime >= 120000)	//两分钟没有发出Ping
	{
		m_LastSendPingTime = tick;

		char buffer[sizeof(EXTEND_HEADER) + sizeof(DWORD)];
		EXTEND_HEADER* pHeader = (EXTEND_HEADER*)buffer;
		pHeader->ProtocolFamily = pf_normal;
		pHeader->ProtocolID = c2s_ping;
		DWORD* pPingTick = (DWORD*)(pHeader + 1);
		*pPingTick = m_LastSendPingTime;

		SendPackage(buffer, sizeof(buffer));

		dTRACE("root client ping: %s", _ip2a(GetSvrIP()));
	}
}

void CRootClient::BeginRoute()
{
}

void CRootClient::EndRoute()
{
	AutoTime();
}


void CRootClient::RecvPackage(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolFamily == pf_normal)
	{
		Proc0_Normal(pData, size);
	}
	else if (pHeader->ProtocolFamily == pf_relay)
	{
		Proc0_Relay(pData, size);
	}
	else if (pHeader->ProtocolFamily == pf_playercommunity)
	{
		Proc0_PlayerCommunity(pData, size);
	}
}

void CRootClient::Proc0_Normal(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == s2c_ping)
	{
		m_LastRevPingTime = GetTickCount();
	}
	else if (pHeader->ProtocolID == s2c_gatewayinfo)
	{
		KServerInfo* pServer = (KServerInfo*)(((char*)pData) + 2);
		if (size == sizeof(KServerInfo) + 2)
		{
			if (pServer->nServerType == server_Login)
			{
				std::_tstring addr(_ip2a(pServer->nValue));

				CNetSockDupEx sockdup = dealrelay::FindRelaySockByIP(pServer->nValue);
				if (!sockdup.IsValid())
				{
					rTRACE("warning: relay client exist: %s", addr.c_str());
					return;
				}

				rTRACE("relay client attempt: %s", addr.c_str());
				if (!gConnectToSibling(addr.c_str()))
				{
					rTRACE("error: relay client error: %s", addr.c_str());
					return;
				}

				return;
			}
		}
	}
	else if (pHeader->ProtocolID == s2c_gatewayverify)
	{
		KAccountUserReturn* pReturn = (KAccountUserReturn*)(((char*)pData) + 2);
		if (size == sizeof(KAccountUserReturn) + 2)
		{
			if (pReturn->nReturn == ACTION_SUCCESS)
			{
				StartAutoTime();
			}
		}
	}
}

void CRootClient::Proc0_Relay(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == relay_c2c_data)
	{
		Proc1_Relay_Data(pData, size);
	}
	else if (pHeader->ProtocolID == relay_c2c_askwaydata)
	{
		Proc1_Relay_AskWay(pData, size);
	}
	else if (pHeader->ProtocolID == relay_s2c_loseway)
	{
		Proc1_Relay_LoseWay(pData, size);
	}
}

// [wxb 2003-7-19]
void CRootClient::Proc2_Relay_Data_Here(const void* pData, size_t size)
{
	RELAY_DATA* pRelayData = (RELAY_DATA*)pData;
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)(pRelayData + 1);
	switch (*((WORD*)pHeader))
	{
	case (MAKEWORD(pf_relay, relay_c2c_askwaydata)):
		Proc1_Relay_AskWay((void*)pHeader, size - sizeof(RELAY_DATA));
		break;
	case (MAKEWORD(pf_gamemaster, gm_c2s_execute)):
		{
			GM_EXECUTE_COMMAND* pGMData = (GM_EXECUTE_COMMAND*)(pHeader);
			char szScriptAction[MAX_PATH];
			memcpy(szScriptAction, ((const char*)pGMData) + sizeof(GM_EXECUTE_COMMAND), min(MAX_PATH - 1, pGMData->wLength));
			szScriptAction[min(MAX_PATH, pGMData->wLength)] = 0;
			ExcuteScript(pRelayData->nFromIP, pRelayData->nFromRelayID, szScriptAction);
		}
		break;
	case (MAKEWORD(pf_gamemaster, gm_c2s_getrolelist)):
		_ASSERT(pRelayData->routeDateLength == sizeof(GM_GET_ROLE_LIST_CMD));
		{
			//用老协议向角色数据库查询
			GM_GET_ROLE_LIST_CMD* pGMData = (GM_GET_ROLE_LIST_CMD*)pHeader;
			int nNewDataLen = sizeof(TProcessData) + strlen(pGMData->AccountName) + 1;
			TProcessData* pNewData = (TProcessData*)_alloca(nNewDataLen);
			pNewData->nProtoId = c2s_roleserver_getrolelist;
			pNewData->nDataLen = strlen(pGMData->AccountName) + 1;
			pNewData->ulIdentity = 0;
			pNewData->pDataBuffer[0] = MAX_PLAYER_PER_ACCOUNT;
			strncpy((char*)(pNewData + 1), pGMData->AccountName, sizeof(pGMData->AccountName));
			g_DBClient.SendPackage(pNewData, nNewDataLen);
		}
		break;

	case (MAKEWORD(pf_gamemaster, gm_c2s_broadcast_chat)):
		{{
		GM_BROADCAST_CHAT_CMD* pGMData = (GM_BROADCAST_CHAT_CMD*)pHeader;


		size_t pckgsize = sizeof(CHAT_EVERYONE) + pGMData->wSentenceLen;

		CHAT_EVERYONE* pCeo = (CHAT_EVERYONE*)_alloca(pckgsize);
		pCeo->ProtocolType = chat_everyone;
		pCeo->wSize = pckgsize - 1;
		pCeo->wChatLength = pGMData->wSentenceLen;

		memcpy(pCeo + 1, pGMData + 1, pGMData->wSentenceLen);

		g_ChatServer.BroadPackage(pCeo, pckgsize);
		}}
		break;

	case (MAKEWORD(pf_gamemaster, gm_c2s_getrole)):
	case (MAKEWORD(pf_gamemaster, gm_c2s_setrole)):
		{
			char* pDBAskBuffer = (char*)_alloca(pRelayData->routeDateLength + 1);
			memcpy(pDBAskBuffer + 1, pHeader, pRelayData->routeDateLength);
			*pDBAskBuffer = (char)c2s_extend;
			g_DBClient.SendPackage(pDBAskBuffer, pRelayData->routeDateLength + 1);
		}
		break;

	default:
		_ASSERT(0);
		break;
	}
}

void CRootClient::Proc1_Relay_Data(const void* pData, size_t size)
{
	RELAY_DATA* pRelayData = (RELAY_DATA*)pData;

	if (pRelayData->nToIP == 0)
	{
		Proc2_Relay_Data_Here(pData, size);
		return;
	}
	else if (pRelayData->nToIP == INADDR_BROADCAST)
	{//broadcast to gamesvrs
		pRelayData->nToIP = 0;
		g_HostServer.BroadPackage(pRelayData, size);
		return;
	}
	else if (pRelayData->nToIP == gGetHostIP(global_adapt))
	{//relay to host
		pRelayData->nToIP = 0;
		if (g_HostServer.IsConnectReady(pRelayData->nToRelayID))
		{
			CNetConnectDup conndup = g_HostServer.FindNetConnect(pRelayData->nToRelayID);
			if (conndup.IsValid())
			{
				conndup.SendPackage(pRelayData, size);
				return;
			}
		}
	}

	assert (pRelayData->nFromIP != 0);

	//lose way
	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(pRelayData->nFromIP, pRelayData->nFromRelayID, 
		pRelayData, size, pLoseDataDup, sizeLoseData);

	SendPackage(pLoseDataDup, sizeLoseData);
}

void CRootClient::Proc1_Relay_AskWay(const void* pData, size_t size)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	if (pAskWayData->seekMethod == rm_account_id)
	{
		Proc2_Relay_AskWay_AccountRoleID(pData, size, TRUE);
	}
	else if (pAskWayData->seekMethod == rm_role_id)
	{
		Proc2_Relay_AskWay_AccountRoleID(pData, size, FALSE);
	}
}

void CRootClient::Proc1_Relay_LoseWay(const void* pData, size_t size)
{
	RELAY_DATA* pLoseData = (RELAY_DATA*)pData;

	if (pLoseData->nFromIP == 0)
	{
		pLoseData->nFromIP = GetSvrIP();
		pLoseData->nFromRelayID = -1;
	}

	if (pLoseData->nToIP == 0)
	{//my data
		//...
		return;
	}
	else if (pLoseData->nToIP == INADDR_BROADCAST)
	{//broadcast to gamesvrs
		assert(FALSE);
		return;
	}
	else if (pLoseData->nToIP == gGetHostIP(global_adapt))
	{//relay to host
		pLoseData->nToIP = 0;
		if (g_HostServer.IsConnectReady(pLoseData->nToRelayID))
		{
			CNetConnectDup conndup = g_HostServer.FindNetConnect(pLoseData->nToRelayID);
			if (conndup.IsValid())
			{
				conndup.SendPackage(pLoseData, size);
				return;
			}
		}
	}

	//lose way
	//don't more lose
}


void CRootClient::Proc2_Relay_AskWay_AccountRoleID(const void* pData, size_t size, BOOL acc)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	std::_tstring strName = (char*)(pAskWayData + 1);
	assert(!strName.empty());

	DWORD nameid = -1;
	unsigned long param = -1;
	CNetConnectDup conndup;

	BOOL find = FALSE;

	if (acc ? g_HostServer.FindPlayerByAcc(NULL, strName, &conndup, NULL, &nameid, &param)
			 : g_HostServer.FindPlayerByRole(NULL, strName, &conndup, NULL, &nameid, &param))
	{//send to target
		assert(conndup.IsValid());

		size_t pckgsize = sizeof(DWORD)*2 + size;
		RELAY_ASKWAY_DATA* pClntWayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);

		gFillClntwayPckg(nameid, param, pAskWayData, size, pClntWayData, pckgsize);

		conndup.SendPackage(pClntWayData, pckgsize);

		return;
	}

	//lose way
	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(pAskWayData->nFromIP, pAskWayData->nFromRelayID, 
		pAskWayData, size, pLoseDataDup, sizeLoseData);

	SendPackage(pLoseDataDup, sizeLoseData);

}

void CRootClient::Proc0_PlayerCommunity(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == playercomm_s2c_gmquerychannelid)
	{
		PLAYERCOMM_GMQUERYCHANNELID* pPlayerCommGmQID = (PLAYERCOMM_GMQUERYCHANNELID*)pData;

		g_ChannelMgr.GmQueryChannelID(pPlayerCommGmQID->channel, pPlayerCommGmQID->force);
	}
	else if (pHeader->ProtocolID == playercomm_s2c_gmfreechannelid)
	{
		PLAYERCOMM_GMFREECHANNELID* pPlayerCommGmFID = (PLAYERCOMM_GMFREECHANNELID*)pData;

		g_ChannelMgr.GmFreeChannID(pPlayerCommGmFID->channelid);
	}
	else if (pHeader->ProtocolID == playercomm_s2c_gmsubscribe)
	{
		PLAYERCOMM_GMSUBSCRIBE* pPlayerCommGmSub = (PLAYERCOMM_GMSUBSCRIBE*)pData;
		if (pPlayerCommGmSub->subscribe)
			g_ChannelMgr.GmSubscribe(pPlayerCommGmSub->channelid);
		else
			g_ChannelMgr.GmUnsubscribe(pPlayerCommGmSub->channelid);
	}
}
