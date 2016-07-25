// HostConnect.cpp: implementation of the CHostConnect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "HostConnect.h"
#include "HostServer.h"
#include "DealRelay.h"
#include "S3Relay.h"
#include "../../Network/s3accserver/AccountLoginDef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHostConnect::CHostConnect(CHostServer* pHostServer, unsigned long id)
	: CNetConnect(pHostServer, id)
{

}

CHostConnect::~CHostConnect()
{

}


void CHostConnect::RecvPackage(const void* pData, size_t size)
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


void CHostConnect::Proc0_Normal(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == c2s_entergame)
	{
		Proc1_Normal_EnterGame(pData, size);
	}
	else if (pHeader->ProtocolID == c2s_leavegame)
	{
		Proc1_Normal_LeaveGame(pData, size);
	}
}

void CHostConnect::Proc1_Normal_EnterGame(const void* pData, size_t size)
{
	tagEnterGame2* pEnterGame2 = (tagEnterGame2*)pData;

	std::_tstring strAcc = (char*)pEnterGame2->szAccountName;
	assert(!strAcc.empty());
	std::_tstring strRole = (char*)pEnterGame2->szCharacterName;
	assert(!strRole.empty());

	ACCINFO infoAcc;
	infoAcc.role = strRole;
	infoAcc.nameID = pEnterGame2->dwNameID;
	infoAcc.param = pEnterGame2->lnID;

	ROLEINFO infoRole;
	infoRole.acc = strAcc;
	infoRole.nameID = pEnterGame2->dwNameID;
	infoRole.param = pEnterGame2->lnID;

	PARAMINFO infoParam;
	infoParam.acc = strAcc;
	infoParam.role = strRole;
	infoParam.nameID = pEnterGame2->dwNameID;

	{{
	DUMMY_AUTOLOCKWRITE(m_lockPlayer);

	m_mapAcc[strAcc] = infoAcc;
	m_mapRole[strRole] = infoRole;
	m_mapParam[pEnterGame2->lnID] = infoParam;
	}}

	{{
	g_FriendMgr.SomeoneLogin(strRole);
	}}

	rTRACE("player login: %s [%s] (%08X, %08X)", strAcc.c_str(), strRole.c_str(), GetIP(), pEnterGame2->lnID);
}

void CHostConnect::Proc1_Normal_LeaveGame(const void* pData, size_t size)
{
	tagLeaveGame2* pLeaveGame = (tagLeaveGame2*)pData;

	std::_tstring strAcc = pLeaveGame->szAccountName;
	assert(!strAcc.empty());

	{{
	DUMMY_AUTOLOCKWRITE(m_lockPlayer);

	ACCMAP::iterator itAcc = m_mapAcc.find(strAcc);
	if (itAcc != m_mapAcc.end())
	{
		ACCINFO& rAccInfo = (*itAcc).second;

		{{
		//ensure
		g_ChannelMgr.B_ClearPlayer(GetIP(), rAccInfo.param);
		g_FriendMgr.SomeoneLogout(rAccInfo.role);
		}}
		

		m_mapRole.erase(rAccInfo.role);
		m_mapParam.erase(rAccInfo.param);

		m_mapAcc.erase(itAcc);
	}
	}}

	rTRACE("player logout: %s", strAcc.c_str());
}


void CHostConnect::Proc0_Relay(const void* pData, size_t size)
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

void CHostConnect::Proc1_Relay_Data(const void* pData, size_t size)
{
 	RELAY_DATA* pRelayData = (RELAY_DATA*)pData;

	assert (pRelayData->nFromIP == 0);
	pRelayData->nFromIP = gGetHostIP(global_adapt);
	pRelayData->nFromRelayID = GetID();

	if (pRelayData->nToIP == 0)
	{
		//arrived
		return;
	}
	else if (pRelayData->nToIP == gGetHostIP(global_adapt))
	{
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
	else if (pRelayData->nToIP == INADDR_BROADCAST)
	{
		if (pRelayData->nToRelayID == 0 || !g_RootClient.IsReady())
		{//local
			pRelayData->nToRelayID = 0;
			g_HostServer.BroadPackage(pRelayData, size);
		}
		else
		{//global
			g_RootClient.SendPackage(pRelayData, size);
		}
		return;
	}
	else
	{
		CNetSockDupEx sockdup = dealrelay::FindRelaySockByIP(pRelayData->nToIP);
		if (sockdup.IsValid())
		{
			pRelayData->nToIP = 0;
			sockdup.SendPackage(pRelayData, size);
			return;
		}

		if (g_RootClient.IsReady())
		{
			g_RootClient.SendPackage(pRelayData, size);
			return;
		}
	}


	//fail to relay, lose data
	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(0, GetID(), 
		pRelayData, size, pLoseDataDup, sizeLoseData);

	SendPackage(pLoseDataDup, sizeLoseData);
}

void CHostConnect::Proc1_Relay_AskWay(const void* pData, size_t size)
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
	else if (pAskWayData->seekMethod == rm_map_id)
	{
		Proc2_Relay_AskWay_MapID(pData, size);
	}
	else if (pAskWayData->seekMethod = rm_gm)
	{
		Proc2_Relay_AskWay_GM(pData, size);
	}

}

