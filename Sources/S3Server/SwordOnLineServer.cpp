
#include <objbase.h>
#include <initguid.h>
#include "KEngine.h"
#include "KCore.h"
#include "KSafeList.h"
#include "SwordOnLineServer.h"
#include "../Core/src/CoreServerShell.h"
#include <crtdbg.h>

#define	GAME_FPS	20

typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
			REFIID	riid,
			void	**ppv
		);


static const int g_snMaxPlayerCount = 500;
static const int g_snPrecision = 10;
static const int g_snMaxBuffer = 500;
static const int g_snBufferSize = 16384;

KPakList	g_PakList;
KSafeList	g_PlayerRemoveList;	

void __stdcall ServerCallBack(LPVOID lpParam, const unsigned long &ulnID, const unsigned long &uEventType);

enum PLAYER_GAME_STATUS
{
	PLAYER_BEGIN = 0,
	PLAYER_LISTLOADING,
	PLAYER_DBLOADING,
	PLAYER_NEWLOADING,
	PLAYER_SYNC_REPLY,
	PLAYER_PLAYING,
};

KSwordOnLineSever::KSwordOnLineSever()
{
	m_bRuning = false;
	m_szInformation[0] = 0;
	m_szStatusString[0] = 0;
	m_pCoreServerShell = NULL;
	m_pServer = NULL;
	m_nUpdateInterval = DEFUALT_UPDATE_INTERFAL;
	m_nLoopElapse = 0;
	m_pnPlayerIndex = NULL;
	m_pPlayerData = NULL;
	m_pPlayerGameStatus = NULL;
	m_pNetConnectStatus = NULL;
	m_pnPlayerAccountKey = NULL;
	m_nMaxPlayer = 0;
	m_hModule = 0;
}

KSwordOnLineSever::~KSwordOnLineSever()
{
	ShutDown();
}

bool KSwordOnLineSever::Init()
{
	m_hModule = ::LoadLibrary( "heaven.dll" );

	if ( m_hModule )
	{
		pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )GetProcAddress( m_hModule, "CreateInterface" );

		IServerFactory *pServerFactory = NULL;

		if ( SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
		{
			pServerFactory->SetEnvironment( g_snMaxPlayerCount, g_snPrecision, g_snMaxBuffer, g_snBufferSize  );

			pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pServer ) );

			pServerFactory->Release();
		}
	}

	if (!m_pServer)
		return false;

	if (FAILED(m_pServer->Startup()))
		return false;
	
	m_pServer->RegisterMsgFilter( reinterpret_cast< void * >( m_pServer ), ServerCallBack );

	m_nMaxPlayer = g_snMaxPlayerCount + g_snPrecision;
	if (m_nMaxPlayer <= 0)
		m_nMaxPlayer = 1;

	if (!m_pnPlayerIndex)
	{
		m_pnPlayerIndex = (int *)new int[m_nMaxPlayer];
	}
	ZeroMemory(m_pnPlayerIndex, sizeof(int) * m_nMaxPlayer);
	
	if (!m_pPlayerData)
	{
		m_pPlayerData = (LoginData *)new LoginData[m_nMaxPlayer];
	}
	ZeroMemory(m_pPlayerData, sizeof(LoginData) * m_nMaxPlayer);

	if (!m_pPlayerGameStatus)
	{
		m_pPlayerGameStatus = (GameStatus *)new GameStatus[m_nMaxPlayer];
	}

	if (!m_pNetConnectStatus)
	{
		m_pNetConnectStatus = (ConnectStatus *)new ConnectStatus[m_nMaxPlayer];
	}

	if (!m_pnPlayerAccountKey)
	{
		m_pnPlayerAccountKey = (DWORD *)new int[m_nMaxPlayer];
	}
	ZeroMemory(m_pnPlayerAccountKey, sizeof(DWORD) * m_nMaxPlayer);

	return true;
}

void KSwordOnLineSever::GetStatus(char* pszStatusString, char* pszInformation)
{
	_ASSERT(pszStatusString);
	_ASSERT(pszInformation);
	strcpy(pszInformation, m_szInformation);
	strcpy(pszStatusString, m_szStatusString);
}

