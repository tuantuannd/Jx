/********************************************************************
	created:	2003/05/11
	file base:	GameServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_GAMESERVER_H__
#define __INCLUDE_GAMESERVER_H__

#include "Buffer.h"

#include "IGServer.h"

#include "KProtocol.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "CriticalSection.h"
#include "Event.h"

#include "KProtocolDef.h"

#include <map>
#include <list>

/*
 * class CGameServer
 */
class CGameServer : public IGServer
{
public:

	typedef struct tagNetworkInterface
	{
		IServer *pServer;
		IClient	*pClient;
		
		HWND	hwndContainer;
		
	}NI, NEAR *PNI, FAR *LPNI;
	
	CGameServer( IServer *pGameSvrServer, 
		IClient	*pAccountClient,
		UINT nIdentityID = ( UINT )( -1 ) );

	virtual ~CGameServer();

	virtual bool Create();
	virtual bool Destroy();
	
	virtual bool Attach( const char *pAccountName );

	virtual void SendText( const char *pText, int nLength, UINT uOption );

	virtual bool AnalyzeRequire( const void *pData, size_t datalength );

	enum enumTask
	{
		enumTaskProtocol = 0x5,

		enumSyncRoleInfo,
		enumPlayerLogicLogout,
	};

	virtual bool DispatchTask( UINT nTask, const void *pData, size_t datalength, WORD nData);

	static bool SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers );

	static void __stdcall GameSvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType );

	static bool Begin( IServer *pGameSvrServer );
	static void End();
	
	static void SendToAll( const char *pText, int nLength, UINT uOption );
	
	static IGServer *QueryServer( UINT nMapID );
	static IGServer *GetServer( size_t nID );

	virtual size_t GetID() { return m_lnIdentityID; };
	virtual size_t GetContent();
	virtual size_t GetCapability() { return m_dwCapability; };
	virtual size_t GetIP( BYTE cType ) { if ( INTERNET_IP == cType ) { return m_nServerIP_Internet; } return m_nServerIP_Intraner; };
	virtual size_t GetIndentify() { return m_lnIdentityID; };
	
protected:

	bool _QueryMapID();
	bool _QueryGameSvrInfo();
	
	bool _RegisterAccount( const void *pData, size_t datalength );
	bool _UpdateMapID( const void *pData, size_t datalength );
	bool _UpdateGameSvrInfo( const void *pData, size_t datalength );
	bool _NotifyPlayerLogin( const void *pData, size_t datalength );
	bool _NotifyEnterGame( const void *pData, size_t datalength );
	bool _NotifyLeaveGame( const void *pData, size_t datalength );
	bool _RequestSvrIP( const void *pData, size_t datalength );

	bool _SyncRoleInfo( const void *pData, size_t datalength, WORD nData);
	bool _PlayerLogicLogout( const void *pData, size_t datalength );
	
	bool LargePackProcess( BYTE cProtocol, const void *pData, size_t datalength );
	bool SmallPackProcess( BYTE cProtocol, const void *pData, size_t datalength );

	bool PushAccount( const char *pAccountName );
	bool PopAccount( const char *pAccountName, bool bUnlockAccount, WORD nExtPoint);

	bool ConsumeMoney( const char *pAccountName );
	bool FreezeMoney( const char *pAccountName, WORD nExtPoint );

private:

	/*
	 * Map ID convert to a game server index
	 */
	static OnlineGameLib::Win32::CCriticalSection	m_csMapIDAction;

	typedef std::list< IGServer * >					stdServerList;
	typedef std::map< size_t, stdServerList >		stdMapIDConvert;

	static stdMapIDConvert							m_theMapIDConvert;

	static bool RegisterServer( UINT nID, IGServer *pGServer );
	
	/*
	 * Process protocol function
	 */
	typedef bool ( CGameServer::*ProcessProtocolFun[c2s_end] )( const void *pData, size_t datalength );

	ProcessProtocolFun	m_theProcessProtocolFun;

private:

	static OnlineGameLib::Win32::CBuffer::Allocator	m_theGlobalAllocator;

	static LONG		m_slnIdentityCounts;
	LONG			m_lnIdentityID;
	
	IServer			*m_pGameSvrServer;
//	IClient			*m_pAccountClient;
	
	OnlineGameLib::Win32::CPackager	m_thePackager;

	OnlineGameLib::Win32::_tstring	m_sServerIPAddr_Intraner;
	OnlineGameLib::Win32::_tstring	m_sServerIPAddr_Internet;

	unsigned long					m_nServerIP_Intraner;
	unsigned long					m_nServerIP_Internet;
	
	unsigned short					m_nServerPort;

	DWORD							m_dwCapability;

	/*
	 * Listen and accept data from network
	 */
	static OnlineGameLib::Win32::CEvent		m_shQuitEvent;
	static OnlineGameLib::Win32::CEvent		m_shStartupManagerThreadEvent;

	static HANDLE m_shManagerThread;

	static DWORD WINAPI	ManagerThreadFunction( void *pParam );

	/*
	 * Game server information
	 */	
	static OnlineGameLib::Win32::CCriticalSection	m_csGameSvrAction;
	
	typedef std::map< size_t, IGServer * >			stdGameSvr;

	static stdGameSvr								m_theGameServers;
	
	/*
	 * Somebody attach the gameserver be save in this table
	 */
	OnlineGameLib::Win32::CCriticalSection			m_csAITS;

	typedef std::map< std::string, void * >			stdAccountAttachIn;

	stdAccountAttachIn								m_theAccountInThisServer;

	bool AttatchAccountToGameServer( const char *pAccountName );
	bool HaveAccountInGameServer( const char *pAccountName );
	bool DetachAccountFromGameServer( const char *pAccountName );
};

#endif // __INCLUDE_GAMESERVER_H__