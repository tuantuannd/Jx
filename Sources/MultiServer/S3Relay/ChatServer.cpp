// ChatServer.cpp: implementation of the CChatServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChatServer.h"
#include "ChatConnect.h"
#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatServer::CChatServer()
{

}

CChatServer::~CChatServer()
{

}


CNetConnect* CChatServer::CreateConnect(CNetServer* pNetServer, unsigned long id)
{
	return new CChatConnect((CChatServer*)pNetServer, id);
}

void CChatServer::DestroyConnect(CNetConnect* pConn)
{
	delete pConn;
}


void CChatServer::OnBuildup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	rTRACE("chat server startup");
}

void CChatServer::OnClearup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.clear();

	rTRACE("chat server shutdown");
}

void CChatServer::OnClientConnectCreate(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect[pConn->GetIP()] = (CChatConnect*)pConn;
}

void CChatServer::OnClientConnectClose(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.erase(pConn->GetIP());
}

CNetConnectDup CChatServer::FindChatConnectByIP(DWORD IP)
{
	AUTOLOCKREAD(m_lockIpMap);

	IP2CONNECTMAP::iterator it = m_mapIp2Connect.find(IP);
	if (it == m_mapIp2Connect.end())
		return CNetConnectDup();

	CChatConnect* pChatConn = (*it).second;
	if (!pChatConn)
		return CNetConnectDup();

	return CNetConnectDup(*pChatConn);
}

BOOL CChatServer::TraceInfo()
{
	AUTOLOCKREAD(m_lockIpMap);

	std::_tstring info("message: [ChatServer] ");
	char buffer[_MAX_PATH];

	sprintf(buffer, "<total: %d> : ", m_mapIp2Connect.size());
	info.append(buffer);

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
