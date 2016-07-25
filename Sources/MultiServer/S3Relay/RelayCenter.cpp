// RelayCenter.cpp: implementation of the CRelayCenter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RelayCenter.h"
#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRelayCenter::CRelayCenter()
{

}

CRelayCenter::~CRelayCenter()
{

}


void CRelayCenter::OnBuildup()
{
	AUTOLOCKWRITE(m_lockIpMap);

}

void CRelayCenter::OnClearup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Client.clear();
}

void CRelayCenter::OnServerEventCreate(CNetClient* pClient)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Client[pClient->GetSvrIP()] = (CRelayClient*)pClient;
}

void CRelayCenter::OnServerEventClose(CNetClient* pClient)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Client.erase(pClient->GetSvrIP());
}


BOOL CRelayCenter::FindRelayClientByIP(DWORD IP, CNetClientDup* pClntDup)
{
	AUTOLOCKREAD(m_lockIpMap);

	IP2CLIENTMAP::iterator it = m_mapIp2Client.find(IP);
	if (it == m_mapIp2Client.end())
		return FALSE;

	CRelayClient* pRelayClnt = (*it).second;
	if (!pRelayClnt)
		return FALSE;

	if (pClntDup != NULL)
		*pClntDup = *pRelayClnt;
	return TRUE;
}


BOOL CRelayCenter::TraceInfo()
{
	AUTOLOCKREAD(m_lockIpMap);

	std::_tstring info("message: [RelayCenter] ");
	char buffer[_MAX_PATH];

	sprintf(buffer, "<total: %d> : ", m_mapIp2Client.size());
	info.append(buffer);

	size_t idx = 0;
	for (IP2CLIENTMAP::iterator it = m_mapIp2Client.begin(); it != m_mapIp2Client.end(); it++)
	{
		if (it != m_mapIp2Client.begin())
			info.append(", ");

		sprintf(buffer, "%08X", (*it).first);
		info.append(buffer);
	}

	rTRACE(info.c_str());

	return TRUE;
}
