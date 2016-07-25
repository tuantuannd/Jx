#include "stdafx.h"
#include "KDBProcessThread.h"
#include "kengine.h"
//循环处理向数据库请求获得玩家信息的操作 

DWORD WINAPI DatabaseLoadThreadFun(void * pParam)
{
	list<TGetRoleInfoFromDB *>::iterator I;
	TGetRoleInfoFromDB GetRoleInfoCmd;
	
	while(1)
	{
		try
		{
			g_LoadDBThreadMark ++;
			if (g_LoadDBThreadMark > MAX_MARK_COUNT) g_LoadDBThreadMark = 0;
			bool	bFind = false;
			
			{
				CCriticalSection::Owner locker( g_GetRoleInfoMutex );
				I = g_DBLoadThreadCmdList.begin();
				
				if (I != g_DBLoadThreadCmdList.end())
				{
					GetRoleInfoCmd.nNetId = (*I)->nNetId;
					strcpy(GetRoleInfoCmd.szRoleName, (*I)->szRoleName);
					GetRoleInfoCmd.ulIdentity = (*I)->ulIdentity;
					delete *I;
					g_DBLoadThreadCmdList.pop_front();
					bFind = true;
				}
			}
			
			if (bFind)
			{
				static unsigned char szRoleInfo[MAX_ROLEINFORMATION_SIZE];
				int nBufferSize = MAX_ROLEINFORMATION_SIZE;
				
				sprintf(g_LoadMarkString, "正在数据库读取角色%s......", GetRoleInfoCmd.szRoleName);
				g_LoadDBThreadMark = MAX_MARK_COUNT + LOADMARK_NO; 
				
				if (!S3DBI_GetRoleInfo(szRoleInfo, GetRoleInfoCmd.szRoleName, nBufferSize))	continue;
				
				if (nBufferSize > MAX_ROLEINFORMATION_SIZE)
				{
					ASSERT(0);
					g_DebugLog("获得的角色数据超长！");
					g_nMistakeCount ++;
				}
				
				//增加角色数据和向两个列表增加数据
				
				const char * szName = (const char *)(GetRoleInfoCmd.szRoleName);
				if (szName)
				{
					{
						CCriticalSection::Owner locker(g_RoleInfoSetMutex);	
						KRoleInfomation * pRoleInfomation =	g_RoleInfoMap[szName] ;
						
						if (!pRoleInfomation)//新加角色！
						{
							KRoleInfomation * pNewRoleInfomation = new KRoleInfomation;
							pNewRoleInfomation->SetRoleInfoBuffer(szRoleInfo, nBufferSize);
							g_RoleInfoMap[szName] = pNewRoleInfomation;
							g_RoleInfoList.push_back(pNewRoleInfomation);
						}
					}
					
					TCmd * pNewCmd		= (TCmd *)new char [sizeof(TCmd)  + strlen(szName) - 1];
					pNewCmd->ulNetId	= GetRoleInfoCmd.nNetId;
					pNewCmd->ProcessData.nProtoId	= c2s_roleserver_getroleinfo;
					pNewCmd->ProcessData.nDataLen	= strlen(szName);
					pNewCmd->ProcessData.ulIdentity = GetRoleInfoCmd.ulIdentity;
					memcpy(&pNewCmd->ProcessData.pDataBuffer[0], szName, strlen(szName));
					
					CCriticalSection::Owner locker(g_MainMsgListMutex);
					g_MainThreadCmdList.push_back(pNewCmd);
				}
				
			}
			Sleep(1);
		}
		catch(...)
		{
			g_nMistakeCount ++;
			g_DebugLog("DBLoad Throw Error!");
		}
		
	}
	return 1;
}
