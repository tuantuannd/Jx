#include "stdafx.h"
#include "KDBProcessThread.h"
#include "kengine.h"
#define MAX_PICKUP_NUM 5

DWORD WINAPI DatabaseSaveThreadFun(void * pParam)
{
	static char RoleBuffer[MAX_ROLEINFORMATION_SIZE];
	BYTE * pRoleBuffer = NULL;
	size_t nSize = 0;
	char szRoleName[100];
	while(1)
	{
		try
		{
			
			g_SaveDBThreadMark ++;
			if (g_SaveDBThreadMark > MAX_MARK_COUNT) g_SaveDBThreadMark = 0;
			nSize = 0;
			
			{
				CCriticalSection::Owner locker( g_RoleInfoSetMutex);
				list<KRoleInfomation*>::iterator I = g_RoleInfoList.begin();
				
				int nNodeCount =	g_RoleInfoList.size();
				int nVisitTime =	0;
				
				
				
				while(I != g_RoleInfoList.end() && nVisitTime < nNodeCount && nSize == 0)
				{
					nVisitTime ++;
					if ((*I)->IsValid())
					{
						if ((*I)->m_bModify)//数据已被修改，但未存档，需要存档
						{
							nSize = MAX_ROLEINFORMATION_SIZE;
							BOOL bResult = (*I)->CopyRoleInfoBuffer((BYTE *)RoleBuffer, nSize);
							
							if (!bResult || nSize == 0) 
							{
								g_DebugLog("获得无效角色数据.普通错误");
								g_nMistakeCount ++;
							}
							else
							{
								sprintf(g_SaveMarkString, "正在保存角色%s......", GetRoleNameFromRoleBuffer(RoleBuffer));
								g_SaveDBThreadMark = MAX_MARK_COUNT + SAVEMARK_NO;
								(*I)->m_bModify = false;
								(*I)->m_nUnModifyTime = 0;
							}
						}
						else
						{
							(*I)->m_nUnModifyTime ++;
							
							if ((*I)->m_nUnModifyTime > 200)
							{
								size_t nTempSize =0;
								
								szRoleName[0] = 0;
								
								size_t nLen = 100;
								BOOL bResult = (*I)->GetRoleName(szRoleName, nLen);
								if (bResult && nLen > 0)
								{
									sprintf(g_RemoveMarkString, "正在释放角色信息%s......", szRoleName);
									g_SaveDBThreadMark = MAX_MARK_COUNT + REMOVEMARK_NO;
								}
								
								delete *I;
								g_RoleInfoList.pop_front();
								if (szRoleName)
									g_RoleInfoMap.erase(szRoleName);
								I = g_RoleInfoList.begin();
								continue;
							}
						}
						
						if (nNodeCount > 1)
						{
							KRoleInfomation * pRoleInfomation = *I;
							g_RoleInfoList.pop_front();
							g_RoleInfoList.push_back(pRoleInfomation);
						}
					}
					else
					{
						delete *I;
						g_RoleInfoList.pop_front();
						if (szRoleName)
							g_RoleInfoMap.erase(szRoleName);
					}
					I = g_RoleInfoList.begin();
				}
			
			}
			
			if (nSize > 0)
			{
				S3DBI_SaveRoleInfo((BYTE*)RoleBuffer, GetRoleNameFromRoleBuffer(RoleBuffer), 1);
			}
			
			Sleep(g_dwPerSaveTime);
		}
		catch(...)
		{
			g_DebugLog("存档线程出错!");
			g_nMistakeCount ++;
		}
	}
	
	return 1;
}
