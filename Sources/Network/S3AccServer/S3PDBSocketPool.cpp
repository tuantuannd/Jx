//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketPool.cpp	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#pragma warning(disable: 4786)

#include "S3PDBSocketPool.h"
#include "S3PAccount.h"
#include "GlobalFun.h"
#include "S3PDBConnectionPool.h"

#include "../../Multiserver/Heaven/Interface/IServer.h"
#include "KProtocolDef.h"

#include "string.h"

#define LEFT_TIME		60000	//允许断线后的等待时间
#define PING_TIME		20000	//允许等待客户端Ping的时间
#define SENDPING_TIME	0x7fffffff	//向客户端发送Ping的时间

S3PDBSocketPool* S3PDBSocketPool::m_pInstance = NULL;

S3PDBSocketPool::S3PDBSocketPool()
{
	m_pServer = NULL;
}

S3PDBSocketPool::~S3PDBSocketPool()
{
	assert(m_pServer == NULL);
	assert(m_clientIDs.size() == 0);
}

S3PDBSocketPool* S3PDBSocketPool::Instance()
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new S3PDBSocketPool;
	}
	return m_pInstance;
}

void S3PDBSocketPool::ReleaseInstance()
{
	delete m_pInstance;
	m_pInstance = NULL;
}

HANDLE S3PDBSocketPool::Start(IServer* pServer, int nMax)
{
	assert(pServer);
	if (NULL == m_pServer)
	{
		m_pServer = pServer;
		pServer->AddRef();
		Lock();
		DWORD nNow = GetTickCount();
		for (int i = 0; i < nMax; i++)
		{
			KGatewayDataProcess* p = new KGatewayDataProcess();
			p->Start(m_pServer, nNow);
			m_clientIDs.push_back(p);
		}
		Unlock();
	}

	return 0;
}

BOOL S3PDBSocketPool::Stop()
{
	BOOL bRet = TRUE;
	if (bRet && m_pServer)
	{
		Lock();
		GatewayArray::iterator i = m_clientIDs.begin();
		while (i != m_clientIDs.end())
		{
			(*i)->Stop();
			delete (*i);
			i++;
		}
		Unlock();
		m_clientIDs.clear();

		m_pServer->Release();
		m_pServer = NULL;
	}

	return bRet;
}

BOOL S3PDBSocketPool::SendData(unsigned long uID, const void * const pData, const size_t &datalength)
{
	if (m_pServer == NULL || pData == NULL || datalength <= 0)
		return FALSE;

	BOOL bRet = FALSE;
	try
	{
		m_pServer->SendData(uID, pData, datalength);
		bRet = TRUE;
	}
	catch(...)
	{
		gTrace("Failed to feed back( S3PDBSocketPool::SendData )");
		bRet = FALSE;
	}
	return bRet;
}

//#include "../../MultiServer/Common/Macro.h"
//#include "../../MultiServer/Common/Buffer.h"
//
//using OnlineGameLib::Win32::CBuffer;
//CBuffer::Allocator	m_theGlobalAllocator( 1024 * 64, 10 );

BOOL S3PDBSocketPool::CustomSend(unsigned long uID, char nProtocol, KAccountHead* pSend)
{
	if (m_pServer == NULL)
		return FALSE;

	BOOL bRet = FALSE;
	try
	{
		assert(pSend);
		pSend->Version = ACCOUNT_CURRENT_VERSION;
		char send[MAX_PATH + 1];
		assert(pSend->Size < MAX_PATH);
		send[0] = nProtocol;
		memcpy(send + 1, pSend, pSend->Size);

		m_pServer->SendData(uID, send, pSend->Size + 1);
					
//		CBuffer *pBuffer = m_theGlobalAllocator.Allocate();
//		
//		BYTE *pData = const_cast< BYTE * >(pBuffer->GetBuffer());
//		*pData = nProtocol;
//		memcpy(pData + 1, pSend, pSend->Size);
//		pBuffer->Use(pSend->Size + 1);
//		
//		m_pServer->PreparePackSink();
//		m_pServer->PackDataToClient(uID, pData, pSend->Size + 1);
//		m_pServer->SendPackToClient();
//
//		SAFE_RELEASE(pBuffer);

		bRet = TRUE;
	}
	catch(...)
	{
		gTrace("Failed to feed back( S3PDBSocketPool::CustomSend )");
		bRet = FALSE;
	}
	return bRet;
}

