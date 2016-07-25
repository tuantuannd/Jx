/********************************************************************
	created:	2003/04/08
	file base:	PlayerManager
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_PLAYERMANAGER_H__
#define __INCLUDE_PLAYERMANAGER_H__

#include "IPlayer.h"
#include "GamePlayer.h"
#include "Thread.h"
#include "Environment.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "Library.h"

#pragma warning(disable : 4786)  // identifier was truncated to '255' characters 
                                 // in the debug information

#include <map>
#include <stack>
#include <list>

class CPlayerManager : protected OnlineGameLib::Win32::CThread
{
public:

	explicit CPlayerManager( ENVIRONMENT &theEnviroment );
	virtual ~CPlayerManager();
	
	bool Create();
	void Destroy();

	virtual int Run();
	
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

	bool StartupNetwork();
	void CleanNetwork();

	typedef HRESULT ( __stdcall * pfnCreateServerInterface )( REFIID riid, void **ppv );
	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	static OnlineGameLib::Win32::CLibrary	m_sHeavenLib;
	static OnlineGameLib::Win32::CLibrary	m_sRainbowLib;

	/*
	 * Network
	 */
	IClient		*m_pDBRoleClient;
	IServer		*m_pPlayerServer;

	void _PlayerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType );
	
	/*
	 * Listen and accept data from network
	 */
	HANDLE m_hHelperThread;

	static unsigned int __stdcall	HelperThreadFunction( void *pV );
	int								Helper();

	bool		AnalyzePlayerRequire( size_t index, const void *pData, size_t datalength );
	
private:
	
	HANDLE m_hQuitEvent;

	ENVIRONMENT &m_theEnviroment;
	
	OnlineGameLib::Win32::CCriticalSection	m_csSystemAction;
	
	typedef std::map< size_t, IPlayer * >	PLAYER_MAP;

	PLAYER_MAP		m_thePlayers;

	typedef std::stack< size_t >	STACK;
	typedef std::list< size_t >		LIST;
	
	STACK			m_freeClientNode;
	LIST			m_usedClientNode;

	OnlineGameLib::Win32::CCriticalSection	m_csUsedClientList;

};

#endif // __INCLUDE_PLAYERMANAGER_H__