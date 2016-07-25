/********************************************************************
	created:	2003/05/31
	file base:	Intercessor
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_INTERCESSOR_H__
#define __INCLUDE_INTERCESSOR_H__

#include "IPlayer.h"
#include "IGServer.h"

#include "GameServer.h"
#include "GamePlayer.h"

#include "msg_define.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "Library.h"
#include "Event.h"

#include "Network.h"

#include <map>
#include <list>

class CIntercessor
{
public:
	
	explicit CIntercessor( unsigned long lnMaxPlayerCount, CNetwork &theNetworkConfig );
	virtual ~CIntercessor();

	bool Create( HWND hwndViewer );
	void Destroy();
	
	size_t		GetGameServerCount();
	const char *GetGameServerInfo( const unsigned long &ulnID );

	/*
	 * Callback function for network
	 */
//	static void __stdcall AccountEventNotify( LPVOID lpParam, 
//						const unsigned long &ulnEventType );

	static void __stdcall DBRoleEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType );

	static void __stdcall PlayerEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType );

protected:

	HWND m_hwndViewer;
	
	/*
	 * Network
	 */
	bool StartupNetwork();
	void CleanNetwork();

	typedef HRESULT ( __stdcall * pfnCreateServerInterface )( REFIID riid, void **ppv );
	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	static OnlineGameLib::Win32::CLibrary	m_theHeavenLib;
	static OnlineGameLib::Win32::CLibrary	m_theRainbowLib;

//	IClient		*m_pAccSvrClient;
	IClient		*m_pDBRoleClient;
	IServer		*m_pGameSvrServer;
	IServer		*m_pPlayerServer;

	/*
	 * Helper function
	 */
	void _PlayerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType );

//	void _AccountEventNotify( const unsigned long &ulnEventType );
	void _DBRoleEventNotify( const unsigned long &ulnEventType );
	
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

	typedef std::map< size_t, IPlayer * >	PLAYER_MAP;

	PLAYER_MAP		m_thePlayers;

	/*
	 * Loader balance information
	 */

	
};

#endif // __INCLUDE_INTERCESSOR_H__