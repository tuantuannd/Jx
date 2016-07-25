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
#include "AccountLoginDef.h"
#include "S3PDBConVBC.h"
#include <list>

using namespace std;

interface IServer;
class S3PDBConVBC;
class S3PDBSocketPool;

#pragma pack(1)
struct UserName
{
	char m_szName[LOGIN_USER_ACCOUNT_MAX_LEN];
};
#pragma pack()

struct UserNameCmp
{
	bool operator() (const UserName& Name1, const UserName& Name2) const
	{
		return stricmp(Name1.m_szName, Name2.m_szName) < 0;
	}
};


class KGatewayDataProcess : public KThread_CS
{
public:
	KGatewayDataProcess();
	~KGatewayDataProcess();

	char m_ServerName[LOGIN_USER_ACCOUNT_MAX_LEN];
	unsigned long m_nGameID;
	unsigned long m_Address;						//服务器域名地址
	unsigned long m_nConnectID;
	short m_Port;									//服务器端口号

	BOOL ConnectErrorWork(unsigned long nID, unsigned long Address, short Port);
	BOOL ConnectFreeWork(unsigned long nID, unsigned long Address, short Port);
	BOOL ConnectFreeForErrorWork(unsigned long nID, unsigned long Address, short Port);
	BOOL ConnectAgainWork(unsigned long nID, unsigned long Address, short Port);
	BOOL OutofWork(unsigned long nID);
	BOOL IsWork();
	BOOL IsWorkAgain();
	BOOL IsWantData();
	BOOL IsError();
	int GetStatus();

	virtual BOOL Stop();
	virtual HANDLE Start(IServer* pServer, DWORD nNow);

	enum
	{
		gdp_free = 0,	//可以接受连接,新建时剩余时间启动
		gdp_verify,	//正在等待校验
		gdp_work,	//校验通过,正常工作
		gdp_again,	//正在等待重连,剩余时间启动
		gdp_verifyagain,	//正在等待重连校验
		gdp_errorconnect,	//当新连接被发现IP重复后,被设置的状态
	};
	
protected:
	S3PDBConVBC* m_pConn;
	S3PDBConVBC* GetDB(DWORD nSleep);
	void AutoTime();
	BOOL CheckConnectAddress(DWORD Address);
	void ProcessClientData(const void * pData, DWORD dwDataSize);

	int m_Status;
	DWORD m_nStatusTime;
	int m_nLeftStatusTime;

	DWORD ProcessData(S3PDBConVBC* pConn, const void* pData, DWORD dwDataSize);

	BOOL (KGatewayDataProcess::*ProcessFunc[16])(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwSize);

	BOOL ProGetwayVerify(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwSize);
	BOOL ProAccountLogin(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwSize);
	BOOL ProGameLogin(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwSize);
	BOOL ProAccountLogout(S3PDBConVBC* pConn, const IBYTE* lpData, const DWORD dwSize);

	BOOL ProUndefine(const IBYTE* lpData, const DWORD dwSize);

	DWORD m_LastPingTime;	//用于防止连接的客户端是否死掉
	int m_nLeftPingTime;	//用于控制是否处理接受到的Ping的时间间隔

	DWORD m_LastSendPingTime;	//用于快速判断连接是否中断

	DWORD Main(LPVOID lpParam);

	IServer* m_pServer;

	typedef std::map<UserName, DWORD, UserNameCmp> BLACKLIST;
	BLACKLIST m_UserNames;

	void SendPing(DWORD dwTime);

	bool SetStatus(int nNews);
	bool IsVerifyTimeout();
};

typedef list<KGatewayDataProcess*> GatewayArray;

class S3PDBSocketPool : public K_CS
{
public:
	static S3PDBSocketPool* Instance();
	static void ReleaseInstance();

	virtual ~S3PDBSocketPool();

	virtual BOOL Stop();
	virtual HANDLE Start(IServer* pServer, int nMax);

	BOOL AddUserClientID(unsigned long uID);
	BOOL RemoveUserClientID(unsigned long uID);
	BOOL ShowAllClientInfo();

	BOOL SendData(unsigned long uID, const void * const	pData, const size_t	&datalength);
	BOOL CustomSend(unsigned long uID, char nProtocol, KAccountHead* pSend);
protected:
	S3PDBSocketPool();

	GatewayArray m_clientIDs;

	static S3PDBSocketPool* m_pInstance;
	
	IServer* m_pServer;
};

#endif	// _S3PDBSOCKETPOOL_H_
