// RelayConnect.h: interface for the CRelayConnect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RELAYCONNECT_H__7E3D39EC_C852_4E37_A8DD_01CA268295E5__INCLUDED_)
#define AFX_RELAYCONNECT_H__7E3D39EC_C852_4E37_A8DD_01CA268295E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetConnect.h"

class CRelayConnect : public CNetConnect  
{
public:
	CRelayConnect(class CRelayServer* pRelayServer, unsigned long id);
	virtual ~CRelayConnect();

protected:
	virtual void RecvPackage(const void* pData, size_t size);

protected:
	virtual void OnClientConnectCreate();
	virtual void OnClientConnectClose();

};

#endif // !defined(AFX_RELAYCONNECT_H__7E3D39EC_C852_4E37_A8DD_01CA268295E5__INCLUDED_)
