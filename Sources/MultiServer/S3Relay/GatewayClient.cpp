// -------------------------------------------------------------------------
//	文件名		：	GatewayClient.cpp
//	创建者		：	万里
//	创建时间	：	2003-6-6 20:42:39
//	功能描述	：	
//
// -------------------------------------------------------------------------

#include "stdafx.h"
#include "Global.h"
#include "GatewayClient.h"
#include "GatewayCenter.h"
#include "S3Relay.h"
#include "../../Network/s3accserver/AccountLoginDef.h"


CGatewayClient::CGatewayClient(CGatewayCenter* pGatewayCenter, BOOL bAutoFree)
	: CNetClient(pGatewayCenter, bAutoFree), m_lastPckgID(0)
{
}

CGatewayClient::~CGatewayClient()
{
	assert(m_mapAskWayMaps.empty());
}


void CGatewayClient::RecvPackage(const void* pData, size_t size)
{
	assert(size > 0 && pData != NULL);
	BYTE protocol = *(BYTE*)pData;

	if (protocol == s2c_notifysvrip)
	{
		Proc0_NotifySvrIP(pData, size);
	}
}


void CGatewayClient::OnServerEventCreate()
{
	gOnOneClientCreate(oneclient_gateway);

	rTRACE("gateway client create: %s", _ip2a(GetSvrIP()));
}

void CGatewayClient::OnServerEventClose()
{
	rTRACE("gateway client close: %s", _ip2a(GetSvrIP()));


	{{
	AUTOLOCKWRITE(m_lockAskWayMap);

	BYTE buffer[max_packagesize];

	//clearup
	for (ID2ASKWAYMAPMAP::iterator it = m_mapAskWayMaps.begin(); it != m_mapAskWayMaps.end(); it++)
	{
		ASKWAYMAP& rAskWayMap = (*it).second;
		size_t sizePckg = sizeof(RELAY_ASKWAY_DATA) + rAskWayMap.pAskWayMap->wMethodDataLength + rAskWayMap.pAskWayMap->routeDateLength;

		size_t sizeLoseData = sizeof(RELAY_DATA) + sizePckg;
		assert(sizeLoseData <= max_packagesize);

		if (sizeLoseData <= max_packagesize)
		{
			CNetConnectDup conndup = g_HostServer.FindHostConnectByIP(rAskWayMap.fromIP);
			if (conndup.IsValid())
			{
				size_t sizeLoseData = sizeof(RELAY_DATA) + sizePckg;
				RELAY_DATA* pLoseDataDup = (RELAY_DATA*)buffer;

				gFillLosewayPckg(0, 0, 
					rAskWayMap.pAskWayMap, sizePckg, pLoseDataDup, sizeLoseData);

				conndup.SendPackage(pLoseDataDup, sizeLoseData);
			}
		}

		free(rAskWayMap.pAskWayMap);
	}

	m_mapAskWayMaps.clear();
	}}


	gOnOneClientClose(oneclient_gateway);


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
	pSvrInfo->nValue = GetSvrIP();

	g_RootClient.SendPackage(pAskWayData, pckgsize);
}


void CGatewayClient::Proc0_NotifySvrIP(const void* pData, size_t size)
{
	tagNotifySvrIp* pN = (tagNotifySvrIp*)pData;
	if (pN->cIPType != INTRANER_IP)	//except !
		return;


	AUTOLOCKWRITE(m_lockAskWayMap);


	ID2ASKWAYMAPMAP::iterator itMap = m_mapAskWayMaps.find(pN->pckgID);
	if (itMap == m_mapAskWayMaps.end())	//miss
		return;


	ASKWAYMAP& rAskWayMap = (*itMap).second;
	assert(rAskWayMap.pAskWayMap);
	DWORD mapID = *(DWORD*)(rAskWayMap.pAskWayMap + 1);

	assert(pN->dwMapID == mapID);
	if (pN->dwMapID != mapID)	//except !
		return;


	BOOL passed = FALSE;


	size_t sizePckg = sizeof(RELAY_ASKWAY_DATA) + rAskWayMap.pAskWayMap->wMethodDataLength + rAskWayMap.pAskWayMap->routeDateLength;

	if (pN->dwSvrIP != 0)
	{
		CNetConnectDup conndup = g_HostServer.FindHostConnectByIP(pN->dwSvrIP);
		if (conndup.IsValid())
		{//relay
			conndup.SendPackage(rAskWayMap.pAskWayMap, sizePckg);

			passed = TRUE;
		}
	}

	if (!passed)
	{//lose
		CNetConnectDup conndup2 = g_HostServer.FindHostConnectByIP(rAskWayMap.fromIP);
		if (conndup2.IsValid())
		{
			size_t sizeLoseData = sizeof(RELAY_DATA) + sizePckg;
			RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

			gFillLosewayPckg(0, 0, 
				rAskWayMap.pAskWayMap, sizePckg, pLoseDataDup, sizeLoseData);

			conndup2.SendPackage(pLoseDataDup, sizeLoseData);
		}
	}

	free(rAskWayMap.pAskWayMap);
	m_mapAskWayMaps.erase(itMap);

}

BOOL CGatewayClient::PassAskWayMap(DWORD fromIP, const RELAY_ASKWAY_DATA* pAskWayMap)
{
	assert(pAskWayMap && pAskWayMap->seekMethod == rm_map_id);
	size_t size = sizeof(RELAY_ASKWAY_DATA) + pAskWayMap->wMethodDataLength + pAskWayMap->routeDateLength;


	AUTOLOCKWRITE(m_lockAskWayMap);

	if (IsReady())
	{
		if (m_mapAskWayMaps.size() < MAX_PACKAGECOUNT)
		{
			for ( ; ; )
			{
				std::pair<ID2ASKWAYMAPMAP::iterator, bool> insret = m_mapAskWayMaps.insert(ID2ASKWAYMAPMAP::value_type(++m_lastPckgID, ASKWAYMAP()));
				if (insret.second)
				{//insert, not existed
					ASKWAYMAP& rAskWayMap = (*insret.first).second;

					rAskWayMap.tick = ::GetTickCount();
					rAskWayMap.fromIP = fromIP;
					rAskWayMap.pAskWayMap = (RELAY_ASKWAY_DATA*)malloc(size);
					memcpy(rAskWayMap.pAskWayMap, pAskWayMap, size);

					break;
				}
			}


			DWORD mapID = *(DWORD*)(pAskWayMap + 1);

			tagRequestSvrIp req;
			req.cProtocol = c2s_requestsvrip;
			req.pckgID = m_lastPckgID;
			req.cIPType = INTRANER_IP;
			req.dwMapID = mapID;

			SendPackage(&req, sizeof(req));

			return TRUE;
		}
	}


	{{
	//lose way
	CNetConnectDup conndup = g_HostServer.FindHostConnectByIP(fromIP);
	if (conndup.IsValid())
	{
		size_t sizeLoseData = sizeof(RELAY_DATA) + size;
		RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

		gFillLosewayPckg(0, 0, 
			pAskWayMap, size, pLoseDataDup, sizeLoseData);

		conndup.SendPackage(pLoseDataDup, sizeLoseData);

		return TRUE;
	}
	}}


	return FALSE;
}
