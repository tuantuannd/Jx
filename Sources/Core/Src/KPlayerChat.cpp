//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerChat.cpp
// Date:	2002.10.05
// Code:	边城浪子
// Desc:	PlayerChat Class
//---------------------------------------------------------------------------

#ifdef _STANDALONE
#include <string>
#endif

#include	"KCore.h"
#ifdef _SERVER
//#include	"KNetServer.h"
//#include "../MultiServer/Heaven/Interface/iServer.h"
#include	"KGMCommand.h"
#else
//#include	"KNetClient.h"
#include "../../Headers/IClient.h"
#include	"CoreShell.h"
#endif
#include	"KNpc.h"
#include	"KNpcSet.h"
#include	"KSubWorld.h"
#include	"KPlayer.h"
#include	"KPlayerSet.h"
#include	"KPlayerTeam.h"
#include	"KPlayerChat.h"
#include	"MsgGenreDef.h"
#include	"CoreUseNameDef.h"

#include	"malloc.h"
#ifdef _SERVER
#include "KNewProtocolProcess.h"
#endif
#include "KTongProtocol.h"


#define		CHAT_TEAM_DEFAULT_NAME		"我的好友"

DWORD	g_dwChannelOr[CHAT_CUR_CHANNEL_NUM] =
{
	0x00000000,		// CHAT_CUR_CHANNEL_NONE
	0x00000001,		// CHAT_CUR_CHANNEL_ALL
	0x00000002,		// CHAT_CUR_CHANNEL_SCREEN
	0x00000004,		// CHAT_CUR_CHANNEL_SINGLE
	0x00000008,		// CHAT_CUR_CHANNEL_TEAM
	0x00000010,		// CHAT_CUR_CHANNEL_FACTION
	0x00000020,		// CHAT_CUR_CHANNEL_TONG
	0x00000040,		// CHAT_CUR_CHANNEL_SCREENSINGLE
	0x00000080,		// CHAT_CUR_CHANNEL_SYSTEM
};

DWORD	g_dwChannelAnd[CHAT_CUR_CHANNEL_NUM] =
{
	0xffffffff,		// CHAT_CUR_CHANNEL_NONE
	0xfffffffe,		// CHAT_CUR_CHANNEL_ALL
	0xfffffffd,		// CHAT_CUR_CHANNEL_SCREEN
	0xfffffffb,		// CHAT_CUR_CHANNEL_SINGLE
	0xfffffff7,		// CHAT_CUR_CHANNEL_TEAM
	0xffffffef,		// CHAT_CUR_CHANNEL_FACTION
	0xffffffdf,		// CHAT_CUR_CHANNEL_TONG
	0xffffffbf,		// CHAT_CUR_CHANNEL_SCREENSINGLE
	0xffffff7f,		// CHAT_CUR_CHANNEL_SYSTEM
};

//---------------------------------------------------------------------------
//	功能：清空
//---------------------------------------------------------------------------
void	KPlayerChat::Release()
{
#ifndef _SERVER
	m_btIniLoadFlag = FALSE;
	m_nCurChannel = CHAT_CUR_CHANNEL_SCREEN;
	CChatApplyListNode *pNode;
	pNode = (CChatApplyListNode*)m_cApplyAddList.GetHead();
	while (pNode)
	{
		m_cApplyAddList.RemoveHead();
		delete pNode;
		pNode = (CChatApplyListNode*)m_cApplyAddList.GetHead();
	}
	m_nLoginGetFirstOneFriendFlag = 0;
#else
	memset(m_nAddFriendList, 0, sizeof(this->m_nAddFriendList));
	m_nListPos = 0;
	m_nSyncState = 0;
	m_pStateSendPos = NULL;
	m_nTimer = CHAT_SPEED;
#endif
	m_dwTakeChannel = 0;
	this->SetTakeChannel(g_dwChannelOr[CHAT_CUR_CHANNEL_SCREEN]);
	m_dwTargetID = 0;
	m_nTargetIdx = 0;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		m_cFriendTeam[i].Release();
	}
	strcpy(m_cFriendTeam[0].m_szTeamName, CHAT_TEAM_DEFAULT_NAME);
}

BOOL	KPlayerChat::CheckExist(DWORD dwID)
{
	CChatFriend	*pNode;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (m_cFriendTeam[i].m_nFriendNo <= 0)
			continue;
		pNode = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pNode)
		{
			if (pNode->m_dwID == dwID)
				return TRUE;
			pNode = (CChatFriend*)pNode->GetNext();
		}
	}
	return FALSE;
}

BOOL	KPlayerChat::CheckExist(char *lpszName)
{
	if (!lpszName || !lpszName[0])
		return FALSE;
	CChatFriend	*pNode;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (m_cFriendTeam[i].m_nFriendNo <= 0)
			continue;
		pNode = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pNode)
		{
			if (strcmp(lpszName, pNode->m_szName) == 0)
				return TRUE;
			pNode = (CChatFriend*)pNode->GetNext();
		}
	}
	return FALSE;
}

CChatFriend*	KPlayerChat::GetFriendNode(DWORD dwID)
{
	CChatFriend	*pNode;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (m_cFriendTeam[i].m_nFriendNo <= 0)
			continue;
		pNode = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pNode)
		{
			if (pNode->m_dwID == dwID)
			{
				return pNode;
			}
			pNode = (CChatFriend*)pNode->GetNext();
		}
	}
	return NULL;
}

int		KPlayerChat::GetTeamNo(DWORD dwID)
{
	CChatFriend		*pNode;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (i != 0 && !m_cFriendTeam[i].m_szTeamName[0])
			continue;
		if (m_cFriendTeam[i].m_nFriendNo <= 0)
			continue;
		pNode = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pNode)
		{
			if (pNode->m_dwID == dwID)
			{
				return i;
			}
			pNode = (CChatFriend*)pNode->GetNext();
		}
	}
	return -1;
}

