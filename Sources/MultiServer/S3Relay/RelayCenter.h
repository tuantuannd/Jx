// RelayCenter.h: interface for the CRelayCenter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RELAYCENTER_H__7E3E4D53_5038_46C8_9B9F_84311729CA97__INCLUDED_)
#define AFX_RELAYCENTER_H__7E3E4D53_5038_46C8_9B9F_84311729CA97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetCenter.h"
#include "RelayClient.h"
#include <map>

class CRelayCenter : public CNetCenter  
{
public:
	CRelayCenter();
	virtual ~CRelayCenter();

protected:
	virtual void OnBuildup();
	virtual void OnClearup();

	virtual void OnServerEventCreate(CNetClient* pClient);
	virtual void OnServerEventClose(CNetClient* pClient);

private:
	typedef std::map<DWORD, CRelayClient*>	IP2CLIENTMAP;
	IP2CLIENTMAP m_mapIp2Client;

	CLockMRSW m_lockIpMap;

public:
	BOOL FindRelayClientByIP(DWORD IP, class CNetClientDup* pClntDup);

public:
	BOOL TraceInfo();
};

#endif // !defined(AFX_RELAYCENTER_H__7E3E4D53_5038_46C8_9B9F_84311729CA97__INCLUDED_)
