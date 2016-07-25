//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketPool.cpp	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#pragma warning(disable: 4786)

#include "S3PDBSocketPool.h"

#include "../../../Headers/inoutmac.h"

#include "S3PAccount.h"
#include "GlobalFun.h"
#include "S3PDBConnectionPool.h"



#include "../../Multiserver/Heaven/Interface/IServer.h"
#include "../../../Headers/KProtocolDef.h"
#include "../../../Headers/KRelayProtocol.h"
#include "../../../Headers/KGmProtocol.h"
#include "../../../Headers/KProtocol.h"
#include "../S3AccServer/AccountLoginDef.h"
#include "crtdbg.h"

#include "string.h"
#include "malloc.h"

S3PDBSocketPool* S3PDBSocketPool::m_pInstance = NULL;
DWORD S3PDBSocketPool::m_SelfAddress = 0;

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
		
		gGetMacAndIPAddress(NULL, NULL, NULL, &m_pInstance->m_SelfAddress);
	}
	return m_pInstance;
}

void S3PDBSocketPool::ReleaseInstance()
{
	delete m_pInstance;
	m_pInstance = NULL;
}

HANDLE S3PDBSocketPool::Start(IServer* pServer)
{
	assert(pServer);
	if (NULL == m_pServer)
	{
		m_pServer = pServer;
		pServer->AddRef();
	}

	return 0;
}

BOOL S3PDBSocketPool::Stop()
{
	BOOL bRet = TRUE;
	if (bRet && m_pServer)
	{
		StopAllUserClientConnect();
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
		//printf("Failed to feed back( S3PDBSocketPool::SendData )");
		bRet = FALSE;
	}
	return bRet;
}

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
		send[0] = pf_normal;
		send[1] = nProtocol;
		memcpy(send + 2, pSend, pSend->Size);

		m_pServer->SendData(uID, send, pSend->Size + 2);
					
		bRet = TRUE;
	}
	catch(...)
	{
		//printf("Failed to feed back( S3PDBSocketPool::CustomSend )");
		bRet = FALSE;
	}
	return bRet;
}

void S3PDBSocketPool::SendRelayData(unsigned long uID, char* szAccount, const void * const pSrcData1, const size_t& datalength1, const void * const pSrcData2, const size_t& datalength2)
{
	if (szAccount == NULL || pSrcData1 == NULL || datalength1 == 0)
		return;

	RELAY_ASKWAY_DATA relay;
	relay.ProtocolFamily = pf_relay;
	relay.ProtocolID = relay_c2c_askwaydata;
	relay.nFromIP = 0;		//由于是服务器主动发给客户端,所以无法填nFromIP,只好填0
	relay.nFromRelayID = 0; //由于是服务器主动发给客户端,所以无法填nFromRelayID,只好填0
	relay.seekRelayCount = 0;
	relay.seekMethod = rm_account_id;
	relay.wMethodDataLength = defACCOUNT_STRING_LENGTH;
	relay.routeDateLength = datalength1 + datalength2;

	BYTE *pData = (BYTE *)_alloca(sizeof(relay) + relay.wMethodDataLength + relay.routeDateLength);

	BYTE *pDataOrg = pData;
	memcpy(pData, &relay, sizeof(relay));
	pData += sizeof(relay);

	strncpy((char*)pData, szAccount, defACCOUNT_STRING_LENGTH);
	pData += relay.wMethodDataLength;

	memcpy(pData, pSrcData1, datalength1);
	pData += datalength1;

	if (pSrcData2 && datalength2 > 0)
		memcpy(pData, pSrcData2, datalength2);

	m_pServer->SendData(uID ,(const void *)pDataOrg, sizeof(relay) + relay.wMethodDataLength + relay.routeDateLength);
}

void S3PDBSocketPool::SendRelayData(unsigned long uID, const void * const pSrcData1, const size_t& datalength1, const void * const pSrcData2, const size_t& datalength2)
{
	if (pSrcData1 == NULL || datalength1 == 0)
		return;

	RELAY_DATA relayIP;
	relayIP.ProtocolFamily = pf_relay;
	relayIP.ProtocolID = relay_c2c_data;
	relayIP.nToIP = 0;
	relayIP.nToRelayID = 0;
	relayIP.nFromIP = 0;		//由于是服务器主动发给客户端,所以无法填nFromIP,只好填0
	relayIP.nFromRelayID = 0;	//由于是服务器主动发给客户端,所以无法填nFromRelayID,只好填0
	relayIP.routeDateLength = datalength1 + datalength2;

	BYTE *pData = (BYTE *)_alloca(sizeof(relayIP) + relayIP.routeDateLength);

	BYTE *pDataOrg = pData;
	memcpy(pData, &relayIP, sizeof(relayIP));
	pData += sizeof(relayIP);

	memcpy(pData, pSrcData1, datalength1);
	pData += datalength1;

	if (pSrcData2 && datalength2 > 0)
		memcpy(pData, pSrcData2, datalength2);

	m_pServer->SendData(uID ,(const void *)pDataOrg, sizeof(relayIP) + relayIP.routeDateLength);
}


