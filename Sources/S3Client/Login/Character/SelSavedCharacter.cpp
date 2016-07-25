/*****************************************************************************************
//	存档角色选择
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-12
------------------------------------------------------------------------------------------
*****************************************************************************************/
#include <Winsock2.h>
#include <time.h>
#include <crtdbg.h>
#include "KEngine.h"
#include "SelSavedCharacter.h"
#include "../../Core/Src/CoreShell.h"
#include "../../Core/Src/KNpcRes.h"

#pragma comment (lib, "Ws2_32.lib")

extern iCoreShell*		g_pCoreShell;

static unsigned gs_holdrand = time(NULL);

static inline unsigned _Rand()
{
    gs_holdrand = gs_holdrand * 244213L + 1541021L;
     
    return gs_holdrand;
}

void RandMemSet(int nSize, unsigned char *pbyBuffer)
{
    _ASSERT(nSize);
    _ASSERT(pbyBuffer);

    while (nSize--)
    {
        *pbyBuffer++ = (unsigned char)_Rand();
    }
}
//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KSelSavedCharacter::KSelSavedCharacter()
{
	m_AccountName[0] = 0;
	m_szProcessingRoleName[0] = 0;
	m_Status = SSC_S_IDLE;
	m_nNumCharacter = 0;
	m_nRequestTime = 0;
	m_nLastOperResult = SSC_R_NOTHING;
}

KSelSavedCharacter::~KSelSavedCharacter()
{
	FreeData();
}

//--------------------------------------------------------------------------
//	功能：开始载入角色数据
//--------------------------------------------------------------------------
int KSelSavedCharacter::LoadData()
{
	if (m_Status != SSC_S_IDLE)
		return false;
	g_NetConnectAgent.UpdateClientRequestTime(false);
	m_Status = SSC_S_LOADING_DATA;
	return true;
}

//--------------------------------------------------------------------------
//	功能：返回值为梅举SSC_STATUS的取值之一，返回值含义请看相关的值声明
//--------------------------------------------------------------------------
int	KSelSavedCharacter::GetStatus()
{
	return ((int)m_Status);
}

//--------------------------------------------------------------------------
//	功能：设置最后一次操作的返回结果
//--------------------------------------------------------------------------
void KSelSavedCharacter::SetLastActionResult(int nResult)
{
	m_nLastOperResult = nResult;
}

