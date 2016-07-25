// HostServer.cpp: implementation of the CHostServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HostServer.h"
#include "HostConnect.h"
#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHostServer::CHostServer()
{

}

CHostServer::~CHostServer()
{

}


CNetConnect* CHostServer::CreateConnect(CNetServer* pNetServer, unsigned long id)
{
	return new CHostConnect((CHostServer*)pNetServer, id);
}

void CHostServer::DestroyConnect(CNetConnect* pConn)
{
	delete pConn;
}

void CHostServer::OnBuildup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	rTRACE("host server startup");
}

void CHostServer::OnClearup()
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.clear();

	rTRACE("host server shutdown");
}

void CHostServer::OnClientConnectCreate(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect[pConn->GetIP()] = (CHostConnect*)pConn;
}

void CHostServer::OnClientConnectClose(CNetConnect* pConn)
{
	AUTOLOCKWRITE(m_lockIpMap);

	m_mapIp2Connect.erase(pConn->GetIP());
}

CNetConnectDup CHostServer::FindHostConnectByIP(DWORD IP)
{
	AUTOLOCKREAD(m_lockIpMap);

	IP2CONNECTMAP::iterator it = m_mapIp2Connect.find(IP);
	if (it == m_mapIp2Connect.end())
		return CNetConnectDup();

	CHostConnect* pHostConn = (*it).second;
	if (!pHostConn)
		return CNetConnectDup();

	return CNetConnectDup(*pHostConn);
}


BOOL CHostServer::FindPlayerByAcc(CHostConnect* pConn, const std::_tstring& acc, CNetConnectDup* pConnDup, std::_tstring* pRole, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealAcc)
{
	AUTOLOCKREAD(m_lockIpMap);

	for (IP2CONNECTMAP::iterator it = m_mapIp2Connect.begin(); it != m_mapIp2Connect.end(); it++)
	{
		CHostConnect* pConnMe = (*it).second;

		if (pConnMe != pConn)
		{
			if (pConnMe->FindPlayerByAcc(acc, pRole, pNameID, pParam, pRealAcc))
			{
				if (pConnDup != NULL)
					*pConnDup = *pConnMe;

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CHostServer::FindPlayerByRole(CHostConnect* pConn, const std::_tstring& role, CNetConnectDup* pConnDup, std::_tstring* pAcc, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealRole)
{
	AUTOLOCKREAD(m_lockIpMap);

	for (IP2CONNECTMAP::iterator it = m_mapIp2Connect.begin(); it != m_mapIp2Connect.end(); it++)
	{
		CHostConnect* pConnMe = (*it).second;

		if (pConnMe != pConn)
		{
			if (pConnMe->FindPlayerByRole(role, pAcc, pNameID, pParam, pRealRole))
			{
				if (pConnDup != NULL)
					*pConnDup = *pConnMe;

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CHostServer::FindPlayerByIpParam(CHostConnect* pConn, DWORD ip, unsigned long param, CNetConnectDup* pConnDup, std::_tstring* pAcc, std::_tstring* pRole, DWORD* pNameID)
{
	AUTOLOCKREAD(m_lockIpMap);

	IP2CONNECTMAP::const_iterator it = m_mapIp2Connect.find(ip);
	if (it == m_mapIp2Connect.end())
		return FALSE;

	CHostConnect* pConnMe = (*it).second;

	if (pConnMe != pConn)
	{
		if (pConnMe->FindPlayerByParam(param, pAcc, pRole, pNameID))
		{
			if (pConnDup != NULL)
				*pConnDup = *pConnMe;

			return TRUE;
		}
	}

	return FALSE;
}

/*
BOOL CHostServer::BroadOnPlayer(BOOL byAcc, DWORD fromIP, unsigned long fromRelayID, const void* pData, size_t size)
{
	AUTOLOCKREAD(m_lockIpMap);

	for (IP2CONNECTMAP::const_iterator it = m_mapIp2Connect.begin(); it != m_mapIp2Connect.end(); it++)
	{
		CHostConnect* pConnMe = (*it).second;

		pConnMe->BroadOnPlayer(byAcc, fromIP, fromRelayID, pData, size);
	}

	return FALSE;
}
*/


size_t CHostServer::GetPlayerCount()
{
	AUTOLOCKREAD(m_lockIpMap);

	size_t total = 0;

	for (IP2CONNECTMAP::iterator it = m_mapIp2Connect.begin(); it != m_mapIp2Connect.end(); it++)
	{
		CHostConnect* pConn = (*it).second;

		assert(pConn);
		if (pConn)
			total += pConn->GetPlayerCount();
	}

	return total;
}


BOOL CHostServer::TraceInfo()
{
	AUTOLOCKREAD(m_lockIpMap);

	std::_tstring info("message: [HostServer] ");
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

BOOL CHostServer::TracePlayer()
{
	AUTOLOCKREAD(m_lockIpMap);

	std::_tstring info("message: [Player] : ");
	char buffer[_MAX_PATH];

	size_t total = 0;

	for (IP2CONNECTMAP::iterator it = m_mapIp2Connect.begin(); it != m_mapIp2Connect.end(); it++)
	{
		CHostConnect* pConn = (*it).second;

		assert(pConn);
		if (pConn)
		{
			if (it != m_mapIp2Connect.begin())
				info.append(", ");

			size_t count = pConn->GetPlayerCount();
			total += count;

			sprintf(buffer, "%08X: %d", (*it).first, count);
			info.append(buffer);
		}
	}

	sprintf(buffer, " <total: %d>", total);
	info.append(buffer);

	rTRACE(info.c_str());

	return TRUE;
}

