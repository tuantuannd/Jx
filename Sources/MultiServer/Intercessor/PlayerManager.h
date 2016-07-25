/********************************************************************
	created:	2003/05/06
	file base:	PlayerManager
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_PLAYERMANAGER_H__
#define __INCLUDE_PLAYERMANAGER_H__

#include "IPlayer.h"
#include "GamePlayer.h"
#include "IGServer.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "Library.h"
#include "Event.h"

#include "Network.h"

#pragma warning(disable : 4786)  // identifier was truncated to '255' characters 
                                 // in the debug information

#include <map>
#include <list>

/*
 * CPlayerManager class
 */
class CPlayerManager
{
public:
	
	explicit CPlayerManager( unsigned long lnMaxPlayerCount, CNetwork &theNetworkConfig );
	virtual ~CPlayerManager();

	bool Create();
	void Destroy();

	/*
	 * Callback function for network
	 */
	static void __stdcall AccountEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType );

	static void __stdcall DBRoleEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType );

	static void __stdcall PlayerEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType );

	static void __stdcall GameSvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType );
	
protected:

	/*
	 * Network
	 */
	bool StartupNetwork();
	void CleanNetwork();

	typedef HRESULT ( __stdcall * pfnCreateServerInterface )( REFIID riid, void **ppv );
	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	static OnlineGameLib::Win32::CLibrary	m_theHeavenLib;
	static OnlineGameLib::Win32::CLibrary	m_theRainbowLib;

	IClient		*m_pAccSvrClient;
	IClient		*m_pDBRoleClient;
	IServer		*m_pGameSvrServer;
	IServer		*m_pPlayerServer;

	/*
	 * Helper function
	 */
	void _PlayerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType );
	
	void _GameServerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType );
	
	/*
	 * Listen and accept data from network
	 */
	HANDLE m_hWorkingThread;
	HANDLE m_hHelperThread;

	static DWORD WINAPI	WorkingThreadFunction( void *pV );
	int								Working();

	static DWORD WINAPI	HelperThreadFunction( void *pV );
	int								Helper();

	bool		AnalyzePlayerRequire( size_t index, const void *pData, size_t datalength );

private:

	OnlineGameLib::Win32::CEvent m_hQuitEvent;
	OnlineGameLib::Win32::CEvent m_hStartupWoringThreadEvent;
	OnlineGameLib::Win32::CEvent m_hStartupHelperThreadEvent;

	CNetwork	&m_theNetworkConfig;
	
	/*
	 * Player information
	 */
	const unsigned long m_lnMaxPlayerCount;

	OnlineGameLib::Win32::CCriticalSection	m_csPlayerAction;
	OnlineGameLib::Win32::CCriticalSection	m_csUsedClientList;

	typedef std::map< size_t, IPlayer * >	PLAYER_MAP;

	PLAYER_MAP		m_thePlayers;

	typedef std::list< size_t >		LIST;
	
	LIST			m_usedClientNode;

	/*
	 * Game server information
	 */
	OnlineGameLib::Win32::CCriticalSection	m_csGameSvrAction;
	
	typedef std::map< size_t, IGServer * >	GAMESVR_MAP;

	GAMESVR_MAP				m_theGameServers;
	
	/*
	 * Loader balance information
	 */

	/*
	 * ID of map convert to ID of game server
	 */
	OnlineGameLib::Win32::CCriticalSection	m_csMI2GSAction;

	typedef std::map< size_t, size_t >		MAPID_MAPTO_GAMESVR;

	MAPID_MAPTO_GAMESVR		m_theMapIDMapToGameSvr;

	/*
	 * ID of player convert to ID of game server
	 */
	OnlineGameLib::Win32::CCriticalSection	m_csPI2GSAction;

	typedef std::map< size_t, size_t >		PLAYERID_MAPTO_GAMESVR;

	PLAYERID_MAPTO_GAMESVR	m_thePlayerIDMapToGameSvr;
	
};

#endif // __INCLUDE_PLAYERMANAGER_H__