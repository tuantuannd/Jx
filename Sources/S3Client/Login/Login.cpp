/************************************************ *****************************************
//	界面--login窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-13
------------------------------------------------------------------------------------------
*****************************************************************************************/
#include "KWin32.h"
#include "KEngine.h"
#include "LoginDef.h"
#include "Login.h"
#include "../NetConnect/NetConnectAgent.h"
#include "KProtocol.h"
#include "crtdbg.h"
#include "../Ui/UiBase.h"
#include "../Ui/UiShell.h"
#include "../../Engine/Src/Cryptography/EDOneTimePad.h"
#include "time.h"

#define	SERVER_LIST_FILE				"\\Settings\\ServerList.ini"

KLogin		g_LoginLogic;


bool GetIpAddress(const char* szAddress, unsigned char* pcAddress)
{
	_ASSERT(pcAddress);
	int nValue[4];
	int nRet = sscanf(szAddress, "%d.%d.%d.%d", &nValue[0], &nValue[1], &nValue[2], &nValue[3]);
	if (nRet == 4 &&
		nValue[0] >= 0 && nValue[0] < 256 &&
		nValue[1] >= 0 && nValue[1] < 256 &&
		nValue[2] >= 0 && nValue[2] < 256 &&
		nValue[3] >= 0 && nValue[3] < 256)
	{
		pcAddress[0] = nValue[0];
		pcAddress[1] = nValue[1];
		pcAddress[2] = nValue[2];
		pcAddress[3] = nValue[3];
		return true;
	}
	return false;
}
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
KLogin::KLogin()
{
	m_Status = LL_S_IDLE;
	m_Result = LL_R_NOTHING;
	m_bInAutoProgress = false;
	m_nNumRole = 0;
	memset(&m_Choices, 0, sizeof(LOGIN_CHOICE));
	ClearAccountPassword(true, true);
	m_LeftTime = 0;
}

void KLogin::ClearAccountPassword(bool bAccount, bool bPassword)
{
	if (bAccount)
		memset(m_Choices.Account, 0xff, sizeof(m_Choices.Account));
	if (bPassword)
		memset(&m_Choices.Password, 0xff, sizeof(m_Choices.Password));
}

//--------------------------------------------------------------------------
//	功能：析构函数
//--------------------------------------------------------------------------
KLogin::~KLogin()
{
	_ASSERT(m_Status == LL_S_IDLE);
}

