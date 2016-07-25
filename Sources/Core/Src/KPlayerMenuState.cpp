//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerMenuState.cpp
// Date:	2002.12.10
// Code:	边城浪子
// Desc:	PlayerMenuState Class
//---------------------------------------------------------------------------

#include	"KCore.h"
#include	"MyAssert.H"
#include	"KPlayer.h"
#ifdef _SERVER
//#include	"KNetServer.h"
//#include "../MultiServer/Heaven/Interface/iServer.h"
#endif
#include	"KPlayerMenuState.h"
#include	"CoreUseNameDef.h"

KPlayerMenuState::KPlayerMenuState()
{
	Release();
}

#ifdef _SERVER
void	KPlayerMenuState::Release()
{
	m_nState = PLAYER_MENU_STATE_NORMAL;
	m_nTradeDest = -1;
	m_nTradeState = 0;
	memset(m_szSentence, 0, MAX_SENTENCE_LENGTH);
	m_nBackState = PLAYER_MENU_STATE_NORMAL;
	memset(m_szBackSentence, 0, MAX_SENTENCE_LENGTH);
}
#endif

#ifndef _SERVER
void	KPlayerMenuState::Release()
{
	m_nState = PLAYER_MENU_STATE_NORMAL;
	m_nTradeDest = -1;
	m_nTradeState = 0;
	m_nTradeDestState = 0;
	memset(m_szSentence, 0, MAX_SENTENCE_LENGTH);
}
#endif

#ifdef _SERVER
void	KPlayerMenuState::BackupState()
{
	m_nBackState = m_nState;
	memcpy(m_szBackSentence, m_szSentence, sizeof(m_szSentence));
};
#endif

#ifdef _SERVER
void	KPlayerMenuState::RestoreState()
{
	m_nState = m_nBackState;
	memcpy(m_szSentence, m_szBackSentence, sizeof(m_szSentence));
};
#endif

#ifdef _SERVER
void	KPlayerMenuState::SetState(int nPlayerIdx, int nState, char *lpszSentence/*=NULL*/, int nSentenceLength/*=0*/, DWORD dwDestNpcID/*=0*/)
{
	if (nState < PLAYER_MENU_STATE_NORMAL || nState >= PLAYER_MENU_STATE_NUM)
		return;
	if (m_nState != nState)
	{
		m_nBackState = m_nState;
		m_nState = nState;
		memcpy(m_szBackSentence, m_szSentence, sizeof(m_szSentence));
	}

	if (lpszSentence == NULL || nSentenceLength <= 0)
	{
		m_szSentence[0] = 0;
	}
	else
	{
		if (nSentenceLength >= MAX_SENTENCE_LENGTH)
			nSentenceLength = MAX_SENTENCE_LENGTH - 1;
		memcpy(m_szSentence, lpszSentence, nSentenceLength);
		m_szSentence[nSentenceLength] = 0;
	}

	switch (nState)
	{
	case PLAYER_MENU_STATE_NORMAL:
		{
			if (m_nBackState == PLAYER_MENU_STATE_TEAMOPEN)
			{
				if (Player[nPlayerIdx].m_cTeam.m_nFlag && g_Team[Player[nPlayerIdx].m_cTeam.m_nID].IsOpen())
					g_Team[Player[nPlayerIdx].m_cTeam.m_nID].SetTeamClose();
			}
			TRADE_CHANGE_STATE_SYNC	sTrade;
			sTrade.ProtocolType = s2c_tradechangestate;
			sTrade.m_btState = 0;
			g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sTrade, sizeof(TRADE_CHANGE_STATE_SYNC));
			
			NPC_SET_MENU_STATE_SYNC	sSync;
			sSync.ProtocolType = s2c_npcsetmenustate;
			sSync.m_dwID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;
			sSync.m_btState = PLAYER_MENU_STATE_NORMAL;
			sSync.m_wLength = sizeof(NPC_SET_MENU_STATE_SYNC) - 1 - sizeof(sSync.m_szSentence);
			Npc[Player[nPlayerIdx].m_nIndex].SendDataToNearRegion((LPVOID)&sSync, sSync.m_wLength + 1);
		}
		break;
	case PLAYER_MENU_STATE_TRADEOPEN:
		{
			TRADE_CHANGE_STATE_SYNC	sTrade;
			sTrade.ProtocolType = s2c_tradechangestate;
			sTrade.m_btState = 1;
			g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sTrade, sizeof(TRADE_CHANGE_STATE_SYNC));

			NPC_SET_MENU_STATE_SYNC	sSync;
			sSync.ProtocolType = s2c_npcsetmenustate;
			sSync.m_dwID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;
			sSync.m_btState = PLAYER_MENU_STATE_TRADEOPEN;
			memset(sSync.m_szSentence, 0, sizeof(sSync.m_szSentence));
			memcpy(sSync.m_szSentence, lpszSentence, nSentenceLength);
			sSync.m_wLength = sizeof(NPC_SET_MENU_STATE_SYNC) - 1 - sizeof(sSync.m_szSentence) + nSentenceLength;
			Npc[Player[nPlayerIdx].m_nIndex].SendDataToNearRegion((LPVOID)&sSync, sSync.m_wLength + 1);
		}
		break;
	case PLAYER_MENU_STATE_TRADING:
		{
			// 给自己发消息
			TRADE_CHANGE_STATE_SYNC	sState;
			sState.ProtocolType = s2c_tradechangestate;
			sState.m_btState = 2;
			sState.m_dwNpcID = dwDestNpcID;
			g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sState, sizeof(TRADE_CHANGE_STATE_SYNC));

			// 给附近玩家发消息
			NPC_SET_MENU_STATE_SYNC	sSync;
			sSync.ProtocolType = s2c_npcsetmenustate;
			sSync.m_btState = PLAYER_MENU_STATE_TRADING;
			sSync.m_wLength = sizeof(NPC_SET_MENU_STATE_SYNC) - 1 - sizeof(sSync.m_szSentence);
			sSync.m_dwID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;
			Npc[Player[nPlayerIdx].m_nIndex].SendDataToNearRegion((LPVOID)&sSync, sSync.m_wLength + 1);
		}
		break;
	}
}
#endif

