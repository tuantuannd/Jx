#include "stdafx.h"
#include "KRoleDBServer.h"
#include "KRoleInfomation.h"
#include "Windows.h"
#include "KDBProcessThread.h"
#include <crtdbg.h>
#include "KException.h"
#pragma warning(disable:4786)
#include <objbase.h>
#include <initguid.h>
extern void * EnumSendMsg(unsigned long nId, size_t& nDataLen);
//using namespace RoleDBServer
RoleDBServer::KNetServer g_NetServer;
size_t g_nMaxSendBuffer = 350;
extern TRoleNetMsg  * pTestCmd;
extern void SendToClient(unsigned long nId, TRoleNetMsg * pMsg, size_t nMsgLen);
extern IServer * g_pNetServer;


using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CBuffer;

namespace RoleDBServer
{
	
	KClientUser::KClientUser()
	{
		m_nProcessLoadCount = m_nProcessWriteCount = 0;
		this->m_szClientName[0] = 0;
		this->m_nCurSendNo = 0;
		m_ulSendBufferCount = 0;
		m_ulRecvBufferCount	= 0;
		m_CurrentProtocol = 0;
	}

	KClientUser::~KClientUser()
	{
		


	}

	
	int KNetServer::Init()
	{
		m_pNetServer = NULL;
		if (g_pNetServer)
		{
			g_pNetServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pNetServer ) );
		}

		if (!m_pNetServer)
		{
			throw KException("NetServer", "Cloning Net Server Error");
		}
		return 1;
	}
	
	void KNetServer::RegisterClient(unsigned long nId)
	{
		int t = ClearClientSend(nId);
		CCriticalSection::Owner locker(g_NetMsgListMutex);
		if (!m_ClientUserSet[nId])
		{
			KClientUser * pNewUser = new KClientUser;
			pNewUser->m_ID = nId;
			m_ClientUserSet[nId] = pNewUser;
		}
	}

	int KNetServer::ClearClientSend(unsigned long nId)
	{
		int i = 0;
		
		{
			CCriticalSection::Owner locker(g_NetMsgListMutex);
			list<TCmd *>::iterator I = g_NetServiceThreadCmdList.begin();
			
			while (I != g_NetServiceThreadCmdList.end())
			{
				if ((*I)->ulNetId == nId)
				{
					i++;
					list<TCmd *>::iterator II = I;
					I ++;
					delete []*II;
					g_NetServiceThreadCmdList.erase(II);
				}
				else
					I ++;
			}
		}

		{
			CCriticalSection::Owner locker(g_MainMsgListMutex);
			list<TCmd*>::iterator MainMsgI = g_MainThreadCmdList.begin();
			while(MainMsgI != g_MainThreadCmdList.end())
			{
				if ((*MainMsgI)->ulNetId == nId)
				{
					
					list<TCmd*>::iterator II1 = MainMsgI;
					MainMsgI ++;
					delete [](*II1);
					g_MainThreadCmdList.erase(II1);
				}
				else 
					MainMsgI ++;
			}
		}	

		{
			CCriticalSection::Owner locker(g_GetRoleInfoMutex);
			list<TGetRoleInfoFromDB*>::iterator DBLoadI = g_DBLoadThreadCmdList.begin();
			while(DBLoadI != g_DBLoadThreadCmdList.end())
			{
				if ((*DBLoadI)->nNetId == nId)
				{
					list<TGetRoleInfoFromDB*>::iterator II2 = DBLoadI;
					DBLoadI ++;
					delete (*II2);
					g_DBLoadThreadCmdList.erase(II2);
				}
				else 
					DBLoadI ++;
			}
		}

		return i;
	}

	void KNetServer::DestoryClient(unsigned long nId)
	{
		CCriticalSection::Owner locker(g_NetMsgListMutex);
		int t = ClearClientSend(nId);
		KClientUser * pDelUse = m_ClientUserSet[nId];
		if (pDelUse)
		{
			delete pDelUse;
			m_ClientUserSet.erase(nId);
		}
	}
	
	//public
	int	KNetServer::Receive()
	{
		KClientUserSet::iterator I;
		try{

			CCriticalSection::Owner locker(g_NetMsgListMutex);
			for ( I = m_ClientUserSet.begin(); I != m_ClientUserSet.end(); I ++	)
			{
				Receive(I->first);
			}
			
		}
		catch(...)
		{
			g_nMistakeCount ++;
		}
		return 1;
	}
	
	int	KNetServer::Send()
	{
		KClientUserSet::iterator I;
		for ( I = m_ClientUserSet.begin(); I != m_ClientUserSet.end(); I ++)
		{
			Send(I->first);
		}
		return 1;
	}
	
	//private
	int	KNetServer::Receive(unsigned long nId)
	{
		//获得数据
		size_t nDataLen = 0;
		const void * pMsg = NULL;
		pMsg = m_pNetServer->GetPackFromClient(nId, nDataLen);
	
		if (nDataLen == 0 || pMsg == NULL)	return 0;
		TProcessData* pData = (TProcessData*)pMsg;
		return AppendData(nId, pMsg, nDataLen);
	}
	
	int KNetServer::AppendData(unsigned long nId, const void * pMsgBuffer, size_t nMsgLen)
	{
		KClientUser * pClientUser = m_ClientUserSet[nId];
		_ASSERT(pClientUser);
		
		pClientUser->m_ulRecvBufferCount += nMsgLen;
		g_dwRecvLen = pClientUser->m_ulRecvBufferCount;
		if (pClientUser->m_ulRecvBufferCount >= 0xFFFF1FFF)
			pClientUser->m_ulRecvBufferCount = 0;

		BYTE btProtoId = CPackager::Peek(pMsgBuffer);
		if (btProtoId > c2s_micropackbegin)
		{
			TProcessData * pProcessData = (TProcessData*)pMsgBuffer;
			if (pProcessData->nDataLen + sizeof(TProcessData) - 1 != nMsgLen)
			{
				_ASSERT(0);
				return 1;
			}
			return Service(pClientUser , (TProcessData *)pMsgBuffer, nId);
		}
		else
		{
			CBuffer* pData = pClientUser->m_RecvPackager.PackUp( pMsgBuffer, nMsgLen );
			if (pData)
			{	
				TProcessData * pProcessData = (TProcessData*)pData->GetBuffer();
				if (pProcessData->nDataLen + sizeof(TProcessData) - 1 != pData->GetUsed())
				{
					_ASSERT(0);
					SAFE_RELEASE(pData);	
					return 1;
				}
				Service(pClientUser , (TProcessData*)pData->GetBuffer(), nId);
				SAFE_RELEASE(pData);
			}
		}
		
		return 1;
	}

	int KNetServer::Service(KClientUser *pClientUser, TProcessData * pProcess, int nId)
	{
		if (!pProcess) return 0;
		TCmd * pNewCmd = (TCmd *) (new char [sizeof(TCmd) + pProcess->nDataLen - 1]);
		pNewCmd->ProcessData.nDataLen = pProcess->nDataLen;
		pNewCmd->ProcessData.nProtoId  = pProcess->nProtoId;
		pNewCmd->ProcessData.ulIdentity = pProcess->ulIdentity;
		pNewCmd->ulNetId	= nId;
		memcpy(&pNewCmd->ProcessData.pDataBuffer[0], &pProcess->pDataBuffer[0], pProcess->nDataLen);
		
		CCriticalSection::Owner locker(g_MainMsgListMutex);
		g_MainThreadCmdList.push_back(pNewCmd);
		pClientUser->m_nProcessLoadCount ++;
		return 1;
	}

	int KNetServer::Send(unsigned long nId)
	{
		KClientUser * pClient = m_ClientUserSet[nId];
		CBuffer  * pSendBuffer = NULL;
		if (pClient)
		{
			if (pClient->m_CurrentProtocol)
			{
				pSendBuffer = pClient->m_SendPackager.GetNextPack( pClient->m_CurrentProtocol);
				if (pSendBuffer)
				{
					int nLen = pSendBuffer->GetUsed();
				}
			}
			
			if ((!pSendBuffer) || !pClient->m_CurrentProtocol) 
			{
				if (pClient->m_CurrentProtocol && pClient->m_CurrentProtocol < 32)
					pClient->m_SendPackager.DelData(pClient->m_CurrentProtocol);
				
				pClient->m_CurrentProtocol = 0;
				
				CCriticalSection::Owner locker(g_NetMsgListMutex);
				list<TCmd *>::iterator I = g_NetServiceThreadCmdList.begin();
				
				while (I != g_NetServiceThreadCmdList.end())
				{
					int nIdd = (*I)->ulNetId;
					if ((*I)->ulNetId == nId)
					{
						if ((*I)->ProcessData.nProtoId >= 32)
						{
							TProcessData * pProc = (TProcessData *)&(*I)->ProcessData;
							m_pNetServer->SendData(nId, &(*I)->ProcessData, (*I)->ProcessData.nDataLen + sizeof(TProcessData) - 1);
							pClient->m_ulSendBufferCount +=  (*I)->ProcessData.nDataLen + sizeof(TProcessData) - 1;
							g_dwSendLen = pClient->m_ulSendBufferCount;
							pClient->m_CurrentProtocol = 0;
							pSendBuffer = NULL;
						}
						else
						{
							pClient->m_CurrentProtocol = (*I)->ProcessData.nProtoId;
							TCmd * pCmd = *I;
							TProcessData * pTestData = (TProcessData *)&(*I)->ProcessData;
							pClient->m_SendPackager.AddData((*I)->ProcessData.nProtoId, (const char *)&(*I)->ProcessData, (*I)->ProcessData.nDataLen + sizeof(TProcessData) - 1, (*I)->ProcessData.ulIdentity );
							pSendBuffer = pClient->m_SendPackager.GetHeadPack(pClient->m_CurrentProtocol);
						}
						delete [](*I);
						g_NetServiceThreadCmdList.erase(I);
						pClient->m_nProcessWriteCount ++;
						break;
					}
					I ++;
				}
			}
			
	/*		TRoleData * pRoleData = pBuffer;
			OnlineGameLib::Win32::CPackager	m_Packager;
			TProcessData * pData = (TProcessData *)new char[pRoleData->dwDataLen + sizeof(TProcessData) -1 + 1];
			pData->nProtoId = c2s_roleserver_saveroleinfo;
			pData->pDataBuffer[0] = create_role;//or pData[0] = save_role    
			pData->nDataLen = pRoleData->dwDataLen + 1 ;
			pData->ulIdentity = id;
			memcpy(&pData->pDataBuffer[1], pRoleData, pRoleData->dwDataLen);
			
			m_Packager.AddData(pData->nProtoId, pData, pData->nDataLen + sizeof(TProcessData) -1, id);
			CBuffer * pBuffer = m_Packager.GetHeadPack(pData->nProtoId);
			
			while(pBuffer)
			{
				Send(pBuffer->GetBuffer());
				SAFE_RELEASE(pBuffer);
				pBuffer = m_Packager.GetNextPack(pData->nProtoId);
			}

			m_Packager.DelData(pData->nProtoId);
	*/		

			if (pSendBuffer)
			{
				m_pNetServer->SendData(nId, pSendBuffer->GetBuffer(), pSendBuffer->GetUsed());
				pClient->m_ulSendBufferCount += pSendBuffer->GetUsed();
				g_dwSendLen = pClient->m_ulSendBufferCount;
				
				if (pClient->m_ulSendBufferCount >= 0xFFFF1FFF)
					pClient->m_ulSendBufferCount = 0;
				SAFE_RELEASE(pSendBuffer);
				g_dwSendLen = pClient->m_ulSendBufferCount;
			}
		}
		return 1;		
	}
}