BOOL S3PDBSocketPool::AddUserClientID(unsigned long uID)
{
	assert(m_pServer);
	const char* pInfo = m_pServer->GetClientInfo(uID);
	unsigned long Address = inet_addr(pInfo);
	int offset = 0;
	while(*(pInfo + offset) != ':')
		offset++;
	offset += 2;	//skip : and blank
	short Port = atoi(pInfo + offset);

	bool bError = false;

	GatewayArray::iterator i = m_clientIDs.begin();
	int n = 0;
	while (i != m_clientIDs.end())
	{
		if ((*i)->ConnectErrorWork(uID, Address, Port))
		{
			bError = true;
			break;
		}
		i++;
		n++;
	}

	if (bError)
	{
		i = m_clientIDs.begin();
		n = 0;
		while (i != m_clientIDs.end())
		{
			if ((*i)->ConnectFreeForErrorWork(uID, Address, Port))
			{
				gTrace("ErrorAdd Gateway(%s) Client %d ! -- At %d", pInfo, uID, n);
				return TRUE;	
			}
			i++;
			n++;
		}
	}
	else
	{
		i = m_clientIDs.begin();
		n = 0;
		while (i != m_clientIDs.end())
		{
			if ((*i)->ConnectAgainWork(uID, Address, Port))
			{
				gTrace("ReAdd Gateway(%s) Client %d ! -- At %d", pInfo, uID, n);
				return TRUE;	
			}
			i++;
			n++;
		}

		i = m_clientIDs.begin();
		n = 0;
		while (i != m_clientIDs.end())
		{
			if ((*i)->ConnectFreeWork(uID, Address, Port))
			{
				gTrace("Add Gateway(%s) Client %d ! -- At %d", pInfo, uID, n);
				return TRUE;	
			}
			i++;
			n++;
		}
	}

	return TRUE;
}

BOOL S3PDBSocketPool::RemoveUserClientID(unsigned long uID)
{
	GatewayArray::iterator i = m_clientIDs.begin();
	int n = 0;
	while (i != m_clientIDs.end ())
	{
		if ((*i)->OutofWork(uID))
		{
			gTrace("Remove Gateway Client %d ! -- At %d", uID, n);
			break;
		}
		i++;
		n++;
	}
	return TRUE;
}

BOOL S3PDBSocketPool::ShowAllClientInfo()
{
	if (IsLocked())
		return FALSE;
	Lock();
	BOOL b = FALSE;
	GatewayArray::iterator i = m_clientIDs.begin();
	int n = 0;
	while (i != m_clientIDs.end())
	{
		if (*i)
		{
			int nStatus = (*i)->GetStatus();
			switch (nStatus)
			{
			case KGatewayDataProcess::gdp_work:
				{
					in_addr add;
					add.s_addr = (*i)->m_Address;
					gTrace("Game %d: %s(%s:%d) is working", (*i)->m_nGameID, (*i)->m_ServerName, inet_ntoa(in_addr(add)), (*i)->m_Port);
				}
				break;
			case KGatewayDataProcess::gdp_again:
				{
					in_addr add;
					add.s_addr = (*i)->m_Address;
					gTrace("Game %d: %s(%s:%d) is waiting", (*i)->m_nGameID, (*i)->m_ServerName, inet_ntoa(in_addr(add)), (*i)->m_Port);
				}
				break;
			case KGatewayDataProcess::gdp_free:
				{
					gTrace("Game is free at %d", n);
				}
			break;
			case KGatewayDataProcess::gdp_verify:
				{
					gTrace("Game is verify at %d", n);
				}
			break;
			case KGatewayDataProcess::gdp_verifyagain:
				{
					gTrace("Game is verifyagain at %d", n);
				}
			break;
			case KGatewayDataProcess::gdp_errorconnect:
				{
					gTrace("Game is error at %d", n);
				}
			break;
			default:
				{
					gTrace("Game is unknown at %d", n);
				}
			break;
			}
		}
		i++;
		n++;
	}
	b = TRUE;
	Unlock();
	return b;
}

//////////////////////////////////////////////////////////////////////////////////////

