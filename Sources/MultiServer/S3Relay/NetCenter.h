// NetCenter.h: interface for the CNetCenter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETCENTER_H__7D615C7A_1D62_4EBC_B25C_7748DC4EEF6F__INCLUDED_)
#define AFX_NETCENTER_H__7D615C7A_1D62_4EBC_B25C_7748DC4EEF6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <set>
#include "NetClient.h"

class CNetCenter  
{
public:
	CNetCenter();
	virtual ~CNetCenter();

public:
	BOOL Initialize();
	BOOL Uninitialize();

	size_t GetClientCount();

	BOOL BroadPackage(const void* pData, size_t size);

private:
	typedef std::set<CNetClient*>	CLIENTSET;
	CLIENTSET m_setClient;

public:
	void _NotifyServerEventCreate(CNetClient* pClient);
	void _NotifyServerEventClose(CNetClient* pClient);

protected:
	virtual void OnBuildup() {}
	virtual void OnClearup() {}

	virtual void OnServerEventCreate(CNetClient* pClient) {}
	virtual void OnServerEventClose(CNetClient* pClient) {}

private:
	CLockMRSW m_lockAccess;
	CLockMRSW m_lockServer;

public:
	BOOL Route();
};

#endif // !defined(AFX_NETCENTER_H__7D615C7A_1D62_4EBC_B25C_7748DC4EEF6F__INCLUDED_)
