// DBClient.cpp: implementation of the CDBClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "DBConnect.h"
#include "S3Relay.h"
#include "DealRelay.h"
#include "crtdbg.h"
#include "../../../Headers/KGmProtocol.h"

#include "../../Network/S3AccServer/AccountLoginDef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBCenter::CDBCenter()
{

}

CDBCenter::~CDBCenter()
{

}


BOOL CDBCenter::TraceInfo()
{
	std::_tstring info("message: [DBRoleCenter] ");
	char buffer[_MAX_PATH];

	BOOL ready = g_DBClient.IsReady();

	sprintf(buffer, "<total: %d> : ", ready ? 1 : 0);
	info.append(buffer);

	if (ready)
	{
		sprintf(buffer, "%08X", g_DBClient.GetSvrIP());
		info.append(buffer);
	}

	rTRACE(info.c_str());

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBClient::CDBClient(CDBCenter* pRootCenter, BOOL bAutoFree)
	: CNetClient(pRootCenter, bAutoFree)
{
}

CDBClient::~CDBClient()
{
}


void CDBClient::OnServerEventCreate()
{
	gOnOneClientCreate(oneclient_db);

	rTRACE("DBRole client create: %s", _ip2a(GetSvrIP()));
}

void CDBClient::OnServerEventClose()
{
	rTRACE("DBRole client close: %s", _ip2a(GetSvrIP()));

	gOnOneClientClose(oneclient_db);
}

void CDBClient::RecvPackage(const void* pData, size_t size)
{
	if (size <= 0)
		return;

	switch (*((BYTE*)pData))
	{
	case s2c_roleserver_getrolelist_result:
		{
			TProcessData* pProtocol = (TProcessData*)pData;
			int nSize = pProtocol->pDataBuffer[0];
			if (nSize > 0 && nSize <= MAX_PLAYER_PER_ACCOUNT)
			{
				int nRelaySize = sizeof(RELAY_ASKWAY_DATA) +
								 sizeof(GM_GET_ROLE_LIST_SYNC) +
								 sizeof(RoleBaseInfo) * MAX_PLAYER_PER_ACCOUNT;
				RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)_alloca(nRelaySize);
				
				pAskWayData->ProtocolFamily = pf_relay;
				pAskWayData->ProtocolID = relay_c2c_askwaydata;
				pAskWayData->nFromIP = 0;
				pAskWayData->nFromRelayID = 0;
				pAskWayData->seekRelayCount = 0;
				pAskWayData->seekMethod = rm_gm;
				pAskWayData->wMethodDataLength = 0;
				pAskWayData->routeDateLength = nRelaySize - sizeof(RELAY_ASKWAY_DATA);

				GM_GET_ROLE_LIST_SYNC* pGMData = (GM_GET_ROLE_LIST_SYNC*)(pAskWayData + 1);
				pGMData->AccountName[0] = 0;
				pGMData->wRoleCount = nSize;
				pGMData->ProtocolFamily = pf_gamemaster;
				pGMData->ProtocolType = gm_s2c_getrolelist;
				memcpy((pGMData + 1), &pProtocol->pDataBuffer[1], sizeof(RoleBaseInfo) * MAX_PLAYER_PER_ACCOUNT);
				g_RootClient.SendPackage(pAskWayData, nRelaySize);
			}
			else
				_ASSERT(0);
		}
		break;
	case s2c_extend:
		if (size > 1)
		{
			int nRelaySize = sizeof(RELAY_ASKWAY_DATA) + size - 1;
			RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)_alloca(nRelaySize);
			
			pAskWayData->ProtocolFamily = pf_relay;
			pAskWayData->ProtocolID = relay_c2c_askwaydata;
			pAskWayData->nFromIP = 0;
			pAskWayData->nFromRelayID = 0;
			pAskWayData->seekRelayCount = 0;
			pAskWayData->seekMethod = rm_gm;
			pAskWayData->wMethodDataLength = 0;
			pAskWayData->routeDateLength = size - 1;

			memcpy(pAskWayData + 1, ((char*)pData) + 1, size - 1);
			g_RootClient.SendPackage(pAskWayData, nRelaySize);
		}
		break;
	default:
		_ASSERT(0);
		break;
	};
}

