// RelayServer.cpp: implementation of the CRelayServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RelayServer.h"
#include "RelayConnect.h"
#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRelayServer::CRelayServer()
{

}

CRelayServer::~CRelayServer()
{

}


CNetConnect* CRelayServer::CreateConnect(CNetServer* pNetServer, unsigned long id)
{
	return new CRelayConnect((CRelayServer*)pNetServer, id);
}

void CRelayServer::DestroyConnect(CNetConnect* pConn)
{
	delete pConn;
}

void CRelayServer::OnBuildup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	rTRACE("relay server startup");
}

void CRelayServer::OnClearup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.clear();

	rTRACE("relay server shutdown");
}

void CRelayServer::OnClientConnectCreate(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect[pConn->GetIP()] = (CRelayConnect*)pConn;
}

void CRelayServer::OnClientConnectClose(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.erase(pConn->GetIP());
}

BOOL CRelayServer::FindRelayConnectByIP(DWORD IP, CNetConnectDup* pConnDup)
{
	AUTOLOCKREAD(m_lockIpMap);

	IP2CONNECTMAP::iterator it = m_mapIp2Connect.find(IP);
	if (it == m_mapIp2Connect.end())
		return FALSE;

	CRelayConnect* pRelayConn = (*it).second;
	if (!pRelayConn)
		return FALSE;

	if (pConnDup != NULL)
		*pConnDup = *pRelayConn;
	return TRUE;
}

BOOL CRelayServer::TraceInfo()
{
	AUTOLOCKREAD(m_lockIpMap);

	std::_tstring info("message: [RelayServer] ");
	char buffer[_MAX_PATH];

	sprintf(buffer, "<total: %d> : ", m_mapIp2Connect.size());
	info.append(buffer);

	size_t idx = 0;
	for (IP2CONNECTMAP::iterator it = m_mapIp2Connect.begin(); it != m_mapIp2Connect.end(); it++)
	{
		if (it != m_mapIp2Connect.begin())
			info.append(", ");

		sprintf(buffer, "%08X", (*it).first);
		info.append(buffer);
	}

	rTRACE(info.c_str());

	return TRUE;
}
