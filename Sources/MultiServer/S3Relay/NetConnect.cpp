// NetConnect.cpp: implementation of the CNetConnect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "NetConnect.h"
#include "NetServer.h"

#include "S3Relay.h"

#include "../../Multiserver/Heaven/Interface/IServer.h"
#include "../../../headers/KProtocolDef.h"
#include "../../../headers/KProtocol.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetConnectDup::CNetConnectDup(const CNetConnect& netconnect)
	 : m_pServer(NULL), m_IP(netconnect.GetIP()), m_id(netconnect.GetID())
{
	CNetServer* pNetServer = netconnect.GetServer();
	if (pNetServer)
	{
		m_pServer = pNetServer->GetServer();
		_SafeAddRef(m_pServer);
	}
}

BOOL CNetConnectDup::SendPackage(const void* pData, size_t size) const
{
	dTRACKSENDDATA(pData, size);

	if(!m_pServer)
		return FALSE;
	m_pServer->SendData(m_id, pData, size);
	return TRUE;
}

CNetConnectDup& CNetConnectDup::operator =(const CNetConnect& src)
{
	_SafeRelease(m_pServer);
	CNetServer* pNetServer = src.GetServer();
	if (pNetServer)
	{
		m_pServer = pNetServer->GetServer();
		_SafeAddRef(m_pServer);
	}
	m_IP = src.GetIP();
	m_id = src.GetID();
	return *this;
}


CNetConnect::CNetConnect(CNetServer* pNetServer, unsigned long id)
	: m_pNetServer(pNetServer), m_id(id), m_IP(0), m_port(0), m_disconnect(0L)
{
	assert(m_pNetServer);

	const char* pInfo = m_pNetServer->GetServer()->GetClientInfo(id);
	if (pInfo)
	{
		m_IP = _a2ip(pInfo);

		const char* pColon = strchr(pInfo, ':');
		if (pColon)
			m_port = atoi(pColon + 1);
	}
}

CNetConnect::~CNetConnect()
{
}


void CNetConnect::_NotifyClientConnectCreate()
{
	OnClientConnectCreate();
}

void CNetConnect::_NotifyClientConnectClose()
{
	if (m_disconnect)
		return;
	++ m_disconnect;

	OnClientConnectClose();
}

HRESULT CNetConnect::OnMessage( VOID* pData, size_t size )
{
	dTRACKRECVDATA(pData, size);
	RecvPackage(pData, size);

	return S_OK;
}

DWORD CNetConnect::Main(LPVOID lpParam)
{
#ifndef _WORKMODE_SINGLETHREAD
	EnterLoop();
#endif

	while (!IsAskingStop())
	{
#ifdef _WORKMODE_MULTITHREAD2
		BeginRoute();
		EndRoute();
		Sleep(2000);
		continue;
#else
		DoRoute();
		Sleep(breathe_interval);
#endif
	}

#ifndef _WORKMODE_SINGLETHREAD
	LeaveLoop();
#endif

	return 0;
}


BOOL CNetConnect::SendPackage(const void* pData, size_t size)
{
	IServer* pServer = m_pNetServer->GetServer();
	if (pServer == NULL)
		return FALSE;

	dTRACKSENDDATA(pData, size);
	pServer->SendData(m_id, pData, size);
	return TRUE;
}

BOOL CNetConnect::IsReady() const
{
	return m_pNetServer->GetServer() != NULL;
}

BOOL CNetConnect::DoRoute()
{
	IServer* pServer = m_pNetServer->GetServer();
	if (pServer == NULL)
		return FALSE;

	BeginRoute();

	for ( ; ; )
	{
		size_t size = 0;
		const void* pData = pServer->GetPackFromClient(m_id, size);
		if (!pData || size <= 0)
			break;

		dTRACKRECVDATA(pData, size);
		RecvPackage(pData, size);
	}

	EndRoute();

	return TRUE;
}

BOOL CNetConnect::Route()
{
	return DoRoute();
}
