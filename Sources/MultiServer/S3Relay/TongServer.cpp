// TongServer.cpp: implementation of the CTongServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TongServer.h"
#include "TongConnect.h"
#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTongServer::CTongServer()
{

}

CTongServer::~CTongServer()
{

}


CNetConnect* CTongServer::CreateConnect(CNetServer* pNetServer, unsigned long id)
{
	return new CTongConnect((CTongServer*)pNetServer, id);
}

void CTongServer::DestroyConnect(CNetConnect* pConn)
{
	delete pConn;
}


void CTongServer::OnBuildup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	rTRACE("tong server startup");
}

void CTongServer::OnClearup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.clear();

	rTRACE("tong server shutdown");
}


void CTongServer::OnClientConnectCreate(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect[pConn->GetIP()] = (CTongConnect*)pConn;
}

void CTongServer::OnClientConnectClose(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.erase(pConn->GetIP());
}


CNetConnectDup CTongServer::FindTongConnectByIP(DWORD IP)
{
	AUTOLOCKREAD(m_lockIpMap);

	IP2CONNECTMAP::iterator it = m_mapIp2Connect.find(IP);
	if (it == m_mapIp2Connect.end())
		return CNetConnectDup();

	CTongConnect* pTongConn = (*it).second;
	if (!pTongConn)
		return CNetConnectDup();

	return CNetConnectDup(*pTongConn);
}


BOOL CTongServer::TraceInfo()
{
	AUTOLOCKREAD(m_lockIpMap);

	std::_tstring info("message: [TongServer] ");
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