BOOL S3PDBSocketPool::AddUserClientID(unsigned long uID)
{
	GatewayIDMap::iterator i = m_clientIDs.find(uID);
	if (i != m_clientIDs.end())
	{
		if (i->second)
		{
			i->second->Stop();
			delete i->second;
		}
	}
	assert(m_pServer);

	Lock();
	const char* pInfo = m_pServer->GetClientInfo(uID);
	unsigned long Address = inet_addr(pInfo);
	int offset = 0;
	while(*(pInfo + offset) != ':')
		offset++;
	offset += 2;	//skip : and blank
	short Port = atoi(pInfo + offset);
	KGatewayDataProcess* p = new KGatewayDataProcess(uID, Address, Port);
	m_clientIDs[uID] = p;
	Unlock();

	p->Start(m_pServer);
	gTrace("Add Relay(%s) Client %d ! -- Count %d", pInfo, uID, m_clientIDs.size());
	return TRUE;
}

BOOL S3PDBSocketPool::RemoveUserClientID(unsigned long uID)
{
	BOOL b = FALSE;
	DWORD Address;
	std::string ServerName;
	KGatewayDataProcess* p = NULL;
	Lock();
	GatewayIDMap::iterator i = m_clientIDs.find(uID);
	if (i != m_clientIDs.end ())
	{
		if (i->second)
		{
			p = i->second;
			b = TRUE;
		}
		m_clientIDs.erase(i);
	}
	Unlock();
	gTrace("Remove Relay Client %d ! -- Count %d", uID, m_clientIDs.size());
	
	if (b && p)
	{
		p->Stop();
		Address = p->m_Address;
		ServerName = p->m_ServerName;
		delete p;
		NotifyRelayLogoutInfo(Address, ServerName);
	}

	return b;
}

BOOL S3PDBSocketPool::ShowAllClientInfo()
{
	if (IsLocked())
		return FALSE;
	Lock();
	BOOL b = FALSE;
	GatewayIDMap::iterator i = m_clientIDs.begin();
	int n = 0;
	while (i != m_clientIDs.end())
	{
		KGatewayDataProcess* p = i->second;
		if (p)
		{
			in_addr add;
			add.s_addr = p->m_Address;
			gTrace("Client %d: %s(%s)", n, p->m_ServerName.c_str(), inet_ntoa(in_addr(add))); 
		}
		i++;
		n++;
	}
	b = TRUE;
	Unlock();
	return b;
}

BOOL S3PDBSocketPool::FindGatewayByServerName(const char* szServerName, DWORD& nGameID)
{
	if (szServerName == NULL ||
		szServerName[0] == 0)
	{
		return FALSE;
	}
	Lock();
	BOOL b = FALSE;
	GatewayIDMap::iterator i = m_clientIDs.begin();
	while (i != m_clientIDs.end())
	{
		if (i->second && strcmp(i->second->m_ServerName.c_str(), szServerName) == 0)
		{
			nGameID = i->second->m_nGameID;
			b = TRUE;
			break;
		}
		i++;
	}
	Unlock();
	return b;
}

KGatewayDataProcess* S3PDBSocketPool::FindGatewayByID(unsigned long nGameID)
{
	KGatewayDataProcess* p = NULL;
	Lock();
	GatewayIDMap::iterator i = m_clientIDs.begin();
	while (i != m_clientIDs.end())
	{
		if (i->second && i->second->m_nGameID == nGameID)
		{
			p = i->second;
			break;
		}
		i++;
	}
	Unlock();
	return p;
}

BOOL S3PDBSocketPool::FindGatewayClientByAddress(DWORD nAddress, DWORD& nClientID)
{
	Lock();
	BOOL b = FALSE;
	GatewayIDMap::iterator i = m_clientIDs.begin();
	while (i != m_clientIDs.end())
	{
		if (i->second && i->second->m_Address == nAddress)
		{
			nClientID = i->first;
			b = TRUE;
			break;
		}
		i++;
	}
	Unlock();
	return b;
}

