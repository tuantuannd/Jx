// NetSockDupEx.cpp: implementation of the CNetSockDupEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetSockDupEx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetSockDupEx::CNetSockDupEx()
{

}

CNetSockDupEx::CNetSockDupEx(const CNetConnectDup& rNetConnectDup)
	: m_NetConnectDup(rNetConnectDup)
{
	;
}

CNetSockDupEx::CNetSockDupEx(const CNetClientDup& rNetClientDup)
	: m_NetClientDup(rNetClientDup)
{
}

CNetSockDupEx::CNetSockDupEx(const CNetSockDupEx& rNetSockDupEx)
	: m_NetConnectDup(rNetSockDupEx.m_NetConnectDup), m_NetClientDup(rNetSockDupEx.m_NetClientDup)
{
}


CNetSockDupEx::~CNetSockDupEx()
{
}


CNetSockDupEx::SOCKTYPE CNetSockDupEx::GetSockType() const
{
	if (m_NetConnectDup.IsValid())
		return socktype_Connect;
	if (m_NetClientDup.IsValid())
		return socktype_Client;
	return socktype_none;
}

BOOL CNetSockDupEx::SendPackage(const void* pData, size_t size) const
{
	if (m_NetConnectDup.IsValid())
		return m_NetConnectDup.SendPackage(pData, size);
	if (m_NetClientDup.IsValid())
		return m_NetClientDup.SendPackage(pData, size);
	return FALSE;
}

DWORD CNetSockDupEx::GetSockIP()const
{
	if (m_NetConnectDup.IsValid())
		return m_NetConnectDup.GetIP();
	if (m_NetClientDup.IsValid())
		return m_NetClientDup.GetSvrIP();
	return 0;
}

unsigned long CNetSockDupEx::GetSockParam() const
{
	if (m_NetConnectDup.IsValid())
		return m_NetConnectDup.GetID();
	//if (m_NetClientDup.IsValid())
	//	return -1;
	return -1;
}


CNetSockDupEx& CNetSockDupEx::operator= (const CNetConnectDup& rNetConnectDup)
{
	m_NetConnectDup = rNetConnectDup;
	m_NetClientDup.Clearup();

	return *this;
}

CNetSockDupEx& CNetSockDupEx::operator= (const CNetClientDup& rNetClientDup)
{
	m_NetConnectDup.Clearup();
	m_NetClientDup = rNetClientDup;

	return *this;
}

CNetSockDupEx& CNetSockDupEx::operator= (const CNetSockDupEx& rNetSockDupEx)
{
	m_NetConnectDup = rNetSockDupEx.m_NetConnectDup;
	m_NetClientDup = rNetSockDupEx.m_NetClientDup;

	return *this;
}

