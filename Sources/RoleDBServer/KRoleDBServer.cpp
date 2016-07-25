
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
namespace RoleDBServer
{
	
	KClientUser::KClientUser()
	{
		m_pCurrentRecvProcessData = NULL;
		m_pCurrentSendProcessData = NULL;
		m_nProcessLoadCount = m_nProcessWriteCount = 0;
		this->m_nCurSendLen = 0;
		this->m_nCurGetRecvLen = 0;
		this->m_szClientName[0] = 0;
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
		KClientUser * pNewUser = new KClientUser;
		pNewUser->m_ID = nId;
		m_ClientUserSet[nId] = pNewUser;
	}

	void KNetServer::DestoryClient(unsigned long nId)
	{
		KClientUser * pDelUse = m_ClientUserSet[nId];
		if (pDelUse)
		{
			m_ClientUserSet.erase(nId);
			delete pDelUse;
		}
	}
	
	//public
	int	KNetServer::Receive()
	{
		KClientUserSet::iterator I;
		try{
			WaitForSingleObject(g_NetEventMutex, INFINITE);
			for ( I = m_ClientUserSet.begin(); I != m_ClientUserSet.end(); I ++	)
			{
				Receive(I->first);
			}
			ReleaseMutex(g_NetEventMutex);
		}
		catch(...)
		{
			
		}
		return 1;
	}
	
	int	KNetServer::Send()
	{
		
		KClientUserSet::iterator I;
		m_pNetServer->PreparePackSink();
		for ( I = m_ClientUserSet.begin(); I != m_ClientUserSet.end(); I ++)
		{
			Send(I->first);
		}
		
		//m_pNetServer->SendPackToClient();
		return 1;
	}
	
	
	//private
	int	KNetServer::Receive(unsigned long nId)
	{
		//获得数据
		size_t nDataLen = 0;
		TRoleNetMsg *pMsg;
		pMsg =(TRoleNetMsg*) m_pNetServer->GetPackFromClient(nId, nDataLen);

		if (nDataLen == 0 || pMsg == NULL)	return 0;
		return AppendData(nId, pMsg, nDataLen);
	}
	
	int KNetServer::AppendData(unsigned long nId, TRoleNetMsg * pMsgBuffer, size_t nMsgLen)
	{
		
		switch(pMsgBuffer->bDataState)
		{
		case	1://游戏服务器向数据库服务器要求保存数据
			{
				KClientUser * pClientUser = m_ClientUserSet[nId];
				_ASSERT(!pClientUser->m_pCurrentRecvProcessData);
				if (!nMsgLen) return 0;
				pClientUser->m_pCurrentRecvProcessData =(TCmd*) new char[pMsgBuffer->nOffset + sizeof(TCmd) - 1];
				pClientUser->m_pCurrentRecvProcessData->nDataLen	= pMsgBuffer->nOffset;
				pClientUser->m_pCurrentRecvProcessData->nProtoId	= pMsgBuffer->nProtoId;
				pClientUser->m_pCurrentRecvProcessData->ulNetId	= nId;
				
				memcpy(pClientUser->m_pCurrentRecvProcessData->pDataBuffer, &pMsgBuffer->pDataBuffer, pMsgBuffer->nDataLen);
				pClientUser->m_nCurGetRecvLen = pMsgBuffer->nDataLen;
				
				if (pClientUser->m_pCurrentRecvProcessData->nDataLen == pClientUser->m_nCurGetRecvLen)
				{	
					InsertProcessData2Set(nId);
					ClearCurProcessData(nId);
				}
				
			}
			break;
		case		0:
			{
				KClientUser * pClientUser = m_ClientUserSet[nId];
				_ASSERT(pClientUser->m_pCurrentRecvProcessData->pDataBuffer);
				//_ASSERT(pClientUser->m_nCurGetRecvLen == pMsgBuffer->nOffset);
				_ASSERT(pClientUser->m_pCurrentRecvProcessData->nProtoId == pMsgBuffer->nProtoId);
				
				if (pClientUser->m_nCurGetRecvLen + pMsgBuffer->nDataLen > pClientUser->m_pCurrentRecvProcessData->nDataLen)
					_ASSERT(0);
				else
				{
					memcpy(pClientUser->m_pCurrentRecvProcessData->pDataBuffer + pClientUser->m_nCurGetRecvLen , &pMsgBuffer->pDataBuffer, pMsgBuffer->nDataLen);
					pClientUser->m_nCurGetRecvLen += pMsgBuffer->nDataLen;
					if (pClientUser->m_pCurrentRecvProcessData->nDataLen == pClientUser->m_nCurGetRecvLen)
					{
						InsertProcessData2Set(nId);
						ClearCurProcessData(nId);
					}
				}
				
			}break;
			
		default:
			{
				_ASSERT(0);
				
			}
		}
		return 1;	
	}
	
