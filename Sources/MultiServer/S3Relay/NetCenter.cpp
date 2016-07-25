// NetCenter.cpp: implementation of the CNetCenter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetCenter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetCenter::CNetCenter()
{

}

CNetCenter::~CNetCenter()
{

}


BOOL CNetCenter::Initialize()
{
	AUTOLOCKWRITE(m_lockAccess);

	OnBuildup();

	return TRUE;
}

BOOL CNetCenter::Uninitialize()
{
	AUTOLOCKWRITE(m_lockAccess);

	for (CLIENTSET::iterator it = m_setClient.begin(); it != m_setClient.end(); it++)
	{
		CNetClient* pClient = (*it);
		assert(pClient);
		if (pClient)
			pClient->Shutdown();
	}
	m_setClient.clear();

	OnClearup();

	return TRUE;
}


void CNetCenter::_NotifyServerEventCreate(CNetClient* pClient)
{
	AUTOLOCKWRITE(m_lockAccess);

	OnServerEventCreate(pClient);

	m_setClient.insert(pClient);
}

void CNetCenter::_NotifyServerEventClose(CNetClient* pClient)
{
	AUTOLOCKWRITE(m_lockAccess);

	m_setClient.erase(pClient);

	OnServerEventClose(pClient);
}

size_t CNetCenter::GetClientCount() 
{
	AUTOLOCKREAD(m_lockAccess);
	return m_setClient.size();
}

BOOL CNetCenter::BroadPackage(const void* pData, size_t size)
{
	AUTOLOCKREAD(m_lockAccess);

	for (CLIENTSET::iterator it = m_setClient.begin(); it != m_setClient.end(); it++)
	{
		CNetClient* pNetClient = (*it);
		assert(pNetClient);
		if (pNetClient)
			pNetClient->SendPackage(pData, size);
	}

	return TRUE;
}


BOOL CNetCenter::Route()
{
	AUTOLOCKREAD(m_lockAccess);

	for (CLIENTSET::iterator it = m_setClient.begin(); it != m_setClient.end(); it++)
	{
		CNetClient* pNetClient = *it;
		pNetClient->Route();
	}

	return TRUE;
}
