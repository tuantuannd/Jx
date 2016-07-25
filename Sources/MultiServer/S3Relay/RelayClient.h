// RelayClient.h: interface for the CRelayClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RELAYCLIENT_H__63FC1085_B9C1_49FA_AAC5_C0552E371E27__INCLUDED_)
#define AFX_RELAYCLIENT_H__63FC1085_B9C1_49FA_AAC5_C0552E371E27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetClient.h"

class CRelayClient : public CNetClient  
{
public:
	CRelayClient(class CRelayCenter* pRelayCenter, BOOL bAutoFree);
	virtual ~CRelayClient();

protected:
	virtual void RecvPackage(const void* pData, size_t size);
	virtual void DeleteThis() {delete this;}

protected:
	virtual void OnStartupFail();
	virtual void OnShutdownFail();

	virtual void OnServerEventCreate();
	virtual void OnServerEventClose();
};


#endif // !defined(AFX_RELAYCLIENT_H__63FC1085_B9C1_49FA_AAC5_C0552E371E27__INCLUDED_)
