#include "KCore.h"

#ifdef _SERVER

#include "KThread.h"
#include "KPlayer.h"
#include "../MultiServer/Heaven/Interface/iServer.h"
#include "CoreServerDataDef.h"
//#define REMOTE_DBSERVER
extern KMutex g_LoadRoleListMutex;
KMutex	g_Mutex;
// need change
BOOL g_bGetRoleList[510];

void * DBThreadFun(void * pParam)
{
	if (S3DBI_InitDBInterface())
		g_DebugLog("Connect DB Success!");
	else
	{
		g_DebugLog("Connect DB is Wrong!");
		return NULL;
	}
	int nPlayerIndex = 0;
	int nCommand = 0;
	void * pParam1;
	void * pParam2;
	int nResult = 0;
	
	while(1)
	{
		pParam1 = pParam2 = NULL;
		g_Mutex.Lock();
		if (g_AccessDBMsgList(DBMSG_POP, &nPlayerIndex, (DBI_COMMAND*)&nCommand, &pParam1, &pParam2))
		{
			switch(nCommand)
			{
			case	DBI_PLAYERSAVE:
				if (nPlayerIndex > 0 && pParam1)
				{			
					BYTE * pBuffer = (BYTE *)((KMemClass * )pParam1)->GetMemPtr();
					if (*(int *)pParam2 == 1)//ÐÂ½¨½ÇÉ«
					{
#ifndef REMOTE_DBSERVER
						nResult = S3DBI_SaveRoleInfo((BYTE *) pBuffer, "", 1);
#else
						
#endif
					}
					else
					{
#ifndef REMOTE_DBSERVER
						nResult = S3DBI_SaveRoleInfo((BYTE *) pBuffer, "", 0);
#else
						
#endif
					}
					((KMemClass *)pParam1)->Free();
					delete pParam1;
					//					Player[nPlayerIndex].DeleteTempValue(pParam1);
					Player[nPlayerIndex].DeleteTempValue(pParam2);
				}
				break;
			case	DBI_PLAYERLOAD:
				if (nPlayerIndex > 0 && pParam1)
				{
					int nBufferLen = 0;
					
#ifndef REMOTE_DBSERVER
					BYTE bysRoleInfo[500000];
					void * pBuffer = S3DBI_GetRoleInfo(bysRoleInfo, Player[nPlayerIndex].GetPlayerName(), nBufferLen);
					if (pBuffer && nBufferLen > 0)
					{
						KMemClass * pMemClass = new KMemClass;
						pMemClass->Alloc(nBufferLen);
						
						//BYTE * pRoleInfo = new BYTE[nBufferLen];
						memcpy(pMemClass->GetMemPtr(), bysRoleInfo, nBufferLen);
						Player[nPlayerIndex].m_pStatusLoadPlayerInfo = (BYTE*)pMemClass;//pRoleInfo;
						
						//testcodes for get test role
						/*KFile File;
						char szFileName[100];
						sprintf(szFileName, "c:\\Role_%s.dat", Player[nPlayerIndex].GetPlayerName());
						File.Create(szFileName);
						File.Write(bysRoleInfo, nBufferLen);
						File.Close();
						*/
						Player[nPlayerIndex].DeleteTempValue(pParam1);
						Player[nPlayerIndex].DeleteTempValue(pParam2);
					}
					else
						g_DebugLog("[Database]LoadDBPlayer Error!");
#else
					
#endif
				}
				break;
			case	DBI_PLAYERDELETE:
#ifndef REMOTE_DBSERVER
				if (nPlayerIndex > 0)
					nResult = S3DBI_DeleteRole(Player[nPlayerIndex].GetPlayerName());
#else
#endif
				break;
			case	DBI_GETPLAYERLISTFROMACCOUNT:
				{
					if (pParam1 && ((char*)pParam1)[0] && pParam2)
					{
						int nClientIndex = nPlayerIndex;
						LoginData * pData = (LoginData*) pParam2;
						pData->nRoleCount = S3DBI_GetRoleListOfAccount(
							(char *)pParam1, 
							(S3DBI_RoleBaseInfo*) pData->PlayerBaseInfo, 
							MAX_PLAYER_IN_ACCOUNT);
						g_LoadRoleListMutex.Lock();
						g_bGetRoleList[nClientIndex] = 1;
						g_LoadRoleListMutex.Unlock();
					}
				}
				break;
			}
			g_Mutex.Unlock();
		}
		else 
		{
			g_Mutex.Unlock();
			Sleep(500);
		}
		
	}
	return NULL;
}
static  KMutex g_DBMsgListMutex;		
int	 CORE_API g_ClearOutOfDateDBMsgInList(int nPlayerIndex)
{
	g_Mutex.Lock();
	g_DBMsgListMutex.Lock();
	KDBMsgNode * pNode = (KDBMsgNode*)g_DBMsgList.GetHead();
	int i = 0;
	while(pNode)
	{
		
		if (pNode->nPlayerIndex == nPlayerIndex && pNode->Command != DBI_PLAYERSAVE)
		{
			KDBMsgNode * pDelNode = pNode;
			pNode = (KDBMsgNode*)pNode->GetNext();
			pDelNode->Remove();
			delete pDelNode;
			i++;
			continue;
		}
		
		pNode = (KDBMsgNode*)pNode->GetNext();
	}
	g_bGetRoleList[nPlayerIndex] = 0;
	g_DBMsgListMutex.Unlock();
	g_Mutex.Unlock();
	return i;
}


BOOL CORE_API g_AccessDBMsgList(DBMESSAGE Msg,  int* pnPlayerIndex, DBI_COMMAND * pnDBICommand, void ** ppParam1, void ** ppParam2)
{
	
	BOOL bResult = FALSE;
	
	switch(Msg)
	{
	case DBMSG_PUSH:
		{
			if (*ppParam1)
			{
				KDBMsgNode * pNewNode = new KDBMsgNode;
				pNewNode->pParam1 = (void *) *ppParam1;
				pNewNode->pParam2 = (void *) *ppParam2;
				pNewNode->nPlayerIndex = *pnPlayerIndex;
				pNewNode->Command = *pnDBICommand;
				g_DBMsgListMutex.Lock();
				g_DBMsgList.AddTail(pNewNode);
				g_DBMsgListMutex.Unlock();
				bResult = TRUE;
			}
		}
		break;
		
	case DBMSG_POP:
		{
			*pnPlayerIndex = 0;
			*ppParam1 = NULL;
			*ppParam2 = NULL;
			KDBMsgNode * pNode = NULL;
			
			g_DBMsgListMutex.Lock();	
			pNode = (KDBMsgNode *)g_DBMsgList.GetHead();
			if (pNode)
			{
				*pnPlayerIndex = pNode->nPlayerIndex;
				*ppParam1 = pNode->pParam1;
				*ppParam2 = pNode->pParam2;
				*pnDBICommand = pNode->Command;
				g_DBMsgList.RemoveHead();
				delete pNode;
				bResult = TRUE;
			}
			g_DBMsgListMutex.Unlock();
		}break;
	}   
	
	return bResult;
	
}

#endif