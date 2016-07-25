/********************************************************************
	created:	2003/05/15
	file base:	Player
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_PLAYER_H__
#define __INCLUDE_PLAYER_H__

#include "tstring.h"
#include "Thread.h"
#include "Event.h"
#include "Buffer.h"
#include "Library.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "CriticalSection.h"

class IPlayer
{
public:
	virtual bool ConnectToGateway() = 0;
	
	virtual ~IPlayer(){};
};

class CPlayer : public IPlayer, protected OnlineGameLib::Win32::CThread
{
public:

	explicit CPlayer( const char *pSrvIP, unsigned short nPort, const char *pAccName );
	virtual ~CPlayer();

	virtual bool ConnectToGateway();

	using CThread::Start;

protected:	
	void Free();
	void LoginGateway();

	void ProcessGateway();
	void ProcessGatewayMsg();
	void ProcessGatewayLoop();

	void ProcessGameSvr();
	void ProcessGameSvrMsg();
	void ProcessGameSvrLoop();
	
	bool ReConnectGateway();
	bool ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid );
	void ShutdownGateway();
	void ShutdownGameSvr();
	bool OnAccountLoginNotify(const char* pMsg);
	bool OnPlayerLoginNotify(const char* pMsg);
	int  OnGetRoleList(const char* pMsg);
	void SyncEnd();

	void SpeakWord( const char *pText, size_t size /*0*/ );
	void WalkTo();
	
	void NotifyRobotManager();

	virtual int Run();

	static void __stdcall ClientEventNotify( LPVOID lpParam, const unsigned long &ulnEventType );
	void _ClientEventNotify( const unsigned long &ulnEventType );

	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );
	static OnlineGameLib::Win32::CLibrary	s_theRainbowLib;

	// 控制线程的退出的事件
	OnlineGameLib::Win32::CEvent			m_hQuit;

private:

	IClient		*m_pClientToGateWay;
	IClient		*m_pClientToGameSvr;

	const OnlineGameLib::Win32::_tstring	m_sGatewayIP;
	const unsigned short					m_nGatewayPort;
	const OnlineGameLib::Win32::_tstring	m_sAccName;		// 帐号的名字，密码的名字，和角色的名字是一致的

	enum
	{
		enumSleep			= 0x10,			// 还未启动网络
		enumStartup,						// 启动网络	
		enumConnectToGateway,				// 开始连接网关
		enumConnectToGatewaySucess,			// 连接网关成功
		enumGetRoleList,					// 从网关得到角色列表
		enumConnectToGameSvr,				// 从网关得到加入游戏服务器的通告
		enumPlayGame						// 初始同步结束，开始游戏
	};

	void SetStatus( LONG nStatus ) { ::InterlockedExchange( const_cast< LONG * >( &m_nStatus ), nStatus ); }
	LONG GetStatus() { return ::InterlockedExchange( const_cast< LONG * >( &m_nStatus ), m_nStatus ); }

	const LONG	m_nStatus;

	static DWORD	m_dwTargetID;
	DWORD			m_dwID;
	POINT			m_ptCurPos;
	POINT			m_ptTagPos;
};

using OnlineGameLib::Win32::CCriticalSection;
extern CCriticalSection		g_csPlayerList;
#endif // __INCLUDE_PLAYER_H__