#ifdef _SERVER
void	KPlayerMenuState::RestoreBackupState(int nPlayerIdx)
{
	_ASSERT(m_nBackState != PLAYER_MENU_STATE_TRADING);
	if (this->m_nState != PLAYER_MENU_STATE_TRADING)
		return;
	switch (this->m_nBackState)
	{
	case PLAYER_MENU_STATE_NORMAL:
	case PLAYER_MENU_STATE_TEAMOPEN:
		this->m_nBackState = PLAYER_MENU_STATE_NORMAL;
		this->m_nState = PLAYER_MENU_STATE_NORMAL;
		this->m_szBackSentence[0] = 0;
		this->m_szSentence[0] = 0;
		this->SetState(nPlayerIdx, PLAYER_MENU_STATE_NORMAL);
		break;
	case PLAYER_MENU_STATE_TRADEOPEN:
		{
			char	szBuffer[MAX_SENTENCE_LENGTH];
			memcpy(szBuffer, m_szBackSentence, sizeof(szBuffer));
			this->m_nState = PLAYER_MENU_STATE_NORMAL;
			this->m_szSentence[0] = 0;
			this->SetState(nPlayerIdx, PLAYER_MENU_STATE_TRADEOPEN, szBuffer, strlen(szBuffer));
		}
		break;
	}
}
#endif

#ifndef _SERVER
void	KPlayerMenuState::SetState(int nState)
{
	if (nState < PLAYER_MENU_STATE_NORMAL || nState >= PLAYER_MENU_STATE_NUM)
		return;
	m_nState = nState;
}
#endif


BOOL	KPlayerMenuStateGraph::Init()
{
	KTabFile	cFile;
//	g_SetFilePath("\\");
	if( !cFile.Load(PLAYER_MENU_STATE_RES_FILE) )
		return FALSE;

	m_szName[0][0] = 0;
	for (int i = 1; i < PLAYER_MENU_STATE_NUM; i++)
	{
		cFile.GetString(i + 1, 2, "", m_szName[i], sizeof(m_szName[i]));
	}

	return TRUE;
}

void	KPlayerMenuStateGraph::GetStateSpr(int nState, char *lpszGetName)
{
	if ( !lpszGetName )
		return;
	if (nState < 0 || nState >= PLAYER_MENU_STATE_NUM)
	{
		lpszGetName[0] = 0;
		return;
	}
	g_StrCpy(lpszGetName, m_szName[nState]);
}
