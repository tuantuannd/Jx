// NetServer.h: interface for the CNetServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETSERVER_H__312A07AD_96A8_40C4_914B_C13DD706CB40__INCLUDED_)
#define AFX_NETSERVER_H__312A07AD_96A8_40C4_914B_C13DD706CB40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include "../../Multiserver/Heaven/Interface/IServer.h"

#include "NetConnect.h"

class CNetServer 
{
public:
	CNetServer();
	virtual ~CNetServer();

private:
	IServer* m_pServer;
	BOOL m_ready;

private:
	static void __stdcall ServerEventNotify (
				LPVOID lpParam,
				const unsigned long &ulnID,
				const unsigned long &ulnEventType );

public:
	BOOL Startup(size_t nPlayerMaxCount, size_t nPrecision, size_t maxFreeBuffers_Cache, size_t bufferSize_Cache, unsigned long ulnAddressToListenOn, unsigned short usnPortToListenOn);
	BOOL Shutdown();

	size_t GetConnectCount();

	BOOL IsConnectReady(unsigned long id);

	BOOL Disconnect(unsigned long id);

	IServer* GetServer() const {return m_ready ? m_pServer : NULL;}

	CNetConnectDup FindNetConnect(unsigned long id);

	BOOL BroadPackage(const void* pData, size_t size);

private:
	typedef std::map<unsigned long, CNetConnect*>	ID2CONNECTMAP;
	ID2CONNECTMAP m_mapId2Connect;

private:
	void _NotifyClientConnectCreate(unsigned long ulnID);
	void _NotifyClientConnectClose(unsigned long ulnID);

protected:
	virtual CNetConnect* CreateConnect(CNetServer* pNetServer, unsigned long id) = NULL;
	virtual void DestroyConnect(CNetConnect* pConn) = NULL;

	virtual void OnBuildup() {}
	virtual void OnClearup() {}

	virtual void OnClientConnectCreate(CNetConnect* pConn) {}
	virtual void OnClientConnectClose(CNetConnect* pConn) {}

private:
	CLockMRSW m_lockAccess;

public:
	BOOL Route();

protected:
	BOOL DoSendPackage(const void* pData, size_t size);

	friend class CNetConnect;
	friend class CNetConnectDup;
};

#endif // !defined(AFX_NETSERVER_H__312A07AD_96A8_40C4_914B_C13DD706CB40__INCLUDED_)