KGatewayDataProcess::KGatewayDataProcess()
{
	m_nGameID = 0;
	m_ServerName[0] = 0;

	m_pServer = NULL;

	m_pConn = NULL;

	memset(ProcessFunc, 0, sizeof(ProcessFunc));
//Multi Fixed By MrChuCong@gmail.com
	ProcessFunc[c2s_accountlogin - c2s_accountbegin] = &KGatewayDataProcess::ProAccountLogin;
	ProcessFunc[c2s_gamelogin - c2s_accountbegin] = &KGatewayDataProcess::ProGameLogin;
	ProcessFunc[c2s_accountlogout - c2s_accountbegin] = &KGatewayDataProcess::ProAccountLogout;
	ProcessFunc[c2s_gatewayverify - c2s_accountbegin] = NULL;
	
	m_Status = gdp_free;
	m_nStatusTime = 0;
	m_nLeftStatusTime = 0;

	m_LastPingTime = 0;
	m_nLeftPingTime = 0;
	m_LastSendPingTime = 0;
}

KGatewayDataProcess::~KGatewayDataProcess()
{
	assert(m_pServer == NULL);
}

void KGatewayDataProcess::AutoTime()
{
	if (IsError())
	{
		m_pServer->ShutdownClient(m_nConnectID);
		return;
	}

	DWORD nNow = GetTickCount();

	BLACKLIST::iterator i = m_UserNames.begin();
	while (i != m_UserNames.end())
	{
		if ((nNow - i->second) / 1000 >= 1)	//每次循环处理1个已经超时的用户
		{
			m_UserNames.erase(i);
			break;
		}
		i++;
	}

	if (m_nLeftStatusTime > 0 && (nNow - m_nStatusTime > m_nLeftStatusTime) && IsWorkAgain())
	{
		S3PDBConVBC* pConn = GetDB(0);
		if (pConn)
		{
			S3PAccount::ElapseAll(pConn, m_nGameID);	//只扣钱,不解锁
			SetStatus(gdp_free);
			return;
		}
	}

	if (m_nLeftPingTime > 0 && (nNow - m_LastPingTime) >= m_nLeftPingTime && IsWork())
	{
		m_pServer->ShutdownClient(m_nConnectID);
	}

	if ((nNow - m_LastSendPingTime) >= SENDPING_TIME && IsWork())
	{
		SendPing(nNow);
		m_LastSendPingTime = nNow;
	}
}

BOOL KGatewayDataProcess::CheckConnectAddress(DWORD Address)
{
	S3PDBConVBC* pConn = GetDB(0);
	if (pConn)
		return S3PAccount::CheckAddress(pConn, Address, m_Port) == ACTION_SUCCESS;
	return FALSE;
}

#pragma pack(push, 1)			
			typedef struct
			{
				BYTE			ProtocolType;
				DWORD			m_dwTime;
			} PING_COMMAND;
#pragma pack(pop)

void KGatewayDataProcess::SendPing(DWORD dwTime)
{
	S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();
	PING_COMMAND PingCmd;
	PingCmd.ProtocolType = s2c_ping;
	PingCmd.m_dwTime = dwTime;
	pSocket->SendData(m_nConnectID, &PingCmd, sizeof(PING_COMMAND));
}

void KGatewayDataProcess::ProcessClientData(const void * pData, DWORD dwDataSize)
{
	if (pData && dwDataSize > 0)
	{	
		char nProtocol = *((char*)pData);
		if (nProtocol == c2s_ping)
		{
			DWORD nSecond = m_LastPingTime;
			m_LastPingTime = GetTickCount();
			PING_COMMAND* p = (PING_COMMAND*)pData;
			SendPing(p->m_dwTime);
			in_addr ar;
			ar.s_addr = m_Address;
			gTrace("Gateway %s(%s): Ping %d(s)", m_ServerName, inet_ntoa(ar), (m_LastPingTime - nSecond) / 1000);
			return;
		}
	
		S3PDBConVBC* pConn = GetDB(10);
		ProcessData(pConn, pData, dwDataSize);
	}
}