int	KSwordOnLineSever::GetClientConnectInfo(ClientConnectInfo* pInfo, int& nPos)
{
	const char*	pChar;
	if (pInfo && nPos >= 0 && nPos < m_nMaxPlayer)
	{
		pInfo->AddrInfo[0] = 0;
		pInfo->Character[0] = 0;
		while(nPos < m_nMaxPlayer)
		{
			pChar = (const char *)m_pServer->GetClientInfo(nPos);

			if (pChar)
			{
				strcpy(pInfo->AddrInfo, pChar);
				if (m_pCoreServerShell)
					m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME,
						(unsigned int)pInfo->Character, nPos);
				nPos++;
				return 1;
			}
			nPos ++;
		}
	}	// need modify later
	return 0;
}


bool KSwordOnLineSever::Launch()
{
	if (m_bRuning)
	{
		strcpy(m_szStatusString, "Already launched.");
		return true;
	}

	if (!m_pServer)
	{
		strcpy(m_szStatusString, "Not init server.");
		return false;
	}

	m_szInformation[0] = 0;
	strcpy(m_szStatusString, "Launching...");
#ifdef _DEBUG
	g_FindDebugWindow("#32770", "DebugWin");
#endif

	g_SetRootPath(NULL);

	g_PakList.Open("config.ini");

	if (FAILED(m_pServer->OpenService(INADDR_ANY, 6666)))
	{
		strcpy(m_szInformation, "Failed to Create NetServer.");
		strcpy(m_szStatusString, "Launch FAILED!");
		return false;
	}

	if (m_pCoreServerShell == NULL)
		m_pCoreServerShell = ::CoreGetServerShell();
	if (m_pCoreServerShell == NULL)
	{
		strcpy(m_szInformation, "Failed to Create CoreShell.");
		strcpy(m_szStatusString, "Launch FAILED!");
		return false;
	}

	IServer *pClonServer = NULL;
	m_pServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &pClonServer ) );
	m_pCoreServerShell->OperationRequest(SSOI_LAUNCH, (unsigned int)pClonServer, 0);

	m_Timer.Start();
	m_GameLoop = 0;
	m_bRuning = true;
	m_szInformation[0] = 0;
	strcpy(m_szStatusString, "Launch SUCCESSED!");
	return true;
}

bool KSwordOnLineSever::ShutDown()
{
	strcpy(m_szInformation, "Restart, if U wish to launch again.");
	strcpy(m_szStatusString, "Server has been shutdown.");
	if (m_pCoreServerShell)
	{
		m_pCoreServerShell->Release();
		m_pCoreServerShell = NULL;
	}

	if (m_pnPlayerIndex)
	{
		delete [] m_pnPlayerIndex;
		m_pnPlayerIndex = NULL;
	}

	if (m_pPlayerData)
	{
		delete [] m_pPlayerData;
		m_pPlayerData = NULL;
	}

	if (m_pPlayerGameStatus)
	{
		delete [] m_pPlayerGameStatus;
		m_pPlayerGameStatus = NULL;
	}

	if (m_pNetConnectStatus)
	{
		delete [] m_pNetConnectStatus;
		m_pNetConnectStatus = NULL;
	}

	if (m_pnPlayerAccountKey)
	{
		delete [] m_pnPlayerAccountKey;
		m_pnPlayerAccountKey = NULL;
	}

	if (m_pServer)
	{
		m_pServer->CloseService();
		m_pServer->Cleanup();
		m_pServer->Release();
		m_pServer = NULL;
	}
	if (m_hModule)
	{
		::FreeLibrary( m_hModule );
		m_hModule = NULL;
	}
	m_bRuning = false;
	return true;
}