void CHostConnect::Proc1_Relay_LoseWay(const void* pData, size_t size)
{
 	RELAY_DATA* pLoseData = (RELAY_DATA*)pData;

	assert (pLoseData->nFromIP == 0);
	pLoseData->nFromIP = gGetHostIP(global_adapt);
	pLoseData->nFromRelayID = GetID();

	if (pLoseData->nToIP == 0)
	{
		//arrived
		return;
	}
	else if (pLoseData->nToIP == gGetHostIP(global_adapt))
	{
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
	else if (pLoseData->nToIP == INADDR_BROADCAST)
	{
		assert(FALSE);
		return;
	}
	else
	{
		CNetSockDupEx sockdup = dealrelay::FindRelaySockByIP(pLoseData->nToIP);
		if (sockdup.IsValid())
		{
			pLoseData->nToIP = 0;
			sockdup.SendPackage(pLoseData, size);
			return;
		}

		if (g_RootClient.IsReady())
		{
			g_RootClient.SendPackage(pLoseData, size);
			return;
		}
	}


	//fail to relay, lose data
	//don't gen more loseway
}

void CHostConnect::Proc2_Relay_AskWay_AccountRoleID(const void* pData, size_t size, BOOL acc)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	assert(pAskWayData->nFromIP == 0);
	pAskWayData->nFromIP = gGetHostIP(global_adapt);
	pAskWayData->nFromRelayID = GetID();
	pAskWayData->seekRelayCount ++;


	std::_tstring strName = (char*)(pAskWayData + 1);
	assert(!strName.empty());

	DWORD nameid = -1;
	unsigned long param = -1;
	CNetConnectDup conndup;


	BOOL find = FALSE;

	if (acc ? FindPlayerByAcc(strName, NULL, &nameid, &param)
			: FindPlayerByRole(strName, NULL, &nameid, &param))
	{ //the player is at this gamesvr
		conndup = *this;

		find = TRUE;
	}
	else if (acc ? g_HostServer.FindPlayerByAcc(this, strName, &conndup, NULL, &nameid, &param)
				 : g_HostServer.FindPlayerByRole(this, strName, &conndup, NULL, &nameid, &param))
	{//the player is at other gamesvr in this group
		find = TRUE;
	}

	if (find)
	{
		assert(conndup.IsValid());

		size_t pckgsize = sizeof(DWORD)*2 + size;
		RELAY_ASKWAY_DATA* pClntWayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);

		gFillClntwayPckg(nameid, param, pAskWayData, size, pClntWayData, pckgsize);

		conndup.SendPackage(pClntWayData, pckgsize);

		return;
	}


	if (acc)
	{
		//the player is at other group, send to root
		if (g_RootClient.IsReady())
		{
			g_RootClient.SendPackage(pAskWayData, size);
			return;
		}
	}

	//lose way
	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(0, GetID(), 
		pAskWayData, size, pLoseDataDup, sizeLoseData);

	SendPackage(pLoseDataDup, sizeLoseData);

}

void CHostConnect::Proc2_Relay_AskWay_MapID(const void* pData, size_t size)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	assert(pAskWayData->nFromIP == 0);
	pAskWayData->nFromIP = gGetHostIP(global_adapt);
	pAskWayData->nFromRelayID = GetID();
	pAskWayData->seekRelayCount ++;

	g_GatewayClient.PassAskWayMap(GetIP(), pAskWayData);
}

