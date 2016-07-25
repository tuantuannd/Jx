// DealPackage.cpp: implementation of the CDealPackage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "S3Relay.h"
#include "DealPackage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDealPackage::CDealPackage()
{
	memset(m_Protocol2Proc, sizeof(m_Protocol2Proc), 0);

}

CDealPackage::~CDealPackage()
{

}


BOOL CDealPackage::Initialize()
{
	memset(m_Protocol2Proc, sizeof(m_Protocol2Proc), 0);

	m_Protocol2Proc[c2s_registerfamily] = Proc_c2sRegisterFamily;
	m_Protocol2Proc[c2s_entergame] = Proc_c2sEnterGame;
	m_Protocol2Proc[c2s_leavegame] = Proc_c2sLeaveGame;
	m_Protocol2Proc[c2s_relay] = Proc_c2sRelay;

	return TRUE;
}

BOOL CDealPackage::Uninitialize()
{
	return TRUE;
}



BOOL CDealPackage::DealPackage(const CNetConnect* pConn, const void* pData, unsigned len)
{
	assert(pConn != NULL && pData != NULL && len >= sizeof(BYTE));

	BYTE Protocol = *(BYTE*)pData;

	in_addr ia;
	ia.s_addr = pConn->GetIP();
	gTrace("%s -- recv size: %d", inet_ntoa(ia), len);

	if (!m_Protocol2Proc[Protocol])
		return FALSE;

	return (this->*m_Protocol2Proc[Protocol])(pConn, pData, len);
}




///////////////////////////
//deal proc ...

BOOL CDealPackage::Proc_c2sRegisterFamily(const CNetConnect* pConn, const void* pData, unsigned len)
{
	if (pData == NULL || len < sizeof(tagRegisterFamily))
		return FALSE;

	tagRegisterFamily* pRegFami = (tagRegisterFamily*)pData;
	assert(pRegFami->cProtocol == c2s_registerfamily);

	if (pRegFami->bRegister)
	{
		g_pConnTrack->RegConnect(pConn->GetID(), pRegFami->nFamily, pRegFami->RelayMethod);
	}
	else
	{
		g_pConnTrack->UnregConnect(pConn->GetID(), pRegFami->nFamily, pRegFami->RelayMethod);
		if (pRegFami->nFamily == 1)
			g_pPlayerTrack->ClearOnSvr(pConn->GetID());
	}

	return TRUE;
}

BOOL CDealPackage::Proc_c2sEnterGame(const CNetConnect* pConn, const void* pData, unsigned len)
{
	if (pData == NULL || len < sizeof(tagEnterGame2))
		return FALSE;

	tagEnterGame2* pEnterGame2 = (tagEnterGame2*)pData;
	assert(pEnterGame2->cProtocol == c2s_entergame);

	if (!g_pConnTrack->IsConnOnMethod(pConn->GetID(), 1))	//forbid
		return FALSE;

	g_pPlayerTrack->Login(pEnterGame2->szAccount, pEnterGame2->szRoleName, pEnterGame2->dwNameID, pConn->GetID(), pEnterGame2->nPlayerIdx);

	return TRUE;
}

BOOL CDealPackage::Proc_c2sLeaveGame(const CNetConnect* pConn, const void* pData, unsigned len)
{
	if (pData == NULL || len < sizeof(tagLeaveGame))
		return FALSE;

	tagLeaveGame* pLeaveGame = (tagLeaveGame*)pData;
	assert(pLeaveGame->cProtocol == c2s_leavegame);

	g_pPlayerTrack->Logout(pLeaveGame->szAccountName);

	return TRUE;
}

BOOL CDealPackage::Proc_c2sRelay(const CNetConnect* pConn, const void* pData, unsigned len)
{
	if (pData == NULL || len < sizeof(RELAY_COMMAND))
		return FALSE;

	RELAY_COMMAND* pRelayCmd = (RELAY_COMMAND*)pData;
	assert(pRelayCmd->ProtocolType == c2s_relay);

	RELAY_COMMAND* pRelayCmdDup = (RELAY_COMMAND*)g_pMemory->Alloc(len);
	assert(pRelayCmdDup != NULL);
	memcpy(pRelayCmdDup, pData, len);
	pRelayCmdDup->ProtocolType = s2c_relay;
	pRelayCmdDup->nIP = pConn->GetIP();
	pRelayCmdDup->port = pConn->GetPort();

	in_addr ia;
	ia.s_addr = pConn->GetIP();
	gTrace("%s -- recv size: %d", inet_ntoa(ia), len);

	if (g_pNetRelay->RelayPackage(pConn->GetID(), pRelayCmd->nIP, pRelayCmd->port, pRelayCmdDup, len))
		return TRUE;

	g_pMemory->Free(pRelayCmdDup);
	return FALSE;
}
