// DealRelay.cpp: implementation of the CDealRelay class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "DealRelay.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


///////////////////////
namespace dealrelay {
///////////////////////

////////////////////////////////
//internal functions

void _Proc2_Relay_AskWay_AccountRoleID(const CNetSockDupEx& NetSockDupEx, const void* pData, size_t size)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	std::_tstring strAcc = (char*)(pAskWayData + 1);
	assert(!strAcc.empty());

	DWORD nameid = -1;
	unsigned long param = -1;
	CNetConnectDup conndup;

	if (g_HostServer.FindPlayerByAcc(NULL, strAcc, &conndup, NULL, &nameid, &param))
	{//find
		size_t pckgsize = sizeof(DWORD)*2 + size;
		RELAY_ASKWAY_DATA* pClntWayDataDup = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);

		gFillClntwayPckg(nameid, param, pAskWayData, size, pClntWayDataDup, pckgsize);

		conndup.SendPackage(pClntWayDataDup, pckgsize);

		return;
	}

	
	//lose way
	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(pAskWayData->nFromIP, pAskWayData->nFromRelayID, 
		pAskWayData, size, pLoseDataDup, sizeLoseData);

	NetSockDupEx.SendPackage(pLoseDataDup, sizeLoseData);

}

void _Proc1_Relay_Data(const CNetSockDupEx& NetSockDupEx, const void* pData, size_t size)
{
	RELAY_DATA* pRelayData = (RELAY_DATA*)pData;

	assert (pRelayData->nFromIP != 0);

	if (pRelayData->nToIP == 0)
	{
		return;
	}

	if (pRelayData->nToIP == INADDR_BROADCAST)
	{
		assert(FALSE);
		return;
	}

	if (pRelayData->nToIP == gGetHostIP(global_adapt))
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

	//lose way
	size_t sizeLoseData = sizeof(RELAY_DATA) + size;
	RELAY_DATA* pLoseDataDup = (RELAY_DATA*)_alloca(sizeLoseData);

	gFillLosewayPckg(pRelayData->nFromIP, pRelayData->nFromRelayID, 
		pRelayData, size, pLoseDataDup, sizeLoseData);

	NetSockDupEx.SendPackage(pLoseDataDup, sizeLoseData);

}

void _Proc1_Relay_AskWay(const CNetSockDupEx& NetSockDupEx, const void* pData, size_t size)
{
	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)pData;

	if (pAskWayData->seekMethod == rm_account_id)
	{
		_Proc2_Relay_AskWay_AccountRoleID(NetSockDupEx, pData, size);
	}
}

void _Proc1_Relay_LoseWay(const CNetSockDupEx& NetSockDupEx, const void* pData, size_t size)
{
	RELAY_DATA* pLoseData = (RELAY_DATA*)pData;

	if (pLoseData->nFromIP == 0)
	{
		pLoseData->nFromIP = NetSockDupEx.GetSockIP();
		pLoseData->nFromRelayID = NetSockDupEx.GetSockParam();
	}

	if (pLoseData->nToIP == 0)
	{
		return;
	}

	if (pLoseData->nToIP == INADDR_BROADCAST)
	{
		assert(FALSE);
		return;
	}

	if (pLoseData->nToIP == gGetHostIP(global_adapt))
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

	//lose way
	//don't more loseway
}

void _Proc0_Relay(const CNetSockDupEx& NetSockDupEx, const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == relay_c2c_data)
	{
		_Proc1_Relay_Data(NetSockDupEx, pData, size);
	}
	else if (pHeader->ProtocolID == relay_c2c_askwaydata)
	{
		_Proc1_Relay_AskWay(NetSockDupEx, pData, size);
	}
	else if (pHeader->ProtocolID == relay_s2c_loseway)
	{
		_Proc1_Relay_LoseWay(NetSockDupEx, pData, size);
	}
}

void _RecvPackage(const CNetSockDupEx& NetSockDupEx, const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;
	if (pHeader->ProtocolFamily == pf_relay)
	{
		_Proc0_Relay(NetSockDupEx, pData, size);
	}
}




///////////////////////////////
//the publish functions' body


void DoRecvPackage(CRelayConnect* pConn, const void* pData, size_t size)
{
	assert(pConn);

	_RecvPackage(CNetSockDupEx(CNetConnectDup(*pConn)), pData, size);
}

void DoRecvPackage(CRelayClient* pClnt, const void* pData, size_t size)
{
	assert(pClnt);

	_RecvPackage(CNetSockDupEx(CNetClientDup(*pClnt)), pData, size);
}


CNetSockDupEx FindRelaySockByIP(DWORD ip)
{
	CNetConnectDup netconndup;
	if (g_RelayServer.FindRelayConnectByIP(ip, &netconndup))
		return CNetSockDupEx(netconndup);

	CNetClientDup netclntdup;
	if (g_RelayCenter.FindRelayClientByIP(ip, &netclntdup))
		return CNetSockDupEx(netclntdup);

	return CNetSockDupEx();
}


BOOL BroadPackage(const void* pData, size_t size)
{
	g_RelayServer.BroadPackage(pData, size);
	g_RelayCenter.BroadPackage(pData, size);

	return FALSE;
}



///////////////////////
} //namespace dealrelay
///////////////////////