DWORD KGatewayDataProcess::ProcessData(S3PDBConVBC* pConn, const void* pData, DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (pConn == NULL || pData == NULL || dwDataSize <= 0)
	{
		return bRet;
	}

	char nProtocol = *((char*)pData);

	S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

	BOOL bStopConn = FALSE;

	if (nProtocol == c2s_gatewayverify || nProtocol == c2s_gatewayverifyagain)
	{
		if (GetStatus() != gdp_verify && GetStatus() != gdp_verifyagain)
		{
			KAccountUserReturn aReturn;
			aReturn.Size = sizeof(KAccountUserReturn);
			aReturn.Type = AccountUserReturn;
			aReturn.Operate = ((KAccountUserLoginInfo*)(((char*)pData) + 1))->Operate;
			aReturn.nReturn = E_ACCOUNT_ACCESSDENIED;
			aReturn.Account[0] = 0;
			pSocket->CustomSend(m_nConnectID, s2c_gatewayverify, &aReturn);

			Sleep(1000);	// 确保返回网关的消息在连接断开前到达
			
			bRet = FALSE;
			bStopConn = TRUE;
		}
		else
		{
			if (nProtocol == c2s_gatewayverifyagain && IsVerifyTimeout())
			{
				KAccountUserReturn aReturn;
				aReturn.Size = sizeof(KAccountUserReturn);
				aReturn.Type = AccountUserReturn;
				aReturn.Operate = ((KAccountUserLoginInfo*)(((char*)pData) + 1))->Operate;
				aReturn.nReturn = E_ACCOUNT_ACCESSDENIED;
				aReturn.Account[0] = 0;
				pSocket->CustomSend(m_nConnectID, s2c_gatewayverify, &aReturn);

				Sleep(1000);	// 确保返回网关的消息在连接断开前到达

				bRet = FALSE;
				bStopConn = TRUE;
			}
			else
			{
				bRet = ProGetwayVerify(pConn, ((char*)pData) + 1, dwDataSize - 1);
				if (bRet)
				{
					if (nProtocol == c2s_gatewayverify)	//gatewayverify成功后要根据状态清理现场
					{
						if (GetStatus() == gdp_verifyagain)
						{
							S3PAccount::ElapseAll(pConn, m_nGameID);
						}
						S3PAccount::UnlockAll(pConn, m_nGameID);
						gTrace("Gateway unlock OK!");
					}

					if (nProtocol == c2s_gatewayverify)
						gTrace("Gateway Connect OK!");
					else
						gTrace("Gateway Connect Again OK!");

					if (SetStatus(gdp_work))
					{
						m_nLeftStatusTime = 0;
						m_nStatusTime = 0;
						m_LastPingTime = GetTickCount();
						m_nLeftPingTime = PING_TIME;
						m_LastSendPingTime = m_LastPingTime;
					}
				}
				else
				{
					Sleep(1000);	// 确保返回网关的消息在连接断开前到达
					bStopConn = TRUE;
				}
			}
		}
	}
	else
	{
		if (GetStatus() == gdp_work)
		{
			if (nProtocol >= c2s_accountbegin && nProtocol < c2s_multiserverbegin &&
				ProcessFunc[nProtocol - c2s_accountbegin])
			{
				bRet = (this->*ProcessFunc[nProtocol - c2s_accountbegin])(pConn, ((char*)pData) + 1, dwDataSize - 1);
			}
			else
				bRet = ProUndefine(((char*)pData) + 1, dwDataSize - 1);
		}
		else
		{
			bRet = FALSE;
		}
	}
	
	if (bStopConn)
	{
		m_pServer->ShutdownClient(m_nConnectID);
	}

	return bRet;
}