//--------------------------------------------------------------------------
//	功能：得到最后一次操作的返回结果
//--------------------------------------------------------------------------
int	KSelSavedCharacter::GetLastActionResult()
{
	int	nRet = m_nLastOperResult;
	m_nLastOperResult = SSC_R_NOTHING;
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：获取角色的数目
//--------------------------------------------------------------------------
int KSelSavedCharacter::GetCharacterNum()
{
	return m_nNumCharacter;
}

//--------------------------------------------------------------------------
//	功能：获取某个角色的信息
//--------------------------------------------------------------------------
int	KSelSavedCharacter::GetCharacterInfo(int nIndex, KNewCharacterInfo* pInfo)
{
	if (nIndex >= 0 && nIndex < m_nNumCharacter)
	{
		if (pInfo)
		{
			strcpy(pInfo->Name, m_BaseInfo[nIndex].szName);
			pInfo->Gender = m_BaseInfo[nIndex].Sex;
			pInfo->Attribute = m_BaseInfo[nIndex].Series;
			pInfo->nLevel = m_BaseInfo[nIndex].Level;
		}
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
//	功能：请求新建一个角色
//--------------------------------------------------------------------------
int KSelSavedCharacter::NewCharacter(KNewCharacterInfo* pData)
{
	if (!pData)
		return false;
	if (pData->Gender < 0 || pData->Gender >= ROLE_NO || pData->Attribute < 0 || pData->Attribute >= series_num)
		return false;
	int nNameLen = strlen(pData->Name);
	if (nNameLen < 1)
		return false;

	char	Data[sizeof(TProcessData) + sizeof(NEW_PLAYER_COMMAND)];
	TProcessData*	pNetCommand = (TProcessData*)&Data;
	NEW_PLAYER_COMMAND* pInfo = (NEW_PLAYER_COMMAND*)pNetCommand->pDataBuffer;
	pInfo->m_btRoleNo = pData->Gender;
	pInfo->m_btSeries = pData->Attribute;
	pInfo->m_NativePlaceId = pData->NativePlaceId;
	memcpy(pInfo->m_szName, pData->Name, nNameLen);
	pInfo->m_szName[nNameLen] = '\0';
	
	strcpy(m_szProcessingRoleName, pData->Name);
	
	pNetCommand->nProtoId = c2s_newplayer;
	pNetCommand->nDataLen = sizeof(NEW_PLAYER_COMMAND) - sizeof(pInfo->m_szName) + nNameLen + 1/* sizeof( '\0' ) */;
	pNetCommand->ulIdentity = 0;

	g_NetConnectAgent.SendMsg(&Data, sizeof(TProcessData) - sizeof(pNetCommand->pDataBuffer) + pNetCommand->nDataLen);
	g_NetConnectAgent.UpdateClientRequestTime(false);

	m_Status = SSC_S_CREATING_CHARACTER;
	m_nLastOperResult = SSC_R_NOTHING;
	return true;
}

//--------------------------------------------------------------------------
//	功能：请求删除一个角色
//--------------------------------------------------------------------------
int KSelSavedCharacter::DeleteCharacter(int nIndex, const char* pszPassword)
{
	if (m_Status != SSC_S_STANDBY || nIndex < 0 || nIndex >= m_nNumCharacter || pszPassword == NULL)
		return false;

	tagDBDelPlayer	NetCommand;
	
	RandMemSet(sizeof(tagDBDelPlayer), (BYTE*)&NetCommand);	// random memory for make a cipher
	
	NetCommand.cProtocol = c2s_roleserver_deleteplayer;
	strcpy(NetCommand.szAccountName, m_AccountName);
	strcpy(NetCommand.szPassword, pszPassword);
	strncpy(NetCommand.szRoleName, m_BaseInfo[nIndex].szName, sizeof(NetCommand.szRoleName));
	g_NetConnectAgent.SendMsg(&NetCommand, sizeof(tagDBDelPlayer));
	memset(&NetCommand.szPassword, 0, sizeof(NetCommand.szPassword));
	g_NetConnectAgent.UpdateClientRequestTime(false);
	strcpy(m_szProcessingRoleName, m_BaseInfo[nIndex].szName);

	m_Status = SSC_S_DELETING_CHARACTER;
	m_nLastOperResult = SSC_R_NOTHING;
	return true;
}

//--------------------------------------------------------------------------
//	功能：选择某个角色
//--------------------------------------------------------------------------
int	KSelSavedCharacter::SelCharacter(int nIndex)
{
	if (m_Status != SSC_S_STANDBY || nIndex < 0 || nIndex >= m_nNumCharacter)
		return false;

	tagDBSelPlayer	NetCommand;
	NetCommand.cProtocol = c2s_dbplayerselect;
	strcpy(NetCommand.szRoleName, m_BaseInfo[nIndex].szName);
	g_NetConnectAgent.SendMsg(&NetCommand, sizeof(tagDBSelPlayer));
	g_NetConnectAgent.UpdateClientRequestTime(false);
	g_DebugLog("Send Select Message to Server");
	strcpy(m_szProcessingRoleName, m_BaseInfo[nIndex].szName);
	m_Status = SSC_S_LOADING_CHARACTER;
	m_nLastOperResult = SSC_R_NOTHING;
	return true;
}

//--------------------------------------------------------------------------
//	功能：释放角色数据
//--------------------------------------------------------------------------
void KSelSavedCharacter::FreeData()
{
	m_Status = SSC_S_IDLE;
	m_nNumCharacter = 0;
	m_nRequestTime = 0;
	m_nLastOperResult = SSC_R_NOTHING;
}

void KSelSavedCharacter::SetCharacterBaseInfo(int nNum, const RoleBaseInfo *pInfo)
{
	if (nNum > MAX_PLAYER_PER_ACCOUNT)
		nNum = MAX_PLAYER_PER_ACCOUNT;

	m_nNumCharacter = 0;
	for (int i = 0; i < nNum; i++)
	{
		if (pInfo[i].szName[0])
			m_nNumCharacter++;
		else
			break;
	}

	if (m_nNumCharacter > 0)
		memcpy(m_BaseInfo, pInfo, sizeof(RoleBaseInfo) * m_nNumCharacter);
	m_Status = SSC_S_STANDBY;
	m_nLastOperResult = SSC_R_UPDATE;
}

// -------------------------------------------------------------------------
// 函数		: KUiSelPlayer::AcceptNetMsg
// 功能		: 处理网络消息
// -------------------------------------------------------------------------
void KSelSavedCharacter::AcceptNetMsg(void* pMsgData)
{
	if (m_Status != SSC_S_STANDBY && m_Status != SSC_S_IDLE)
	{
		PROTOCOL_MSG_TYPE	eProtoId = *(PROTOCOL_MSG_TYPE*)pMsgData;
		switch(eProtoId)
		{
		case s2c_roleserver_getrolelist_result:
			if (m_Status == SSC_S_LOADING_DATA)
			{
				TProcessData* pProtocol = (TProcessData*)pMsgData;
				SetCharacterBaseInfo(pProtocol->pDataBuffer[0], (RoleBaseInfo*)&pProtocol->pDataBuffer[1]);
				g_NetConnectAgent.UpdateClientRequestTime(true);
			}
			break;
		case s2c_notifyplayerlogin:
			if (m_Status == SSC_S_LOADING_CHARACTER)
			{
				tagNotifyPlayerLogin* pNPL = (tagNotifyPlayerLogin*)pMsgData;
				if (strcmp((const char*)pNPL->szRoleName, m_szProcessingRoleName) == 0 && pNPL->bPermit)
				{
					// 开始与GameSvr进行连接
					if (g_NetConnectAgent.ConnectToGameSvr((const unsigned char*)&pNPL->nIPAddr, pNPL->nPort, &pNPL->guid))
					{
						g_NetConnectAgent.UpdateClientRequestTime(false);
						m_nLastOperResult = SSC_R_IN_PROGRESS;
					}
					else
					{
						m_nLastOperResult = SSC_R_FAILED;
						m_Status = SSC_S_STANDBY;
					}

					// 断开与网关的连接
					g_NetConnectAgent.DisconnectClient();
					g_NetConnectAgent.UpdateClientRequestTime(true);

				}
				else
				{
						m_nLastOperResult = SSC_R_SVR_DOWN;
						m_Status = SSC_S_STANDBY;
				}
			}
			break;
		case s2c_rolenewdelresponse:
			tagNewDelRoleResponse* pResponse = (tagNewDelRoleResponse*)pMsgData;
//			if (strcmp(pResponse->szRoleName, m_szProcessingRoleName) == 0)	/* 刘鹏调试版 */
			{
				if (m_Status == SSC_S_CREATING_CHARACTER)
				{
					if (pResponse->bSucceeded)
					{
						g_NetConnectAgent.UpdateClientRequestTime(false);
						m_nLastOperResult = SSC_R_CREATE_ROLE_SUCCEED;
						m_Status = SSC_S_LOADING_CHARACTER;
					}
					else
					{
						g_NetConnectAgent.UpdateClientRequestTime(true);
						m_nLastOperResult = SSC_R_INVALID_ROLENAME;
						m_Status = SSC_S_STANDBY;
					}
				}
				else if (m_Status == SSC_S_DELETING_CHARACTER)
				{
					g_NetConnectAgent.UpdateClientRequestTime(true);
					if (pResponse->bSucceeded)
					{
						m_nLastOperResult = SSC_R_UPDATE;

						for (int i = 0; i < m_nNumCharacter; i++)
						{
							if (strcmp(m_BaseInfo[i].szName, m_szProcessingRoleName) == 0)
							{
								m_nNumCharacter--;
								for (; i < m_nNumCharacter; i++)
									m_BaseInfo[i] = m_BaseInfo[i + 1];
								break;
							}
						}
					}
					else
					{
						m_nLastOperResult = SSC_R_FAILED;
					}
					m_Status = SSC_S_STANDBY;
				}
			}
			break;
		}
	}
}

void KSelSavedCharacter::SetAccountName(const char* pAccount)
{
	if (pAccount)
		strcpy(m_AccountName, pAccount);
}