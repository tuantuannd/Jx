#include "KEngine.h"
#include "KCore.h"
#ifndef _STANDALONE
#include "../../Headers/IClient.h"
#else
#include "IClient.h"
#endif
#include "KObjSet.h"
#include "KNpcSet.h"
#include "KPlayer.h"
#include "KPlayerSet.h"
#include "KPlayerTeam.h"
#include "KPlayerFaction.h"
#include "KNpc.h"
#include "KSubWorld.h"
#include "LuaFuns.h"
#include "KNewProtocolProcess.h"
#include "KSkills.h"
#include "KItemSet.h"
#include "KBuySell.h"
#include "KSubWorldSet.h"
//#include "MyAssert.h"
#include "Scene/ObstacleDef.h"
#include "KMath.h"
#ifndef _STANDALONE
#include "CoreShell.h"
#include "..\..\..\Headers\KGmProtocol.h"
#else
#include "KGmProtocol.h"
#endif
BOOL TextMsgProcessGMCmd(int nPlayerIdx, const char * pGMCmd, int nLen);
void KNewProtocolProcess::P_ProcessGMExecute(IClient* pTransfer,
											 DWORD dwFromIP, DWORD dwFromRelayID,
											 int nPlayerIndex, BYTE* pData, int nDataSize)
{
	_ASSERT(pf_gamemaster == *pData && gm_c2s_execute == *(pData + 1));
	GM_EXECUTE_COMMAND* pGMData = (GM_EXECUTE_COMMAND*)pData;
	TextMsgProcessGMCmd(nPlayerIndex, ((const char*)pData) + sizeof(GM_EXECUTE_COMMAND), pGMData->wLength);
}

void KNewProtocolProcess::P_ProcessGMFindPlayer(IClient* pTransfer,
												DWORD dwFromIP, DWORD dwFromRelayID,
												int nPlayerIndex, BYTE* pData, int nDataSize)
{
	char szBuffer[sizeof(RELAY_ASKWAY_DATA) + sizeof(GM_GET_PLAYER_AT_GW_SYNC) + 64];
	_ASSERT(pf_gamemaster == *pData && gm_c2s_findplayer == *(pData + 1));

	if (nPlayerIndex <= 0 && nPlayerIndex >= MAX_PLAYER)
		return;

	int i = Player[nPlayerIndex].m_nNetConnectIdx;

	if (i >= 0)
	{
		if (Player[nPlayerIndex].m_dwID && !Player[nPlayerIndex].m_bExchangeServer)
		{
			RELAY_ASKWAY_DATA* pAskwayData = (RELAY_ASKWAY_DATA*)szBuffer;
			pAskwayData->ProtocolFamily = pf_relay;
			pAskwayData->ProtocolID = relay_c2c_askwaydata;
			pAskwayData->nFromIP = 0;
			pAskwayData->nFromRelayID = 0;
			pAskwayData->seekRelayCount = 0;
			pAskwayData->seekMethod = rm_gm;
			pAskwayData->wMethodDataLength = 0;
			pAskwayData->routeDateLength = sizeof(GM_GET_PLAYER_AT_GW_SYNC);
			GM_GET_PLAYER_AT_GW_SYNC* pGetPlayerData = (GM_GET_PLAYER_AT_GW_SYNC*)(pAskwayData + 1);
			strncpy(pGetPlayerData->AccountName, ((GM_HEADER*)pData)->AccountName, sizeof(pGetPlayerData->AccountName));
			strncpy(pGetPlayerData->szCurrentRoleName, Player[nPlayerIndex].m_PlayerName, sizeof(Player[nPlayerIndex].m_PlayerName));
			Npc[Player[nPlayerIndex].m_nIndex].GetMpsPos(&(pGetPlayerData->nX), &(pGetPlayerData->nY));
			pGetPlayerData->nX /= 32;
			pGetPlayerData->nY /= 32;
			pGetPlayerData->nWorldID = SubWorld[Npc[Player[nPlayerIndex].m_nIndex].m_SubWorldIndex].m_SubWorldID;
			pGetPlayerData->ProtocolFamily = pf_gamemaster;
			pGetPlayerData->ProtocolType = gm_s2c_findplayer;
			pTransfer->SendPackToServer(szBuffer, sizeof(RELAY_ASKWAY_DATA) + sizeof(GM_GET_PLAYER_AT_GW_SYNC));
		}
	}
}

void KNewProtocolProcess::ExecuteLocalServer(const void * const pData, const size_t datalength)
{
	if (pData && datalength > 0)
	{
		if (pf_gamemaster == *(char*)pData && gm_c2s_execute == *((char*)pData + 1))
		{
			GM_EXECUTE_COMMAND* pGMData = (GM_EXECUTE_COMMAND*)pData;
			TextMsgProcessGMCmd(-1, ((const char*)pData) + sizeof(GM_EXECUTE_COMMAND), pGMData->wLength);
		}
	}
}