//返回值表示状态是否更新
bool KSwordOnLineSever::Breathe()
{
	static KTimer s_Timer;
	DWORD	t1;

	if (m_bRuning && m_pCoreServerShell)
	{
		s_Timer.Start();
		MessageLoop();	//处理网络消息
		t1 = s_Timer.GetElapse();
		g_DebugLog("[FPS]Message:%dms", t1);

		if (m_GameLoop * 1000 <= m_Timer.GetElapse() * GAME_FPS)
		{
			s_Timer.Start();
			MainLoop();
			t1 = s_Timer.GetElapse();
			g_DebugLog("[FPS]MainLoop:%dms", t1);
		}
		else
		{
			//SwitchToThread();
			Sleep(1);
		}

#define STRING_MIN_LEN	9
#define	STRING_MAX_LEN	15
		char		szStatusString[20] = "@ Runing.........";
		static int  s_nStatusLen = STRING_MIN_LEN;
		if ((++m_nLoopElapse) >= m_nUpdateInterval)
		{
/*			m_nLoopElapse = 0;
			if (szStatusString[0] == '@')
			{
				memcpy(m_szStatusString, szStatusString, s_nStatusLen);
				m_szStatusString[s_nStatusLen] = 0;
				if ((++s_nStatusLen) > STRING_MAX_LEN)
					s_nStatusLen = STRING_MIN_LEN;
			}
*/
			int nTime = m_pCoreServerShell->GetLoopRate();
			int nHour, nMinute, nSecond, nMinSecond;
#define	defHOUR_TIME		(3600 * 20)
#define	defMINUTE_TIME		(60 * 20)
#define	defSECOND_TIME		20
#define	defMINSECOND_TIME	2
			nHour = nTime / defHOUR_TIME;
			nTime -= nHour * defHOUR_TIME;
			nMinute = nTime / defMINUTE_TIME;
			nTime -= nMinute * defMINUTE_TIME;
			nSecond = nTime / defSECOND_TIME;
			nTime -= nSecond * defSECOND_TIME;
			nMinSecond = nTime / defMINSECOND_TIME;
			sprintf(m_szStatusString, "Time:(%3d:%3d:%3d.%d)", nHour, nMinute, nSecond, nMinSecond);
			int	nClientCount = -1;
			KCoreConnectInfo	Info;
			nClientCount = m_pServer->GetClientCount();
			m_pCoreServerShell->GetConnectInfo(&Info);

			sprintf(m_szInformation, "ClientNum:%d PlayerNum:%d", nClientCount, Info.nNumPlayer);
			
			return true;
		}
	}
	return false;
}

void KSwordOnLineSever::SetUpdateInterval(int nInterval)
{
	m_nUpdateInterval = nInterval;
}

void KSwordOnLineSever::MessageLoop()
{
	DWORD	dwNetTime = 0;
	DWORD	dwProcTime = 0;
	const char*	pChar = NULL;
	unsigned int	nSize = 0;
	KTimer	sTimer;

	_ASSERT(m_pCoreServerShell);

	g_PlayerRemoveList.Lock();
	KIndexNode* pNode = (KIndexNode *)g_PlayerRemoveList.GetHead();
	while (pNode)
	{
		KIndexNode* pTempNode = (KIndexNode *)pNode->GetNext();
		m_pCoreServerShell->ClientDisconnect(pNode->m_nIndex);

		pNode->Remove();
		if (pNode)
		{
			delete pNode;
			pNode = NULL;
		}
		pNode = pTempNode;
	}
	g_PlayerRemoveList.Unlock();
	
	for (int i = 0; i < m_nMaxPlayer; i++)
	{

		if (NET_UNCONNECT == GetNetStatus(i))
			continue;

		do
		{
			sTimer.Start();
			pChar = (const char*)m_pServer->GetPackFromClient(i, nSize);
			dwNetTime += sTimer.GetElapseFrequency();

			if (!pChar || 0 == nSize)
				break;

			sTimer.Start();
			if (!m_pCoreServerShell->CheckProtocolSize(pChar, nSize))
				break;
			MessageProcess(i, pChar, nSize);
			dwProcTime += sTimer.GetElapseFrequency();
		} while (1);
	}

	g_DebugLog("[FPS]Net:Proc(%d:%d)", dwNetTime, dwProcTime);
}


BOOL KSwordOnLineSever::ProcessLoginProtocol(int nClient, const char* pChar, int nSize)
{
	const char* pBuffer = pChar;

	if (*pBuffer != c2s_login)
	{
		return FALSE;
	}
	else
	{
		strcpy(m_pPlayerData[nClient].szAccount, (const char *)pBuffer + 1);
		DBI_COMMAND Command = DBI_GETPLAYERLISTFROMACCOUNT;
		memset(m_pPlayerData[nClient].PlayerBaseInfo, 0, sizeof(S3DBI_RoleBaseInfo) * MAX_PLAYER_IN_ACCOUNT);
		void*	pTmpPoint1 = m_pPlayerData[nClient].szAccount;
		void*	pTmpPoint2 = &m_pPlayerData[nClient];
		
		g_AccessDBMsgList(DBMSG_PUSH, &nClient, (DBI_COMMAND*)&Command, (void **)&pTmpPoint1, (void **)&pTmpPoint2);
		return TRUE;
	}
	return FALSE;
}