	void KNetServer::InsertProcessData2Set(int nId)
	{
		KClientUser * pClientUser = m_ClientUserSet[nId];
		TCmd * pNewCmd = (TCmd *) (new char [sizeof(TCmd) + pClientUser->m_pCurrentRecvProcessData->nDataLen - 1]);
		pNewCmd->ulNetId = nId;
		pNewCmd->nDataLen = pClientUser->m_pCurrentRecvProcessData->nDataLen;
		pNewCmd->nProtoId  = pClientUser->m_pCurrentRecvProcessData->nProtoId;
		memcpy(&pNewCmd->pDataBuffer[0], pClientUser->m_pCurrentRecvProcessData->pDataBuffer, pClientUser->m_pCurrentRecvProcessData->nDataLen );
		
		EnterCriticalSection(&g_MainMsgListMutex);
		g_MainThreadCmdList.push_back(pNewCmd);
		pClientUser->m_nProcessLoadCount ++;
		LeaveCriticalSection(&g_MainMsgListMutex);
	}
	
	
	int KNetServer::Send(unsigned long nId)
	{
		KClientUser * pClient = m_ClientUserSet[nId];
		size_t ntmpSendLen = 0; //发送实际数据中的长度
		TRoleNetMsg * pMsg = NULL;
		bool	bCompleteSend = false;
		char * pSendBuffer = NULL;
		if (pClient->m_pCurrentSendProcessData)
		{
			if (pClient->m_nCurSendLen == 0)
			{
				//如果一次性能够发完的
				if (pClient->m_pCurrentSendProcessData->nDataLen + sizeof(TRoleNetMsg) - 1 < g_nMaxSendBuffer)
				{
					pSendBuffer = new char [pClient->m_pCurrentSendProcessData->nDataLen + sizeof(TRoleNetMsg) - 1];
					ntmpSendLen = pClient->m_pCurrentSendProcessData->nDataLen ;
					bCompleteSend = true;
					
				}
				else
				{
					pSendBuffer = new char[g_nMaxSendBuffer];
					ntmpSendLen = g_nMaxSendBuffer - sizeof(TRoleNetMsg) + 1;
					bCompleteSend = false;
					pClient->m_nCurSendLen = ntmpSendLen;
				}
				
				pMsg = (TRoleNetMsg*)pSendBuffer;
				pMsg->nProtoId = pClient->m_pCurrentSendProcessData->nProtoId;
				pMsg->bDataState = 1;//first package send;
				pMsg->nOffset = pClient->m_pCurrentSendProcessData->nDataLen;
				pMsg->nDataLen = ntmpSendLen;
				
				memcpy(pMsg->pDataBuffer, pClient->m_pCurrentSendProcessData->pDataBuffer, ntmpSendLen);
				
			}
			else
			{
				if (pClient->m_pCurrentSendProcessData->nDataLen > pClient->m_nCurSendLen + g_nMaxSendBuffer - sizeof(TRoleNetMsg) + 1 )
				{
					ntmpSendLen = g_nMaxSendBuffer - sizeof(TRoleNetMsg) + 1;
					pSendBuffer = new char[g_nMaxSendBuffer] ;
					bCompleteSend = false;
				}
				else
				{
					ntmpSendLen = pClient->m_pCurrentSendProcessData->nDataLen - pClient->m_nCurSendLen ;
					pSendBuffer = new char[ntmpSendLen + sizeof(TRoleNetMsg) - 1];
					bCompleteSend = true;
				}
				pMsg = (TRoleNetMsg*)pSendBuffer;
				pMsg->nProtoId = pClient->m_pCurrentSendProcessData->nProtoId;
				pMsg->nOffset =  pClient->m_nCurSendLen;
				pMsg->nDataLen = ntmpSendLen;
				pMsg->bDataState = 0;
				memcpy(pMsg->pDataBuffer, pClient->m_pCurrentSendProcessData->pDataBuffer + pClient->m_nCurSendLen, ntmpSendLen);
				pClient->m_nCurSendLen += ntmpSendLen;
				
			}
			 m_pNetServer->PackDataToClient(nId, pMsg, ntmpSendLen + sizeof(TRoleNetMsg) - 1);
		}
		else
		{
			bCompleteSend = true;
		}
		
		
		
		if (bCompleteSend)
		{
			if (pClient->m_pCurrentSendProcessData)
			{
				delete pClient->m_pCurrentSendProcessData;
				pClient->m_pCurrentSendProcessData = NULL;
			}
			
			EnterCriticalSection(&g_NetMsgListMutex);
			list<TCmd *>::iterator I = g_NetServiceThreadCmdList.begin();
			
			if (I != g_NetServiceThreadCmdList.end())
			{
				TProcessData * pNewProcess = (TCmd *)new char[(*I)->nDataLen + sizeof(TProcessData) - 1];
				pNewProcess->nDataLen = (*I)->nDataLen;
				pNewProcess->nProtoId = (*I)->nProtoId;
				pNewProcess->ulNetId = (*I)->ulNetId;
				
				
				memcpy(&pNewProcess->pDataBuffer[0], (*I)->pDataBuffer, (*I)->nDataLen);
				
				pClient->m_pCurrentSendProcessData = pNewProcess;
				//how to delete 
				g_NetServiceThreadCmdList.pop_front();
				pClient->m_nCurSendLen = 0;
				pClient->m_nProcessWriteCount ++;
			}
			
			LeaveCriticalSection(&g_NetMsgListMutex);

		}
		return 1;
	}
	
	void KNetServer::ClearCurProcessData(unsigned long nId)
	{
		KClientUser * pClientUser = m_ClientUserSet[nId];
		delete pClientUser->m_pCurrentRecvProcessData;
		pClientUser->m_pCurrentRecvProcessData = NULL;
		
	}
	
}