void S3PDBSocketPool::BroadGMData(const IBYTE* lpData, const DWORD dwDataSize)
{
	Lock();
	GatewayIDMap::iterator i = m_clientIDs.begin();
	while (i != m_clientIDs.end())
	{
		if (i->second &&
			i->second->m_ServerName[0] == 'g' &&
			i->second->m_ServerName[1] == 'm' &&
			i->second->m_ServerName[2] == '-')
		{
			SendData(i->first, lpData, dwDataSize);
		}
		i++;
	}
	Unlock();
}
BOOL S3PDBSocketPool::StopAllUserClientConnect()
{
	while(m_clientIDs.size() > 0)
	{
		GatewayIDMap::iterator i = m_clientIDs.begin();
		if (i != m_clientIDs.end() && i->second)
			i->second->Stop();
		Sleep(10);
	}
	return TRUE;
}

void S3PDBSocketPool::NotifyRelayLoginInfo(DWORD nGameID, DWORD nClientID, DWORD Address, const std::string& ServerName)
{
	KServerInfo aInfo;
	aInfo.Size = sizeof(KServerInfo);
	aInfo.Type = ServerInfo;
	aInfo.Operate = 0;
	aInfo.Version = ACCOUNT_CURRENT_VERSION;
	aInfo.nValue = 0;

	Lock();
	GatewayIDMap::iterator i = m_clientIDs.begin();
	aInfo.nServerType = server_Login;
	aInfo.nValue = Address;
	strncpy(aInfo.Account, ServerName.c_str(), LOGIN_USER_ACCOUNT_MAX_LEN);
	while (i != m_clientIDs.end())	//先给已登陆者发送刚登陆者的信息
	{
		if (i->second)
		{
			if (i->second->m_nGameID != nGameID)
			{
				CustomSend(i->first, s2c_gatewayinfo, &aInfo);
			}
		}
		i++;
	}
	//再给刚登陆者发送已登陆者的信息
	aInfo.nServerType = server_LoginAlready;
	i = m_clientIDs.begin();
	while (i != m_clientIDs.end())
	{
		if (i->second)
		{
			if (i->second->m_nGameID != nGameID)
			{
				aInfo.nValue = i->second->m_Address;
				strncpy(aInfo.Account, i->second->m_ServerName.c_str(), LOGIN_USER_ACCOUNT_MAX_LEN);
				CustomSend(nClientID, s2c_gatewayinfo, &aInfo);
			}
		}
		i++;
	}
	Unlock();
}