void KSwordOnLineSever::SendPlayerListToClient(int nClient)
{
	ROLE_LIST_SYNC	PlayerListSync;
	PlayerListSync.ProtocolType = s2c_syncrolelist;
	for (int i = 0; i < MAX_PLAYER_IN_ACCOUNT; i++)
	{
		strcpy(PlayerListSync.m_RoleList[i].szName, m_pPlayerData[nClient].PlayerBaseInfo[i].szRoleName);
		PlayerListSync.m_RoleList[i].Sex = (BYTE)m_pPlayerData[nClient].PlayerBaseInfo[i].nSex;
		PlayerListSync.m_RoleList[i].ArmorType = (BYTE)m_pPlayerData[nClient].PlayerBaseInfo[i].nArmorType;
		PlayerListSync.m_RoleList[i].HelmType = (BYTE)m_pPlayerData[nClient].PlayerBaseInfo[i].nHelmType;
		PlayerListSync.m_RoleList[i].WeaponType = (BYTE)m_pPlayerData[nClient].PlayerBaseInfo[i].nWeaponType;
		PlayerListSync.m_RoleList[i].Level = (BYTE)m_pPlayerData[nClient].PlayerBaseInfo[i].nLevel;
	}
	m_pServer->PackDataToClient(nClient, &PlayerListSync, sizeof(PlayerListSync));
	
}

// return : -1 失败，协议不对  0 成功 1 协议正确，数据包错误  2 协议正确，校验错误 ……
int KSwordOnLineSever::RecvClientPlayerSelect(int nClient, const char* pChar, int nSize, int *pnSel)
{
	const char*	pBuffer = pChar;

	if (*pBuffer != c2s_dbplayerselect)
	{
		return -1;
	}
	else
	{
		if (sizeof(DB_PLAYERSELECT_COMMAND) != nSize)
			return 1;
		
		DB_PLAYERSELECT_COMMAND* pCommand = (DB_PLAYERSELECT_COMMAND*)pBuffer;
		if (pCommand->m_nSelect >= 0 && pCommand->m_nSelect < m_pPlayerData[nClient].nRoleCount)
		{
			*pnSel = pCommand->m_nSelect;
			return 0;
		}
		else
			return 2;
	}
}
//--------------------------------------------------------------------------------
// return : -1 失败，协议不对  0 成功 1 协议正确，数据包错误
//          2 协议正确，校验错误1 同名  3 协议正确，校验错误2 角色数量已满 ……
//--------------------------------------------------------------------------------
int		KSwordOnLineSever::RecvClientPlayerNew(int nClient, const char* pChar, int nSize, int *pnRole, int *pnSeries, char *lpszName)
{
	//BYTE*	pTemp = m_NetServer.Client_data_array[nClient].RecieveBuffer;
	const char* pBuffer = pChar;
//	BYTE*	pBuffer = pTemp;
	if (*pBuffer != c2s_newplayer)
	{
		return -1;
	}
	else
	{
		NEW_PLAYER_COMMAND	*pNew = (NEW_PLAYER_COMMAND*)pBuffer;
		if (pNew->m_btRoleNo > 1 || pNew->m_btSeries >= series_num)
			return 1;
		char	szName[32];
		memset(szName, 0, sizeof(szName));
		memcpy(szName, pNew->m_szName, pNew->m_wLength - 2 - 2);
		
		// if CheckCanNew()		判断是否可以添加新角色 not end
		
		*pnRole = pNew->m_btRoleNo;
		*pnSeries = pNew->m_btSeries;
		strcpy(lpszName, szName);
		return 0;
	}
}

BOOL KSwordOnLineSever::ProcessSyncReplyProtocol(int nClient, const char* pChar, int nSize)
{
	const char*	pBuffer = pChar;
	if (*pBuffer != c2s_syncend)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
	return FALSE;
}

int KSwordOnLineSever::OperationRequest(unsigned int uOper, unsigned int uParam, int nParam)
{
	if (m_pCoreServerShell)
		return m_pCoreServerShell->OperationRequest(uOper, uParam, nParam);
	return 0;
}

