// HostServer.h: interface for the CHostServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HOSTSERVER_H__8CF1F726_AF29_41C0_A9AD_0AD14A7370A7__INCLUDED_)
#define AFX_HOSTSERVER_H__8CF1F726_AF29_41C0_A9AD_0AD14A7370A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include "NetServer.h"
#include "HostConnect.h"

class CHostServer : public CNetServer  
{
public:
	CHostServer();
	virtual ~CHostServer();

protected:
	virtual CNetConnect* CreateConnect(CNetServer* pNetServer, unsigned long id);
	virtual void DestroyConnect(CNetConnect* pConn);

	virtual void OnBuildup();
	virtual void OnClearup();

	virtual void OnClientConnectCreate(CNetConnect* pConn);
	virtual void OnClientConnectClose(CNetConnect* pConn);

private:
	typedef std::map<DWORD, CHostConnect*>	IP2CONNECTMAP;
	IP2CONNECTMAP m_mapIp2Connect;

	CLockMRSW m_lockIpMap;

public:
	class CNetConnectDup FindHostConnectByIP(DWORD IP);

	BOOL FindPlayerByAcc(CHostConnect* pConn, const std::_tstring& acc, CNetConnectDup* pConnDup, std::_tstring* pRole, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealAcc = NULL);
	BOOL FindPlayerByRole(CHostConnect* pConn, const std::_tstring& role, CNetConnectDup* pConnDup, std::_tstring* pAcc, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealRole = NULL);
	BOOL FindPlayerByIpParam(CHostConnect* pConn, DWORD ip, unsigned long param, CNetConnectDup* pConnDup, std::_tstring* pAcc, std::_tstring* pRole, DWORD* pNameID);

	//BOOL BroadOnPlayer(BOOL byAcc, DWORD fromIP, unsigned long fromRelayID, const void* pData, size_t size);

public:
	size_t GetPlayerCount();

public:
	BOOL TraceInfo();
	BOOL TracePlayer();
};

#endif // !defined(AFX_HOSTSERVER_H__8CF1F726_AF29_41C0_A9AD_0AD14A7370A7__INCLUDED_)