void S3PDBSocketPool::NotifyRelayLogoutInfo(DWORD Address, const std::string& ServerName)
{
	KServerInfo aInfo;
	aInfo.Size = sizeof(KServerInfo);
	aInfo.Type = ServerInfo;
	aInfo.Operate = 0;
	aInfo.Version = ACCOUNT_CURRENT_VERSION;
	aInfo.nValue = 0;

	Lock();
	GatewayIDMap::iterator i = m_clientIDs.begin();
	aInfo.nServerType = server_Logout;
	strncpy(aInfo.Account, ServerName.c_str(), LOGIN_USER_ACCOUNT_MAX_LEN);
	aInfo.nValue = Address;
	while (i != m_clientIDs.end())
	{
		if (i->second)
		{
			CustomSend(i->first, s2c_gatewayinfo, &aInfo);
		}
		i++;
	}
	Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////

KGatewayDataProcess::KGatewayDataProcess(unsigned long nID, unsigned long Address, short Port)
{
	m_nClientID = nID;
	m_Address = Address;
	m_Port = Port;

	m_nGameID = 0;
	m_hClosed = KPICreateEvent(NULL, TRUE, FALSE, NULL);

	m_hStartAutoTime = KPICreateEvent(NULL, TRUE, FALSE, NULL);

	m_pServer = NULL;

	m_pConn = NULL;
}

KGatewayDataProcess::~KGatewayDataProcess()
{
	assert(m_pServer == NULL);
}

void KGatewayDataProcess::Close()
{
	if (m_nGameID > 0)
	{
		S3PDBConVBC* pCon = GetDB(1);
		KAccountUser acc;
		acc.Size = sizeof(KAccountUser);
		acc.Type = AccountUser;
		acc.Version = ACCOUNT_CURRENT_VERSION;
		acc.Operate = m_nGameID;
		acc.Account[0] = 0;
		ProRelayClose(pCon, (const IBYTE*)&acc, acc.Size);

		KPIWaitForSingleObject(m_hClosed, INFINITE);
	}
}

void KGatewayDataProcess::AutoTime()
{
	if (!IsStartAutoTime())
		return;
	DWORD nNow = GetTickCount();
	if (nNow - m_LastMoneyTime >= 600000)	//十分钟催一次钱
	{			
		m_LastMoneyTime = nNow;
		S3PDBConVBC* pConn = GetDB(0);
		if (pConn)
			ProAutoTime(pConn);
	}

	if (nNow - m_LastPingTime >= 180000)	//三分钟没有Ping
	{
		KPISetEvent(m_hStop);
	}
}

BOOL KGatewayDataProcess::CheckConnectAddress()
{
	S3PDBConVBC* pConn = GetDB(0);
	if (pConn)
		return S3PAccount::CheckAddress(pConn, m_Address, m_Port) == ACTION_SUCCESS;
	return FALSE;
}

BOOL KGatewayDataProcess::IsStartAutoTime()
{
	return KPIWaitForSingleObject(m_hStartAutoTime, 0) == 1;
}

BOOL KGatewayDataProcess::StartAutoTime()
{
	if (!IsStartAutoTime())
	{
		m_LastPingTime = GetTickCount();
		m_LastMoneyTime = m_LastPingTime;
		KPISetEvent(m_hStartAutoTime);
	}

	return TRUE;
}

void KGatewayDataProcess::ProcessClientData(const void * pData, DWORD dwDataSize)
{
	if (pData && dwDataSize > 0)
	{	
		BYTE nFamily = *((char*)pData);
		BYTE nProtocol = *(((char*)pData) + 1);
		if (nFamily == pf_normal && nProtocol == c2s_ping)
		{
			DWORD nSecond = m_LastPingTime;
			m_LastPingTime = GetTickCount();
#pragma pack(push, 1)
			typedef struct
			{
				BYTE			nFamily;
				BYTE			ProtocolType;
				DWORD			m_dwTime;
			} PING_COMMAND;
#pragma pack(pop)
			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();
			PING_COMMAND* p = (PING_COMMAND*)pData;
			PING_COMMAND PingCmd;
			PingCmd.nFamily = pf_normal;
			PingCmd.ProtocolType = s2c_ping;
			PingCmd.m_dwTime = p->m_dwTime;
			pSocket->SendData(m_nClientID, &PingCmd, sizeof(PING_COMMAND));
			in_addr ar;
			ar.s_addr = m_Address;
			gTrace("Relay %s(%s): Ping %d(s)", m_ServerName.c_str(), inet_ntoa(ar), (m_LastPingTime - nSecond) / 1000);
		}
		else if (nFamily == pf_relay && (nProtocol == relay_c2c_data || nProtocol == relay_s2c_loseway))
		{
			ProRelayIPData((const IBYTE*)pData, dwDataSize);
		}
		else
		{
			//不需要数据库的在前面处理
			S3PDBConVBC* pConn = GetDB(10);
			ProcessData(pConn, pData, dwDataSize);
		}
	}
}

DWORD KGatewayDataProcess::ProcessData(S3PDBConVBC* pConn, const void* pData, DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (pConn == NULL || pData == NULL || dwDataSize <= 0)
	{
		return bRet;
	}

	BYTE nFamily = *((char*)pData);
	BYTE nProtocol = *(((char*)pData) + 1);

	S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

	BOOL bStopConn = FALSE;

	if (nFamily == pf_normal && nProtocol == c2s_gatewayverify)
	{
		if (m_nGameID > 0)
		{
			KAccountUserReturn aReturn;
			aReturn.Size = sizeof(KAccountUserReturn);
			aReturn.Type = AccountUserReturn;
			aReturn.Operate = ((KAccountUserLoginInfo*)(((char*)pData) + 1))->Operate;
			aReturn.nReturn = E_ACCOUNT_ACCESSDENIED;
			aReturn.Account[0] = 0;
			pSocket->CustomSend(m_nClientID, s2c_gatewayverify, &aReturn);
			bRet = FALSE;
			bStopConn = TRUE;
		}
		else
		{
			bRet = ProRelayVerify(pConn, ((char*)pData) + 2, dwDataSize - 2);
			if (bRet)
			{
				StartAutoTime();

				pSocket->NotifyRelayLoginInfo(m_nGameID, m_nClientID, m_Address, m_ServerName);
			}
			else
				bStopConn = TRUE;
		}
	}
	else if (nFamily == pf_normal && nProtocol == c2s_gatewayinfo)
	{
		if (m_nGameID == 0)
		{
			bStopConn = TRUE;
		}
		else
		{
			bRet = ProRelayInfo(pConn, ((char*)pData) + 2, dwDataSize - 2);
		}
	}
	else if (nFamily == pf_normal && nProtocol == c2s_accountlogout)	//unlock account
	{
		if (m_nGameID == 0)
		{
			bStopConn = TRUE;
		}
		else
		{
			bRet = ProAccountLogout(pConn, ((char*)pData) + 2, dwDataSize - 2);
		}
	}
	else if (nFamily == pf_normal && nProtocol == c2s_gamelogin)	//freeze account
	{
		if (m_nGameID == 0)
		{
			bStopConn = TRUE;
		}
		else
		{
			bRet = ProGameLogin(pConn, ((char*)pData) + 2, dwDataSize - 2);
		}
	}
	else
	{
		if (m_nGameID > 0)
		{
			if (nFamily == pf_relay && nProtocol == relay_c2c_askwaydata)
			{
				bRet = ProRelayAskData(pConn, (const IBYTE*)pData, dwDataSize);
			}
			else if (nFamily == pf_gamemaster)
			{
				bRet = ProGMFamily(pConn, (const IBYTE*)pData, dwDataSize);
			}
		}
		else
			bStopConn = TRUE;
	}
	
	if (bStopConn)
	{
		KPISetEvent(m_hStop);
	}

	return bRet;
}

BOOL KGatewayDataProcess::ProRelayVerify(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
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
				if (pSocket->FindGatewayByID(nGameID) == NULL)
				{
					m_ServerName = pAccInfo->Account;
					m_nGameID = nGameID;
					bRet = TRUE;
				}
				else
				{
					aReturn.nReturn = E_ACCOUNT_EXIST;
				}
			}

			char szmac[15];
			sprintf(szmac, "%02X%02X-%02X%02X-%02X%02X", pAccInfo->MacAddress[0], pAccInfo->MacAddress[1], pAccInfo->MacAddress[2], pAccInfo->MacAddress[3], pAccInfo->MacAddress[4], pAccInfo->MacAddress[5]);
			szmac[14] = 0;
			if (bRet)
				gTrace("Relay %s(%s)%s: Verify OK !", m_ServerName.c_str(), inet_ntoa(ar), szmac);
			else
			{
				if (aReturn.nReturn == E_ACCOUNT_EXIST)
					gTrace("Relay %s(%s)%s: Already login !", m_ServerName.c_str(), inet_ntoa(ar), szmac);
				else if (aReturn.nReturn == E_ACCOUNT_OR_PASSWORD)
					gTrace("Relay %s(%s)%s: Name, Password !", m_ServerName.c_str(), inet_ntoa(ar), szmac);
				else if (aReturn.nReturn == E_ADDRESS_OR_PORT)
					gTrace("Relay %s(%s)%s: Address, Port !", m_ServerName.c_str(), inet_ntoa(ar), szmac);
			}
			
			pSocket->CustomSend(m_nClientID, s2c_gatewayverify, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_gatewayverify);
	}
	else
		gTrace("Protocol %d data is error", c2s_gatewayverify);
	return bRet;
}