//--------------------------------------------------------------------------
//	功能：与（账号）服务器建立连接
//	状态切换：成功 LL_S_IDLE -> LL_S_WAIT_INPUT_ACCOUNT
//			  失败 状态保持
//--------------------------------------------------------------------------
int KLogin::CreateConnection(const unsigned char* pAddress)
{
	int nRet;
	if (m_Status == LL_S_IDLE && pAddress &&
		ConnectAccountServer(pAddress))
	{
		RegistNetAgent();
		m_Status = LL_S_WAIT_INPUT_ACCOUNT;
		m_Result = LL_R_NOTHING;

		if (m_bInAutoProgress)
		{
			char	szAccount[32];
            KSG_PASSWORD Password;
			GetAccountPassword(szAccount, &Password);
			AccountLogin(szAccount, Password, false);
			memset(szAccount, 0, sizeof(szAccount));
			memset(&Password, 0, sizeof(Password));
		}
		nRet = true;
	}
	else
	{
		if (m_bInAutoProgress)
			m_bInAutoProgress = false;
		m_Result = LL_R_CONNECT_FAILED;
		nRet = false;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：传入帐号密码，账号登陆
//	状态切换：成功 LL_S_WAIT_INPUT_ACCOUNT -> LL_S_ACCOUNT_CONFIRMING
//			  失败 状态保持
//--------------------------------------------------------------------------
int	KLogin::AccountLogin(const char* pszAccount, const KSG_PASSWORD& crPassword, bool bOrignPassword)
{
	int nRet;
	if (m_Status == LL_S_WAIT_INPUT_ACCOUNT &&
		pszAccount && 
		Request(pszAccount, &crPassword, LOGIN_A_LOGIN))
	{
        if (bOrignPassword)
        {
    		SetAccountPassword(pszAccount, &crPassword);
        }
		m_Status = LL_S_ACCOUNT_CONFIRMING;
		m_Result = LL_R_NOTHING;
		nRet = true;
	}
	else
	{
		if (m_bInAutoProgress)
			m_bInAutoProgress = false;
		m_Result = LL_R_CONNECT_FAILED;
		nRet = false;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：选中游戏角色
//	状态切换：成功 LL_S_ROLE_LIST_READY -> LL_S_WAIT_TO_LOGIN_GAMESERVER
//			  失败 状态保持
//--------------------------------------------------------------------------
int	KLogin::SelectRole(int nIndex)
{
	int nRet;
	if (m_Status == LL_S_ROLE_LIST_READY && nIndex >= 0 && nIndex < m_nNumRole)
	{
		tagDBSelPlayer	NetCommand;
		NetCommand.cProtocol = c2s_dbplayerselect;
		strcpy(NetCommand.szRoleName, m_RoleList[nIndex].Name);
		g_NetConnectAgent.SendMsg(&NetCommand, sizeof(tagDBSelPlayer));
		g_NetConnectAgent.UpdateClientRequestTime(false);
		strcpy(m_Choices.szProcessingRoleName, NetCommand.szRoleName);
		m_Status = LL_S_WAIT_TO_LOGIN_GAMESERVER;
		m_Result = LL_R_NOTHING;
		nRet = true;
	}
	else
	{
		if (m_bInAutoProgress)
			m_bInAutoProgress = false;
		m_Result = LL_R_CONNECT_FAILED;
		nRet = false;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：请求新建游戏角色
//	状态切换：成功 LL_S_ROLE_LIST_READY -> LL_S_CREATING_ROLE
//			  失败 状态保持
//--------------------------------------------------------------------------
int	KLogin::CreateRole(KRoleChiefInfo* pCreateInfo)
{
	int nRet = false;
	m_Result = LL_R_CONNECT_FAILED;

	if (m_Status == LL_S_ROLE_LIST_READY && pCreateInfo && m_nNumRole < MAX_PLAYER_PER_ACCOUNT &&
		pCreateInfo->Attribute >= 0 && pCreateInfo->Attribute < series_num)
	{
		int nNameLen = strlen(pCreateInfo->Name);
		if (nNameLen >= 1 && nNameLen < sizeof(pCreateInfo->Name))
		{
			char	Data[sizeof(TProcessData) + sizeof(NEW_PLAYER_COMMAND)];
			TProcessData*	pNetCommand = (TProcessData*)&Data;
			NEW_PLAYER_COMMAND* pInfo = (NEW_PLAYER_COMMAND*)pNetCommand->pDataBuffer;
			pInfo->m_btRoleNo = pCreateInfo->Gender;
			pInfo->m_btSeries = pCreateInfo->Attribute;
			pInfo->m_NativePlaceId = pCreateInfo->NativePlaceId;
			memcpy(pInfo->m_szName, pCreateInfo->Name, nNameLen);
			pInfo->m_szName[nNameLen] = '\0';

			pNetCommand->nProtoId = c2s_newplayer;
			pNetCommand->nDataLen = sizeof(NEW_PLAYER_COMMAND) - sizeof(pInfo->m_szName) + nNameLen + 1/* sizeof( '\0' ) */;
			pNetCommand->ulIdentity = 0;

			g_NetConnectAgent.SendMsg(&Data, sizeof(TProcessData) - sizeof(pNetCommand->pDataBuffer) + pNetCommand->nDataLen);
			g_NetConnectAgent.UpdateClientRequestTime(false);

			memcpy(m_Choices.szProcessingRoleName, pCreateInfo->Name, nNameLen);
			m_Choices.szProcessingRoleName[nNameLen] = 0;

			m_Status = LL_S_CREATING_ROLE;
			m_Result = LL_R_NOTHING;
			nRet = true;
		}
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：请求删除游戏角色
//	状态切换：成功 LL_S_ROLE_LIST_READY -> LL_S_DELETING_ROLE
//			  失败 状态保持
//--------------------------------------------------------------------------
int	KLogin::DeleteRole(int nIndex, const KSG_PASSWORD &crSupperPassword)
{
	int nRet;

	if (m_Status == LL_S_ROLE_LIST_READY && nIndex >= 0 && nIndex < m_nNumRole)
	{
		tagDBDelPlayer	NetCommand;
		RandMemSet(sizeof(tagDBDelPlayer), (BYTE*)&NetCommand);	// random memory for make a cipher

		NetCommand.cProtocol = c2s_roleserver_deleteplayer;
		GetAccountPassword(NetCommand.szAccountName, NULL);
        NetCommand.Password = crSupperPassword;
		strncpy(NetCommand.szRoleName, m_RoleList[nIndex].Name, sizeof(NetCommand.szRoleName));
        NetCommand.szRoleName[sizeof(NetCommand.szRoleName) - 1] = '\0';

		g_NetConnectAgent.SendMsg(&NetCommand, sizeof(tagDBDelPlayer));
		memset(&NetCommand.Password, 0, sizeof(NetCommand.Password));
		g_NetConnectAgent.UpdateClientRequestTime(false);

		strcpy(m_Choices.szProcessingRoleName, m_RoleList[nIndex].Name);

		m_Status = LL_S_DELETING_ROLE;
		m_Result = LL_R_NOTHING;
		nRet = true;
	}
	else
	{
		nRet = false;
		m_Result = LL_R_CONNECT_FAILED;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：通知等待返回结果超时了
//	状态切换：成功 LL_S_??? -> LL_S_IDLE
//--------------------------------------------------------------------------
void KLogin::NotifyTimeout()
{
	if (m_Status != LL_S_IDLE)
	{
		ReturnToIdle();
		m_Result = LL_R_CONNECT_TIMEOUT;
	}
}

//通知网络连接（意外）断开了
void KLogin::NotifyDisconnect()
{
	if (m_Status != LL_S_IDLE)
	{
		ReturnToIdle();
		m_Result = LL_R_CONNECT_FAILED;
	}
}

//--------------------------------------------------------------------------
//	功能：通知要开始游戏了
//	状态切换：成功 LL_S_ENTERING_GAME -> LL_S_IN_GAME
//--------------------------------------------------------------------------
void KLogin::NotifyToStartGame()
{
	if (m_Status == LL_S_ENTERING_GAME)
	{
		g_NetConnectAgent.UpdateClientRequestTime(true);

		char	szAccount[32];
		GetAccountPassword(szAccount, NULL);
		g_UiBase.SetUserAccount(szAccount, m_Choices.szProcessingRoleName);

		m_Status = LL_S_IN_GAME;
		m_Result = LL_R_NOTHING;
		if (m_bInAutoProgress)
			m_bInAutoProgress = false;
		UiOnGameServerStartSyncEnd();
	}
}

//--------------------------------------------------------------------------
//	功能：回到空闲状态
//	状态切换：LL_S_??? -> LL_S_IN_GAME
//--------------------------------------------------------------------------
void KLogin::ReturnToIdle()
{
	if (m_Status != LL_S_IDLE)
	{
		UnRegistNetAgent();
		g_NetConnectAgent.DisconnectGameSvr();
		g_NetConnectAgent.DisconnectClient();
		m_Status = LL_S_IDLE;
	}
	m_Choices.bIsRoleNewCreated = false;
	m_Result = LL_R_NOTHING;
	m_bInAutoProgress = false;
}

//--------------------------------------------------------------------------
//	功能：全程自动连接
//--------------------------------------------------------------------------
void KLogin::AutoLogin()
{
	ReturnToIdle();
	if (IsAutoLoginEnable())
	{
		m_bInAutoProgress = true;
		if (m_Choices.AccountServer.Address[0] == 0 &&
			m_Choices.AccountServer.Address[1] == 0 &&
			m_Choices.AccountServer.Address[2] == 0 &&
			m_Choices.AccountServer.Address[3] == 0)
		{
			int nCount, nSel;
			KLoginServer* pList = GetServerList(-1, nCount, nSel);
			if (pList)
			{
				free(pList);
				pList = NULL;
			}
		}
		CreateConnection(m_Choices.AccountServer.Address);
	}
}

//--------------------------------------------------------------------------
//	功能：判断是否可以执行全程自动连接
//--------------------------------------------------------------------------
int	KLogin::IsAutoLoginEnable()
{
	return ((~m_Choices.Account[0]) &&
		(~m_Choices.Password.szPassword[0]) &&
		m_Choices.szProcessingRoleName[0] &&
		m_Choices.AccountServer.Title[0]);
}

//设置纪录标记
void KLogin::SetRememberAccountFlag(bool bEnable)
{
	m_Choices.bRememberAccount = bEnable;
	if (bEnable == false)
		m_Choices.bRememberAll = false;		
}

//设置纪录标记
void KLogin::SetRememberAllFlag(bool bEnable)
{
	m_Choices.bRememberAll = bEnable;
	if (bEnable)
		m_Choices.bRememberAccount = true;
}


//--------------------------------------------------------------------------
//	功能：获取某个角色的信息
//--------------------------------------------------------------------------
int	KLogin::GetRoleInfo(int nIndex, KRoleChiefInfo* pInfo)
{
	if (nIndex >= 0 && nIndex < m_nNumRole)
	{
		if (pInfo)
			*pInfo = m_RoleList[nIndex];
		return true;
	}
	return false;
}


//--------------------------------------------------------------------------
//	功能：处理账号登陆的响应
//	状态切换：成功 LL_S_ACCOUNT_CONFIRMING -> LL_S_WAIT_ROLE_LIST
//			  失败 LL_S_ACCOUNT_CONFIRMING -> LL_S_IDLE
//--------------------------------------------------------------------------
void KLogin::ProcessAccountLoginResponse(KLoginStructHead* pResponse)
{
	//_ASSERT(m_Status == LL_S_ACCOUNT_CONFIRMING && pResponse != NULL);

	if (((pResponse->Param & LOGIN_ACTION_FILTER) == LOGIN_A_LOGIN) &&	//操作性为要匹配
		pResponse->Size >= sizeof(KLoginAccountInfo))				//数据内容的大小也要匹配
	{
		KLoginAccountInfo* pInfo = (KLoginAccountInfo*)pResponse;
		char	szAccount[32];
        KSG_PASSWORD Password;
		GetAccountPassword(szAccount, &Password);
		if (strcmp(pInfo->Account,  szAccount)  == 0 &&
			strcmp(pInfo->Password.szPassword, Password.szPassword) == 0)
		{			
			int nResult = ((pResponse->Param) & ~LOGIN_ACTION_FILTER);
			if (nResult == LOGIN_R_SUCCESS)
			{
				g_NetConnectAgent.UpdateClientRequestTime(false);
				m_Status = LL_S_WAIT_ROLE_LIST;
				m_Result = LL_R_ACCOUNT_CONFIRM_SUCCESS;
				m_LeftTime = pInfo->nLeftTime;
			}
			else
			{
				LOGIN_LOGIC_RESULT_INFO eResult = LL_R_NOTHING;
				switch(nResult)
				{
				case LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR:
					eResult = LL_R_ACCOUNT_PWD_ERROR;
					m_Status = LL_S_WAIT_INPUT_ACCOUNT;
					break;
				case LOGIN_R_ACCOUNT_EXIST:
					eResult = LL_R_ACCOUNT_LOCKED;
					m_Status = LL_S_WAIT_INPUT_ACCOUNT;
					break;
				case LOGIN_R_FREEZE:
					eResult = LL_R_ACCOUNT_FREEZE;
					m_Status = LL_S_WAIT_INPUT_ACCOUNT;
					break;
				case LOGIN_R_INVALID_PROTOCOLVERSION:
					eResult = LL_R_INVALID_PROTOCOLVERSION;
					break;
				case LOGIN_R_FAILED:
					eResult = LL_R_CONNECT_SERV_BUSY;
					break;
				case LOGIN_R_TIMEOUT:
					eResult = LL_R_ACCOUNT_NOT_ENOUGH_POINT;
					m_Status = LL_S_WAIT_INPUT_ACCOUNT;
					break;
				default:
					eResult = LL_R_CONNECT_FAILED;
					break;
				}
				if (m_Status != LL_S_WAIT_INPUT_ACCOUNT ||
					m_bInAutoProgress)
				{
					ReturnToIdle();
				}
				m_Result = eResult;
			}
		}
		memset(szAccount, 0, sizeof(szAccount));
		memset(&Password, 0, sizeof(Password));
	}
}

//--------------------------------------------------------------------------
//	功能：角色列表返回
//	状态切换：成功 LL_S_WAIT_ROLE_LIST -> LL_S_ROLE_LIST_READY
//			  失败 状态保持
//--------------------------------------------------------------------------
void KLogin::ProcessRoleListResponse(TProcessData* pResponse)
{
	//_ASSERT(m_Status == LL_S_WAIT_ROLE_LIST && pResponse != NULL);
	if (pResponse->nProtoId == s2c_roleserver_getrolelist_result)
	{
		m_nNumRole = pResponse->pDataBuffer[0];
		if (m_nNumRole > MAX_PLAYER_PER_ACCOUNT)
			m_nNumRole = MAX_PLAYER_PER_ACCOUNT;
		else if (m_nNumRole < 0)
			m_nNumRole = 0;

		RoleBaseInfo* pList = (RoleBaseInfo*)&pResponse->pDataBuffer[1];
		for (int i = 0; i < m_nNumRole; i++)
		{
			if (pList->szName[0])
			{
				strcpy(m_RoleList[i].Name, pList->szName);
				m_RoleList[i].Attribute = pList->Series;
				m_RoleList[i].Gender = pList->Sex;
				m_RoleList[i].nLevel = pList->Level;
				pList ++;
			}
			else
			{
				m_nNumRole = i;
				break;
			}
		}

		g_NetConnectAgent.UpdateClientRequestTime(true);
		m_Status = LL_S_ROLE_LIST_READY;
		m_Result = LL_R_NOTHING;

		if (m_bInAutoProgress)
		{
			int nAdviceChoice;
			GetRoleCount(nAdviceChoice);
			SelectRole(nAdviceChoice);
		}
	}
}

//--------------------------------------------------------------------------
//	功能：新建角色列表的响应
//	状态切换：LL_S_CREATING_ROLE -> LL_S_ROLE_LIST_READY
//--------------------------------------------------------------------------
void KLogin::ProcessCreateRoleResponse(tagNewDelRoleResponse* pResponse)
{
	//_ASSERT(m_Status == LL_S_DELETING_ROLE && pResponse != NULL);
	if (pResponse->cProtocol == s2c_rolenewdelresponse)
	{
	//	if (strcmp(pResponse->szRoleName, m_Choices.szProcessingRoleName) == 0)	//to be check/* 刘鹏调试版 */
		{
			if (pResponse->bSucceeded)
			{
				g_NetConnectAgent.UpdateClientRequestTime(false);
				m_Choices.bIsRoleNewCreated = true;
				m_Status = LL_S_WAIT_TO_LOGIN_GAMESERVER;
				m_Result = LL_R_CREATE_ROLE_SUCCESS;
			}
			else
			{
				g_NetConnectAgent.UpdateClientRequestTime(true);
				m_Status = LL_S_ROLE_LIST_READY;
				m_Result = LL_R_INVALID_ROLENAME;
			}
		}
	}
}

//--------------------------------------------------------------------------
//	功能：删除角色列表的响应
//	状态切换：LL_S_DELETING_ROLE -> LL_S_ROLE_LIST_READY
//--------------------------------------------------------------------------
void KLogin::ProcessDeleteRoleResponse(tagNewDelRoleResponse* pResponse)
{
	//_ASSERT(m_Status == LL_S_DELETING_ROLE && pResponse != NULL);
	if (pResponse->cProtocol == s2c_rolenewdelresponse)
	{
//		if (strcmp(pResponse->szRoleName, m_Choices.szProcessingRoleName) == 0)	//to be check/* 刘鹏调试版 */
		{
			g_NetConnectAgent.UpdateClientRequestTime(true);
			m_Status = LL_S_ROLE_LIST_READY;
			if (pResponse->bSucceeded)
			{
				char	szAccount[32];
				GetAccountPassword(szAccount, NULL);
				g_UiBase.SetUserAccount(szAccount, m_Choices.szProcessingRoleName);
				g_UiBase.CleanPrivateDataFolder();

				for (int i = 0; i < m_nNumRole; i++)
				{
					if (strcmp(m_RoleList[i].Name, m_Choices.szProcessingRoleName) == 0)
					{
						m_nNumRole--;
						for (; i < m_nNumRole; i++)
							m_RoleList[i] = m_RoleList[i + 1];
						break;
					}
				}
				m_Result = LL_R_NOTHING;
			}
			else
			{
				m_Result = LL_R_INVALID_PASSWORD;
			}
		}
	}
}

//--------------------------------------------------------------------------
//	功能：准备登陆游戏服务器的响应
//	状态切换：LL_S_WAIT_TO_LOGIN_GAMESERVER -> LL_S_ENTERING_GAME
//--------------------------------------------------------------------------
void KLogin::ProcessToLoginGameServResponse(tagNotifyPlayerLogin* pResponse)
{
	//_ASSERT(m_Status == LL_S_WAIT_TO_LOGIN_GAMESERVER && pResponse != NULL);
	if (pResponse->cProtocol == s2c_notifyplayerlogin)
	{
		if (strcmp((const char*)pResponse->szRoleName, m_Choices.szProcessingRoleName) == 0)
		{
			g_NetConnectAgent.UpdateClientRequestTime(true);

			// 开始与GameSvr进行连接
			if (g_NetConnectAgent.ConnectToGameSvr(
				(const unsigned char*)&pResponse->nIPAddr,
				pResponse->nPort, &pResponse->guid))
			{
				m_Status = LL_S_ENTERING_GAME;
				m_Result = LL_R_NOTHING;
			}
			else
			{
				ReturnToIdle();
				m_Result = LL_R_CONNECT_FAILED;
			}

			// 断开与网关的连接
			g_NetConnectAgent.DisconnectClient();
		}
		else
		{
			ReturnToIdle();
			m_Result = LL_R_SERVER_SHUTDOWN;
		}
	}
}

//--------------------------------------------------------------------------
//	功能：接受网络消息
//--------------------------------------------------------------------------
void KLogin::AcceptNetMsg(void* pMsgData)
{
	if (pMsgData == NULL)
		return;

	switch(m_Status)
	{
	case LL_S_ACCOUNT_CONFIRMING:
		ProcessAccountLoginResponse((KLoginStructHead*) (((char*)pMsgData) + PROTOCOL_MSG_SIZE));
		break;
	case LL_S_WAIT_ROLE_LIST:
		ProcessRoleListResponse((TProcessData*)pMsgData);
		break;
	case LL_S_CREATING_ROLE:
		ProcessCreateRoleResponse((tagNewDelRoleResponse*)pMsgData);
		break;
	case LL_S_DELETING_ROLE:
		ProcessDeleteRoleResponse((tagNewDelRoleResponse*)pMsgData);
		break;
	case LL_S_WAIT_TO_LOGIN_GAMESERVER:
		ProcessToLoginGameServResponse((tagNotifyPlayerLogin*)pMsgData);
		break;
	}
}

//获取操作的结果信息
LOGIN_LOGIC_RESULT_INFO KLogin::GetResult()
{
	LOGIN_LOGIC_RESULT_INFO eReturn = m_Result;
	m_Result = LL_R_NOTHING;
	return eReturn;
}


//获取角色的数目
int KLogin::GetRoleCount(int& nAdviceChoice)
{
	nAdviceChoice = 0;
	if (m_Choices.szProcessingRoleName[0])
	{
		for (int i = 0; i < m_nNumRole; i++)
		{
			if (strcmp(m_Choices.szProcessingRoleName, m_RoleList[i].Name) == 0)
			{
				nAdviceChoice = i;
				break;
			}
		}
	}
	return m_nNumRole;
}

//--------------------------------------------------------------------------
//	功能：获取建议（旧的）登陆账号
//--------------------------------------------------------------------------
bool KLogin::GetLoginAccount(char* pszAccount)
{
	if (pszAccount)
		GetAccountPassword(pszAccount, NULL);
	return m_Choices.bRememberAccount;
}

#define	$LOGIN			"Login"
#define	$LAST_ACCOUNT	"LastAccount"
#define	$LAST_PASSWORD	"LastPassword"

//--------------------------------------------------------------------------
//	功能：读取以前的的登陆选择
//--------------------------------------------------------------------------
void KLogin::LoadLoginChoice()
{
	if (m_Choices.bLoaded)
		return;
	memset(&m_Choices, 0, sizeof(m_Choices));
	ClearAccountPassword(true, true);

	m_Choices.bLoaded = true;

	KIniFile* pSetting = g_UiBase.GetCommSettingFile();
	char	szAccount[32];
    KSG_PASSWORD Password;
	if (pSetting)
	{
		pSetting->GetInteger($LOGIN, "SelServerRegion", 0, &m_Choices.nServerRegionIndex);
		pSetting->GetString($LOGIN, "LastGameServer", "", m_Choices.AccountServer.Title, sizeof(m_Choices.AccountServer.Title));

		szAccount[0] = 0;
		pSetting->GetStruct($LOGIN, $LAST_ACCOUNT, szAccount, sizeof(szAccount));
		if (szAccount[0])
		{
			EDOneTimePad_Decipher(szAccount, strlen(szAccount));
			m_Choices.bRememberAccount = true;
			SetAccountPassword(szAccount, NULL);

			Password.szPassword[0] = '\0';
			pSetting->GetStruct($LOGIN, $LAST_PASSWORD, Password.szPassword, sizeof(Password.szPassword));
			if (Password.szPassword[0])
			{
				EDOneTimePad_Decipher(Password.szPassword, strlen(Password.szPassword));
				m_Choices.bRememberAll = true;
				SetAccountPassword(NULL, &Password);
				memset(&Password, 0, sizeof(Password));
			}
		}

		if (szAccount[0])
		{
			KIniFile* pPrivate = g_UiBase.GetPrivateSettingFile();
			if (pPrivate)
			{
				if (pPrivate->GetString("Main", "LastSelCharacter", "",
					m_Choices.szProcessingRoleName, sizeof(m_Choices.szProcessingRoleName)))
				{
					EDOneTimePad_Decipher(m_Choices.szProcessingRoleName, strlen(m_Choices.szProcessingRoleName));
				}
			}
			g_UiBase.ClosePrivateSettingFile(false);
		}

		g_UiBase.CloseCommSettingFile(false);
	}
}

//--------------------------------------------------------------------------
//	功能：保存登陆选择设置
//--------------------------------------------------------------------------
void KLogin::SaveLoginChoice()
{

	KIniFile*	pSetting = g_UiBase.GetCommSettingFile();
	int	i;
	if (pSetting)
	{
		//----纪录选择的服务器----
		pSetting->WriteInteger($LOGIN, "SelServerRegion", m_Choices.nServerRegionIndex);
		if (m_Choices.AccountServer.Title[0])
		{
			pSetting->WriteString($LOGIN, "LastGameServer", m_Choices.AccountServer.Title);
		}

		char	szBuffer[32];
		//----纪录最后一次登陆账号----
		pSetting->EraseKey($LOGIN, $LAST_ACCOUNT);

		if (m_Choices.bRememberAccount)
		{
			GetAccountPassword(szBuffer, NULL);
			i = strlen(szBuffer);
			EDOneTimePad_Encipher(szBuffer, i);
			pSetting->WriteStruct($LOGIN, $LAST_ACCOUNT, szBuffer, sizeof(szBuffer));

			if (m_Choices.bRememberAll)
			{
                KSG_PASSWORD Password;
				GetAccountPassword(NULL, &Password);
				i = strlen(Password.szPassword);
				EDOneTimePad_Encipher(Password.szPassword, i);
				pSetting->WriteStruct($LOGIN, $LAST_PASSWORD, Password.szPassword, sizeof(Password.szPassword));
			}

			KIniFile*	pPrivate = g_UiBase.GetPrivateSettingFile();
			if (pPrivate)
			{
				if (m_Choices.szProcessingRoleName[0])
				{
					i = strlen(m_Choices.szProcessingRoleName);
					memcpy(szBuffer, m_Choices.szProcessingRoleName, i);
					szBuffer[i] = 0;
					EDOneTimePad_Encipher(szBuffer, i);
					pPrivate->WriteString("Main", "LastSelCharacter", szBuffer);
				}
				g_UiBase.ClosePrivateSettingFile(true);
			}
		}

		g_UiBase.CloseCommSettingFile(true);
	}
}

//--------------------------------------------------------------------------
//	功能：获取服务器区域的列表
//--------------------------------------------------------------------------
KLoginServer*	KLogin::GetServerRegionList(int& nCount, int& nAdviceChoice)
{
	KLoginServer* pServers = NULL;
	nCount = 0;
	nAdviceChoice = 0;

	KIniFile	File;
	int			i;
	if (File.Load(SERVER_LIST_FILE))
	{
		int		nReadCount = 0;
		char	szKey[32];
		File.GetInteger("List", "RegionCount", 0, &nReadCount);
		if (nReadCount > 0)
		{
			pServers = (KLoginServer*)malloc(sizeof(KLoginServer) * nReadCount);
			if (pServers)
			{
				for (i = 0; i < nReadCount; i++)
				{
					sprintf(szKey, "Region_%d", i);
					if (File.GetString("List", szKey, "", pServers[nCount].Title,
						sizeof(pServers[nCount].Title)) &&
						pServers[nCount].Title[0])
					{
						nCount ++;
					}
				}
				if (nCount == 0)
				{
					free(pServers);
					pServers = NULL;
				}
			}
		}
	}

	if (m_Choices.nServerRegionIndex < 0 || m_Choices.nServerRegionIndex >= nCount)
		m_Choices.nServerRegionIndex = 0;
	nAdviceChoice = m_Choices.nServerRegionIndex;
	return pServers;
}

//--------------------------------------------------------------------------
//	功能：登陆服务器列表获取
//--------------------------------------------------------------------------
KLoginServer* KLogin::GetServerList(int nRegion, int& nCount, int& nAdviceChoice)
{
	KLoginServer* pServers = NULL;
	nCount = 0;
	nAdviceChoice = 0;

	if (nRegion < 0)
	{
		KIniFile* pSetting = g_UiBase.GetCommSettingFile();
		if (pSetting)
		{
			pSetting->GetInteger($LOGIN, "SelServerRegion", 0, &nRegion);
			g_UiBase.CloseCommSettingFile(false);
		}
	}

	KIniFile	File;
	int			i;
	if (File.Load(SERVER_LIST_FILE))
	{
		int		nReadCount = 0;
		char	szSection[32], szKey[32], szBuffer[32];

		File.GetInteger("List", "RegionCount", 0, &nReadCount);	//区域的数目

		if (nReadCount > 0 || nRegion >= 0 && nRegion < nReadCount)
		{
			m_Choices.nServerRegionIndex = nRegion;
			sprintf(szSection, "Region_%d", nRegion);
			File.GetInteger(szSection, "Count", 0, &nReadCount);	//该区域服务器的数目
			if (nReadCount > 0)
			{
				pServers = (KLoginServer*)malloc(sizeof(KLoginServer) * nReadCount);
				if (pServers)
				{
					for (i = 0; i < nReadCount; i++)
					{
						sprintf(szKey, "%d_Address", i);
						if (!File.GetString(szSection, szKey, "", szBuffer, sizeof(szBuffer)) ||
							GetIpAddress(szBuffer, pServers[nCount].Address) == false)
						{
							continue;
						}
						sprintf(szKey, "%d_Title", i);
						if (File.GetString(szSection, szKey, "", pServers[nCount].Title,
							sizeof(pServers[nCount].Title)) &&
							pServers[nCount].Title[0])
						{
							nCount ++;
						}
					}
					if (nCount == 0)
					{
						free(pServers);
						pServers = NULL;
					}
				}
			}
		}
	}

	if (nCount)
	{
		for (i = 0; i < nCount; i++)
		{
			if (strcmp(pServers[i].Title, m_Choices.AccountServer.Title) == 0)
			{
				nAdviceChoice = i;
				break;
			}
		}
		if (i >= nCount)
			strcpy(m_Choices.AccountServer.Title, pServers[nAdviceChoice].Title);
		m_Choices.AccountServer.Address[0] = pServers[nAdviceChoice].Address[0];
		m_Choices.AccountServer.Address[1] = pServers[nAdviceChoice].Address[1];
		m_Choices.AccountServer.Address[2] = pServers[nAdviceChoice].Address[2];
		m_Choices.AccountServer.Address[3] = pServers[nAdviceChoice].Address[3];
	}
	return pServers;
}

int KLogin::SetAccountServer(const KLoginServer &rcSelectServer)
{
    m_Choices.AccountServer = rcSelectServer;
    
    return true;
}


extern void RandMemSet(int nSize, unsigned char *pbyBuffer);
//--------------------------------------------------------------------------
//	功能：申请账号
//--------------------------------------------------------------------------
int KLogin::Request(const char* pszAccount, const KSG_PASSWORD* pcPassword, int nAction)
{
	BYTE		Buff[sizeof(KLoginAccountInfo) + PROTOCOL_MSG_SIZE];

	RandMemSet(sizeof(Buff), (BYTE*)Buff);		// random memory for make a cipher

	if (pszAccount && pcPassword)
	{
		(*(PROTOCOL_MSG_TYPE*)Buff) = c2s_login;
		KLoginAccountInfo* pInfo = (KLoginAccountInfo*)&Buff[PROTOCOL_MSG_SIZE];
		pInfo->Size  = sizeof(KLoginAccountInfo);
		pInfo->Param = nAction | LOGIN_R_REQUEST;
		strncpy(pInfo->Account,  pszAccount, sizeof(pInfo->Account));
        pInfo->Account[sizeof(pInfo->Account) - 1] = '\0';
		pInfo->Password = *pcPassword;

        #ifdef USE_KPROTOCOL_VERSION
        // Add by Freeway Chen in 2003.7.1
        pInfo->ProtocolVersion = KPROTOCOL_VERSION;    //  传输协议版本，以便校验是否兼容
        #endif

		if (g_NetConnectAgent.SendMsg(Buff, sizeof(KLoginAccountInfo) + PROTOCOL_MSG_SIZE))
		{
			g_NetConnectAgent.UpdateClientRequestTime(false);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------------
//	功能：连接游戏服务
//--------------------------------------------------------------------------
int KLogin::ConnectAccountServer(const unsigned char* pIpAddress)
{
	KIniFile	IniFile;
	if (pIpAddress && IniFile.Load("\\Config.ini"))
	{			
		int nPort;
		IniFile.GetInteger("Server", "GameServPort", 8888, &nPort);
		return g_NetConnectAgent.ClientConnectByNumericIp(pIpAddress, nPort);
	}
	return false;
}

void KLogin::RegistNetAgent()
{
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_login, this);
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_roleserver_getrolelist_result, this);
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_notifyplayerlogin, this);
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_rolenewdelresponse, this);
}

void KLogin::UnRegistNetAgent()
{
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_login, NULL);
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_roleserver_getrolelist_result, NULL);
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_notifyplayerlogin, NULL);
	g_NetConnectAgent.RegisterMsgTargetObject(s2c_rolenewdelresponse, NULL);
}

void KLogin::SetAccountPassword(const char* pszAccount, const KSG_PASSWORD* pcPassword)
{
	int i;
	if (pszAccount)
	{
		strncpy(m_Choices.Account, pszAccount, sizeof(m_Choices.Account));
		for (i = 0; i < 32; i++)
			m_Choices.Account[i] = ~m_Choices.Account[i];
	}
	if (pcPassword)
	{
		m_Choices.Password = *pcPassword;
		for (i = 0; i < KSG_PASSWORD_MAX_SIZE; i++)
			m_Choices.Password.szPassword[i] = ~m_Choices.Password.szPassword[i];
	}
}

void KLogin::GetAccountPassword(char* pszAccount, KSG_PASSWORD* pPassword)
{
	int i;
	if (pszAccount)
	{
		memcpy(pszAccount, m_Choices.Account, sizeof(m_Choices.Account));
		for (i = 0; i < 32; i++)
			pszAccount[i] = ~pszAccount[i];
	}
	if (pPassword)
	{
        *pPassword = m_Choices.Password;
		for (i = 0; i < KSG_PASSWORD_MAX_SIZE; i++)
			pPassword->szPassword[i] = ~pPassword->szPassword[i];
	}
}