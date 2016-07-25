//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KPlayerTrade.cpp
// Date:	2003.02.17
// Code:	边城浪子
// Desc:	Trade Class
//---------------------------------------------------------------------------

#include	"KCore.h"
#include	"KNpc.h"
#include	"KPlayer.h"
#include	"KPlayerSet.h"
#include	"KPlayerTrade.h"
//#include	"MyAssert.h"
#ifndef _SERVER
#include	"CoreShell.h"
#endif
KTrade::KTrade()
{
	Release();
#ifdef _SERVER
	m_nApplyIdx = -1;
#endif
}

void	KTrade::Release()
{
	m_nIsTrading			= 0;
	m_nTradeDest			= -1;
	m_nTradeState			= 0;
	m_nTradeLock			= 0;
#ifndef _SERVER
	m_nBackEquipMoney		= 0;
	m_nBackRepositoryMoney	= 0;
	m_nTradeDestState		= 0;
	m_nTradeDestLock		= 0;
	m_szDestName[0]			= 0;
#endif
}

BOOL	KTrade::StartTrade(int nDest, int nEquipMoney, int nRepositoryMoney)
{
	if (m_nIsTrading)
		return FALSE;
	if (nDest < 0)
		return FALSE;

	m_nIsTrading = 1;
	m_nTradeDest = nDest;
	m_nTradeState = 0;
	m_nTradeLock = 0;
#ifndef _SERVER
	m_nBackEquipMoney = nEquipMoney;
	m_nBackRepositoryMoney = nRepositoryMoney;
	m_nTradeDestState = 0;
	m_nTradeDestLock = 0;
	int nIdx = NpcSet.SearchID(nDest);
	if (nIdx > 0)
		strcpy(m_szDestName, Npc[nIdx].Name);
	else
		m_szDestName[0] = 0;
#endif
	return TRUE;
}

void	KTrade::Lock(BOOL bFlag)
{
	if (bFlag)
	{
		m_nTradeLock = 1;
	}
	else
	{
		m_nTradeLock = 0;
#ifndef _SERVER
		m_nTradeDestLock = 0;
#endif
	}
}

#ifndef _SERVER
void	KTrade::SetTradeState(int nSelfLock, int nDestLock, int nSelfStateOk, int nDestStateOk)
{
	if (nSelfLock)
	{
		if (m_nTradeLock == 0)
		{
			KSystemMessage	sMsg;
			sprintf(sMsg.szMessage, MSG_TRADE_SELF_LOCK);
			sMsg.eType = SMT_NORMAL;
			sMsg.byConfirmType = SMCT_NONE;
			sMsg.byPriority = 0;
			sMsg.byParamSize = 0;
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		}
	}
	else
	{
		if (m_nTradeLock != 0)
		{
			KSystemMessage	sMsg;
			sprintf(sMsg.szMessage, MSG_TRADE_SELF_UNLOCK);
			sMsg.eType = SMT_NORMAL;
			sMsg.byConfirmType = SMCT_NONE;
			sMsg.byPriority = 0;
			sMsg.byParamSize = 0;
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		}
	}

	if (nDestLock)
	{
		if (m_nTradeDestLock == 0)
		{
			KSystemMessage	sMsg;
			sprintf(sMsg.szMessage, MSG_TRADE_DEST_LOCK, m_szDestName);
			sMsg.eType = SMT_NORMAL;
			sMsg.byConfirmType = SMCT_NONE;
			sMsg.byPriority = 0;
			sMsg.byParamSize = 0;
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		}
	}
	else
	{
		if (m_nTradeDestLock != 0)
		{
			KSystemMessage	sMsg;
			sprintf(sMsg.szMessage, MSG_TRADE_DEST_UNLOCK, m_szDestName);
			sMsg.eType = SMT_NORMAL;
			sMsg.byConfirmType = SMCT_NONE;
			sMsg.byPriority = 0;
			sMsg.byParamSize = 0;
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		}
	}

	this->m_nTradeLock = (nSelfLock != 0);
	this->m_nTradeDestLock = (nDestLock != 0);
	this->m_nTradeState = (nSelfStateOk != 0);
	this->m_nTradeDestState = (nDestStateOk != 0);

	// 通知界面按钮状态改变
	CoreDataChanged(GDCNI_TRADE_OPER_DATA, 0, 0);
}
#endif

#ifndef _SERVER
void	KTrade::ReplyInvite(int nDestIdx, int nReply)
{
	if (nDestIdx < 0)
		return;
	TRADE_REPLY_START_COMMAND	sReply;
	sReply.ProtocolType = c2s_tradereplystart;
	sReply.m_bDecision = (nReply != 0);
	sReply.m_nDestIdx = nDestIdx;
	if (g_pClient)
		g_pClient->SendPackToServer(&sReply, sizeof(TRADE_REPLY_START_COMMAND));
}
#endif