BOOL KGatewayDataProcess::ProRelayClose(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize == sizeof(KAccountUser))
	{
		KAccountUser* pAccInfo = (KAccountUser*)(lpData);
		if (pAccInfo->Version == ACCOUNT_CURRENT_VERSION &&
			pAccInfo->Type == AccountUser)
		{
			S3PAccount::ServerLogout(pConn, pAccInfo->Operate, 0);
			
			bRet = TRUE;
			KPISetEvent(m_hClosed);
		}
	}
	return bRet;
}

BOOL KGatewayDataProcess::ProRelayInfo(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize == sizeof(KServerInfo))
	{
		KServerInfo* pSerInfo = (KServerInfo*)(lpData);
		if (pSerInfo->Version == ACCOUNT_CURRENT_VERSION &&
			pSerInfo->Type == ServerInfo)
		{
			DWORD dValue = 0;
			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();
			unsigned long nGameID;
			if (pSerInfo->nServerType & server_OnlinePlayerCount)
			{
				nGameID = 0;
				pSocket->FindGatewayByServerName(pSerInfo->Account, nGameID);
				S3PAccount::GetAccountCount(pConn, nGameID, TRUE, dValue);
			}
			else if (pSerInfo->nServerType & server_RegisterCount)
			{
				//所有注册用户
				S3PAccount::GetAccountCount(pConn, 0, FALSE, dValue);
			}
			else if (pSerInfo->nServerType & server_PlayerWhere)
			{
				DWORD nID = 0;
				int iRet = S3PAccount::GetAccountGameID(pConn, pSerInfo->Account, nID);
				if (iRet == E_ACCOUNT_OR_PASSWORD)
					dValue = 0xFFFFFFFF;
				else if (iRet == ACTION_SUCCESS)
				{
					if (nID >= 0)
					{
						KGatewayDataProcess* p = pSocket->FindGatewayByID(nID);
						if (p)
						{
							dValue = p->m_Address;
						}
					}
					else
						dValue = 0;
				}
			}
			else if (pSerInfo->nServerType & server_PlayerWhereID)
			{
				DWORD nID = 0;
				int iRet = S3PAccount::GetAccountGameID(pConn, pSerInfo->Account, nID);
				if (iRet == E_ACCOUNT_OR_PASSWORD)
					dValue = 0xFFFFFFFF;
				else if (iRet == ACTION_SUCCESS)
				{
					if (nID >= 0)
					{
						KGatewayDataProcess* p = pSocket->FindGatewayByID(nID);
						if (p)
						{
							dValue = p->m_nClientID;
						}
					}
					else
						dValue = 0xFFFFFFFF;
				}
			}
			else if (pSerInfo->nServerType & server_GWRelayID)
			{
				if (pSocket->FindGatewayByServerName(pSerInfo->Account, nGameID))
				{
					if (nGameID >= 0)
					{
						KGatewayDataProcess* p = pSocket->FindGatewayByID(nGameID);
						if (p)
						{
							dValue = p->m_nClientID;
						}
					}
					else
						dValue = 0xFFFFFFFF;
				}
			}
			else
				assert(0);
			
			KServerInfo aReturn;
			aReturn.Size = sizeof(KServerInfo);
			aReturn.Type = ServerInfo;
			aReturn.Operate = 0;
			aReturn.Version = ACCOUNT_CURRENT_VERSION;
			strncpy(aReturn.Account, pSerInfo->Account, LOGIN_USER_ACCOUNT_MAX_LEN);
			aReturn.nServerType = pSerInfo->nServerType;
			aReturn.nValue = dValue;
			
			pSocket->CustomSend(m_nClientID, s2c_gatewayinfo, &aReturn);

			bRet = TRUE;
		}
		else
			gTrace("Protocol %d data is error", c2s_gatewayinfo);
	}
	else
		gTrace("Protocol %d data is error", c2s_gatewayinfo);
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
			
			aReturn.nReturn = S3PAccount::UnlockAccount(pConn, pAccInfo->Account);

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
				szDetail = " Failed !";
			}

			gTrace("Gateway %s(%s): User %s Unlock%s", m_ServerName.c_str(), inet_ntoa(ar), pAccInfo->Account, szDetail);
			
			pSocket->CustomSend(m_nClientID, s2c_accountlogout, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_accountlogout);
	}
	else
		gTrace("Protocol %d data is error", c2s_accountlogout);
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
			aReturn.nReturn = S3PAccount::FreezeAccount(pConn, pAccInfo->Account);

			S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

			char* szDetail = NULL;
			in_addr ar;
			ar.s_addr = m_Address;
			if (aReturn.nReturn == ACTION_SUCCESS)
			{
				bRet = TRUE;
				char szS[200];
				sprintf(szS, "GlobalExecute(\"dw KickOutAccount([[%s]])\")", pAccInfo->Account);
				ExecuteAction(szS);//踢人
				gTrace("Account(%s) is KickOff!", pAccInfo->Account);
				szDetail = " Success !";
			}
			else
			{
				szDetail = " Failed !";
			}

			gTrace("Gateway %s(%s): User %s Freeze Account%s", m_ServerName.c_str(), inet_ntoa(ar), pAccInfo->Account, szDetail);
			
			pSocket->CustomSend(m_nClientID, s2c_gamelogin, &aReturn);
		}
		else
			gTrace("Protocol %d data is error", c2s_gamelogin);
	}
	else
		gTrace("Protocol %d data is error", c2s_gamelogin);
	return bRet;
}

