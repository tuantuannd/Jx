#include "stdafx.h"
#include "KDBProcessThread.h"
#include "KRoleDBServer.h"
#include "kengine.h"
//主循环，负责处理网络端发来的完整的请求数据
DWORD 	WINAPI RoleServerMainThreadFun(void * pParam)
{
	
	TCmd * pCopyCmd = NULL;
	while(1)
	{
		try
		{
			
			g_MainLoopMark ++;
			if (g_MainLoopMark > MAX_MARK_COUNT) g_MainLoopMark = 0;
			
			{
				CCriticalSection::Owner locker( g_MainMsgListMutex);
				list<TCmd *>::iterator I = g_MainThreadCmdList.begin();
				if (I == g_MainThreadCmdList.end()) 
					pCopyCmd = NULL;
				else
				{
					TCmd * pCmd = (*I);
					pCopyCmd = (TCmd *)(new char[sizeof(TCmd) + (*I)->ProcessData.nDataLen - 1] );
					pCopyCmd->ProcessData.nDataLen = pCmd->ProcessData.nDataLen;
					pCopyCmd->ProcessData.nProtoId = pCmd->ProcessData.nProtoId;
					pCopyCmd->ProcessData.ulIdentity = pCmd->ProcessData.ulIdentity;
					pCopyCmd->ulNetId = pCmd->ulNetId;
					memcpy(&pCopyCmd->ProcessData.pDataBuffer[0], &(*I)->ProcessData.pDataBuffer[0], (*I)->ProcessData.nDataLen);
					delete []pCmd;
					g_MainThreadCmdList.pop_front();
				}
			}
			
			
			if (pCopyCmd)
			{
				switch(pCopyCmd->ProcessData.nProtoId)
				{
					//Save
				case c2s_roleserver_createroleinfo:
				case c2s_roleserver_saveroleinfo://游戏服务器向数据库服务器要求保存数据
					{
						
						const char * szName = (const char *)GetRoleNameFromRoleBuffer((char *)&pCopyCmd->ProcessData.pDataBuffer[0]);
						if (!szName[0]) 
						{
							g_DebugLog("存档过程中发现客户端传来的数据不合法.");
							g_nMistakeCount ++;
							break;
						}
						BOOL bExistRole = 0;
						bExistRole = S3DBI_IsRoleNameExisted((char *)szName);
						char btResult = -1;
						
						if (pCopyCmd->ProcessData.nProtoId == c2s_roleserver_createroleinfo)
						{
							if (bExistRole)
							{
								btResult = -2;
							}
							else
								btResult = 1;
						}
						else if (pCopyCmd->ProcessData.nProtoId == c2s_roleserver_saveroleinfo)
						{
							if (!bExistRole)
							{
								btResult = -2;
							}
							else
								btResult = 1;
						}
						else
							btResult = -1;
						
						
						
						{
							CCriticalSection::Owner locker(g_RoleInfoSetMutex);	
							if (btResult > 0 && szName)
							{
								KRoleInfomation * pRoleInfomation =  g_RoleInfoMap[szName] ;
								
								if (!pRoleInfomation)//
								{
									KRoleInfomation * pNewRoleInfomation = new KRoleInfomation;
									
									if (!pNewRoleInfomation->SetRoleInfoBuffer((unsigned char *)&(pCopyCmd->ProcessData.pDataBuffer[0]), pCopyCmd->ProcessData.nDataLen))
									{
										btResult = -1;
										delete pNewRoleInfomation;
										g_nMistakeCount ++;
									}
									else
									{
										g_RoleInfoMap[szName] = pNewRoleInfomation;
										g_RoleInfoList.push_back(pNewRoleInfomation);
									}
								}
								else
								{
									pRoleInfomation->SetRoleInfoBuffer((unsigned char *)(&pCopyCmd->ProcessData.pDataBuffer[0]), pCopyCmd->ProcessData.nDataLen);
								}
							}
						}				
						
						
						TCmd * pNewCmd = (TCmd *)new BYTE[sizeof(TCmd) + 1 - 1];
						
						pNewCmd->ProcessData.nProtoId = (pCopyCmd->ProcessData.nProtoId == c2s_roleserver_saveroleinfo) ? s2c_roleserver_saverole_result: s2c_roleserver_createrole_result;
						
						pNewCmd->ProcessData.pDataBuffer[0] = (BYTE)btResult;
						pNewCmd->ProcessData.nDataLen = 1;
						
						pNewCmd->ulNetId = pCopyCmd->ulNetId;
						pNewCmd->ProcessData.ulIdentity = pCopyCmd->ProcessData.ulIdentity;
						
						
						CCriticalSection::Owner locker(g_NetMsgListMutex);
						
						if (g_NetServer.m_ClientUserSet[pNewCmd->ulNetId])
						{
							g_NetServiceThreadCmdList.push_back(pNewCmd);
						}
						else
						{
							delete []pNewCmd;
						}
					}
					break;
					
					//Load
				case c2s_roleserver_getroleinfo://获得角色数据
					{
						char szName[100];
						_ASSERT(pCopyCmd->ProcessData.nDataLen <= 32);
						memcpy(szName, pCopyCmd->ProcessData.pDataBuffer, pCopyCmd->ProcessData.nDataLen);// GetRoleNameFromRoleBuffer((char *)&pCopyCmd->pDataBuffer);
						
						szName[pCopyCmd->ProcessData.nDataLen] = 0;
						if (szName)
						{
							CCriticalSection::Owner locker(g_RoleInfoSetMutex);	
							KRoleInfomation * pRoleInfomation = g_RoleInfoMap[szName];
							size_t BufferSize = 0;
							BYTE * pBuffer = NULL;
							
							BOOL bLoadSuccess = FALSE;
							if (pRoleInfomation)
							{
								TCmd *pNewCmd = pRoleInfomation->CreateRoleInfoTCmdData(BufferSize);
								if (pNewCmd)
								{
									pNewCmd->ProcessData.nProtoId = s2c_roleserver_getroleinfo_result;
									pNewCmd->ProcessData.nDataLen = BufferSize;
									pNewCmd->ulNetId = pCopyCmd->ulNetId;
									pNewCmd->ProcessData.ulIdentity = pCopyCmd->ProcessData.ulIdentity;
									
									CCriticalSection::Owner locker(g_NetMsgListMutex);	
									if (g_NetServer.m_ClientUserSet[pNewCmd->ulNetId])
									{
										g_NetServiceThreadCmdList.push_back(pNewCmd);
										bLoadSuccess = TRUE;
									}
									else
									{
										delete []pNewCmd;
										g_nMistakeCount ++;
									}	
								}
								else
								{
									delete []pNewCmd;
									g_nMistakeCount ++;
									
								}
							}
							
							if (FALSE == bLoadSuccess)//没有则需要实际的取数据，需要向LoadThread发送消息
							{
								if (pRoleInfomation)
									g_nMistakeCount ++;
								TGetRoleInfoFromDB * pNewCmd = new TGetRoleInfoFromDB;
								pNewCmd->nNetId = pCopyCmd->ulNetId;
								strcpy(pNewCmd->szRoleName, szName);
								pNewCmd->ulIdentity = pCopyCmd->ProcessData.ulIdentity;
								CCriticalSection::Owner locker(g_GetRoleInfoMutex);
								g_DBLoadThreadCmdList.push_back(pNewCmd);
							}
							
						}
						
					}break;
					
					//LoadRoleList
				case c2s_roleserver_getrolelist://获得某个帐号的角色列表
					{
						unsigned char  MaxRoleCount = (unsigned char)pCopyCmd->ProcessData.pDataBuffer[0];
						
						unsigned char * pNewBuffer = new  unsigned char [sizeof(TCmd) + sizeof(S3DBI_RoleBaseInfo) * MaxRoleCount + 1 - 1];//列表和1个
						TCmd * pCmd = (TCmd*)pNewBuffer;
						pCmd->ulNetId = pCopyCmd->ulNetId;
						pCmd->ProcessData.nProtoId	=  s2c_roleserver_getrolelist_result;
						pCmd->ProcessData.nDataLen  =  sizeof(S3DBI_RoleBaseInfo) * MaxRoleCount + 1;
						pCmd->ProcessData.ulIdentity = pCopyCmd->ProcessData.ulIdentity;
						
						S3DBI_RoleBaseInfo * pRoleBaseInfoList  = (S3DBI_RoleBaseInfo*) ((char*)&pCmd->ProcessData.pDataBuffer[0] + 1);
						
						char  szAccountName[32];
						memcpy(szAccountName, &pCopyCmd->ProcessData.pDataBuffer[0] + 1, pCopyCmd->ProcessData.nDataLen - 1 );
						szAccountName[pCopyCmd->ProcessData.nDataLen - 1] = 0;
						
						pCmd->ProcessData.pDataBuffer[0]  = (unsigned char )S3DBI_GetRoleListOfAccount(szAccountName, pRoleBaseInfoList, MaxRoleCount);
						
						{
							CCriticalSection::Owner locker(g_RoleInfoSetMutex);	
							
							for (int i  = 0; i < pCmd->ProcessData.pDataBuffer[0]; i ++)
							{
								KRoleInfomation * pRoleInfo = g_RoleInfoMap[(pRoleBaseInfoList + i)->szName];
								if (pRoleInfo)
								{
									size_t nSize = 0;
									TRoleBaseInfo BaseInfo;
									BOOL bBase = pRoleInfo->GetBaseRoleInfo(&BaseInfo);
									if (bBase)
									{


										(pRoleBaseInfoList + i)->ArmorType = BaseInfo.iarmorres;
										(pRoleBaseInfoList + i)->HelmType  = BaseInfo.ihelmres;
										(pRoleBaseInfoList + i)->Level     = BaseInfo.ifightlevel;
										(pRoleBaseInfoList + i)->Sex		  = BaseInfo.bSex;
										(pRoleBaseInfoList + i)->WeaponType = BaseInfo.iweaponres;
									}
								}
							}
							
						}
						
						CCriticalSection::Owner locker(g_NetMsgListMutex);
						
						if (g_NetServer.m_ClientUserSet[pCmd->ulNetId])
						{
							g_NetServiceThreadCmdList.push_back(pCmd);
						}
						else
						{
							delete []pCmd;
						}
					}
					break;
				case c2s_roleserver_deleteplayer:
					{
						char szDelRoleName[100];
						if (pCopyCmd->ProcessData.nDataLen > 32) 
						{
							g_nMistakeCount ++;
							break;
						}
						
						memcpy(szDelRoleName, &pCopyCmd->ProcessData.pDataBuffer[0], pCopyCmd->ProcessData.nDataLen);
						szDelRoleName[pCopyCmd->ProcessData.nDataLen] = 0;
						
						int nResult = S3DBI_DeleteRole(szDelRoleName);
						
						{
							CCriticalSection::Owner locker(g_RoleInfoSetMutex);	
							g_RoleInfoMap[szDelRoleName]->Clear();
							g_RoleInfoMap.erase(szDelRoleName);
						}
						
						
						
						TCmd * pNewCmd = (TCmd *)new BYTE[sizeof(TCmd) + 1 - 1];
						
						pNewCmd->ProcessData.nProtoId = s2c_roleserver_deleterole_result;
						
						pNewCmd->ProcessData.pDataBuffer[0] = (BYTE)nResult;
						pNewCmd->ProcessData.nDataLen = 1;
						
						pNewCmd->ulNetId = pCopyCmd->ulNetId;
						pNewCmd->ProcessData.ulIdentity = pCopyCmd->ProcessData.ulIdentity;
						
						CCriticalSection::Owner locker(g_NetMsgListMutex);
						
						if (g_NetServer.m_ClientUserSet[pNewCmd->ulNetId])
						{
							g_NetServiceThreadCmdList.push_back(pNewCmd);
						}
						else
						{
							delete []pNewCmd;
						}
						
					}break;
					//Exit
			}
			delete []pCopyCmd;
			pCopyCmd = NULL;
		}
		Sleep(1);
		}
		catch(...)
		{
			g_nMistakeCount ++;
		}
	}
	return 1;
}

