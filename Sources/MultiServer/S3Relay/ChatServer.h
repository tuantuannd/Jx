// ChatServer.h: interface for the CChatServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATSERVER_H__3351C024_0A4B_4A13_B229_230CF4AC05F1__INCLUDED_)
#define AFX_CHATSERVER_H__3351C024_0A4B_4A13_B229_230CF4AC05F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include "NetServer.h"
#include "ChatConnect.h"

class CChatServer : public CNetServer  
{
public:
	CChatServer();
	virtual ~CChatServer();

protected:
	virtual CNetConnect* CreateConnect(CNetServer* pNetServer, unsigned long id);
	virtual void DestroyConnect(CNetConnect* pConn);

	virtual void OnBuildup();
	virtual void OnClearup();

	virtual void OnClientConnectCreate(CNetConnect* pConn);
	virtual void OnClientConnectClose(CNetConnect* pConn);

private:
	typedef std::map<DWORD, CChatConnect*>	IP2CONNECTMAP;
	IP2CONNECTMAP m_mapIp2Connect;

	CLockMRSW m_lockIpMap;

public:
	class CNetConnectDup FindChatConnectByIP(DWORD IP);

public:
	BOOL TraceInfo();
};

#endif // !defined(AFX_CHATSERVER_H__3351C024_0A4B_4A13_B229_230CF4AC05F1__INCLUDED_)