BOOL KGatewayDataProcess::ProAccountLogin(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize == sizeof(KAccountUserLoginInfo))
	{
		KAccountUserLoginInfo* pAccInfo = (KAccountUserLoginInfo*)lpData;
		if (pAccInfo->Version == ACCOUNT_CURRENT_VERSION &&
			(pAccInfo->Type == AccountUserLoginInfo || pAccInfo->Type == AccountUserVerify)
			)
		{
			KAccountUserReturnExt aReturn;
			aReturn.Size = sizeof(KAccountUserReturnExt);
			aReturn.Type = AccountUserReturnEx;
			aReturn.Operate = pAccInfo->Operate;
			aReturn.nLeftTime = aReturn.nExtPoint = 0;
			memcpy(aReturn.Account, pAccInfo->Account, LOGIN_USER_ACCOUNT_MAX_LEN);

			if (m_UserNames.find(*((UserName*)pAccInfo->Account)) == m_UserNames.end())	//不在黑名单上
			{
				if (pAccInfo->Type == AccountUserLoginInfo)
					aReturn.nReturn = S3PAccount::Login(pConn, pAccInfo->Account, pAccInfo->Password, m_nGameID, aReturn.nExtPoint, aReturn.nLeftTime);
				else
					aReturn.nReturn = S3PAccount::VerifyUserModifyPassword(pConn, m_nGameID, pAccInfo->Account, pAccInfo->Password);
			}
			else
				aReturn.nReturn = E_ACCOUNT_OR_PASSWORD;
			
			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();
			char* szDetail = NULL;
			in_addr ar;
			ar.s_addr = m_Address;
			char suc[64];
			if (aReturn.nReturn == ACTION_SUCCESS)
			{
				bRet = TRUE;
				sprintf(suc, " Success ! (Ext:%d)", aReturn.nExtPoint);
				szDetail = suc;
				m_UserNames.erase(*((UserName*)pAccInfo->Account));
			}
			else
			{
				if (aReturn.nReturn == E_ACCOUNT_OR_PASSWORD)
				{
					szDetail = " Failed ! --- Name, Password";
					m_UserNames[*((UserName*)pAccInfo->Account)] = GetTickCount();
				}
				else if (aReturn.nReturn == E_ACCOUNT_EXIST)
					szDetail = " Failed ! --- Already login";
				else if (aReturn.nReturn == E_ACCOUNT_NODEPOSIT)
					szDetail = " Failed ! --- No Money";
				else if (aReturn.nReturn == E_ADDRESS_OR_PORT)
					szDetail = " Failed ! --- Address, Port";
				else if (aReturn.nReturn == E_ACCOUNT_FREEZE)
					szDetail = " Failed ! --- Account Freeze";
				else
					szDetail = " Failed !";
			}
			if (pAccInfo->Type == AccountUserLoginInfo)
				gTrace("Gateway %s(%s): User %s Login%s", m_ServerName, inet_ntoa(ar), pAccInfo->Account, szDetail);
			else
				gTrace("Gateway %s(%s): User %s SecondPassword%s", m_ServerName, inet_ntoa(ar), pAccInfo->Account, szDetail);
			
			pSocket->CustomSend(m_nConnectID, s2c_accountlogin, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_accountlogin);
	}
	else
		gTrace("Protocol %d data is error", c2s_accountlogin);
	return bRet;
}

BOOL KGatewayDataProcess::ProGameLogin(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize == sizeof(KAccountUser))
	{
		KAccountUser* pAccInfo = (KAccountUser*)(lpData);
		if (pAccInfo->Version == ACCOUNT_CURRENT_VERSION &&
			pAccInfo->Type == AccountUser)
		{
			KAccountUserReturn aReturn;
			aReturn.Size = sizeof(KAccountUserReturn);
			aReturn.Type = AccountUserReturn;
			aReturn.Operate = pAccInfo->Operate;
			memcpy(aReturn.Account, pAccInfo->Account, LOGIN_USER_ACCOUNT_MAX_LEN);
			aReturn.nReturn = S3PAccount::LoginGame(pConn, m_nGameID, pAccInfo->Account);

			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

			char* szDetail = NULL;
			in_addr ar;
			ar.s_addr = m_Address;
			if (aReturn.nReturn == ACTION_SUCCESS)
			{
				bRet = TRUE;
				szDetail = " Success !";
			}
			else
			{
				if (aReturn.nReturn == E_ACCOUNT_OR_PASSWORD)
					szDetail = " Failed ! --- Name, Password";
				else if (aReturn.nReturn == E_ACCOUNT_ACCESSDENIED)
					szDetail = " Failed ! --- Login first";
				else
					szDetail = " Failed !";
			}

			gTrace("Gateway %s(%s): User %s Enter Game%s", m_ServerName, inet_ntoa(ar), pAccInfo->Account, szDetail);
			
			pSocket->CustomSend(m_nConnectID, s2c_gamelogin, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_gamelogin);
	}
	else
		gTrace("Protocol %d data is error", c2s_gamelogin);
	return bRet;
}


