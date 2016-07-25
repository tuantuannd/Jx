#include "KCore.h"
#include "KNewProtocolProcess.h"
#include "KPlayer.h"
#include <malloc.h>
#include "IClient.h"
#include "KPlayerSet.h"

using OnlineGameLib::Win32::CBuffer;

KNewProtocolProcess g_NewProtocolProcess;

KNewProtocolProcess::KNewProtocolProcess()
	: m_theAllocator( 1024, 500 )
{
	m_ProtocolTable[MAKEWORD(pf_gamemaster, gm_c2s_execute)] = P_ProcessGMExecute;
	m_ProtocolTable[MAKEWORD(pf_gamemaster, gm_c2s_findplayer)] = P_ProcessGMFindPlayer;

	m_ProtocolTable[MAKEWORD(pf_playercommunity, playercomm_channelchat)] = P_ProcessPlayerCommExtend;
	m_ProtocolTable[MAKEWORD(pf_playercommunity, playercomm_someonechat)] = P_ProcessPlayerCommExtend;
	m_ProtocolTable[MAKEWORD(pf_playercommunity, playercomm_s2c_notifychannelid)] = P_ProcessPlayerCommExtend;

	m_ProtocolTable[MAKEWORD(pf_chat, chat_someonechat)] = P_ProcessChatExtend;
	m_ProtocolTable[MAKEWORD(pf_chat, chat_channelchat)] = P_ProcessChatExtend;
}

KNewProtocolProcess::~KNewProtocolProcess()
{
}

void KNewProtocolProcess::ProcessNetMsg(IClient* pTransfer,
										DWORD dwFromIP, DWORD dwFromRelayID,
										int nPlayerIndex, BYTE* pData, int nDataSize)
{
	FUN_PROCESS_NEW_PROTOCOL func = m_ProtocolTable[(*((WORD*)pData))];
	if (func)
		(*func)(pTransfer, dwFromIP, dwFromRelayID, nPlayerIndex, pData, nDataSize);
}

void KNewProtocolProcess::SendNetMsgToTransfer(IClient* pClient)
{
	if (pClient)
	{
		DATAList::iterator it;
		for (it = m_theTransList.begin(); it != m_theTransList.end(); ++it)
		{
			CBuffer *pBuffer = *it;

			if (pBuffer)
			{
				pClient->SendPackToServer( 
					( const void * )( pBuffer->GetBuffer() ), 
					pBuffer->GetUsed() );

				SAFE_RELEASE( pBuffer );
			}
		}
		m_theTransList.clear();
	}
}

void KNewProtocolProcess::SendNetMsgToChat(IClient* pClient)
{
	if (pClient)
	{
		DATAList::iterator it;
		for (it = m_theChatList.begin(); it != m_theChatList.end(); ++it)
		{
			CBuffer *pBuffer = *it;

			if (pBuffer)
			{
				pClient->SendPackToServer( 
					( const void * )( pBuffer->GetBuffer() ), 
					pBuffer->GetUsed() );

				SAFE_RELEASE( pBuffer );
			}
		}
		m_theChatList.clear();
	}
}

void KNewProtocolProcess::SendNetMsgToTong(IClient* pClient)
{
	if (pClient)
	{
		DATAList::iterator it;
		for (it = m_theTongList.begin(); it != m_theTongList.end(); ++it)
		{
			CBuffer *pBuffer = *it;

			if (pBuffer)
			{
				pClient->SendPackToServer( 
					( const void * )( pBuffer->GetBuffer() ), 
					pBuffer->GetUsed() );

				SAFE_RELEASE( pBuffer );
			}
		}
		m_theTongList.clear();
	}
}

void KNewProtocolProcess::PushMsgInTransfer(const void * const pData, const size_t datalength)
{
	if (pData && datalength > 0)
	{
		CBuffer *pBuffer = m_theAllocator.Allocate();

		ASSERT(pBuffer);

		pBuffer->AddData((BYTE*) pData, datalength);

		m_theTransList.push_back(pBuffer);
	}
}

void KNewProtocolProcess::PushMsgInChat(const void * const pData, const size_t datalength)
{
	if (pData && datalength > 0)
	{
		CBuffer *pBuffer = m_theAllocator.Allocate();

		ASSERT(pBuffer);

		pBuffer->AddData((BYTE*) pData, datalength);

		m_theChatList.push_back(pBuffer);
	}
}

void KNewProtocolProcess::PushMsgInTong(const void * const pData, const size_t datalength)
{
	if (pData && datalength > 0)
	{
		CBuffer *pBuffer = m_theAllocator.Allocate();

		ASSERT(pBuffer);

		pBuffer->AddData((BYTE*) pData, datalength);

		m_theTongList.push_back(pBuffer);
	}
}

void KNewProtocolProcess::BroadcastGlobal(const void * const pData, const size_t datalength)
{
	if (pData && datalength > 0)
	{
		CBuffer *pBuffer = m_theAllocator.Allocate();

		ASSERT(pBuffer);

		RELAY_DATA data;
		data.ProtocolFamily = pf_relay;
		data.ProtocolID = relay_c2c_data;
		data.nToIP = INADDR_BROADCAST;
		data.nToRelayID = 0;
		data.nFromIP = 0;
		data.nFromRelayID = 0;
		data.routeDateLength = 1 + datalength;

		pBuffer->AddData((BYTE*) &data, sizeof(RELAY_DATA));

		char cp = s2s_broadcast;
		pBuffer->AddData((BYTE*) &cp, sizeof(char));

		pBuffer->AddData((BYTE*) pData, datalength);

		m_theTransList.push_back(pBuffer);
	}
}

void KNewProtocolProcess::BroadcastLocalServer(const void * const pData, const size_t datalength)
{
	if (pData && datalength > 0)
	{
		int nTargetIdx;
		nTargetIdx = PlayerSet.GetFirstPlayer();
		while (nTargetIdx)
		{
			g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pData, datalength);
			nTargetIdx = PlayerSet.GetNextPlayer();
		}
	}
}

void KNewProtocolProcess::P_ProcessPlayerCommExtend(IClient* pTransfer,
					   DWORD dwFromIP, DWORD dwFromRelayID,
					   int nPlayerIndex, BYTE* pData, int nDataSize)
{
	size_t pckgsize = sizeof(tagExtendProtoHeader) + nDataSize;
#ifdef WIN32
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
	pExHeader->ProtocolType = s2c_extend;
	pExHeader->wLength = pckgsize - 1;
	memcpy(pExHeader + 1, pData, nDataSize);

	g_pServer->PackDataToClient(Player[nPlayerIndex].m_nNetConnectIdx, pExHeader, pckgsize);
#ifndef WIN32
	delete ((char*)pExHeader);
#endif
}

void KNewProtocolProcess::P_ProcessChatExtend(IClient* pTransfer,
					   DWORD dwFromIP, DWORD dwFromRelayID,
					   int nPlayerIndex, BYTE* pData, int nDataSize)
{
	void* pChatPckg = pData + 1;
	size_t chatsize = nDataSize - 1;
	size_t pckgsize = sizeof(tagExtendProtoHeader) + chatsize;
#ifdef WIN32
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
	pExHeader->ProtocolType = s2c_extendchat;
	pExHeader->wLength = pckgsize - 1;
	memcpy(pExHeader + 1, pChatPckg, chatsize);

	g_pServer->PackDataToClient(Player[nPlayerIndex].m_nNetConnectIdx, pExHeader, pckgsize);
#ifndef WIN32
	delete ((char*)pExHeader);
#endif
}