BOOL KGatewayDataProcess::ProRelayIPData(const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize >= sizeof(RELAY_DATA))
	{
		S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

		RELAY_DATA* pIPData = (RELAY_DATA*)lpData;
		if (pIPData->routeDateLength + sizeof(RELAY_DATA) != dwDataSize)
			return FALSE;

		if (pIPData->nFromIP == 0)	//刚进入Relay体系，填写nFromIP和nFromRelayID
		{
			pIPData->nFromIP = m_Address;
			pIPData->nFromRelayID = m_nClientID;
		}

		if (pIPData->nToIP == 0)	//目的地已经到达，开始处理实际协议
		{
			char* pRelayPackage = (char*)(pIPData + 1);
			ProcessClientData(pRelayPackage, pIPData->routeDateLength);
			bRet = TRUE;
		}
		else if (pIPData->nToIP == pSocket->m_SelfAddress)	//最后一个Relay
		{
			DWORD nClient = pIPData->nToRelayID;
			pIPData->nToIP = 0;
			pIPData->nToRelayID = 0;
			
			pSocket->SendData(nClient, pIPData, dwDataSize);
			bRet = TRUE;
		}
		else
		{
			DWORD nClient = 0;
			if (pSocket->FindGatewayClientByAddress(pIPData->nToIP, nClient))
			{
				pSocket->SendData(nClient, pIPData, dwDataSize);
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL KGatewayDataProcess::ProRelayAskData(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	if (NULL != lpData && dwDataSize >= sizeof(RELAY_ASKWAY_DATA))
	{
		S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

		RELAY_ASKWAY_DATA* pIPData = (RELAY_ASKWAY_DATA*)lpData;

		if (pIPData->routeDateLength + pIPData->wMethodDataLength + sizeof(RELAY_ASKWAY_DATA) != dwDataSize)
			return FALSE;

		if (pIPData->seekMethod == rm_account_id ||
			pIPData->seekMethod == rm_gm)
		{
			if (pIPData->nFromIP == 0)	//刚进入Relay体系，填写nFromIP和nFromRelayID
			{
				pIPData->nFromIP = m_Address;
				pIPData->nFromRelayID = m_nClientID;
			}

			KGatewayDataProcess* p = NULL;

			if (pIPData->seekMethod == rm_account_id)
			{
				DWORD nGameID = 0;
				char name[33];
				strncpy(name, lpData + sizeof(RELAY_ASKWAY_DATA), 32);
				if (ACTION_SUCCESS == S3PAccount::GetAccountGameID(pConn, name, nGameID))
					p = pSocket->FindGatewayByID(nGameID);
			}
			else if (pIPData->seekMethod == rm_gm)
			{
				pSocket->BroadGMData(lpData, dwDataSize);
				bRet = TRUE;
			}
			
			if (p)
			{
				pSocket->SendData(p->m_nClientID, lpData, dwDataSize);
				bRet = TRUE;
			}
		}

		if (!bRet)	//地址不可达，通知来源relay_s2c_loseway
		{
			size_t sizeLose = sizeof(RELAY_DATA) + dwDataSize;
			RELAY_DATA* pLoseData = (RELAY_DATA*)_alloca(sizeLose);

			pLoseData->ProtocolFamily = pIPData->ProtocolFamily;
			pLoseData->ProtocolID = relay_s2c_loseway;
			pLoseData->nToIP = pIPData->nFromIP;
			pLoseData->nToRelayID = pIPData->nFromRelayID;
			pLoseData->nFromIP = pSocket->m_SelfAddress;
			pLoseData->nFromRelayID = -1;
			pLoseData->routeDateLength = dwDataSize;
			memcpy(pLoseData + 1, pIPData, dwDataSize);

			pSocket->SendData(m_nClientID, pLoseData, sizeLose);
		}
	}
	return bRet;
}

BOOL KGatewayDataProcess::ProGMFamily(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize)
{
	int nResult = ACTION_SUCCESS;
	switch(((GM_HEADER*)lpData)->ProtocolType)
	{
	case gm_c2s_execute:
		_ASSERT(0);
		break;
	case gm_c2s_disable:
		_ASSERT(0);
		break;
	case gm_c2s_enable:
		_ASSERT(0);
		break;
	case gm_c2s_tracking:
		_ASSERT(0);
		break;
	case gm_c2s_setrole:
		_ASSERT(0);
		break;
	case gm_c2s_getrole:
		_ASSERT(0);
		break;
	case gm_c2s_findplayer:
		_ASSERT(0);
		break;
	case gm_c2s_unlock:
		{
			DWORD nGameID = 0;
			S3PAccount::GetServerID(pConn, ((GM_UNLOCK_ACCOUNT*)lpData)->AccountName, nGameID);
			if (nGameID > 0)
			{
				nResult = S3PAccount::UnlockServer(pConn, nGameID);
			}
		}
		break;
	default:
		_ASSERT(0);
		break;
	}
	return (ACTION_SUCCESS == nResult);
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

	BOOL bValidID = CheckConnectAddress();
	
	while (bValidID)
	{
		DWORD dwResult =
			KPIWaitForSingleObject(m_hStop, 0);
		if (dwResult == 1)
			break;
		else if (dwResult == 2)
		{
			size_t datalength = 0;

			const void *pData = m_pServer->GetPackFromClient(m_nClientID, datalength);

			AutoTime();

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

	Close();

	if (m_pConn)
	{
		if (pDB->ReturnDBCon(m_pConn))
			m_pConn = NULL;
	}

	m_pServer->ShutdownClient(m_nClientID);

	return dwRet;
}

HANDLE KGatewayDataProcess::Start(IServer* pServer)
{
	assert(pServer);
	if (NULL == m_pServer)
	{
		m_pServer = pServer;
		pServer->AddRef();
	}

	return KThread::Start();
}

BOOL KGatewayDataProcess::Stop()
{
	BOOL bRet = KThread::Stop();
	if (bRet && m_pServer)
	{
		m_pServer->Release();
		m_pServer = NULL;
	}

	return bRet;
}

BOOL KGatewayDataProcess::ProAutoTime(S3PDBConVBC* pConn)
{
	BOOL bRet = FALSE;
	DWORD dwMin = 0;
	dwMin = 1800;	//默认30分钟
	char szMessage[128];
	if (dwMin > 0)
	{
		AccountTimeList TimeList;
		if (S3PAccount::GetAccountsTime(pConn, m_nGameID, dwMin, TimeList) == ACTION_SUCCESS)
		{
			AccountTimeList::iterator i = TimeList.begin();
			while (i != TimeList.end())
			{
				if (i->nTime == 0)		//踢人
				{
					char szS[200];
					sprintf(szS, "GlobalExecute(\"dw KickOutAccount([[%s]])\")", i->Account);
					ExecuteAction(szS);//踢人
					gTrace("Account(%s) is KickOff!", i->Account);
				}
				else	//催促充值
				{
					int nLen = 0;
					if (i->nTime >= 60)
						nLen = sprintf(szMessage, "您在游戏中还可以停留%d分钟, 请赶快充值!", i->nTime / 60);
					else
						nLen = sprintf(szMessage, "您在游戏中还可以停留%d秒, 请赶快充值!", i->nTime);
					SendSystemInfo(m_nClientID, i->Account, "GM", szMessage, nLen);
				}
				i++;
			}
			bRet = TRUE;
		}
	}
	return bRet;
}

void KGatewayDataProcess::ExecuteAction(char* szAccount, char* szScript)
{
	int nszLen = 0;
	if (szScript)
		nszLen = strlen(szScript);
	GM_EXECUTE_COMMAND exe;
	exe.ProtocolFamily = pf_gamemaster;
	exe.ProtocolType = gm_c2s_execute;
	strncpy(exe.AccountName, szAccount, LOGIN_USER_ACCOUNT_MAX_LEN);
	exe.wLength = nszLen;
	exe.wExecuteID = 1;

	S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

	pSocket->SendRelayData(m_nClientID, szAccount, &exe, sizeof(exe), szScript, nszLen);
}

void KGatewayDataProcess::ExecuteAction(char* szScript)
{
	int nszLen = 0;
	if (szScript)
		nszLen = strlen(szScript);
	GM_EXECUTE_COMMAND exe;
	exe.ProtocolFamily = pf_gamemaster;
	exe.ProtocolType = gm_c2s_execute;
	strncpy(exe.AccountName, "GM", LOGIN_USER_ACCOUNT_MAX_LEN);
	exe.wLength = nszLen;
	exe.wExecuteID = 1;

	S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

	pSocket->SendRelayData(m_nClientID, &exe, sizeof(exe), szScript, nszLen);
}

void KGatewayDataProcess::SendSystemInfo(unsigned long uID, char* szAccount, char *lpszSendName, char *lpszSentence, int nSentenceLength)
{
	size_t chatsize = 1 + sizeof(CHAT_CHANNELCHAT_SYNC) + nSentenceLength;

	BYTE* pFamily = (BYTE*)_alloca(chatsize);
	*pFamily = pf_chat;
	CHAT_CHANNELCHAT_SYNC* pCccSync= (CHAT_CHANNELCHAT_SYNC*)(pFamily + 1);
	pCccSync->ProtocolType = chat_channelchat;
	pCccSync->wSize = chatsize - 1;
	pCccSync->packageID = -1;
	strncpy(pCccSync->someone, lpszSendName, _NAME_LEN - 1); // 可能需要根据玩家身份改动
	pCccSync->channelid = -1;
	pCccSync->sentlen = nSentenceLength;
	memcpy(pCccSync + 1, lpszSentence, nSentenceLength);

	S3PDBSocketPool* pSocket = S3PDBSocketPool::Instance();

	pSocket->SendRelayData(uID, szAccount, pFamily, chatsize, NULL, 0);
}