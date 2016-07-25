// TongServer.h: interface for the CTongServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TONGSERVER_H__01285A06_8149_4948_ABD4_175225328427__INCLUDED_)
#define AFX_TONGSERVER_H__01285A06_8149_4948_ABD4_175225328427__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetServer.h"
#include "TongConnect.h"
#include <map>

class CTongServer : public CNetServer  
{
public:
	CTongServer();
	virtual ~CTongServer();

protected:
	virtual CNetConnect* CreateConnect(CNetServer* pNetServer, unsigned long id);
	virtual void DestroyConnect(CNetConnect* pConn);

	virtual void OnBuildup();
	virtual void OnClearup();

	virtual void OnClientConnectCreate(CNetConnect* pConn);
	virtual void OnClientConnectClose(CNetConnect* pConn);

private:
	typedef std::map<DWORD, CTongConnect*>	IP2CONNECTMAP;
	IP2CONNECTMAP m_mapIp2Connect;

	CLockMRSW m_lockIpMap;

public:
	class CNetConnectDup FindTongConnectByIP(DWORD IP);

public:
	BOOL TraceInfo();
};

#endif // !defined(AFX_TONGSERVER_H__01285A06_8149_4948_ABD4_175225328427__INCLUDED_)