void CHostConnect::Proc2_Relay_AskWay_GM(const void* pData, size_t size)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	assert (pAskWayData->nFromIP == 0);
	pAskWayData->nFromIP = gGetHostIP(global_adapt);
	pAskWayData->nFromRelayID = GetID();
	pAskWayData->seekRelayCount ++;

	if (g_RootClient.IsReady())
	{
		g_RootClient.SendPackage(pData, size);
		return;
	}

	//fail to relay, lose data

	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(0, GetID(), 
		pData, size, pLoseDataDup, sizeLoseData);

	SendPackage(pLoseDataDup, sizeLoseData);
}


void CHostConnect::Proc0_PlayerCommunity(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == playercomm_c2s_querychannelid)
	{
		Proc1_PlayerCommunity_QueryChannelID(pData, size);
	}
	else if (pHeader->ProtocolID == playercomm_c2s_freechannelid)
	{
		Proc1_PlayerCommunity_FreeChannelID(pData, size);
	}
	else if (pHeader->ProtocolID == playercomm_c2s_subscribe)
	{
		Proc1_PlayerCommunity_Subscribe(pData, size);
	}
}


void CHostConnect::Proc1_PlayerCommunity_QueryChannelID(const void* pData, size_t size)
{
	if (size < sizeof(PLAYERCOMM_QUERYCHANNELID) + sizeof(tagPlusSrcInfo))
		return;

	PLAYERCOMM_QUERYCHANNELID* pPlayerCommQID = (PLAYERCOMM_QUERYCHANNELID*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	std::_tstring rolename;
	if (!FindPlayerByParam(pSrcInfo->lnID, NULL, &rolename, NULL))
		return;

	g_ChannelMgr.B_QueryChannelID(pPlayerCommQID->channel, GetIP(), pSrcInfo->lnID, pSrcInfo->nameid);
}

void CHostConnect::Proc1_PlayerCommunity_FreeChannelID(const void* pData, size_t size)
{
	if (size < sizeof(PLAYERCOMM_FREECHANNELID) + sizeof(tagPlusSrcInfo))
		return;

	PLAYERCOMM_FREECHANNELID* pPlayerCommFID = (PLAYERCOMM_FREECHANNELID*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	g_ChannelMgr.B_FreeChannID(pPlayerCommFID->channelid, GetIP(), pSrcInfo->lnID);
}

void CHostConnect::Proc1_PlayerCommunity_Subscribe(const void* pData, size_t size)
{
	if (size < sizeof(PLAYERCOMM_SUBSCRIBE) + sizeof(tagPlusSrcInfo))
		return;

	PLAYERCOMM_SUBSCRIBE* pPlayerCommSub = (PLAYERCOMM_SUBSCRIBE*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	if (pPlayerCommSub->subscribe)
		g_ChannelMgr.B_Subscribe(GetIP(), pSrcInfo->lnID, pPlayerCommSub->channelid);
	else
		g_ChannelMgr.B_Unsubscribe(GetIP(), pSrcInfo->lnID, pPlayerCommSub->channelid);

}


void CHostConnect::OnClientConnectCreate()
{
	rTRACE("host connect create: %s", _ip2a(GetIP()));
}

void CHostConnect::OnClientConnectClose()
{
	rTRACE("host connect close: %s", _ip2a(GetIP()));

	if (!g_RootClient.IsReady())
		return;

	size_t pckgsize = sizeof(RELAY_ASKWAY_DATA) + sizeof(EXTEND_HEADER) + sizeof(KServerInfo);

	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);
	pAskWayData->ProtocolFamily = pf_relay;
	pAskWayData->ProtocolID = relay_c2c_askwaydata;
	pAskWayData->nFromIP = 0;
	pAskWayData->nFromRelayID = 0;
	pAskWayData->seekRelayCount = 0;
	pAskWayData->seekMethod = rm_gm;
	pAskWayData->wMethodDataLength = 0;
	pAskWayData->routeDateLength = sizeof(EXTEND_HEADER) + sizeof(KServerInfo);

	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)(pAskWayData + 1);
	pHeader->ProtocolFamily = pf_normal;
	pHeader->ProtocolID = s2c_gatewayinfo;
	KServerInfo* pSvrInfo = (KServerInfo*)(pHeader + 1);
	pSvrInfo->Size = sizeof(KServerInfo);
	pSvrInfo->Type = ServerInfo;
	pSvrInfo->Operate = 0;
	pSvrInfo->Version = ACCOUNT_CURRENT_VERSION;
	pSvrInfo->nValue = 0;
	pSvrInfo->Account[0] = 0;
	pSvrInfo->nServerType = server_Logout;
	pSvrInfo->nValue = GetIP();

	g_RootClient.SendPackage(pAskWayData, pckgsize);
}


BOOL CHostConnect::FindPlayerByAcc(const std::_tstring& acc, std::_tstring* pRole, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealAcc)
{
	DUMMY_AUTOLOCKREAD(m_lockPlayer);

	ACCMAP::iterator it = m_mapAcc.find(acc);
	if (it == m_mapAcc.end())
		return FALSE;

	const ACCINFO& infoAcc = (*it).second;

	if (pRole != NULL)
		*pRole = infoAcc.role;
	if (pNameID != NULL)
		*pNameID = infoAcc.nameID;
	if (pParam != NULL)
		*pParam = infoAcc.param;

	if (pRealAcc != NULL)
		*pRealAcc = (*it).first;

	return TRUE;
}

BOOL CHostConnect::FindPlayerByRole(const std::_tstring& role, std::_tstring* pAcc, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealRole)
{
	DUMMY_AUTOLOCKREAD(m_lockPlayer);

	ROLEMAP::iterator it = m_mapRole.find(role);
	if (it == m_mapRole.end())
		return FALSE;

	const ROLEINFO& infoRole = (*it).second;

	if (pAcc != NULL)
		*pAcc = infoRole.acc;
	if (pNameID != NULL)
		*pNameID = infoRole.nameID;
	if (pParam != NULL)
		*pParam = infoRole.param;

	if (pRealRole != NULL)
		*pRealRole = (*it).first;

	return TRUE;
}

BOOL CHostConnect::FindPlayerByParam(unsigned long param, std::_tstring* pAcc, std::_tstring* pRole, DWORD* pNameID)
{
	DUMMY_AUTOLOCKREAD(m_lockPlayer);

	PARAMMAP::iterator it = m_mapParam.find(param);
	if (it == m_mapParam.end())
		return FALSE;

	const PARAMINFO& infoParam = (*it).second;

	if (pAcc != NULL)
		*pAcc = infoParam.acc;
	if (pRole != NULL)
		*pRole = infoParam.role;
	if (pNameID != NULL)
		*pNameID = infoParam.nameID;

	return TRUE;
}

/*
BOOL CHostConnect::BroadOnPlayer(BOOL byAcc, DWORD fromIP, unsigned long fromRelayID, const void* pData, size_t size)
{
	assert(pData != NULL && size > 0);

	size_t methodsize = _NAME_LEN + sizeof(DWORD) * 2;
	size_t pckgsize = sizeof(RELAY_ASKWAY_DATA) + methodsize + size;

	RELAY_ASKWAY_DATA* pAskwayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);
	pAskwayData->ProtocolFamily = pf_relay;
	pAskwayData->ProtocolID = relay_c2c_askwaydata;
	pAskwayData->nFromIP = fromIP;
	pAskwayData->nFromRelayID = fromRelayID;
	pAskwayData->seekRelayCount = 0;
	pAskwayData->seekMethod = byAcc ? rm_account_id : rm_role_id;
	pAskwayData->wMethodDataLength = methodsize;
	pAskwayData->routeDateLength = size;

	char* pTheName = (char*)(pAskwayData + 1);
	DWORD* pTheInfo = (DWORD*)(pTheName + _NAME_LEN);

	void* pRouteData = pTheInfo + 2;
	memcpy(pRouteData, pData, size);


	DUMMY_AUTOLOCKREAD(m_lockPlayer);

	for (PARAMMAP::const_iterator it = m_mapParam.begin(); it != m_mapParam.end(); it++)
	{
		const PARAMINFO& infoParam = (*it).second;
		strcpy(pTheName, byAcc ? infoParam.acc.c_str() : infoParam.role.c_str());
		pTheInfo[0] = infoParam.nameID;
		pTheInfo[1] = (*it).first;

		SendPackage(pAskwayData, pckgsize);
	}

	return TRUE;
}
*/

void CHostConnect::PrepareRecvs()
{
}

void CHostConnect::UnprepareRecvs()
{
	g_ChannelMgr.DoBlockOp(-1);
}


size_t CHostConnect::GetPlayerCount()
{
	DUMMY_AUTOLOCKREAD(m_lockPlayer);

	return m_mapRole.size();
}