extern int	 CORE_API g_ClearOutOfDateDBMsgInList(int nPlayerIndex);
void KSwordOnLineSever::MainLoop()
{
	DWORD	dwNowTime;

	m_pServer->PreparePackSink();

	for (int i = 0; i < m_nMaxPlayer; i++)
	{
		if (NET_UNCONNECT == GetNetStatus(i))
			continue;

		m_pPlayerGameStatus[i].sMutex.Lock();
		switch(m_pPlayerGameStatus[i].nGameStatus)
		{
		case PLAYER_LISTLOADING:
			if (m_pCoreServerShell->IsCharacterListLoadFinished(i))
			{
				SendPlayerListToClient(i);
				m_pnPlayerIndex[i] = 0;
				m_pPlayerGameStatus[i].nGameStatus = PLAYER_DBLOADING;
				g_DebugLog("[TRACE]Send List to Client:%d", i);
			}
			break;
		case PLAYER_NEWLOADING:
			if (m_pnPlayerIndex[i] > 0)
			{
				int bSyncEnd = (m_pPlayerData[i].nStep == STEP_SYNC_END);
				m_pCoreServerShell->NewPlayerIniLoading(m_pnPlayerIndex[i],
					bSyncEnd, m_pPlayerData[i].nStep, m_pPlayerData[i].nParam);
				if (bSyncEnd)
				{
					m_pPlayerGameStatus[i].nGameStatus = PLAYER_SYNC_REPLY;
					BYTE	SyncEnd;
					SyncEnd = s2c_syncend;
					m_pServer->PackDataToClient(i, &SyncEnd, sizeof(BYTE));
					m_pCoreServerShell->SaveNewRole(m_pnPlayerIndex[i]);
					g_DebugLog("[TRACE]SyncEnd:%d", i);
				}
			}
			break;
		case PLAYER_DBLOADING:
/*			dwNowTime = m_Timer.GetElapse();
			if (dwNowTime - m_pPlayerGameStatus[i].dwLastOperationTime > 600000)
			{
				m_pServer->ShutdownClient(i);
				break;
			}*/
			if (m_pnPlayerIndex[i] > 0)
			{
				int bSyncEnd = (m_pPlayerData[i].nStep == STEP_SYNC_END);
				m_pCoreServerShell->PlayerDbLoading(m_pnPlayerIndex[i],
					bSyncEnd, m_pPlayerData[i].nStep, m_pPlayerData[i].nParam);
				if (bSyncEnd)
				{
					m_pPlayerGameStatus[i].dwLastOperationTime = m_Timer.GetElapse();
					m_pPlayerGameStatus[i].nGameStatus = PLAYER_SYNC_REPLY;
					BYTE	SyncEnd;
					SyncEnd = s2c_syncend;
					m_pServer->PackDataToClient(i, &SyncEnd, sizeof(BYTE));
					g_DebugLog("[TRACE]SyncEnd:%d", i);
				}
			}
			break;
/*		case PLAYER_SYNC_REPLY:
			dwNowTime = m_Timer.GetElapse();
			if (dwNowTime - m_pPlayerGameStatus[i].dwLastOperationTime > 600000)
			{
				m_pServer->ShutdownClient(i);
				break;
			}
			break;*/
		default:
			break;
		}
		m_pPlayerGameStatus[i].sMutex.Unlock();
	}

	m_pCoreServerShell->Breathe();

	m_pServer->SendPackToClient();
	m_GameLoop++;

}

