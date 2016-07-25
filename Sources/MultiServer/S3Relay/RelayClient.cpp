// RelayClient.cpp: implementation of the CRelayClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "RelayClient.h"
#include "RelayCenter.h"
#include "DealRelay.h"
#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRelayClient::CRelayClient(CRelayCenter* pRelayCenter, BOOL bAutoFree)
	: CNetClient(pRelayCenter, bAutoFree)
{

}

CRelayClient::~CRelayClient()
{

}


void CRelayClient::OnStartupFail()
{
	rTRACE("relay client startup fail: %s", _ip2a(GetSvrIP()));
}

void CRelayClient::OnShutdownFail()
{
	rTRACE("relay client shutdown fail: %s", _ip2a(GetSvrIP()));
}


void CRelayClient::OnServerEventCreate()
{
	rTRACE("relay client create: %s", _ip2a(GetSvrIP()));
}

void CRelayClient::OnServerEventClose()
{
	rTRACE("relay client close: %s", _ip2a(GetSvrIP()));
}


void CRelayClient::RecvPackage(const void* pData, size_t size)
{
	dealrelay::DoRecvPackage(this, pData, size);
}