BOOL	KPlayerChat::GetName(DWORD dwID, char *lpszName)
{
	if ( !lpszName )
		return FALSE;
	CChatFriend	*pNode;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (m_cFriendTeam[i].m_nFriendNo <= 0)
			continue;
		pNode = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pNode)
		{
			if (pNode->m_dwID == dwID)
			{
				g_StrCpy(lpszName, pNode->m_szName);
				return TRUE;
			}
			pNode = (CChatFriend*)pNode->GetNext();
		}
	}
	return FALSE;
}

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：设定当前发送频道
//---------------------------------------------------------------------------
void	KPlayerChat::SetCurChannel(int nChannelNo, DWORD dwID, int nIdx)
{
	if (nChannelNo <= CHAT_CUR_CHANNEL_NONE || nChannelNo >= CHAT_CUR_CHANNEL_NUM)
		return;
	switch (nChannelNo)
	{
	case CHAT_CUR_CHANNEL_SINGLE:
		m_nCurChannel = nChannelNo;
		this->m_dwTargetID = dwID;
		this->m_nTargetIdx = nIdx;
		break;
	case CHAT_CUR_CHANNEL_SCREENSINGLE:
		m_nCurChannel = nChannelNo;
		this->m_dwTargetID = dwID;
		this->m_nTargetIdx = nIdx;
		break;
	case CHAT_CUR_CHANNEL_SYSTEM:
		// 检测玩家身份，确定是否可以设定为此频道 not end
		m_nCurChannel = nChannelNo;
		break;
	default:
		m_nCurChannel = nChannelNo;
		break;
	}
	CoreDataChanged(GDCNI_SEND_CHAT_CHANNEL, 0, 0);
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：增加订阅频道
//---------------------------------------------------------------------------
void	KPlayerChat::AddChannel(int nChannelNo)
{
	if (CHAT_CUR_CHANNEL_NONE >= nChannelNo || nChannelNo >= CHAT_CUR_CHANNEL_NUM)
		return;
	switch (nChannelNo)
	{
	case CHAT_CUR_CHANNEL_ALL:
		// 玩家身份判断，普通玩家默认订阅此频道，不能取消 not end
		m_dwTakeChannel |= g_dwChannelOr[nChannelNo];
		break;
	default:
		m_dwTakeChannel |= g_dwChannelOr[nChannelNo];
		break;
	}

	this->SaveTakeChannelInfo(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name);
	SendTakeChannel();
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：取消订阅频道
//---------------------------------------------------------------------------
void	KPlayerChat::SubChannel(int nChannelNo)
{
	if (CHAT_CUR_CHANNEL_NONE >= nChannelNo || nChannelNo >= CHAT_CUR_CHANNEL_NUM)
		return;

	switch (nChannelNo)
	{
	case CHAT_CUR_CHANNEL_ALL:
	case CHAT_CUR_CHANNEL_SYSTEM:
	case CHAT_CUR_CHANNEL_SCREEN:
		// 玩家身份判断，普通玩家默认订阅此频道，不能取消 not end
		break;
	default:
		m_dwTakeChannel &= g_dwChannelAnd[nChannelNo];
		break;
	}

	this->SaveTakeChannelInfo(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name);
	SendTakeChannel();
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：客户端发送订阅频道给服务器
//---------------------------------------------------------------------------
void	KPlayerChat::SendTakeChannel()
{
	CHAT_SET_CHANNEL_COMMAND	sChannel;

	sChannel.ProtocolType = c2s_chatsetchannel;
	sChannel.m_dwTakeChannel = this->m_dwTakeChannel;

	if (g_pClient)
		g_pClient->SendPackToServer(&sChannel, sizeof(CHAT_SET_CHANNEL_COMMAND));
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：客户端发送聊天语句给服务器
//---------------------------------------------------------------------------
void	KPlayerChat::SendSentence(KUiMsgParam *pMsg, char *lpszSentence)
{
	if (!lpszSentence || !pMsg || pMsg->nMsgLength <= 0)
		return;
	int nLength = pMsg->nMsgLength;
	if (nLength >= MAX_SENTENCE_LENGTH)
		nLength = MAX_SENTENCE_LENGTH - 1;

	if (pMsg->eGenre == MSG_G_CHAT)
	{
		PLAYER_SEND_CHAT_COMMAND	sChat;
		sChat.ProtocolType = c2s_playersendchat;
		sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_COMMAND) - 1 - sizeof(sChat.m_szSentence) + nLength + pMsg->cChatPrefixLen;
		sChat.m_btCurChannel = this->m_nCurChannel;
		sChat.m_btType = pMsg->eGenre;
		sChat.m_btChatPrefixLen = pMsg->cChatPrefixLen;
		sChat.m_wSentenceLen = nLength;
		sChat.m_dwTargetID = m_dwTargetID;
		sChat.m_nTargetIdx = m_nTargetIdx;
		memcpy(&sChat.m_szSentence[0], (char*)pMsg->cChatPrefix, pMsg->cChatPrefixLen);
		memcpy(&sChat.m_szSentence[pMsg->cChatPrefixLen], lpszSentence, nLength);
		
		if (g_pClient)
			g_pClient->SendPackToServer(&sChat, sChat.m_wLength + 1);

		if (m_nCurChannel == CHAT_CUR_CHANNEL_SINGLE)
		{
			char	szName[32];
			szName[0] = 0;
			if (GetName(m_dwTargetID, szName))
			{
				if (szName[0])
					m_cRecorder.SaveSentenceSingle(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, szName, Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, lpszSentence);
			}
		}
		else
		{
			this->m_cRecorder.SaveSentenceChannel(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, lpszSentence);
		}
	}
	else if (pMsg->eGenre == MSG_G_CMD)	// not end
	{
	}
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：客户端发送聊天语句给服务器(QQ专用)
//---------------------------------------------------------------------------
void	KPlayerChat::QQSendSentence(DWORD dwID, int nIdx, KUiMsgParam *pMsg, char *lpszSentence)
{
	if (!lpszSentence || !pMsg || pMsg->nMsgLength <= 0)
		return;
	int nLength = pMsg->nMsgLength;
	if (nLength >= MAX_SENTENCE_LENGTH)
		nLength = MAX_SENTENCE_LENGTH - 1;

	PLAYER_SEND_CHAT_COMMAND	sChat;
	sChat.ProtocolType = c2s_playersendchat;
	sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_COMMAND) - 1 - sizeof(sChat.m_szSentence) + nLength + pMsg->cChatPrefixLen;
	sChat.m_btCurChannel = CHAT_CUR_CHANNEL_SINGLE;
	sChat.m_btType = pMsg->eGenre;
	sChat.m_btChatPrefixLen = pMsg->cChatPrefixLen;
	sChat.m_wSentenceLen = nLength;
	sChat.m_dwTargetID = dwID;
	sChat.m_nTargetIdx = nIdx;
	memcpy(&sChat.m_szSentence[0], (char*)pMsg->cChatPrefix, pMsg->cChatPrefixLen);
	memcpy(&sChat.m_szSentence[pMsg->cChatPrefixLen], lpszSentence, nLength);

	if (g_pClient)
		g_pClient->SendPackToServer(&sChat, sChat.m_wLength + 1);

	char	szName[32];
	szName[0] = 0;
	if (GetName(dwID, szName))
	{
		if (szName[0])
			m_cRecorder.SaveSentenceSingle(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, szName, Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, lpszSentence);
	}
}
#endif

//---------------------------------------------------------------------------
//	功能：服务器收到客户端发来的订阅聊天频道信息
//---------------------------------------------------------------------------
void	KPlayerChat::SetTakeChannel(DWORD dwChannel)
{
	// 玩家身份判断，不同玩家系统赋予的权限不同 not end
	m_dwTakeChannel = dwChannel | g_dwChannelOr[CHAT_CUR_CHANNEL_ALL] | g_dwChannelOr[CHAT_CUR_CHANNEL_SCREEN] | g_dwChannelOr[CHAT_CUR_CHANNEL_SYSTEM];
}

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：服务器收到客户端发来的聊天信息
//---------------------------------------------------------------------------
void	KPlayerChat::ServerSendChat(int nPlayerIdx, BYTE* pProtocol)
{
	if (!pProtocol)
		return;

	PLAYER_SEND_CHAT_COMMAND	*pChat = (PLAYER_SEND_CHAT_COMMAND*)pProtocol;
	int		nLen = pChat->m_wSentenceLen;
	if (pChat->m_btCurChannel <= CHAT_CUR_CHANNEL_NONE || pChat->m_btCurChannel >= CHAT_CUR_CHANNEL_NUM)
		return;
	if (pChat->m_btChatPrefixLen > CHAT_MSG_PREFIX_MAX_LEN || pChat->m_wSentenceLen >= MAX_SENTENCE_LENGTH)
		return;
	if (pChat->m_wLength != sizeof(PLAYER_SEND_CHAT_COMMAND) - 1 - sizeof(pChat->m_szSentence) + pChat->m_btChatPrefixLen + pChat->m_wSentenceLen)
	{
		g_DebugLog("[chat]聊天数据包错误！");
		return;
	}

	if (pChat->m_btType == MSG_G_CHAT)
	{
		switch (pChat->m_btCurChannel)
		{
		case CHAT_CUR_CHANNEL_ALL:
			{
				// 检查玩家权限，普通玩家没有这个权限 not end
				break;

				if (m_nTimer < CHAT_SPEED)
					break;
				m_nTimer = 0;

				PLAYER_SEND_CHAT_SYNC	sChat;
				sChat.ProtocolType = s2c_playersendchat;
				sChat.m_btCurChannel = pChat->m_btCurChannel;
				sChat.m_btNameLen = strlen(Npc[Player[nPlayerIdx].m_nIndex].Name);
				sChat.m_btChatPrefixLen = pChat->m_btChatPrefixLen;
				sChat.m_wSentenceLen = pChat->m_wSentenceLen;
				sChat.m_dwSourceID = Player[nPlayerIdx].m_dwID;	// 发送者的player id 可能需要根据玩家身份改动
				sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
				memcpy(&sChat.m_szSentence[0], Npc[Player[nPlayerIdx].m_nIndex].Name, sChat.m_btNameLen); // 可能需要根据玩家身份改动
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen], &pChat->m_szSentence[0], sChat.m_btChatPrefixLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], &pChat->m_szSentence[pChat->m_btChatPrefixLen], sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
				int nTargetIdx;
				nTargetIdx = PlayerSet.GetFirstPlayer();
				while (nTargetIdx)
				{
					g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
					nTargetIdx = PlayerSet.GetNextPlayer();
				}
			}
			break;
		case CHAT_CUR_CHANNEL_SCREEN:
			{
				if (m_nTimer < CHAT_SPEED)
					break;
				m_nTimer = 0;

				PLAYER_SEND_CHAT_SYNC	sChat;
				sChat.ProtocolType = s2c_playersendchat;
				sChat.m_btCurChannel = pChat->m_btCurChannel;
				sChat.m_btNameLen = strlen(Npc[Player[nPlayerIdx].m_nIndex].Name);
				sChat.m_btChatPrefixLen = pChat->m_btChatPrefixLen;
				sChat.m_wSentenceLen = pChat->m_wSentenceLen;
				sChat.m_dwSourceID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;	// 此处传的是发送者的 npc id
				sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
				memcpy(&sChat.m_szSentence[0], Npc[Player[nPlayerIdx].m_nIndex].Name, sChat.m_btNameLen); // 可能需要根据玩家身份改动
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen], &pChat->m_szSentence[0], sChat.m_btChatPrefixLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], &pChat->m_szSentence[pChat->m_btChatPrefixLen], sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
#ifdef _CHAT_SCRIPT_OPEN
				if (TextGMFilter(nPlayerIdx,(const char *) (pChat->m_szSentence + pChat->m_btChatPrefixLen), sChat.m_wSentenceLen))
					return;
#endif
				Npc[Player[nPlayerIdx].m_nIndex].SendDataToNearRegion((BYTE*)&sChat, sChat.m_wLength + 1);
			}
			break;
		case CHAT_CUR_CHANNEL_SINGLE:
			{
				DWORD	dwCheckID = 0;
				int		nIndex = 0;
				if (pChat->m_nTargetIdx <= 0 || pChat->m_nTargetIdx >= MAX_PLAYER)
				{
					int nSubWorld = Npc[Player[nPlayerIdx].m_nIndex].m_SubWorldIndex;
					int nRegion = Npc[Player[nPlayerIdx].m_nIndex].m_RegionIndex;
					nIndex = SubWorld[nSubWorld].m_Region[nRegion].FindPlayer(pChat->m_dwTargetID);
					if (nIndex <= 0)
					{
						for (int i = 0; i < 8; i++)
						{
							int nConRegion = SubWorld[nSubWorld].m_Region[nRegion].m_nConnectRegion[i];
							if (nConRegion == -1)
								continue;
							nIndex = SubWorld[nSubWorld].m_Region[nConRegion].FindPlayer(pChat->m_dwTargetID);
							if (nIndex > 0)
								break;
						}
						if (nIndex <= 0)
							return;
					}
					dwCheckID = Player[nIndex].m_dwID;
				//	return;
				}
				else
				{
					dwCheckID = pChat->m_dwTargetID;
					nIndex = pChat->m_nTargetIdx;
				}
				if (Player[nIndex].m_nIndex == 0 || 
					Player[nIndex].m_dwID != dwCheckID)
					return;
				PLAYER_SEND_CHAT_SYNC	sChat;
				sChat.ProtocolType = s2c_playersendchat;
				sChat.m_btCurChannel = pChat->m_btCurChannel;
				sChat.m_btNameLen = 0;
				sChat.m_btChatPrefixLen = pChat->m_btChatPrefixLen;
				sChat.m_wSentenceLen = pChat->m_wSentenceLen;
				sChat.m_dwSourceID = Player[nPlayerIdx].m_dwID;
				sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
				// QQ聊天不传名字
//				memcpy(&sChat.m_szSentence[0], Npc[Player[nPlayerIdx].m_nIndex].Name, sChat.m_btNameLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen], &pChat->m_szSentence[0], sChat.m_btChatPrefixLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], &pChat->m_szSentence[pChat->m_btChatPrefixLen], sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
				g_pServer->PackDataToClient(Player[nIndex].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
			}
			break;
		case CHAT_CUR_CHANNEL_TEAM:
			{
				if (m_nTimer < CHAT_SPEED)
					break;
				m_nTimer = 0;

				if ( !Player[nPlayerIdx].m_cTeam.m_nFlag )		// 不在队伍中
					return;
				PLAYER_SEND_CHAT_SYNC	sChat;
				sChat.ProtocolType = s2c_playersendchat;
				sChat.m_btCurChannel = pChat->m_btCurChannel;
				sChat.m_btNameLen = strlen(Npc[Player[nPlayerIdx].m_nIndex].Name);
				sChat.m_btChatPrefixLen = pChat->m_btChatPrefixLen;
				sChat.m_wSentenceLen = pChat->m_wSentenceLen;
				sChat.m_dwSourceID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;	// 此处传的是发送者的 npc id
				sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
				memcpy(&sChat.m_szSentence[0], Npc[Player[nPlayerIdx].m_nIndex].Name, sChat.m_btNameLen); // 可能需要根据玩家身份改动
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen], &pChat->m_szSentence[0], sChat.m_btChatPrefixLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], &pChat->m_szSentence[pChat->m_btChatPrefixLen], sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
				int nTargetIdx;
				// 给队长发
				nTargetIdx = g_Team[Player[nPlayerIdx].m_cTeam.m_nID].m_nCaptain;
				if (nTargetIdx != nPlayerIdx && (Player[nTargetIdx].m_cChat.m_dwTakeChannel & g_dwChannelOr[CHAT_CUR_CHANNEL_TEAM]))
					g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
				// 给队员发
				for (int i = 0; i <	MAX_TEAM_MEMBER; i++)
				{
					nTargetIdx = g_Team[Player[nPlayerIdx].m_cTeam.m_nID].m_nMember[i];
					if (nTargetIdx < 0 || nTargetIdx == nPlayerIdx)
						continue;
					if (Player[nTargetIdx].m_cChat.m_dwTakeChannel & g_dwChannelOr[CHAT_CUR_CHANNEL_TEAM])
						g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
				}
			}
			break;
		case CHAT_CUR_CHANNEL_FACTION:
			{
				if (m_nTimer < CHAT_SPEED)
					break;
				m_nTimer = 0;

				int		nFaction = Player[nPlayerIdx].m_cFaction.m_nCurFaction;
				if ( nFaction < 0)	// 不在门派中
					return;
				PLAYER_SEND_CHAT_SYNC	sChat;
				sChat.ProtocolType = s2c_playersendchat;
				sChat.m_btCurChannel = pChat->m_btCurChannel;
				sChat.m_btNameLen = strlen(Npc[Player[nPlayerIdx].m_nIndex].Name);
				sChat.m_btChatPrefixLen = pChat->m_btChatPrefixLen;
				sChat.m_wSentenceLen = pChat->m_wSentenceLen;
				sChat.m_dwSourceID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;	// 此处传的是发送者的 npc id
				sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
				memcpy(&sChat.m_szSentence[0], Npc[Player[nPlayerIdx].m_nIndex].Name, sChat.m_btNameLen); // 可能需要根据玩家身份改动
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen], &pChat->m_szSentence[0], sChat.m_btChatPrefixLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], &pChat->m_szSentence[pChat->m_btChatPrefixLen], sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
				int nTargetIdx;
				nTargetIdx = PlayerSet.GetFirstPlayer();
				while (nTargetIdx)
				{
					if (Player[nTargetIdx].m_cFaction.m_nCurFaction == nFaction && (Player[nTargetIdx].m_cChat.m_dwTakeChannel & g_dwChannelOr[CHAT_CUR_CHANNEL_FACTION]))
						g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
					nTargetIdx = PlayerSet.GetNextPlayer();
				}
			}
			break;
		case CHAT_CUR_CHANNEL_TONG:
			{
				if (m_nTimer < CHAT_SPEED)
					break;
				m_nTimer = 0;

			}
			break;
		case CHAT_CUR_CHANNEL_SCREENSINGLE:
			{
				if (m_nTimer < CHAT_SPEED)
					break;
				m_nTimer = 0;

				int nTargetIdx;
				nTargetIdx = Player[nPlayerIdx].FindAroundPlayer(pChat->m_dwTargetID);
				if (nTargetIdx == -1)
					return;
				if ( !Player[nTargetIdx].m_cChat.CheckTalkChannel(CHAT_CUR_CHANNEL_SCREENSINGLE) )
				{
					CHAT_SCREENSINGLE_ERROR_SYNC	sError;
					sError.ProtocolType = s2c_chatscreensingleerror;
					sError.m_btError = 0;
					memset(sError.m_szName, 0, sizeof(sError.m_szName));
					g_StrCpy(sError.m_szName, Npc[Player[nTargetIdx].m_nIndex].Name);
					sError.m_wLength = 2 + 1 + strlen(sError.m_szName);
					g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sError, sError.m_wLength + 1);
					return;
				}
				PLAYER_SEND_CHAT_SYNC	sChat;
				sChat.ProtocolType = s2c_playersendchat;
				sChat.m_btCurChannel = pChat->m_btCurChannel;
				sChat.m_btNameLen = strlen(Npc[Player[nPlayerIdx].m_nIndex].Name);
				sChat.m_btChatPrefixLen = pChat->m_btChatPrefixLen;
				sChat.m_wSentenceLen = pChat->m_wSentenceLen;
				sChat.m_dwSourceID = Npc[Player[nPlayerIdx].m_nIndex].m_dwID;	// 此处传的是发送者的 npc id
				sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
				memcpy(&sChat.m_szSentence[0], Npc[Player[nPlayerIdx].m_nIndex].Name, sChat.m_btNameLen); // 可能需要根据玩家身份改动
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen], &pChat->m_szSentence[0], sChat.m_btChatPrefixLen);
				memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], &pChat->m_szSentence[pChat->m_btChatPrefixLen], sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
				g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
			}
			break;
		case CHAT_CUR_CHANNEL_SYSTEM:
			// 在此种情况下普通玩家不允许使用此频道 not end
			if (pChat->m_dwTargetID == 0)	// to all
			{
				char	szSentence[MAX_SENTENCE_LENGTH], szPrefix[CHAT_MSG_PREFIX_MAX_LEN];
				memcpy(szSentence, &pChat->m_szSentence[pChat->m_btChatPrefixLen], nLen); // 可能需要根据玩家身份改动
				szSentence[nLen] = 0;
				memcpy(szPrefix, &pChat->m_szSentence[0], pChat->m_btChatPrefixLen);
				KPlayerChat::SendSystemInfo(0, 0, MESSAGE_SYSTEM_ANNOUCE_HEAD, szSentence, nLen);
			}
			else
			{
				char	szSentence[MAX_SENTENCE_LENGTH], szPrefix[CHAT_MSG_PREFIX_MAX_LEN];
				int		nIdx;
				memcpy(szSentence, &pChat->m_szSentence[pChat->m_btChatPrefixLen], nLen); // 可能需要根据玩家身份改动
				szSentence[nLen] = 0;
				memcpy(szPrefix, &pChat->m_szSentence[0], pChat->m_btChatPrefixLen);
				nIdx = PlayerSet.FindSame(pChat->m_dwTargetID);
				if (nIdx > 0)
					KPlayerChat::SendSystemInfo(1, nIdx, MESSAGE_SYSTEM_ANNOUCE_HEAD, szSentence, nLen);
			}
			break;
		}
	}
	else if (pChat->m_btType == MSG_G_CMD)
	{
	}
	else
	{
		return;
	}
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：客户端收到聊天语句
//---------------------------------------------------------------------------
void	KPlayerChat::GetChat(PLAYER_SEND_CHAT_SYNC *pChat)
{
	if (!pChat)
		return;

	char	szBuf[MAX_SENTENCE_LENGTH];
	KUiMsgParam	Param;

	Param.cChatPrefixLen = pChat->m_btChatPrefixLen;
	Param.nMsgLength = pChat->m_wSentenceLen;
	memcpy(szBuf, &pChat->m_szSentence[pChat->m_btNameLen + pChat->m_btChatPrefixLen], pChat->m_wSentenceLen);
	szBuf[pChat->m_wSentenceLen] = 0;
	memset(Param.cChatPrefix, 0, sizeof(Param.cChatPrefix));
	memset(Param.szName, 0, sizeof(Param.szName));

	switch (pChat->m_btCurChannel)
	{
	case CHAT_CUR_CHANNEL_ALL:
		{
			if (pChat->m_dwSourceID == Player[CLIENT_PLAYER_INDEX].m_dwID)
				return;
			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;
			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	case CHAT_CUR_CHANNEL_SCREEN:
		{
			if (pChat->m_dwSourceID == Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwID)
				return;
			int nTalker = NpcSet.SearchID(pChat->m_dwSourceID);
			if (nTalker == 0)
				return;

			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;

			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	case CHAT_CUR_CHANNEL_SINGLE:
		{
			if (pChat->m_dwSourceID == Player[CLIENT_PLAYER_INDEX].m_dwID)
				return;
			CChatFriend	*pFriend = NULL;
			pFriend = this->GetFriendNode(pChat->m_dwSourceID);
			if ( pFriend == NULL )
			{
				// TODO: Temp change by wanli
/*				CHAT_REDELETE_FRIEND_COMMAND	sApply;
				sApply.ProtocolType = c2s_chatredeletefriend;
				sApply.m_dwID = pChat->m_dwSourceID;
				if (g_pClient)
					g_pClient->SendPackToServer(&sApply, sizeof(CHAT_REDELETE_FRIEND_COMMAND));
				break;*/
				int nIndex = NpcSet.SearchNameID(pChat->m_dwSourceID);
				if (nIndex == 0)
					break;
				KUiChatMessage	sMsg;
				sMsg.nContentLen = pChat->m_wSentenceLen;
				memcpy(sMsg.szContent, &pChat->m_szSentence[pChat->m_btNameLen + pChat->m_btChatPrefixLen], sMsg.nContentLen);
				sMsg.szContent[sMsg.nContentLen] = 0;
				if (pChat->m_btChatPrefixLen == 4)
				{
					((char*)(&sMsg.uColor))[3] = pChat->m_szSentence[pChat->m_btNameLen];
					((char*)(&sMsg.uColor))[2] = pChat->m_szSentence[pChat->m_btNameLen + 1];
					((char*)(&sMsg.uColor))[1] = pChat->m_szSentence[pChat->m_btNameLen + 2];
					((char*)(&sMsg.uColor))[0] = pChat->m_szSentence[pChat->m_btNameLen + 3];
				}
				else
				{
					sMsg.uColor = 0;
				}
				KUiPlayerItem	sPlayer;
				strcpy(sPlayer.Name, Npc[nIndex].Name);
				sPlayer.uId = pChat->m_dwSourceID;
				sPlayer.nIndex = 0;
				sPlayer.nData = 0;
				
				CoreDataChanged(GDCNI_CHAT_MESSAGE, (unsigned int)&sMsg, (int)&sPlayer);
				break;
			}

			KUiChatMessage	sMsg;
			sMsg.nContentLen = pChat->m_wSentenceLen;
			memcpy(sMsg.szContent, &pChat->m_szSentence[pChat->m_btNameLen + pChat->m_btChatPrefixLen], sMsg.nContentLen);
			sMsg.szContent[sMsg.nContentLen] = 0;
			if (pChat->m_btChatPrefixLen == 4)
			{
				((char*)(&sMsg.uColor))[3] = pChat->m_szSentence[pChat->m_btNameLen];
				((char*)(&sMsg.uColor))[2] = pChat->m_szSentence[pChat->m_btNameLen + 1];
				((char*)(&sMsg.uColor))[1] = pChat->m_szSentence[pChat->m_btNameLen + 2];
				((char*)(&sMsg.uColor))[0] = pChat->m_szSentence[pChat->m_btNameLen + 3];
			}
			else
			{
				sMsg.uColor = 0;
			}

			KUiPlayerItem	sPlayer;
			strcpy(sPlayer.Name, pFriend->m_szName);
			sPlayer.uId = pFriend->m_dwID;
			sPlayer.nIndex = pFriend->m_nPlayerIdx;
			sPlayer.nData = 0;

			CoreDataChanged(GDCNI_CHAT_MESSAGE, (unsigned int)&sMsg, (int)&sPlayer);
		}
		break;
	case CHAT_CUR_CHANNEL_TEAM:
		{
			if (pChat->m_dwSourceID == Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwID)
				return;
			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;
			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	case CHAT_CUR_CHANNEL_FACTION:
		{
			if (pChat->m_dwSourceID == Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwID)
				return;
			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;
			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	case CHAT_CUR_CHANNEL_TONG:
		{
			if (pChat->m_dwSourceID == Player[CLIENT_PLAYER_INDEX].m_dwID)
				return;
			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;
			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	case CHAT_CUR_CHANNEL_SCREENSINGLE:
		{
			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;
			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	case CHAT_CUR_CHANNEL_SYSTEM:
		{
			memcpy(Param.cChatPrefix, &pChat->m_szSentence[pChat->m_btNameLen], pChat->m_btChatPrefixLen);
			memcpy(Param.szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
			Param.szName[pChat->m_btNameLen] = 0;
			CoreDataChanged(GDCNI_MSG_ARRIVAL, (unsigned int)szBuf, (int)&Param);
		}
		break;
	}

	// 保存聊天纪录
	if (pChat->m_btCurChannel == CHAT_CUR_CHANNEL_SINGLE)
	{
		char	szName[32], szSentence[MAX_SENTENCE_LENGTH];
		memcpy(szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
		szName[pChat->m_btNameLen] = 0;
		memcpy(szSentence, &pChat->m_szSentence[pChat->m_btNameLen + pChat->m_btChatPrefixLen], pChat->m_wSentenceLen);
		szSentence[pChat->m_wSentenceLen] = 0;
		m_cRecorder.SaveSentenceSingle(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, szName, szName, szSentence);
	}
	else
	{
		char	szName[32], szSentence[MAX_SENTENCE_LENGTH];
		memcpy(szName, &pChat->m_szSentence[0], pChat->m_btNameLen);
		szName[pChat->m_btNameLen] = 0;
		memcpy(szSentence, &pChat->m_szSentence[pChat->m_btNameLen + pChat->m_btChatPrefixLen], pChat->m_wSentenceLen);
		szSentence[pChat->m_wSentenceLen] = 0;
		m_cRecorder.SaveSentenceChannel(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name, szName, szSentence);
	}
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：申请添加聊天好友
//---------------------------------------------------------------------------
void	KPlayerChat::ApplyAddFriend(DWORD dwNpcID, char *lpszSentence/* = NULL*/)
{
	if (dwNpcID <= 0)
		return;

	char	szName[32];
	szName[0] = 0;
	if (this->GetName(dwNpcID, szName))
	{
		// 通知界面此人已经是他的好友了
		KSystemMessage	sMsg;
		sprintf(sMsg.szMessage, MSG_CHAT_FRIEND_HAD_IN, szName);
		sMsg.eType = SMT_FRIEND;
		sMsg.byConfirmType = SMCT_CLICK;
		sMsg.byPriority = 1;
		sMsg.byParamSize = 0;
		CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		return;
	}

	CHAT_APPLY_ADD_FRIEND_COMMAND	sAdd;
	sAdd.ProtocolType = c2s_chatapplyaddfriend;
	sAdd.m_dwTargetNpcID = dwNpcID;
	memset(sAdd.m_szInfo, 0, sizeof(sAdd.m_szInfo));
	if (lpszSentence)
	{
		if (strlen(lpszSentence) < sizeof(sAdd.m_szInfo))
			strcpy(sAdd.m_szInfo, lpszSentence);
		else
			memcpy(sAdd.m_szInfo, lpszSentence, sizeof(sAdd.m_szInfo) - 1);
	}
	sAdd.m_wLength = sizeof(CHAT_APPLY_ADD_FRIEND_COMMAND) - 1 - sizeof(sAdd.m_szInfo) + strlen(sAdd.m_szInfo);
	if (g_pClient)
		g_pClient->SendPackToServer(&sAdd, sAdd.m_wLength + 1);
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：界面获取聊天好友添加申请人信息
//---------------------------------------------------------------------------
BOOL	KPlayerChat::GetApplyOneInfo(int *lpnPlayerIdx, char *lpszName, char *lpszInfo)
{
	if ( m_cApplyAddList.IsEmpty() )
		return FALSE;
	if (!lpnPlayerIdx || !lpszName || !lpszInfo)
		return FALSE;

	CChatApplyListNode	*pApply = NULL;
	BOOL	bFind = FALSE;
	pApply = (CChatApplyListNode*)m_cApplyAddList.GetHead();
	while (pApply)
	{
		if ( !pApply->m_nAccessFlag )
		{
			bFind = TRUE;
			break;
		}
		pApply = (CChatApplyListNode*)pApply->GetNext();
	}
	if ( !bFind )
		return FALSE;

	pApply->m_nAccessFlag = 1;
	*lpnPlayerIdx = pApply->m_nPlayerIdx;
	strcpy(lpszName, pApply->m_szName);
	strcpy(lpszInfo, pApply->m_szInfo);

	return TRUE;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：保存所有好友分组信息
//	char *lpszName : 人名
//---------------------------------------------------------------------------
void	KPlayerChat::SaveTeamInfo(char *lpszName)
{
	if (!lpszName || !lpszName[0])
		return;

	// 产生出相应路径
	char	szPath[MAX_PATH], szBuf[MAX_PATH];
	g_StrCpy(szPath, CHAT_PATH);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, lpszName);
	g_CreatePath(szPath);
//	g_SetFilePath(szPath);

	KIniFile	cTeam;
	char	szKey[32], szSect[64];
	int		i;

	// 保存组名
	for (i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		sprintf(szKey, "%d", i);
		cTeam.WriteString("TeamName", szKey, m_cFriendTeam[i].m_szTeamName);
	}
	// 保存每个组成员信息
	for (i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if ( !m_cFriendTeam[i].m_szTeamName[0] )
			continue;
		sprintf(szSect, "T_%s", m_cFriendTeam[i].m_szTeamName);
		cTeam.WriteInteger(szSect, "FriendNo", m_cFriendTeam[i].m_nFriendNo);

		int		j = 0;
		CChatFriend	*pOne;
		pOne = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pOne)
		{
			sprintf(szKey, "%d", j);
			cTeam.WriteString(szSect, szKey, pOne->m_szName);
			pOne = (CChatFriend*)pOne->GetNext();
			j++;
		}
	}
	g_UnitePathAndName(szPath, CHAT_TEAM_INFO_FILE_NAME, szBuf);
	cTeam.Save(szBuf);
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：保存频道订阅信息
//	char *lpszName : 人名
//---------------------------------------------------------------------------
void	KPlayerChat::SaveTakeChannelInfo(char *lpszName)
{
	if (!lpszName || !lpszName[0])
		return;

	// 产生出相应路径
	char	szPath[MAX_PATH], szBuf[MAX_PATH];
	g_StrCpy(szPath, CHAT_PATH);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, lpszName);
	g_CreatePath(szPath);
//	g_SetFilePath(szPath);

	KIniFile	cChannel;

	cChannel.WriteInteger("Channel", "Channel", this->m_dwTakeChannel);
	g_UnitePathAndName(szPath, CHAT_CHANNEL_INFO_FILE_NAME, szBuf);
	cChannel.Save(szBuf);
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：载入频道订阅信息
//	char *lpszName : 人名
//---------------------------------------------------------------------------
void	KPlayerChat::LoadTakeChannelInfo(char *lpszName)
{
	if (!lpszName || !lpszName[0])
		return;

	// 产生出相应路径
	char	szPath[MAX_PATH], szBuf[MAX_PATH];
	DWORD	dwTakeChannel;

	g_StrCpy(szPath, CHAT_PATH);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, lpszName);
	g_CreatePath(szPath);
//	g_SetFilePath(szPath);

	this->SetTakeChannel(g_dwChannelOr[CHAT_CUR_CHANNEL_SCREEN]);

	KIniFile	cChannel;

	g_UnitePathAndName(szPath, CHAT_CHANNEL_INFO_FILE_NAME, szBuf);
	if (!cChannel.Load(szBuf))
	{
		m_dwTakeChannel |= g_dwChannelOr[CHAT_CUR_CHANNEL_TEAM] | g_dwChannelOr[CHAT_CUR_CHANNEL_FACTION] | g_dwChannelOr[CHAT_CUR_CHANNEL_SCREENSINGLE];
		this->SaveTakeChannelInfo(lpszName);
		this->SendTakeChannel();
		return;
	}
	cChannel.GetInteger("Channel", "Channel", this->m_dwTakeChannel, (int*)&dwTakeChannel);

	this->SetTakeChannel(dwTakeChannel);
	this->SendTakeChannel();
}
#endif

#ifndef _SERVER
/*---------------------------------------------------------------------------
**	功能：判断某个ID的好友在哪个组里
**	dwID 被判断的 id
**  lpszName 本客户端玩家名字
**  lpszGetName 被判断的 id 的名字
**-------------------------------------------------------------------------*/
int		KPlayerChat::CheckTeamNo(char *lpszName, char *lpszGetName)
{
	int		i;
	if (!lpszName || !lpszName[0] || !lpszGetName || !lpszGetName[0])
		return -1;
	if (m_btIniLoadFlag == FALSE)
	{
		if (!lpszName || !lpszName[0])
			return -1;
		// 产生出相应路径
		char	szPath[MAX_PATH], szBuf[MAX_PATH];
		g_StrCpy(szPath, CHAT_PATH);
		g_StrCat(szPath, "\\");
		g_StrCat(szPath, lpszName);
		g_CreatePath(szPath);
//		g_SetFilePath(szPath);

		g_UnitePathAndName(szPath, CHAT_TEAM_INFO_FILE_NAME, szBuf);
		if ( !m_cTeamInfo.Load(szBuf) )
		{
			this->SaveTeamInfo(lpszName);
//			g_SetFilePath(szPath);
			m_cTeamInfo.Load(szBuf);
		}
		else
		{
			char	szKey[32], szTemp[32];
			strcpy(m_cFriendTeam[0].m_szTeamName, CHAT_TEAM_DEFAULT_NAME);
			for (i = 1; i < MAX_FRIEND_TEAM; i++)
			{
				sprintf(szKey, "%d", i);
				m_cTeamInfo.GetString("TeamName", szKey, "", szTemp, sizeof(szTemp));
				if ( szTemp[0] )
				{
					strcpy(this->m_cFriendTeam[i].m_szTeamName, szTemp);
				}
				else
				{
					this->m_cFriendTeam[i].m_szTeamName[0] = 0;
				}
			}
		}
		m_btIniLoadFlag = TRUE;
	}

	char	szKey[32], szTemp[32], szSect[64];
	int		nFriendNo, j;
	for (i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		sprintf(szKey, "%d", i);
		m_cTeamInfo.GetString("TeamName", szKey, "", szTemp, sizeof(szTemp));
		if ( !szTemp[0] )
			continue;
		sprintf(szSect, "T_%s", szTemp);
		m_cTeamInfo.GetInteger(szSect, "FriendNo", 0, &nFriendNo);
		for (j = 0; j < nFriendNo; j++)
		{
			sprintf(szKey, "%d", j);
			m_cTeamInfo.GetString(szSect, szKey, "", szTemp, sizeof(szTemp));
			if (strcmp(szTemp, lpszGetName) == 0)
				return i;
		}
	}

	return -1;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：载入组名(初始化的时候调用，其他时候不允许调用)
//---------------------------------------------------------------------------
void	KPlayerChat::LoadTeamName(char *lpszPlayerName)
{
	if (m_btIniLoadFlag)
		return;
	if (!lpszPlayerName || !lpszPlayerName[0])
		return;

	this->Release();

	// 产生出相应路径
	char		szPath[MAX_PATH], szBuf[MAX_PATH];
	KIniFile	cIni;

	g_StrCpy(szPath, CHAT_PATH);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, lpszPlayerName);
	g_CreatePath(szPath);
//	g_SetFilePath(szPath);

	g_UnitePathAndName(szPath, CHAT_TEAM_INFO_FILE_NAME, szBuf);
	if (!cIni.Load(szBuf))
	{
		this->SaveTeamInfo(lpszPlayerName);
		return;
	}

	char	szKey[32], szTemp[32];
	strcpy(m_cFriendTeam[0].m_szTeamName, CHAT_TEAM_DEFAULT_NAME);
	for (int i = 1; i < MAX_FRIEND_TEAM; i++)
	{
		sprintf(szKey, "%d", i);
		cIni.GetString("TeamName", szKey, "", szTemp, sizeof(szTemp));
		if ( !szTemp[0] )
			continue;
		strcpy(this->m_cFriendTeam[i].m_szTeamName, szTemp);
	}

}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：添加一个成员
//---------------------------------------------------------------------------
void	KPlayerChat::AddOne(DWORD dwID, int nPlayerIdx, char *lpszName, int nTeamNo)
{
	if (!lpszName || !lpszName[0])
		return;
	if (nTeamNo < 0 || nTeamNo >= MAX_FRIEND_TEAM)
		return;
	CChatFriend	*pFriend;
	pFriend = new CChatFriend;
	pFriend->m_dwID = dwID;
	pFriend->m_nPlayerIdx = nPlayerIdx;
	g_StrCpy(pFriend->m_szName, lpszName);
	m_cFriendTeam[nTeamNo].m_cEveryOne.AddTail(pFriend);
	m_cFriendTeam[nTeamNo].m_nFriendNo++;

	// 通知界面
//	CoreDataChanged(GDCNI_CHAT_GROUP, 0, 0);
	CoreDataChanged(GDCNI_CHAT_FRIEND, 0, nTeamNo);
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：判断某个playeridx是否好友
//---------------------------------------------------------------------------
BOOL	KPlayerChat::CheckIsFriend(int nPlayerIdx)
{
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (m_cFriendTeam[i].m_nFriendNo == 0)
			continue;
		CChatFriend	*pFriend;
		pFriend = (CChatFriend*)m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pFriend)
		{
			if (pFriend->m_nPlayerIdx == nPlayerIdx)
				return TRUE;
			pFriend = (CChatFriend*)pFriend->GetNext();
		}
	}
	return FALSE;
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：玩家登陆的时候从数据库获得数据时专用
//	参数：nSelfIdx	本玩家自己在Player中的位置	dwID  好友ID  lpszName  好友名字
//---------------------------------------------------------------------------
void	KPlayerChat::DataBaseAddOne(int nSelfIdx, DWORD dwID, char *lpszName)
{
	this->m_cFriendTeam[0].DataBaseAddOne(nSelfIdx, dwID, lpszName);
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：添加好友数据
//---------------------------------------------------------------------------
void	KPlayerChat::AddFriendData(int nSelfIdx, int nDestIdx)
{
	if (this->CheckExist(Player[nDestIdx].m_dwID))
		return;
	CChatFriend	*pFriend = NULL;
	pFriend = new CChatFriend;
	if ( !pFriend )
		return;
	pFriend->m_dwID = Player[nDestIdx].m_dwID;
	pFriend->m_nPlayerIdx = nDestIdx;
	g_StrCpy(pFriend->m_szName, Npc[Player[nDestIdx].m_nIndex].Name);
	m_cFriendTeam[0].m_cEveryOne.AddTail(pFriend);
	m_cFriendTeam[0].m_nFriendNo++;

	CHAT_ADD_FRIEND_SYNC	sAdd;
	sAdd.ProtocolType = s2c_chataddfriend;
	sAdd.m_dwID = Player[nDestIdx].m_dwID;
	sAdd.m_nIdx = nDestIdx;
	memset(sAdd.m_szName, 0, 32);
	g_StrCpy(sAdd.m_szName, Npc[Player[nDestIdx].m_nIndex].Name);

	g_pServer->PackDataToClient(Player[nSelfIdx].m_nNetConnectIdx, (BYTE*)&sAdd, sizeof(CHAT_ADD_FRIEND_SYNC));
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：重新发送某个好友数据
//  dwID 好友 id   nPlayerIdx 玩家自己
//---------------------------------------------------------------------------
void	KPlayerChat::ResendOneFriendData(DWORD dwID, int nPlayerIdx)
{
	BOOL	bFind = FALSE;
	CChatFriend	*pFriend;
	pFriend = (CChatFriend*)m_cFriendTeam[0].m_cEveryOne.GetHead();
	while (pFriend)
	{
		if (pFriend->m_dwID == dwID)
		{
			bFind = TRUE;
			break;
		}
		pFriend = (CChatFriend*)pFriend->GetNext();
	}
	if ( !bFind )
		return;
	CHAT_ONE_FRIEND_DATA_SYNC	sData;
	sData.ProtocolType = s2c_chatonefrienddata;
	sData.m_dwID = dwID;
	sData.m_nPlayerIdx = pFriend->m_nPlayerIdx;
	memset(sData.m_szName, 0, sizeof(sData.m_szName));
	g_StrCpy(sData.m_szName, pFriend->m_szName);

	g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sData, sizeof(CHAT_ONE_FRIEND_DATA_SYNC));
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：玩家登录时加载数据库玩家好友数据完成后自动依次发送好友数据给客户端
//---------------------------------------------------------------------------
BOOL KPlayerChat::SyncFriendData(int nPlayerIdx, unsigned int &nParam)
{
	BOOL bRet = FALSE;
	if ( !m_nSyncState )				// 平时不做任何处理 m_nSyncState == 0
		return TRUE;

	if ( m_nSyncState == 1 )			// 玩家刚登录，发送不带名字的聊天好友数据
	{
		if ( !m_pStateSendPos )
		{
			m_nSyncState = 0;
			nParam = 2;
			return TRUE;
		}

		CHAT_LOGIN_FRIEND_NONAME_SYNC	sFriend;
		sFriend.ProtocolType = s2c_chatloginfriendnoname;
		sFriend.m_dwID = m_pStateSendPos->m_dwID;
		sFriend.m_nPlayerIdx = m_pStateSendPos->m_nPlayerIdx;
		if (SUCCEEDED(g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sFriend, sizeof(CHAT_LOGIN_FRIEND_NONAME_SYNC))))
		{
			bRet = TRUE;
		}
		else
		{
			printf("player Packing friend sync data failed...\n");
			bRet = FALSE;
		}

		m_pStateSendPos = (CChatFriend*)m_pStateSendPos->GetNext();
		return bRet;
	}

	if ( m_nSyncState == 2 )			// 玩家刚登录，发送带名字的聊天好友数据(玩家客户端没有好友分组和名字信息)
	{
		if ( !m_pStateSendPos )
		{
			m_nSyncState = 0;
			nParam = 2;
			return TRUE;
		}

		CHAT_LOGIN_FRIEND_NAME_SYNC	sFriend;
		sFriend.ProtocolType = s2c_chatloginfriendname;
		sFriend.m_dwID = m_pStateSendPos->m_dwID;
		sFriend.m_nPlayerIdx = m_pStateSendPos->m_nPlayerIdx;
		g_StrCpy(sFriend.m_szName, m_pStateSendPos->m_szName);
		sFriend.m_wLength = sizeof(CHAT_LOGIN_FRIEND_NAME_SYNC) - 1 - sizeof(sFriend.m_szName) + strlen(sFriend.m_szName);
		if (SUCCEEDED(g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, (BYTE*)&sFriend, sFriend.m_wLength + 1)))
		{
			bRet = TRUE;
		}
		else
		{
			printf("player Packing friend 2 sync data failed...\n");
			bRet = FALSE;
		}

		m_pStateSendPos = (CChatFriend*)m_pStateSendPos->GetNext();
		return bRet;
	}
	return TRUE;
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：玩家登录时加载数据库玩家好友数据完成后调用次方法开始发送给客户端好友数据
//---------------------------------------------------------------------------
void	KPlayerChat::StartSendFriendData()
{
	this->SetTakeChannel(g_dwChannelOr[CHAT_CUR_CHANNEL_SCREEN]);
	m_pStateSendPos = (CChatFriend*)m_cFriendTeam[0].m_cEveryOne.GetHead();
	m_nSyncState = 2;
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：删除某个好友数据（会通知对方相应处理）
//---------------------------------------------------------------------------
void	KPlayerChat::DeleteFriend(DWORD dwID, DWORD dwSelfID)
{
	CChatFriend	*pFriend;
	pFriend = (CChatFriend*)m_cFriendTeam[0].m_cEveryOne.GetHead();
	while (pFriend)
	{
		if (pFriend->m_dwID == dwID)
		{
			pFriend->Remove();
			m_cFriendTeam[0].m_nFriendNo--;

			// 通知被删除方执行相应处理
			if (pFriend->m_nPlayerIdx)
			{	// 如果对方在线，id 正确
				if (pFriend->m_nPlayerIdx < MAX_PLAYER && Player[pFriend->m_nPlayerIdx].m_nIndex && Player[pFriend->m_nPlayerIdx].m_dwID == dwID)
				{
					Player[pFriend->m_nPlayerIdx].m_cChat.DeleteFriendData(dwSelfID, pFriend->m_nPlayerIdx);
				}
			}

			delete pFriend;
			return;
		}
		pFriend = (CChatFriend*)pFriend->GetNext();
	}
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：删除某个好友数据（会通知自己客户端）
//---------------------------------------------------------------------------
void	KPlayerChat::DeleteFriendData(DWORD dwID, int nSelfIndex)
{
	CChatFriend	*pFriend;
	pFriend = (CChatFriend*)this->m_cFriendTeam[0].m_cEveryOne.GetHead();
	while (pFriend)
	{
		if (pFriend->m_dwID == dwID)
		{
			pFriend->Remove();
			delete pFriend;
			m_cFriendTeam[0].m_nFriendNo--;
			// 通知客户端
			CHAT_DELETE_FRIEND_SYNC	sDelete;
			sDelete.ProtocolType = s2c_chatdeletefriend;
			sDelete.m_dwID = dwID;
			g_pServer->PackDataToClient(Player[nSelfIndex].m_nNetConnectIdx, (BYTE*)&sDelete, sizeof(CHAT_DELETE_FRIEND_SYNC));

			return;
		}
		pFriend = (CChatFriend*)pFriend->GetNext();
	}
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：再次删除某好友
//---------------------------------------------------------------------------
void	KPlayerChat::RedeleteFriend(DWORD dwID, int nSelfIndex)
{
	if( !CheckExist(dwID) )
	{
		int nIdx;
		nIdx = PlayerSet.FindSame(dwID);
		if (nIdx > 0)
			Player[nIdx].m_cChat.DeleteFriendData(Player[nSelfIndex].m_dwID, nIdx);
	}
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：本玩家下线
//---------------------------------------------------------------------------
void	KPlayerChat::OffLine(DWORD dwSelfID)
{
	CChatFriend	*pNode;
	pNode = (CChatFriend*)m_cFriendTeam[0].m_cEveryOne.GetHead();
	while (pNode)
	{
		if (pNode->m_nPlayerIdx)
		{
			Player[pNode->m_nPlayerIdx].m_cChat.GetMsgOffLine(dwSelfID, pNode->m_nPlayerIdx);
		}
		pNode = (CChatFriend*)pNode->GetNext();
	}
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：得到通知有好友下线
//---------------------------------------------------------------------------
void	KPlayerChat::GetMsgOffLine(DWORD dwID, int nSelfIdx)
{
	CChatFriend	*pNode;
	pNode = (CChatFriend*)m_cFriendTeam[0].m_cEveryOne.GetHead();
	while (pNode)
	{
		if (pNode->m_nPlayerIdx && pNode->m_dwID == dwID)
		{
			pNode->m_nPlayerIdx = 0;
			// 通知客户端
			CHAT_FRIEND_OFFLINE_SYNC	sSync;
			sSync.ProtocolType = s2c_chatfriendoffline;
			sSync.m_dwID = dwID;
			g_pServer->PackDataToClient(Player[nSelfIdx].m_nNetConnectIdx, (BYTE*)&sSync, sizeof(CHAT_FRIEND_OFFLINE_SYNC));
			return;
		}
		pNode = (CChatFriend*)pNode->GetNext();
	}
}
#endif

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：聊天时间计数加一
//---------------------------------------------------------------------------
void	KPlayerChat::TimerAdd()
{
	this->m_nTimer++;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：添加组，返回第几组，if return value == -1 失败
//---------------------------------------------------------------------------
int		KPlayerChat::AddTeam(char *lpszPlayerName, char *lpszTeamName)
{
	if (!lpszPlayerName || !lpszPlayerName[0])
		return -1;
	if (!lpszTeamName || !lpszTeamName[0])
		return -1;

	int		i;
	// 判断是否有同名组
	for (i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (strcmp(lpszTeamName, m_cFriendTeam[i].m_szTeamName) == 0)
		{
			KSystemMessage	sMsg;
			sprintf(sMsg.szMessage, MSG_CHAT_CREATE_TEAM_FAIL1);
			sMsg.eType = SMT_NORMAL;
			sMsg.byConfirmType = SMCT_NONE;
			sMsg.byPriority = 0;
			sMsg.byParamSize = 0;
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
			sprintf(sMsg.szMessage, MSG_CHAT_CREATE_TEAM_FAIL2);
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
			return -1;
		}
	}

	// 第 0 组是默认拥有的组
	for (i = 1; i < MAX_FRIEND_TEAM; i++)
	{
		if ( !m_cFriendTeam[i].m_szTeamName[0] && m_cFriendTeam[i].m_nFriendNo == 0)
		{
			g_StrCpy(m_cFriendTeam[i].m_szTeamName, lpszTeamName);
			SaveTeamInfo(lpszPlayerName);
			return i;
		}
	}

	KSystemMessage	sMsg;
	sprintf(sMsg.szMessage, MSG_CHAT_CREATE_TEAM_FAIL1);
	sMsg.eType = SMT_NORMAL;
	sMsg.byConfirmType = SMCT_NONE;
	sMsg.byPriority = 0;
	sMsg.byParamSize = 0;
	CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
	sprintf(sMsg.szMessage, MSG_CHAT_CREATE_TEAM_FAIL3);
	CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
	return -1;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：组改名
//---------------------------------------------------------------------------
BOOL	KPlayerChat::RenameTeam(int nTeamNo, char *lpszTeamName, char *lpszPlayerName)
{
	if (!lpszPlayerName || !lpszPlayerName[0])
		return FALSE;
	if (!lpszTeamName || !lpszTeamName[0])
		return FALSE;
	if (nTeamNo < 1 || nTeamNo >= MAX_FRIEND_TEAM)
		return FALSE;
	if (!m_cFriendTeam[nTeamNo].m_szTeamName[0])
		return FALSE;
	if (strlen(lpszTeamName) >= sizeof(m_cFriendTeam[nTeamNo].m_szTeamName))
		return FALSE;

	int		i;
	// 判断是否有同名组
	for (i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		if (i == nTeamNo)
			continue;
		if (strcmp(lpszTeamName, m_cFriendTeam[i].m_szTeamName) == 0)
		{
			KSystemMessage	sMsg;
			sprintf(sMsg.szMessage, MSG_CHAT_RENAME_TEAM_FAIL);
			sMsg.eType = SMT_NORMAL;
			sMsg.byConfirmType = SMCT_NONE;
			sMsg.byPriority = 0;
			sMsg.byParamSize = 0;
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
			sprintf(sMsg.szMessage, MSG_CHAT_CREATE_TEAM_FAIL2);
			CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
			return FALSE;
		}
	}

	g_StrCpy(m_cFriendTeam[nTeamNo].m_szTeamName, lpszTeamName);
	SaveTeamInfo(lpszPlayerName);

	return TRUE;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：删除组，返回成功与否
//---------------------------------------------------------------------------
BOOL	KPlayerChat::DeleteTeam(int nTeamNo, char *lpszPlayerName)
{
	if (!lpszPlayerName || !lpszPlayerName[0])
		return FALSE;
	if (nTeamNo == 0)
	{
		KSystemMessage	sMsg;
		sprintf(sMsg.szMessage, MSG_CHAT_DELETE_TEAM_FAIL1);
		sMsg.eType = SMT_NORMAL;
		sMsg.byConfirmType = SMCT_NONE;
		sMsg.byPriority = 0;
		sMsg.byParamSize = 0;
		CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		sprintf(sMsg.szMessage, MSG_CHAT_DELETE_TEAM_FAIL2);
		CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
		return FALSE;
	}
	if (nTeamNo < 1 || nTeamNo >= MAX_FRIEND_TEAM)
		return FALSE;

	if (!this->m_cFriendTeam[nTeamNo].m_szTeamName[0])
		return FALSE;

	CChatFriend	*pFriend;
	pFriend = (CChatFriend*)m_cFriendTeam[nTeamNo].m_cEveryOne.GetHead();
	while (pFriend)
	{
		m_cFriendTeam[nTeamNo].m_cEveryOne.RemoveHead();
		m_cFriendTeam[0].m_cEveryOne.AddTail(pFriend);
		m_cFriendTeam[0].m_nFriendNo++;
		pFriend = (CChatFriend*)m_cFriendTeam[nTeamNo].m_cEveryOne.GetHead();
	}
	m_cFriendTeam[nTeamNo].m_szTeamName[0] = 0;
	m_cFriendTeam[nTeamNo].m_nFriendNo = 0;
	SaveTeamInfo(lpszPlayerName);

	return TRUE;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：把一个好友从一个组移动到另一个组
//---------------------------------------------------------------------------
BOOL	KPlayerChat::OneFriendChangeTeam(DWORD dwID, int nSrcTeam, int nDestTeam)
{
	if (nSrcTeam < 0 || nSrcTeam >= MAX_FRIEND_TEAM)
		return FALSE;
	if (nDestTeam < 0 || nDestTeam >= MAX_FRIEND_TEAM)
		return FALSE;
	if ( !m_cFriendTeam[nDestTeam].m_szTeamName[0] )
		return FALSE;

	CChatFriend	*pFriend;
	pFriend = (CChatFriend*)m_cFriendTeam[nSrcTeam].m_cEveryOne.GetHead();
	while (pFriend)
	{
		if (pFriend->m_dwID == dwID)
		{
			pFriend->Remove();
			m_cFriendTeam[nSrcTeam].m_nFriendNo--;
			m_cFriendTeam[nDestTeam].m_cEveryOne.AddTail(pFriend);
			m_cFriendTeam[nDestTeam].m_nFriendNo++;

			this->SaveTeamInfo(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].Name);

			return TRUE;
		}
		pFriend = (CChatFriend*)pFriend->GetNext();
	}

	return FALSE;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：得到通知有好友下线
//---------------------------------------------------------------------------
void	KPlayerChat::GetMsgOffLine(DWORD dwID)
{
	CChatFriend	*pFriend;
	for (int i = 0; i < MAX_FRIEND_TEAM; i++)
	{
		pFriend = (CChatFriend*)this->m_cFriendTeam[i].m_cEveryOne.GetHead();
		while (pFriend)
		{
			if (pFriend->m_dwID == dwID)
			{
				pFriend->m_nPlayerIdx = 0;
				// 通知界面有好友下线
				KUiPlayerItem	sPlayer;
				strcpy(sPlayer.Name, pFriend->m_szName);
				sPlayer.uId = dwID;
				sPlayer.nIndex = 0;
				sPlayer.nData = CHAT_S_DISCONNECT;
				CoreDataChanged(GDCNI_CHAT_FRIEND_STATUS, (unsigned int)&sPlayer, i);

				KSystemMessage	sMsg;
				sprintf(sMsg.szMessage, MSG_CHAT_FRIEND_OFFLINE, pFriend->m_szName);
				sMsg.eType = SMT_NORMAL;
				sMsg.byConfirmType = SMCT_NONE;
				sMsg.byPriority = 0;
				sMsg.byParamSize = 0;
				CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);

				return;
			}
			pFriend = (CChatFriend*)pFriend->GetNext();
		}
	}
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：判断是否订阅了某个频道
//---------------------------------------------------------------------------
BOOL	KPlayerChat::CheckTakeChannel(int nChannelNo)
{
	if (nChannelNo < CHAT_CUR_CHANNEL_NONE || nChannelNo >= CHAT_CUR_CHANNEL_NUM)
		return FALSE;
	return (this->m_dwTakeChannel & g_dwChannelOr[nChannelNo]);
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：客户端申请删除某好友
//---------------------------------------------------------------------------
void	KPlayerChat::ApplyDeleteFriend(DWORD dwID, int nTeamNo, char *lpszSelfName)
{
	if (!lpszSelfName || !lpszSelfName[0])
		return;
	if (nTeamNo < 0 || nTeamNo >= MAX_FRIEND_TEAM)
		return;
	CChatFriend	*pFriend;

	pFriend = (CChatFriend*)m_cFriendTeam[nTeamNo].m_cEveryOne.GetHead();
	while (pFriend)
	{
		if (pFriend->m_dwID == dwID)
		{
			pFriend->Remove();
			delete pFriend;
			m_cFriendTeam[nTeamNo].m_nFriendNo--;
			SaveTeamInfo(lpszSelfName);

			CHAT_DELETE_FRIEND_COMMAND	sDelete;
			sDelete.ProtocolType = c2s_chatdeletefriend;
			sDelete.m_dwID = dwID;
			if (g_pClient)
				g_pClient->SendPackToServer(&sDelete, sizeof(CHAT_DELETE_FRIEND_COMMAND));

//			CoreDataChanged(GDCNI_CHAT_GROUP, 0, 0);
			CoreDataChanged(GDCNI_CHAT_FRIEND, 0, nTeamNo);

			return;
		}
		pFriend = (CChatFriend*)pFriend->GetNext();
	}

}
#endif

BOOL	KPlayerChat::CheckTalkChannel(int nChannel)
{
	if (nChannel <= CHAT_CUR_CHANNEL_NONE || nChannel >= CHAT_CUR_CHANNEL_NUM)
		return FALSE;
	return (this->m_dwTakeChannel & g_dwChannelOr[nChannel]);
}

//---------------------------------------------------------------------------
//	功能：发送系统消息
//  nType == 0 给全体玩家发送 nType == 1 给某个特定玩家发送
//  dwTargetID:  if (nType == 1) 目标玩家在 player 数组中的位置
//  lpszSendName:  发送者名字，可能是 系统消息、通知、警告、注意 等等 长度不超过 32
//  lpszSentence:  需要发送的语句
//  nSentenceLength:  需要发送语句的长度
//---------------------------------------------------------------------------
#ifdef _SERVER
void	KPlayerChat::SendSystemInfo(int nType, int nTargetIdx, char *lpszSendName, char *lpszSentence, int nSentenceLength)
{
	if (!lpszSendName || !lpszSentence)
		return;
	if (nSentenceLength >= MAX_SENTENCE_LENGTH)
		nSentenceLength = MAX_SENTENCE_LENGTH;


	BOOL bAll = nType == 0;


	size_t chatsize = sizeof(CHAT_CHANNELCHAT_SYNC) + nSentenceLength;
	size_t pckgsize = sizeof(tagExtendProtoHeader) + chatsize;

#ifdef WIN32
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
	pExHeader->ProtocolType = s2c_extendchat;
	pExHeader->wLength = pckgsize - 1;

	CHAT_CHANNELCHAT_SYNC* pCccSync= (CHAT_CHANNELCHAT_SYNC*)(pExHeader + 1);
	pCccSync->ProtocolType = chat_channelchat;
	pCccSync->wSize = chatsize - 1;
	pCccSync->packageID = -1;
	strncpy(pCccSync->someone, lpszSendName, _NAME_LEN - 1); // 可能需要根据玩家身份改动
	pCccSync->channelid = -1;
	pCccSync->sentlen = nSentenceLength;
	memcpy(pCccSync + 1, lpszSentence, nSentenceLength);

	if (bAll)	// 给全体玩家发送
	{
		int nTargetIdx;
		nTargetIdx = PlayerSet.GetFirstPlayer();
		while (nTargetIdx)
		{
			g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pExHeader, pckgsize);
			nTargetIdx = PlayerSet.GetNextPlayer();
		}
	}
	else			// 给某个特定玩家发送
	{
		if (nTargetIdx <= 0)
		{
#ifndef WIN32
			if (pExHeader)
				delete ((char*)pExHeader);
#endif
			return;
		}
		g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pExHeader, pckgsize);
	}

#ifndef WIN32
			if (pExHeader)
				delete ((char*)pExHeader);
#endif
	return;


/*
	if (!lpszSendName || !lpszSentence)
		return;
	if (nSentenceLength >= MAX_SENTENCE_LENGTH)
		nSentenceLength = MAX_SENTENCE_LENGTH;
	if (nType == 0)	// 给全体玩家发送
	{
		PLAYER_SEND_CHAT_SYNC	sChat;
		sChat.ProtocolType = s2c_playersendchat;
		sChat.m_btCurChannel = CHAT_CUR_CHANNEL_SYSTEM;
		sChat.m_dwSourceID = 0;							// 发送者的player id 可能需要根据玩家身份改动
		sChat.m_btNameLen = strlen(lpszSendName);
		if (sChat.m_btNameLen >= 32)
			sChat.m_btNameLen = 31;
		if (lpszPrefix && nPrefixLen > 0)
		{
			sChat.m_btChatPrefixLen = nPrefixLen;
			if (sChat.m_btChatPrefixLen >= CHAT_MSG_PREFIX_MAX_LEN)
				sChat.m_btChatPrefixLen = CHAT_MSG_PREFIX_MAX_LEN - 1;
		}
		else
		{
			sChat.m_btChatPrefixLen = 0;
		}
		sChat.m_wSentenceLen = nSentenceLength;
		sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
		memcpy(&sChat.m_szSentence[0], lpszSendName, sChat.m_btNameLen); // 可能需要根据玩家身份改动
		if (lpszPrefix)
			memcpy(&sChat.m_szSentence[sChat.m_btNameLen], lpszPrefix, sChat.m_btChatPrefixLen);
		memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], lpszSentence, sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
		int nTargetIdx;
		nTargetIdx = PlayerSet.GetFirstPlayer();
		while (nTargetIdx)
		{
			g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
			nTargetIdx = PlayerSet.GetNextPlayer();
		}
	}
	else			// 给某个特定玩家发送
	{
		int nIdx;
		nIdx = nTargetIdx;
//		nIdx = PlayerSet.FindSame(dwTargetID);
		if (nIdx <= 0)
			return;
		PLAYER_SEND_CHAT_SYNC	sChat;
		sChat.ProtocolType = s2c_playersendchat;
		sChat.m_btCurChannel = CHAT_CUR_CHANNEL_SYSTEM;
		sChat.m_dwSourceID = 0;							// 发送者的player id 可能需要根据玩家身份改动
		sChat.m_btNameLen = strlen(lpszSendName);
		if (sChat.m_btNameLen >= 32)
			sChat.m_btNameLen = 31;
		if (lpszPrefix && nPrefixLen > 0)
		{
			sChat.m_btChatPrefixLen = nPrefixLen;
			if (sChat.m_btChatPrefixLen >= CHAT_MSG_PREFIX_MAX_LEN)
				sChat.m_btChatPrefixLen = CHAT_MSG_PREFIX_MAX_LEN - 1;
		}
		else
		{
			sChat.m_btChatPrefixLen = 0;
		}
		sChat.m_wSentenceLen = nSentenceLength;
		sChat.m_wLength = sizeof(PLAYER_SEND_CHAT_SYNC) - 1 - sizeof(sChat.m_szSentence) + sChat.m_btNameLen + sChat.m_btChatPrefixLen + sChat.m_wSentenceLen;
		memcpy(&sChat.m_szSentence[0], lpszSendName, sChat.m_btNameLen); // 可能需要根据玩家身份改动
		if (lpszPrefix)
			memcpy(&sChat.m_szSentence[sChat.m_btNameLen], lpszPrefix, sChat.m_btChatPrefixLen);
		memcpy(&sChat.m_szSentence[sChat.m_btNameLen + sChat.m_btChatPrefixLen], lpszSentence, sChat.m_wSentenceLen); // 可能需要根据玩家身份改动
		g_pServer->PackDataToClient(Player[nIdx].m_nNetConnectIdx, (BYTE*)&sChat, sChat.m_wLength + 1);
	}
*/
}
#endif

#ifdef _SERVER
void KPlayerChat::SendGlobalSystemInfo(char *lpszSendName, char *lpszSentence, int nSentenceLength)
{
	if (!lpszSendName || !lpszSentence)
		return;
	if (nSentenceLength >= MAX_SENTENCE_LENGTH)
		nSentenceLength = MAX_SENTENCE_LENGTH;

	size_t chatsize = sizeof(CHAT_CHANNELCHAT_SYNC) + nSentenceLength;
	size_t pckgsize = sizeof(tagExtendProtoHeader) + chatsize;

#ifdef WIN32
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
	pExHeader->ProtocolType = s2c_extendchat;
	pExHeader->wLength = pckgsize - 1;

	CHAT_CHANNELCHAT_SYNC* pCccSync= (CHAT_CHANNELCHAT_SYNC*)(pExHeader + 1);
	pCccSync->ProtocolType = chat_channelchat;
	pCccSync->wSize = chatsize - 1;
	pCccSync->packageID = -1;
	strncpy(pCccSync->someone, lpszSendName, _NAME_LEN - 1); // 可能需要根据玩家身份改动
	pCccSync->channelid = -1;
	pCccSync->sentlen = nSentenceLength;
	memcpy(pCccSync + 1, lpszSentence, nSentenceLength);

	g_NewProtocolProcess.BroadcastGlobal(pExHeader, pckgsize);

#ifndef WIN32
	if (pExHeader)
		delete ((char*)pExHeader);
#endif
}
#endif

#ifdef _SERVER
void KPlayerChat::SendInfoToGM(char *lpszAccName, char *lpszRoleName, char *lpszSentence, int nSentenceLength)
{
	if (!lpszAccName ||
		!lpszRoleName ||
		!lpszSentence)
		return;
	if (nSentenceLength >= MAX_SENTENCE_LENGTH)
		nSentenceLength = MAX_SENTENCE_LENGTH;

	size_t chatsize = sizeof(CHAT_MSG_EX) + nSentenceLength;
	size_t pckgsize = sizeof(RELAY_ASKWAY_DATA) + chatsize;


#ifdef WIN32
	RELAY_ASKWAY_DATA* pExHeader = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);
#else
	RELAY_ASKWAY_DATA* pExHeader = (RELAY_ASKWAY_DATA*)(new char[pckgsize]);
#endif
	pExHeader->ProtocolFamily = pf_relay;
	pExHeader->ProtocolID = relay_c2c_askwaydata;
	pExHeader->nFromIP = 0;
	pExHeader->nFromRelayID = 0;
	pExHeader->seekRelayCount = 0;
	pExHeader->seekMethod = rm_gm;
	pExHeader->wMethodDataLength = 0;
	pExHeader->routeDateLength = chatsize;

	CHAT_MSG_EX* pChatMsgEx= (CHAT_MSG_EX*)(pExHeader + 1);
	pChatMsgEx->ProtocolFamily = pf_playercommunity;
	pChatMsgEx->ProtocolID = playercomm_channelchat;
	strcpy(pChatMsgEx->m_szSourceName, lpszRoleName);
	strcpy(pChatMsgEx->m_szAccountName, lpszAccName);
	pChatMsgEx->SentenceLength = nSentenceLength;
	memcpy(pChatMsgEx + 1, lpszSentence, nSentenceLength);

	g_NewProtocolProcess.PushMsgInTransfer(pExHeader, pckgsize);

#ifndef WIN32
	if (pExHeader)
		delete ((char*)pExHeader);
#endif
}
#endif

#ifdef _SERVER
void KPlayerChat::SendInfoToIP(DWORD nIP, DWORD nID, char *lpszAccName, char *lpszRoleName, char *lpszSentence, int nSentenceLength)
{
	if (!lpszAccName ||
		!lpszRoleName ||
		!lpszSentence)
		return;
	if (nSentenceLength >= MAX_SENTENCE_LENGTH)
		nSentenceLength = MAX_SENTENCE_LENGTH;

	size_t chatsize = sizeof(CHAT_MSG_EX) + nSentenceLength;
	size_t pckgsize = sizeof(RELAY_DATA) + chatsize;


#ifdef WIN32
	RELAY_DATA* pExHeader = (RELAY_DATA*)_alloca(pckgsize);
#else
	RELAY_DATA* pExHeader = (RELAY_DATA*)(new char[pckgsize]);
#endif
	pExHeader->ProtocolFamily = pf_relay;
	pExHeader->ProtocolID = relay_c2c_data;
	pExHeader->nToIP = nIP;
	pExHeader->nToRelayID = nID;
	pExHeader->nFromIP = 0;
	pExHeader->nFromRelayID = 0;
	pExHeader->routeDateLength = chatsize;

	CHAT_MSG_EX* pChatMsgEx= (CHAT_MSG_EX*)(pExHeader + 1);
	pChatMsgEx->ProtocolFamily = pf_playercommunity;
	pChatMsgEx->ProtocolID = playercomm_channelchat;
	strcpy(pChatMsgEx->m_szSourceName, lpszRoleName);
	strcpy(pChatMsgEx->m_szAccountName, lpszAccName);
	pChatMsgEx->SentenceLength = nSentenceLength;
	memcpy(pChatMsgEx + 1, lpszSentence, nSentenceLength);

	g_NewProtocolProcess.PushMsgInTransfer(pExHeader, pckgsize);

#ifndef WIN32
	if (pExHeader)
		delete ((char*)pExHeader);
#endif
}
#endif

#ifdef _SERVER
#define BROTHER_UNITNAME "亲人\n"
void KPlayerChat::MakeBrother(const STRINGLIST& brothers)
{
	if (brothers.size() == 0)
		return;

	static const size_t max_packagesize = 1000;
	char buffer[max_packagesize];	//max package size
	size_t maxsize = max_packagesize - 1;	//留个0的位置
	size_t basesize = sizeof(FRIEND_ASSOCIATEBEVY);

	FRIEND_ASSOCIATEBEVY* pGf = (FRIEND_ASSOCIATEBEVY*)(buffer);
	pGf->ProtocolFamily = pf_friend;
	pGf->ProtocolID = friend_c2s_associatebevy;

	int nG = strlen(BROTHER_UNITNAME) + 1;
	strcpy(buffer + basesize, BROTHER_UNITNAME);
	basesize += nG;

	size_t cursor = basesize;

	for (STRINGLIST::const_iterator itFriend = brothers.begin(); itFriend != brothers.end(); itFriend++)
	{
		std::string dst = *itFriend;
		size_t appendsize = dst.size() + 1;

		if (cursor + appendsize > maxsize)
		{
			buffer[cursor++] = specOver;	//加个结尾,发走

			g_NewProtocolProcess.PushMsgInTong(buffer, cursor);

			cursor = basesize;	//从头开始
		}

		strcpy(buffer + cursor, dst.c_str());
		cursor += appendsize;
	}

	if (cursor > basesize)
	{
		buffer[cursor++] = specOver;

		g_NewProtocolProcess.PushMsgInTong(buffer, cursor);
	}
}
#endif

#ifdef _SERVER
#define ENEMY_UNITNAME	 "仇人\n"
void KPlayerChat::MakeEnemy(char* szPlayer, char* szEnemy)
{
	if (!szPlayer || szPlayer[0] == 0 ||
		!szEnemy || szEnemy[0] == 0)
		return;

	int nG = strlen(ENEMY_UNITNAME) + 1;
	int nP = strlen(szPlayer) + 1;
	int nE = strlen(szEnemy) + 1;
	size_t fsize = sizeof(FRIEND_ASSOCIATE) + nG + nP + nE;

#ifdef WIN32
	FRIEND_ASSOCIATE* pCccSync = (FRIEND_ASSOCIATE*)_alloca(fsize);
#else
	FRIEND_ASSOCIATE* pCccSync = (FRIEND_ASSOCIATE*)(new char[fsize]);
#endif
	pCccSync->ProtocolFamily = pf_friend;
	pCccSync->ProtocolID = friend_c2s_associate;
	pCccSync->bidir = 0;
	char* pBuf = (char*)(pCccSync + 1);
	strcpy(pBuf, ENEMY_UNITNAME);
	pBuf += nG;
	strcpy(pBuf, szPlayer);
	pBuf += nP;
	strcpy(pBuf, szEnemy);
	pBuf += nE;

	g_NewProtocolProcess.PushMsgInTong(pCccSync, fsize);

#ifndef WIN32
	if (pCccSync)
		delete ((char*)pCccSync);
#endif
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：发送可选择聊天频道列表给界面
//---------------------------------------------------------------------------
int		KPlayerChat::SendSelectChannelList(KUiChatChannel *pList, int nCount)
{
	// 根据玩家不同的身份作不同的处理 not end

	int		i, nTempCount = 0;
	// 目前开放大部分功能
	if (!pList || nCount <= 0)
	{
		for (i = CHAT_CUR_CHANNEL_NONE + 1; i < CHAT_CUR_CHANNEL_NUM; i++)
		{
			switch (i)
			{
			case CHAT_CUR_CHANNEL_ALL:
				// 一般人不能使用这个频道
//				nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_SCREEN:
				// 这个频道始终开放
				nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_SINGLE:
				// QQ聊天使用，不用于选择
				break;
			case CHAT_CUR_CHANNEL_TEAM:
				// 组对才可以选
				if (Player[CLIENT_PLAYER_INDEX].m_cTeam.m_nFlag)
					nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_FACTION:
				// 一般人不能使用这个频道
				nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_TONG:
				// 帮派目前还没有
				break;
			case CHAT_CUR_CHANNEL_SCREENSINGLE:
				// 这个频道不是用来选择的，通过别的操作自动选定
				break;
			case CHAT_CUR_CHANNEL_SYSTEM:
				// 服务器使用，客户端不提供
				break;
			default:
				break;
			}
		}
		return nTempCount;
	}

	for (i = CHAT_CUR_CHANNEL_NONE + 1; i < CHAT_CUR_CHANNEL_NUM; i++)
	{
		if (nTempCount >= nCount)
			break;
		switch (i)
		{
		case CHAT_CUR_CHANNEL_ALL:
			// 一般人不能使用这个频道
			/*
			pList[nTempCount].nChannelNo = CHAT_CUR_CHANNEL_ALL;
			pList[nTempCount].uChannelId = 0;
			pList[nTempCount].nChannelIndex = 0;
			memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
			strcpy(pList[nTempCount].cTitle, CHAT_CHANNEL_NAME_ALL);
			nTempCount++;
			*/
			break;
		case CHAT_CUR_CHANNEL_SCREEN:
			// 这个频道始终开放
			pList[nTempCount].nChannelNo = CHAT_CUR_CHANNEL_SCREEN;
			pList[nTempCount].uChannelId = 0;
			pList[nTempCount].nChannelIndex = 0;
			memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
			strcpy(pList[nTempCount].cTitle, CHAT_CHANNEL_NAME_SCREEN);
			nTempCount++;
			break;
		case CHAT_CUR_CHANNEL_SINGLE:
			// QQ聊天使用，不用于选择
			break;
		case CHAT_CUR_CHANNEL_TEAM:
			// 组队才可以选
			if (Player[CLIENT_PLAYER_INDEX].m_cTeam.m_nFlag)
			{
				pList[nTempCount].nChannelNo = CHAT_CUR_CHANNEL_TEAM;
				pList[nTempCount].uChannelId = 0;
				pList[nTempCount].nChannelIndex = 0;
				memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
				strcpy(pList[nTempCount].cTitle, CHAT_CHANNEL_NAME_TEAM);
				nTempCount++;
			}
			break;
		case CHAT_CUR_CHANNEL_FACTION:
			// 一般人不能使用这个频道
			if (Player[CLIENT_PLAYER_INDEX].m_cFaction.GetCurFactionNo() >= 0)
			{
				pList[nTempCount].nChannelNo = CHAT_CUR_CHANNEL_FACTION;
				pList[nTempCount].uChannelId = 0;
				pList[nTempCount].nChannelIndex = 0;
				memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
				strcpy(pList[nTempCount].cTitle, CHAT_CHANNEL_NAME_FACTION);
				nTempCount++;
			}
			break;
		case CHAT_CUR_CHANNEL_TONG:
			// 帮派目前还没有
			break;
		case CHAT_CUR_CHANNEL_SCREENSINGLE:
			// 这个频道不是用来选择的，通过别的操作自动选定
			break;
		case CHAT_CUR_CHANNEL_SYSTEM:
			// 服务器使用，客户端不提供
			break;
		default:
			break;
		}
	}
	if (nTempCount < nCount)
		memset(&pList[nTempCount], 0, sizeof(KUiChatChannel) * (nCount - nTempCount));

	return nTempCount;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：发送可订阅聊天频道列表给界面
//---------------------------------------------------------------------------
int		KPlayerChat::SendTakeChannelList(KUiChatChannel *pList, int nCount)
{
	// 根据玩家身份的不同作不同的处理 not end

	int		i, nTempCount = 0;
	// 目前开放大部分功能
	if (!pList || nCount <= 0)
	{
		for (i = CHAT_CUR_CHANNEL_NONE + 1; i < CHAT_CUR_CHANNEL_NUM; i++)
		{
			switch (i)
			{
			case CHAT_CUR_CHANNEL_ALL:
				// 这个频道默认订阅，不能取消
				break;
			case CHAT_CUR_CHANNEL_SCREEN:
				// 这个频道默认订阅，不能取消
				break;
			case CHAT_CUR_CHANNEL_SINGLE:
				// QQ聊天使用，默认订阅，不能取消
				break;
			case CHAT_CUR_CHANNEL_TEAM:
				// 这个频道如果订阅了，只有在组队情况下才起作用
				nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_FACTION:
				// 这个频道如果订阅了，只有在门派中才起作用
				nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_TONG:
				// 帮派目前还没有
				break;
			case CHAT_CUR_CHANNEL_SCREENSINGLE:
				// 这个频道如果不订阅，别人就无法直接在屏幕内联系你
				nTempCount++;
				break;
			case CHAT_CUR_CHANNEL_SYSTEM:
				// 服务器使用，默认订阅，不能取消
				break;
			default:
				break;
			}
		}
		return nTempCount;
	}

	for (i = CHAT_CUR_CHANNEL_NONE + 1; i < CHAT_CUR_CHANNEL_NUM; i++)
	{
		if (nTempCount >= nCount)
			break;
		switch (i)
		{
		case CHAT_CUR_CHANNEL_ALL:
			// 这个频道默认订阅，不能取消
			break;
		case CHAT_CUR_CHANNEL_SCREEN:
			// 这个频道默认订阅，不能取消
			break;
		case CHAT_CUR_CHANNEL_SINGLE:
			// QQ聊天使用，默认订阅，不能取消
			break;
		case CHAT_CUR_CHANNEL_TEAM:
			pList[nTempCount].nChannelNo = i;
			pList[nTempCount].uChannelId = 0;
			pList[nTempCount].nChannelIndex = this->CheckTakeChannel(i);
			memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
			strcpy(pList[nTempCount].cTitle, CHAT_TAKE_CHANNEL_NAME_TEAM);
			nTempCount++;
			break;
		case CHAT_CUR_CHANNEL_FACTION:
			pList[nTempCount].nChannelNo = i;
			pList[nTempCount].uChannelId = 0;
			pList[nTempCount].nChannelIndex = this->CheckTakeChannel(i);
			memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
			strcpy(pList[nTempCount].cTitle, CHAT_TAKE_CHANNEL_NAME_FACTION);
			nTempCount++;
			break;
		case CHAT_CUR_CHANNEL_TONG:
			// 帮派目前还没有
			break;
		case CHAT_CUR_CHANNEL_SCREENSINGLE:
			pList[nTempCount].nChannelNo = i;
			pList[nTempCount].uChannelId = 0;
			pList[nTempCount].nChannelIndex = this->CheckTakeChannel(i);
			memset(pList[nTempCount].cTitle, 0, sizeof(pList[nTempCount].cTitle));
			strcpy(pList[nTempCount].cTitle, CHAT_TAKE_CHANNEL_NAME_SCREENSINGLE);
			nTempCount++;
			break;
		case CHAT_CUR_CHANNEL_SYSTEM:
			// 服务器使用，默认订阅，不能取消
			break;
		default:
			break;
		}
	}
	if (nTempCount < nCount)
		memset(&pList[nTempCount], 0, sizeof(KUiChatChannel) * (nCount - nTempCount));

	return nTempCount;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：获得当前使用聊天频道信息
//---------------------------------------------------------------------------
int		KPlayerChat::GetCurChannel(KUiChatChannel *pList)
{
	if (!pList)
		return FALSE;
	pList->nChannelNo = this->m_nCurChannel;
	pList->uChannelId = this->m_dwTargetID;
	pList->nChannelIndex = this->m_nTargetIdx;
	switch (m_nCurChannel)
	{
	case CHAT_CUR_CHANNEL_ALL:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_ALL);
		break;
	case CHAT_CUR_CHANNEL_SCREEN:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_SCREEN);
		break;
	case CHAT_CUR_CHANNEL_SINGLE:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_SINGLE);
		break;
	case CHAT_CUR_CHANNEL_TEAM:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_TEAM);
		break;
	case CHAT_CUR_CHANNEL_FACTION:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_FACTION);
		break;
	case CHAT_CUR_CHANNEL_TONG:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_TONG);
		break;
	case CHAT_CUR_CHANNEL_SCREENSINGLE:
		{
			int	nIdx;
			nIdx = NpcSet.SearchID(this->m_dwTargetID);
			if (nIdx > 0)
			{
				strcpy(pList->cTitle, Npc[nIdx].Name);
			}
			else
			{
				strcpy(pList->cTitle, CHAT_CHANNEL_NAME_SCREENSINGLE);
			}
		}
		break;
	case CHAT_CUR_CHANNEL_SYSTEM:
		strcpy(pList->cTitle, CHAT_CHANNEL_NAME_SYSTEM);
		break;
	}
	return TRUE;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：获得某个分组信息，返回下一个有效组编号(如果是最后一个有效组，返回-1)
//---------------------------------------------------------------------------
int		KPlayerChat::IGetTeamInfo(int nTeamNo, KUiChatGroupInfo *psTeamInfo)
{
	if (nTeamNo < 0 || nTeamNo >= MAX_FRIEND_TEAM || !psTeamInfo)
		return -1;

	for (int i = nTeamNo; i < MAX_FRIEND_TEAM; i++)
	{
		if (m_cFriendTeam[i].m_szTeamName[0])
		{
			strcpy(psTeamInfo->szTitle, this->m_cFriendTeam[i].m_szTeamName);
			psTeamInfo->nNumFriend = this->m_cFriendTeam[i].m_nFriendNo;
			return i;
		}
	}
	return -1;
}
#endif

#ifndef _SERVER
//---------------------------------------------------------------------------
//	功能：获得某个分组的所有成员的信息
//---------------------------------------------------------------------------
int		KPlayerChat::IGetTeamFriendInfo(int nTeamNo, KUiPlayerItem *pList)
{
	if (nTeamNo < 0 || nTeamNo >= MAX_FRIEND_TEAM || !pList)
		return 0;
	if (!m_cFriendTeam[nTeamNo].m_szTeamName[0] || m_cFriendTeam[nTeamNo].m_nFriendNo <= 0)
		return 0;

	int			nRet = 0;
	CChatFriend	*pNode = NULL;
	pNode = (CChatFriend*)m_cFriendTeam[nTeamNo].m_cEveryOne.GetHead();
	while (pNode)
	{
		strcpy(pList[nRet].Name, pNode->m_szName);
		if (pNode->m_nPlayerIdx)
			pList[nRet].nData = CHAT_S_ONLINE;
		else
			pList[nRet].nData = CHAT_S_DISCONNECT;
		pList[nRet].nIndex = pNode->m_nPlayerIdx;
		pList[nRet].uId = pNode->m_dwID;

		pNode = (CChatFriend*)pNode->GetNext();

		nRet++;
	}

	return nRet;
}
#endif





//---------------------------------------------------------------------------
//	功能：构造函数
//---------------------------------------------------------------------------
CChatFriendTeam::CChatFriendTeam()
{
	m_nFriendNo = 0;
	memset(m_szTeamName, 0, 32);
}

//---------------------------------------------------------------------------
//	功能：清空
//---------------------------------------------------------------------------
void	CChatFriendTeam::Release()
{
	m_nFriendNo = 0;
	memset(m_szTeamName, 0, 32);
	CChatFriend *pFriend = NULL;
	pFriend = (CChatFriend*)m_cEveryOne.GetHead();
	while (pFriend)
	{
		m_cEveryOne.RemoveHead();
		delete pFriend;
		pFriend = (CChatFriend*)m_cEveryOne.GetHead();
	}
}

#ifdef _SERVER
//---------------------------------------------------------------------------
//	功能：玩家登陆的时候从数据库获得数据时专用
//---------------------------------------------------------------------------
void	CChatFriendTeam::DataBaseAddOne(int nSelfIdx, DWORD dwID, char *lpszName)
{
	if (!lpszName || !lpszName[0])
		return;

	// 判断好友是否在线
	CChatFriend	*pFriend = NULL;
	pFriend = new CChatFriend;
	if ( !pFriend )
		return;
	pFriend->m_dwID = dwID;
	pFriend->m_nPlayerIdx = PlayerSet.FindSame(dwID);
	g_StrCpy(pFriend->m_szName, lpszName);
	m_cEveryOne.AddTail(pFriend);
	m_nFriendNo++;

	// 通知好友自己上线了
	if (pFriend->m_nPlayerIdx > 0)
		Player[pFriend->m_nPlayerIdx].ChatFriendOnLine(Player[nSelfIdx].m_dwID, nSelfIdx);
}
#endif

#ifndef _SERVER
void	KCHAT_RECORDER::SetTime()
{
	SYSTEMTIME	sTime;
	GetLocalTime(&sTime);
	sprintf(m_szDay, "%04d.%02d.%02d", sTime.wYear, sTime.wMonth, sTime.wDay);
	sprintf(m_szTime, "%02d:%02d:%02d", sTime.wHour, sTime.wMinute, sTime.wSecond);
}

void	KCHAT_RECORDER::SaveSentenceSingle(char *lpszSelfName, char *lpszTarget, char *lpszTalkerName, char *lpszSentence)
{
	// 暂时关闭
	return;

	if (!lpszSelfName || !lpszTarget || !lpszTalkerName || !lpszSentence)
		return;
	if (!lpszSelfName[0] || !lpszTarget[0] || !lpszTalkerName[0] || !lpszSentence[0])
		return;

	// 产生出相应路径
	char	szPath[MAX_PATH], szBuf[MAX_PATH], szName[MAX_PATH];
	g_StrCpy(szPath, CHAT_PATH);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, lpszSelfName);
	g_CreatePath(szPath);
//	g_SetFilePath(szPath);

	SetTime();

	KFile	cFile;
	sprintf(szName, "%s.rcd", lpszTarget);
	g_UnitePathAndName(szPath, szName, szBuf);
	if ( !cFile.Append(szBuf) )
		return;
	cFile.Write(m_szDay, sizeof(m_szDay) - 1);
	cFile.Write("  ", 2);
	cFile.Write(this->m_szTime, sizeof(m_szTime) - 1);
	cFile.Write("  ", 2);
	cFile.Write(lpszTalkerName, strlen(lpszTalkerName));
	cFile.Write("\r\n", 1);
	if (strlen(lpszSentence) < MAX_SENTENCE_LENGTH)
		cFile.Write(lpszSentence, strlen(lpszSentence));
	else
		cFile.Write(lpszSentence, MAX_SENTENCE_LENGTH - 1);
	cFile.Write("\r\n", 1);
	cFile.Close();
}

void	KCHAT_RECORDER::SaveSentenceChannel(char *lpszSelfName, char *lpszTalkerName, char *lpszSentence)
{
	// 暂时关闭
	return;

	if (!lpszSelfName || !lpszTalkerName || !lpszSentence)
		return;
	if (!lpszSelfName[0] || !lpszTalkerName[0] || !lpszSentence[0])
		return;

	SetTime();
	m_szDay[4] = '_';
	m_szDay[7] = '_';

	// 产生出相应路径
	char	szPath[MAX_PATH], szName[MAX_PATH], szBuf[MAX_PATH];
	g_StrCpy(szPath, CHAT_PATH);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, lpszSelfName);
	g_StrCat(szPath, "\\");
	g_StrCat(szPath, m_szDay);
	g_CreatePath(szPath);
//	g_SetFilePath(szPath);

	sprintf(szName, "%s.rcd", m_szDay);
	g_UnitePathAndName(szPath, szName, szBuf);
	KFile	cFile;
	if ( !cFile.Append(szBuf) )
		return;
	cFile.Write(m_szTime, sizeof(m_szTime) - 1);
	cFile.Write("  ", 2);
	cFile.Write(lpszTalkerName, strlen(lpszTalkerName));
	cFile.Write("\r\n", 1);
	if (strlen(lpszSentence) < MAX_SENTENCE_LENGTH)
		cFile.Write(lpszSentence, strlen(lpszSentence));
	else
		cFile.Write(lpszSentence, MAX_SENTENCE_LENGTH - 1);
	cFile.Write("\r\n", 1);
	cFile.Close();
}
#endif



