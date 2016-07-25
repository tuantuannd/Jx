/*****************************************************************************************
//	网络连接，汇集欲发送消息包与派送抵达消息包的代理中心
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-10-6
------------------------------------------------------------------------------------------
	Engine里的KNetClient模块包装实现了网络联接与传送包，此模块为KNetClient具体应用时的代理，
主要用于汇集具体应用中需要发送的网络包，以及把抵达的网络包派送到各相关处理接受模块。
*****************************************************************************************/
#pragma once
//#include "KNetClient.h"
#include "../../../Headers/KProtocol.h"
#include "../../../Headers/iClient.h"

struct iKNetMsgTargetObject;
typedef void (*fnNetMsgCallbackFunc)(void* pMsgData);

typedef HRESULT ( __stdcall * pfnCreateClientInterface )(
			REFIID	riid,
			void	**ppv
		);


//====默认的超时时限====
#define	DEF_TIMEOUT_LIMIT	60000	//60sec

class KNetConnectAgent
{
public:
	KNetConnectAgent();
	~KNetConnectAgent();
	//初始化
	int		Initialize();
	//退出
	void	Exit();

	//建立连接
	int		ClientConnectByNumericIp(const unsigned char* pIpAddress, unsigned short pszPort);
	//关闭连接
	void	DisconnectClient();

	int		ConnectToGameSvr(const unsigned char* pIpAddress, unsigned short uPort, GUID* pGuid);
	void	DisconnectGameSvr();

	//发送消息
	int		SendMsg(const void *pBuffer, int nSize);
	//持续性行为
	void	Breathe();

	void	UpdateClientRequestTime(bool bCancel, unsigned int uTimeLimit = DEF_TIMEOUT_LIMIT);

	//注册抵达消息响应函数
	void	RegisterMsgTargetObject(PROTOCOL_MSG_TYPE Msg, iKNetMsgTargetObject* pObject);

	int		IsConnecting(int bGameServ);

	void	TobeDisconnect();

private:
	bool	ProcessSwitchGameSvrMsg(void* pMsgData);			//处理游戏世界服务器的网络消息

private:
	IClient*				m_pClient;
	IClient*				m_pGameSvrClient;

private:
#define	MAX_MSG_COUNT	1 << (PROTOCOL_MSG_SIZE * 8)
	iKNetMsgTargetObject*	m_MsgTargetObjs[MAX_MSG_COUNT];

	HMODULE					    m_hModule;
	pfnCreateClientInterface    m_pFactroyFun;
	IClientFactory             *m_pClientFactory;



	bool					m_bIsClientConnecting;
	bool					m_bIsGameServConnecting;
	bool					m_bTobeDisconnect;
	unsigned int			m_uClientRequestTime;		//发出情求的时间
	unsigned int			m_uClientTimeoutLimit;
};

extern KNetConnectAgent g_NetConnectAgent;