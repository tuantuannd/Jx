#pragma once
#pragma warning(disable:4786)
#include <map>
#include <vector>
#include "KRoleInfomation.h"
#include "IServer.h"
#include "HeavenInterface.h"
#include "KRoleDBHeader.h"

//typedef map<unsigned long ulIdentity, KRoleBlockProcess >;
namespace RoleDBServer
{
	using namespace std;
	
	class	KClientUser
	{
	public:
		KClientUser();
		char		 m_szClientName[100];
		TProcessData *m_pCurrentRecvProcessData;
		unsigned long m_nCurGetRecvLen;
		TProcessData  *m_pCurrentSendProcessData;
		unsigned long m_nCurSendLen;
		unsigned long m_nProcessLoadCount;
		unsigned long m_nProcessWriteCount;
		unsigned long m_ID;		
		
		
		
		//		KProcessDataSet m_DBProcessSet;		//待数据库接口处理的数据集
		//KProcessDataSet m_UserProcessSet;	//待发往服务器的数据集
	};
	typedef map<unsigned long , KClientUser *> KClientUserSet;
	
	class KNetServer
	{
	public:
		int Init();
		int	Receive();
		int Send();
		//private:
		int	Receive(unsigned long nId);//
		int	Send(unsigned long nId);//Send
		//int	SendBuffer(TMsg * pMsgBuffer);
		int	AppendData(unsigned long nId, TRoleNetMsg * pMsgBuffer, size_t nMsgLen);
		void InsertProcessData2Set(int nId);
		void ClearCurProcessData(unsigned long nId);
		void RegisterClient(unsigned long nId);
		void DestoryClient(unsigned long nId);
		IServer * m_pNetServer;
		KClientUserSet	m_ClientUserSet;	//与其连接的游戏服务器集合
	};
	extern KNetServer g_NetServer;
	
	
	
	typedef vector<TProcessData *> KProcessDataSet;
}

