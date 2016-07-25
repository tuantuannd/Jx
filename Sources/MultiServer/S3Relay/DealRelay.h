// DealRelay.h: interface for the CDealRelay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEALRELAY_H__BA05D4D5_0D2A_4D88_97A6_6C59CB6C266E__INCLUDED_)
#define AFX_DEALRELAY_H__BA05D4D5_0D2A_4D88_97A6_6C59CB6C266E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RelayConnect.h"
#include "RelayClient.h"
#include "NetSockDupEx.h"

///////////////////////
namespace dealrelay {
///////////////////////

void DoRecvPackage(CRelayConnect* pConn, const void* pData, size_t size);
void DoRecvPackage(CRelayClient* pClnt, const void* pData, size_t size);

CNetSockDupEx FindRelaySockByIP(DWORD ip);
BOOL BroadPackage(const void* pData, size_t size);

///////////////////////
} //namespace dealrelay
///////////////////////

#endif // !defined(AFX_DEALRELAY_H__BA05D4D5_0D2A_4D88_97A6_6C59CB6C266E__INCLUDED_)
