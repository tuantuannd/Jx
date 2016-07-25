//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketPool.h		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBSOCKETPOOL_H_
#define _S3PDBSOCKETPOOL_H_

#include "KThread.h"
#include <map>
#include "GlobalDTD.h"
#include "S3PDBConVBC.h"
#include <list>
#include "../S3AccServer/AccountLoginDef.h"

using namespace std;

interface IServer;
class S3PDBConVBC;
class S3PDBSocketPool;

class KGatewayDataProcess : public KThread_CS
{
public:
	KGatewayDataProcess(unsigned long nID, unsigned long Address, short Port);
	~KGatewayDataProcess();
	std::string m_ServerName;
	unsigned long m_nGameID;
	unsigned long m_Address;						//服务器域名地址
	unsigned long m_nClientID;
	virtual BOOL Stop();
	virtual HANDLE Start(IServer* pServer);
protected:
	S3PDBConVBC* m_pConn;
	S3PDBConVBC* GetDB(DWORD nSleep);
	void Close();
	void AutoTime();
	BOOL CheckConnectAddress();
	void ProcessClientData(const void * pData, DWORD dwDataSize);

	short m_Port;									//服务器端口号
	HANDLE m_hClosed;

	DWORD ProcessData(S3PDBConVBC* pConn, const void* pData, DWORD dwDataSize);

	BOOL ProRelayVerify(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProRelayClose(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProRelayInfo(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProAccountLogout(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProGameLogin(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProAutoTime(S3PDBConVBC* pConn);

	BOOL ProGMFamily(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProRelayAskData(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwDataSize);
	BOOL ProRelayIPData(const IBYTE* lpData, const DWORD dwDataSize);

	HANDLE m_hStartAutoTime;
	DWORD m_LastMoneyTime;
	DWORD m_LastPingTime;
	BOOL IsStartAutoTime();
	BOOL StartAutoTime();
	void ExecuteAction(char* szAccount, char* szScript);
	void ExecuteAction(char* szScript);	//广播执行脚本
	void SendSystemInfo(unsigned long uID, char* szAccount, char *lpszSendName, char *lpszSentence, int nSentenceLength);

	DWORD Main(LPVOID lpParam);

	IServer* m_pServer;
};

typedef map<unsigned long, KGatewayDataProcess*> GatewayIDMap;

class S3PDBSocketPool : public K_CS
{
public:
	static S3PDBSocketPool* Instance();
	static void ReleaseInstance();

	virtual ~S3PDBSocketPool();

	static DWORD m_SelfAddress;
	
	virtual BOOL Stop();
	virtual HANDLE Start(IServer* pServer);

	BOOL AddUserClientID(unsigned long uID);
	BOOL RemoveUserClientID(unsigned long uID);
	BOOL ShowAllClientInfo();
	BOOL StopAllUserClientConnect();
	BOOL FindGatewayByServerName(const char* szServerName, DWORD& nGameID);
	KGatewayDataProcess* FindGatewayByID(unsigned long nGameID);
	BOOL FindGatewayClientByAddress(DWORD nAddress, DWORD& nClientID);
	void BroadGMData(const IBYTE* lpData, const DWORD dwDataSize);

	void NotifyRelayLoginInfo(DWORD nGameID, DWORD nClientID, DWORD Address, const std::string& ServerName);
	void NotifyRelayLogoutInfo(DWORD Address, const std::string& ServerName);

	BOOL SendData(unsigned long uID, const void * const	pData, const size_t	&datalength);
	BOOL CustomSend(unsigned long uID, char nProtocol, KAccountHead* pSend);
	void SendRelayData(unsigned long uID, char* szAccount, const void * const pSrcData1, const size_t& datalength1, const void * const pSrcData2, const size_t& datalength2);
	void SendRelayData(unsigned long uID, const void * const pSrcData1, const size_t& datalength1, const void * const pSrcData2, const size_t& datalength2);

protected:
	S3PDBSocketPool();

	GatewayIDMap m_clientIDs;

	static S3PDBSocketPool* m_pInstance;
	
	IServer* m_pServer;
};

#endif	// _S3PDBSOCKETPOOL_H_