BOOL KGatewayDataProcess::ProAccountLogout(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize == sizeof(KAccountUserLogout))
	{
		KAccountUserLogout* pAccInfo = (KAccountUserLogout*)(lpData);
		if (pAccInfo->Version == ACCOUNT_CURRENT_VERSION &&
			pAccInfo->Type == AccountUserLogout)
		{
			KAccountUserReturn aReturn;
			aReturn.Size = sizeof(KAccountUserReturn);
			aReturn.Type = AccountUserReturn;
			aReturn.Operate = pAccInfo->Operate;
			memcpy(aReturn.Account, pAccInfo->Account, LOGIN_USER_ACCOUNT_MAX_LEN);
			
			aReturn.nReturn = S3PAccount::Logout(pConn, m_nGameID, pAccInfo->Account, pAccInfo->nExtPoint);

			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();
			char* szDetail = NULL;
			in_addr ar;
			ar.s_addr = m_Address;
			char suc[64];
			if (aReturn.nReturn == ACTION_SUCCESS)
			{
				bRet = TRUE;
				sprintf(suc, " Success ! (Ext:%d)", pAccInfo->nExtPoint);
				szDetail = suc;
			}
			else
			{
				if (aReturn.nReturn == E_ACCOUNT_OR_PASSWORD)
					szDetail = " Failed ! --- Name, Password";
				else if (aReturn.nReturn == E_ACCOUNT_ACCESSDENIED)
					szDetail = " Failed ! --- Login first";
				else
					szDetail = " Failed !";
			}

			gTrace("Gateway %s(%s): User %s Logout%s", m_ServerName, inet_ntoa(ar), pAccInfo->Account, szDetail);
			
			pSocket->CustomSend(m_nConnectID, s2c_accountlogout, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_accountlogout);
	}
	else
		gTrace("Protocol %d data is error", c2s_accountlogout);
	return bRet;
}

BOOL KGatewayDataProcess::ProGetwayVerify(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize == sizeof(KServerAccountUserLoginInfo))
	{
		KServerAccountUserLoginInfo* pAccInfo = (KServerAccountUserLoginInfo*)(lpData);
		if (pAccInfo->Version == ACCOUNT_CURRENT_VERSION &&
			pAccInfo->Type == ServerAccountUserLoginInfo)
		{
			KAccountUserReturn aReturn;
			aReturn.Size = sizeof(KAccountUserReturn);
			aReturn.Type = AccountUserReturn;
			aReturn.Operate = pAccInfo->Operate;
			memcpy(aReturn.Account, pAccInfo->Account, LOGIN_USER_ACCOUNT_MAX_LEN);
			DWORD nGameID = 0;
			aReturn.nReturn = S3PAccount::ServerLogin(pConn, pAccInfo->Account, pAccInfo->Password, m_Address, m_Port, pAccInfo->MacAddress, nGameID);
			
			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

			in_addr ar;
			ar.s_addr = m_Address;

			if (aReturn.nReturn == ACTION_SUCCESS)
			{
				assert(nGameID > 0);
				strncpy(m_ServerName, pAccInfo->Account, LOGIN_USER_ACCOUNT_MAX_LEN);
				m_nGameID = nGameID;
				bRet = TRUE;
			}

			char szmac[15];
			sprintf(szmac, "%02X%02X-%02X%02X-%02X%02X", pAccInfo->MacAddress[0], pAccInfo->MacAddress[1], pAccInfo->MacAddress[2], pAccInfo->MacAddress[3], pAccInfo->MacAddress[4], pAccInfo->MacAddress[5]);
			szmac[14] = 0;
			if (bRet)
				gTrace("Gateway %s(%s)%s: Verify OK !", m_ServerName, inet_ntoa(ar), szmac);
			else
			{
				if (aReturn.nReturn == E_ACCOUNT_EXIST)
					gTrace("Gateway %s(%s)%s: Already login !", m_ServerName, inet_ntoa(ar), szmac);
				else if (aReturn.nReturn == E_ACCOUNT_OR_PASSWORD)
					gTrace("Gateway %s(%s)%s: Name, Password !", m_ServerName, inet_ntoa(ar), szmac);
				else if (aReturn.nReturn == E_ADDRESS_OR_PORT)
					gTrace("Gateway %s(%s)%s: Address, Port !", m_ServerName, inet_ntoa(ar), szmac);
			}
				
		
			pSocket->CustomSend(m_nConnectID, s2c_gatewayverify, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_gatewayverify);
	}
	else
		gTrace("Protocol %d data is error", c2s_gatewayverify);
	return bRet;
}

