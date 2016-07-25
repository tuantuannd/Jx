// RelayServer.h: interface for the CRelayServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RELAYSERVER_H__AA2F8FC9_AC25_4060_A136_DC9545C92699__INCLUDED_)
#define AFX_RELAYSERVER_H__AA2F8FC9_AC25_4060_A136_DC9545C92699__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetServer.h"
#include "RelayConnect.h"

class CRelayServer : public CNetServer  
{
public:
	CRelayServer();
	virtual ~CRelayServer();

private:
	virtual CNetConnect* CreateConnect(CNetServer* pNetServer, unsigned long id);
	virtual void DestroyConnect(CNetConnect* pConn);

	virtual void OnBuildup();
	virtual void OnClearup();

	virtual void OnClientConnectCreate(CNetConnect* pConn);
	virtual void OnClientConnectClose(CNetConnect* pConn);

private:
	typedef std::map<DWORD, CRelayConnect*>	IP2CONNECTMAP;
	IP2CONNECTMAP m_mapIp2Connect;

	CLockMRSW m_lockIpMap;

public:
	BOOL FindRelayConnectByIP(DWORD IP, class CNetConnectDup* pConnDup);

public:
	BOOL TraceInfo();
};

#endif // !defined(AFX_RELAYSERVER_H__AA2F8FC9_AC25_4060_A136_DC9545C92699__INCLUDED_)