void KSwordOnLineSever::MessageProcess(const unsigned long i, const char* pChar, size_t nSize)
{
	_ASSERT(pChar && nSize);

	m_pPlayerGameStatus[i].sMutex.Lock();
	switch(m_pPlayerGameStatus[i].nGameStatus)
	{
	case PLAYER_PLAYING:
		m_pCoreServerShell->ProcessClientMessage(i, pChar, nSize);
		break;
	case PLAYER_BEGIN:
		if (ProcessLoginProtocol(i, pChar, nSize))
		{
			m_pPlayerGameStatus[i].nGameStatus = PLAYER_LISTLOADING;
			m_pPlayerGameStatus[i].dwLastOperationTime = m_Timer.GetElapse();
			strcpy(m_szStatusString, "A player apply login.");
			g_DebugLog("[Login]%s", m_szStatusString);
		}
		break;
	case PLAYER_DBLOADING:
		{
			int		nRet, nSel;
			int		nRole, nSeries;
			char	szName[32];
			if( (nRet = RecvClientPlayerSelect(i, pChar, nSize, &nSel)) != -1 )
			{
				if (nRet == 0)	// 成功
				{
					g_DebugLog("[TRACE]Recv role select:%d", nSel);
					m_pPlayerGameStatus[i].dwLastOperationTime = m_Timer.GetElapse();
					m_pnPlayerIndex[i] = m_pCoreServerShell->AddCharacter(
						m_pPlayerData[i].PlayerBaseInfo[nSel].szRoleName, i, m_pPlayerData[i].szAccount);
					strcpy(m_szStatusString, "A player Selected character.");
					g_DebugLog("[TRACE]%s", m_szStatusString);
					if (m_pnPlayerIndex[i] > 0)
					{
						int bSyncEnd = (m_pPlayerData[i].nStep == STEP_SYNC_END);
						m_pCoreServerShell->PlayerDbLoading(m_pnPlayerIndex[i],
							bSyncEnd, m_pPlayerData[i].nStep, m_pPlayerData[i].nParam);
						if (bSyncEnd)
						{
							m_pPlayerGameStatus[i].nGameStatus = PLAYER_SYNC_REPLY;
							BYTE	SyncEnd;
							SyncEnd = s2c_syncend;
							m_pServer->SendData(i, &SyncEnd, sizeof(BYTE));
							g_DebugLog("[TRACE]SyncEnd:%d", i);
						}
					}
				}
				else	// 失败
				{
				}
			}
			else if ( (nRet = RecvClientPlayerNew(i, pChar, nSize, &nRole, &nSeries, szName)) != -1 )
			{
				if (nRet == 0)	// 成功
				{
					m_pPlayerGameStatus[i].dwLastOperationTime = m_Timer.GetElapse();
					m_pnPlayerIndex[i] = m_pCoreServerShell->CreateNewPlayer(i, nRole, nSeries, szName, m_pPlayerData[i].szAccount);
					strcpy(m_szStatusString, "Create a new role.");
					if (m_pnPlayerIndex[i] > 0)
					{
						m_pPlayerData[i].nStep = 0;
						m_pPlayerData[i].nParam = 0;
						m_pPlayerGameStatus[i].nGameStatus = PLAYER_NEWLOADING;
						m_pCoreServerShell->NewPlayerIniLoading(m_pnPlayerIndex[i],
							FALSE, m_pPlayerData[i].nStep, m_pPlayerData[i].nParam);
					}
				}
				else if (nRet == 1)	// 数据包错
				{
				}
				else if (nRet == 2)	// 同名 错
				{
				}
				else if (nRet == 3)	// 角色数量已满 错
				{
				}
			}
		}
		break;
	case PLAYER_SYNC_REPLY:
		if (ProcessSyncReplyProtocol(i, pChar, nSize))
		{
			m_pCoreServerShell->PlayerSyncReplay(m_pnPlayerIndex[i]);
			m_pPlayerGameStatus[i].nGameStatus = PLAYER_PLAYING;
			g_DebugLog("[TRACE]Socket %d playing", i);
		}
		break;
	default:
		break;
	}
	m_pPlayerGameStatus[i].sMutex.Unlock();
}

void KSwordOnLineSever::SetNetStatus(const unsigned long lnID, NetStatus nStatus)
{
	if (lnID >= m_nMaxPlayer)
		return;

//	m_pNetConnectStatus[lnID].sMutex.Lock();

	if (nStatus == NET_UNCONNECT)
	{
		g_ClearOutOfDateDBMsgInList(lnID);
//		int nIndex = m_pCoreServerShell->GetPlayerIndex(lnID);
		KIndexNode*	pNode = new KIndexNode;
		pNode->m_nIndex = lnID;
		g_PlayerRemoveList.Lock();
		g_PlayerRemoveList.AddTail(pNode);
		g_PlayerRemoveList.Unlock();
	}
	
	if (nStatus == NET_CONNECTED)
	{
		m_pPlayerGameStatus[lnID].sMutex.Lock();
		m_pPlayerGameStatus[lnID].nGameStatus = PLAYER_BEGIN;
		m_pnPlayerIndex[lnID] = 0;
		m_pPlayerGameStatus[lnID].sMutex.Unlock();
	}

	::InterlockedExchange( &( m_pNetConnectStatus[lnID].nNetStatus ), nStatus );
//	m_pNetConnectStatus[lnID].nNetStatus = nStatus;

//	m_pNetConnectStatus[lnID].sMutex.Unlock();
}

int KSwordOnLineSever::GetNetStatus(const unsigned long lnID)
{
	if (lnID >= m_nMaxPlayer)
		return NET_UNCONNECT;

	int nStatus;
//	m_pNetConnectStatus[lnID].sMutex.Lock();
	nStatus = ::InterlockedExchange( &( m_pNetConnectStatus[lnID].nNetStatus ), 
		m_pNetConnectStatus[lnID].nNetStatus );
	//nStatus = m_pNetConnectStatus[lnID].nNetStatus;
//	m_pNetConnectStatus[lnID].sMutex.Unlock();
	return nStatus;
}