BOOL KGatewayDataProcess::ProUndefine(const IBYTE* lpData, const DWORD dwSize)
{
	BOOL bRet = FALSE;
	return bRet;
}

S3PDBConVBC* KGatewayDataProcess::GetDB(DWORD nSleep)
{
	if (!m_pConn)
	{
		S3PDBConnectionPool* pDB = S3PDBConnectionPool::Instance();
		if (nSleep)
		{
			while (m_pConn == NULL)
			{
				pDB->RemoveDBCon(&m_pConn);
				Sleep(nSleep);
			}
		}
		else
			pDB->RemoveDBCon(&m_pConn);
	}

	return m_pConn;
}

DWORD KGatewayDataProcess::Main(LPVOID lpParam)
{
	assert(m_hStop);
	assert(m_pServer);
	DWORD dwRet = 0;
	DWORD LastTime = GetTickCount();
	
	S3PDBConnectionPool* pDB = S3PDBConnectionPool::Instance();

	while (1)
	{
		DWORD dwResult =
			KPIWaitForSingleObject(m_hStop, 0);
		if (dwResult == 1)
			break;
		else if (dwResult == 2)
		{
			AutoTime();

			size_t datalength = 0;

			const void *pData = NULL;

			if (IsWantData())
				pData = m_pServer->GetPackFromClient(m_nConnectID, datalength);

			if (pData && 0 != datalength)
			{
				ProcessClientData(pData, datalength);
				LastTime = GetTickCount();
			}
			else
			{
				if (GetTickCount() - LastTime >= 1000)
				{
					if (m_pConn)
					{
						if (pDB->ReturnDBCon(m_pConn))
							m_pConn = NULL;
					}
				}
				Sleep(1);
			}
		}
	}

	if ((IsWork() || IsWorkAgain()) && GetDB(0))
	{
		S3PAccount::ElapseAll(m_pConn, m_nGameID);	//只扣钱,不解锁
	}

	if (m_pConn)
	{
		if (pDB->ReturnDBCon(m_pConn))
			m_pConn = NULL;
	}

	return dwRet;
}

HANDLE KGatewayDataProcess::Start(IServer* pServer, DWORD nNow)
{
	assert(pServer);
	if (NULL == m_pServer)
	{
		m_pServer = pServer;
		pServer->AddRef();
	}

	m_Status = gdp_free;
	m_nStatusTime = nNow;
	m_nLeftStatusTime = LEFT_TIME;

	return KThread::Start();
}

BOOL KGatewayDataProcess::Stop()
{
	BOOL bRet = KThread::Stop();
	if (bRet)
	{
		if (m_pServer)
		{
			m_pServer->Release();
			m_pServer = NULL;
		}
	}

	return bRet;
}

BOOL KGatewayDataProcess::ConnectErrorWork(unsigned long nID, unsigned long Address, short Port)
{
	BOOL bRet = FALSE;
	Lock();

	if (m_Status == gdp_verify ||	//只有这几个状态下的m_Address有效
		m_Status == gdp_work ||
		m_Status == gdp_verifyagain ||
		m_Status == gdp_errorconnect
		)
	{
		if (m_Address == Address)	//有已经连接的Address
		{
			bRet = TRUE;
		}
		goto exit0;
	}

exit0:
	Unlock();
	return bRet;
}


BOOL KGatewayDataProcess::ConnectFreeWork(unsigned long nID, unsigned long Address, short Port)
{
	BOOL bRet = FALSE;
	Lock();

	if (m_Status == gdp_free)
	{
		if (CheckConnectAddress(Address))
		{
			m_nConnectID = nID;
			m_Address = Address;
			m_Port = Port;
			m_Status = gdp_verify;
			bRet = TRUE;
		}
		goto exit0;
	}

exit0:
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::ConnectAgainWork(unsigned long nID, unsigned long Address, short Port)
{
	BOOL bRet = FALSE;
	Lock();

	if (m_Status == gdp_again)
	{
		if (m_Address == Address && CheckConnectAddress(Address))
		{
			m_nConnectID = nID;
			m_Address = Address;
			m_Port = Port;
			m_Status = gdp_verifyagain;
			bRet = TRUE;
		}
		goto exit0;
	}

exit0:
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::ConnectFreeForErrorWork(unsigned long nID, unsigned long Address, short Port)
{
	BOOL bRet = FALSE;
	Lock();

	if (m_Status == gdp_free)
	{
		m_nConnectID = nID;
		m_Address = Address;
		m_Port = Port;
		m_Status = gdp_errorconnect;
		bRet = TRUE;
		goto exit0;
	}

exit0:
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::OutofWork(unsigned long nID)
{
	BOOL bRet = FALSE;
	Lock();
	if (m_nConnectID == nID)
	{
		m_nConnectID = -1;

		if (m_Status == gdp_work)
		{
			m_Status = gdp_again;
			m_nLeftStatusTime = LEFT_TIME;
			m_nStatusTime = GetTickCount();	//将剩余时间设上,为了客户端能重连上
			
			m_LastPingTime = 0;
			m_nLeftPingTime = 0;
			m_LastSendPingTime = 0;
			bRet = TRUE;
			goto exit0;
		}

		if (m_Status == gdp_verifyagain)
		{
			m_Status = gdp_again;
			//不改剩余时间,以继续计时
			bRet = TRUE;
			goto exit0;
		}

		if (m_Status == gdp_verify)
		{
			m_Status = gdp_free;
			//不改剩余时间,以继续计时
			bRet = TRUE;
			goto exit0;
		}

		if (m_Status == gdp_errorconnect)
		{
			m_Status = gdp_free;
			m_nLeftStatusTime = 0;
			m_nStatusTime = 0;
			m_LastPingTime = 0;
			m_nLeftPingTime = 0;
			m_LastSendPingTime = 0;
			bRet = TRUE;
			goto exit0;
		}
	}

exit0:
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::IsWork()
{
	BOOL bRet = FALSE;
	Lock();
	if (m_Status == gdp_work)
	{
		bRet = TRUE;
	}
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::IsWorkAgain()
{
	BOOL bRet = FALSE;
	Lock();
	if (m_Status == gdp_again)
	{
		bRet = TRUE;
	}
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::IsWantData()
{
	BOOL bRet = FALSE;
	Lock();
	if (m_Status == gdp_work ||
		m_Status == gdp_again ||
		m_Status == gdp_verify ||
		m_Status == gdp_verifyagain)
	{
		bRet = TRUE;
	}
	Unlock();
	return bRet;
}

BOOL KGatewayDataProcess::IsError()
{
	BOOL bRet = FALSE;
	Lock();
	if (m_Status == gdp_errorconnect)
	{
		bRet = TRUE;
	}
	Unlock();
	return bRet;
}

int KGatewayDataProcess::GetStatus()
{
	int nStatus = -1;
	Lock();
	nStatus = m_Status;
	Unlock();
	return nStatus;
}

bool KGatewayDataProcess::SetStatus(int nNews)
{
	bool bRet = false;
	Lock();
	if (m_Status == gdp_free &&
		nNews == gdp_verify)
	{
		m_Status = nNews;
		bRet = true;
	}
	else if (m_Status == gdp_verify &&
		(nNews == gdp_work ||
		 nNews == gdp_free))
	{
		m_Status = nNews;
		bRet = true;
	}
	else if (m_Status == gdp_work &&
		nNews == gdp_again)
	{
		m_Status = nNews;
		bRet = true;
	}
	else if (m_Status == gdp_again &&
		(nNews == gdp_verifyagain ||
		 nNews == gdp_free))
	{
		m_Status = nNews;
		bRet = true;
	}
	else if (m_Status == gdp_verifyagain &&
		(nNews == gdp_work ||
		 nNews == gdp_again))
	{
		m_Status = nNews;
		bRet = true;
	}
	else if (m_Status == gdp_errorconnect &&
		nNews == gdp_free)
	{
		m_Status = nNews;
		bRet = true;
	}
	Unlock();

	return bRet;
}

bool KGatewayDataProcess::IsVerifyTimeout()
{
	bool bRet = true;
	Lock();
	if (m_Status == gdp_verify || m_Status == gdp_verifyagain)
	{
		if ((GetTickCount() - m_nStatusTime) <= m_nLeftStatusTime)
		{	//正确状态的有限时间内,才叫不超时
			bRet = false;
		}
	}
	Unlock();
	return bRet;
}