#include "StdAfx.h"
#include <objbase.h>
#include <initguid.h>
#include <process.h>		//Thread define

#include "..\..\s3client\login\LoginDef.h"
#include "KProtocol.h"

#include "MGateWay.h"
#include "PlayerManager.h"
#include "define.h"

#include "KRoleDBHeader.h"
#include "KRoleBlockProcess.h"

#include "tstring.h"
#include "Utils.h"
#include "Macro.h"
#include "Exception.h"

using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::ToString;

OnlineGameLib::Win32::CLibrary	CPlayerManager::m_sHeavenLib( "heaven.dll" );
OnlineGameLib::Win32::CLibrary	CPlayerManager::m_sRainbowLib( "rainbow.dll" );

/*
 * Player server
 */
static const int g_snPSMaxPlayerCount = 5000;
static const int g_snPSPrecision = 10;
static const int g_snPSMaxBuffer = 2000;
static const int g_snPSBufferSize = 1024;

/*
 * Database client
 */
static const int g_scnDBRoleMaxFreeBuffers = 50;
static const int g_scnDBRoleBufferSize = 1024 * 32;

static const size_t g_snPlayerExchangeMemory = 48 * 1024;
#define CALC_MAXFREEBUFFER_BY_COUNT( x ) ( ( ( x ) * 2 / 3 + 3 ) & ~3 );

CPlayerManager::CPlayerManager( ENVIRONMENT &theEnviroment )
			: m_hHelperThread( NULL )
			, m_theEnviroment( theEnviroment )
			, m_pDBRoleClient( NULL )
			, m_pPlayerServer( NULL )
{
	m_hQuitEvent = ::CreateEvent( NULL, TRUE, FALSE, "PMThread_QuitEvent" );
}

CPlayerManager::~CPlayerManager()
{
	SAFE_CLOSEHANDLE( m_hQuitEvent );
	SAFE_CLOSEHANDLE( m_hHelperThread );
}

bool CPlayerManager::Create()
{
	bool ret = false;

	/*
	 * Startup thread
	 */
	try
	{
		Start();

		ret = true;
	}
	catch( const CException &e )
	{
		/*
		 * Call to unqualified virtual function
		 */
		Output( _T("CGamePlayer::CGamePlayer() - Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		/*
		 * Call to unqualified virtual function
		 */
		Output( _T("CGamePlayer::CGamePlayer() - Unexpected exception") );
	}

	ret &= StartupNetwork();

	return ret;
}

void CPlayerManager::Destroy()
{
	::SetEvent( m_hQuitEvent );

	CleanNetwork();

	try
	{
		if ( !Wait( THREAD_TERMINATE_TIMEROUT ) )
		{
			Terminate( -1 );
		}

		if ( WAIT_TIMEOUT != ::WaitForSingleObject( m_hHelperThread, THREAD_TERMINATE_TIMEROUT ) )
		{
			::TerminateThread( m_hHelperThread, -1 );
		}
	}
	catch( const CException &e )
	{
		/*
		 * Call to unqualified virtual function
		 */
		Output( _T("CPlayerManager::Run() - Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		/*
		 * Call to unqualified virtual function
		 */
		Output( _T("CPlayerManager::Run() - Unexpected exception") );
	}
}

int CPlayerManager::Run()
{
	while ( WAIT_OBJECT_0 != ::WaitForSingleObject( m_hQuitEvent, 0L ) )
	{

		CCriticalSection::Owner locker( m_csUsedClientList );

		LIST::iterator it;
		for ( it = m_usedClientNode.begin(); it != m_usedClientNode.end(); it ++ )
		{
			size_t index = *it;

			/*
			 * Get data from player
			 */
			size_t datalength = 0;

			const void *pData = m_pPlayerServer->GetPackFromClient( index, datalength );

			while ( pData && datalength )
			{
				AnalyzePlayerRequire( index, pData, datalength );

				pData = m_pPlayerServer->GetPackFromClient( index, datalength );
			}

			/*
			 * Execute work
			 */
			IPlayer *pPlayer = m_thePlayers[index];

			ASSERT( pPlayer );

			pPlayer->Run();
		}

		Sleep( 1 );
	}

	return 0;
}

void __stdcall CPlayerManager::DBRoleEventNotify( LPVOID lpParam, 
					const unsigned long &ulnEventType )
{
	CPlayerManager *pPM = reinterpret_cast< CPlayerManager * >( lpParam );
	
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		SetShowInfo( "RoleDB Connection", "OK", SIF_COLON );
		
		break;
	case enumServerConnectClose:
		SetShowInfo( "RoleDB Connection", "" );
		
		break;
	}
}

void __stdcall CPlayerManager::AccountEventNotify( LPVOID lpParam, 
					const unsigned long &ulnEventType )
{
	CPlayerManager *pPM = reinterpret_cast< CPlayerManager * >( lpParam );
	
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		
		break;
	case enumServerConnectClose:
		
		break;
	}
}

void __stdcall CPlayerManager::PlayerEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CPlayerManager *pPM = ( CPlayerManager * )( lpParam );

	ASSERT( pPM );

	pPM->_PlayerEventNotify( ulnID, ulnEventType );
}

void __stdcall CPlayerManager::GameSvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumClientConnectCreate:
		
		
		break;
	case enumClientConnectClose:
		
		break;
	}
}

void CPlayerManager::_PlayerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CCriticalSection::Owner locker( m_csUsedClientList );

	switch( ulnEventType )
	{
	case enumClientConnectCreate:

		m_usedClientNode.push_back( ulnID );

		break;

	case enumClientConnectClose:

		PLAYER_MAP::iterator it;

		if ( m_thePlayers.end() != ( it = m_thePlayers.find( ulnID ) ) )
		{
			IPlayer *pPlayer = m_thePlayers[ulnID];

			ASSERT( pPlayer );

			pPlayer->Destroy( NULL );
		}

		m_usedClientNode.remove( ulnID );
		
		break;
	}
}

bool CPlayerManager::StartupNetwork()
{
	bool bDBRoleOK = false;
	bool bPlayerServerOK = false;

	/*
	 * There is connectted the heaven by the rainbow
	 */
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( m_sRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( g_scnDBRoleMaxFreeBuffers, g_scnDBRoleBufferSize );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pDBRoleClient ) );
		
		SAFE_RELEASE( pClientFactory );
	}

	if ( m_pDBRoleClient )
	{
		m_pDBRoleClient->Startup();

		m_pDBRoleClient->RegisterMsgFilter( reinterpret_cast< void * >( this ), DBRoleEventNotify );

		if ( SUCCEEDED( m_pDBRoleClient->ConnectTo( m_theEnviroment.sRoleDBServerIP.c_str(), m_theEnviroment.usRoleDBSvrPort ) ) )
		{
			bDBRoleOK = true;
		}
	}
	
	/*
	 * We open the heaven to wait for the rainbow
	 */

	pfnCreateServerInterface pServerFactroyFun = ( pfnCreateServerInterface )( m_sHeavenLib.GetProcAddress( _T( "CreateInterface" ) ) );

	IServerFactory *pServerFactory = NULL;

	if ( pServerFactroyFun && SUCCEEDED( pServerFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( g_snPSMaxPlayerCount, g_snPSPrecision, g_snPSMaxBuffer, g_snPSBufferSize  );
		
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pPlayerServer ) );
		
		pServerFactory->Release();		
	}

	if ( m_pPlayerServer )
	{
		m_pPlayerServer->Startup();

		m_pPlayerServer->RegisterMsgFilter( reinterpret_cast< void * >( this ), PlayerEventNotify );

		if ( SUCCEEDED( m_pPlayerServer->OpenService( INADDR_ANY, m_theEnviroment.usClientOpenPort ) ) )
		{
			bPlayerServerOK = true;
		}
	}

	_tstring msg;

	if ( !bDBRoleOK )
	{
		msg += _T( "It's failed to connect the RoleDB server\n" );
	}

	if ( !bPlayerServerOK )
	{
		msg += _T( "It's failed to create server for player\n" );
	}

	if ( !msg.empty() )
	{
		::MessageBox( NULL, msg.c_str(), "StartupNetwork", MB_ICONSTOP );
	}
	
	if ( bDBRoleOK && bPlayerServerOK )
	{
		CCriticalSection::Owner lock( m_csSystemAction );
		
		/*
		 * Setup the Global Allocator
		 */
		const size_t buffersize = g_snPlayerExchangeMemory;
		const size_t maxfreebuffer = CALC_MAXFREEBUFFER_BY_COUNT( m_theEnviroment.lnPlayerMaxCount );
		CGamePlayer::SetupGlobalAllocator( buffersize, maxfreebuffer );

		/*
		 * Initialize all of player
		 */
		for ( size_t index = 0; index < m_theEnviroment.lnPlayerMaxCount; index ++ )
		{
			m_freeClientNode.push( index );
			
			IClient *pRoleDBClone = NULL;
			m_pDBRoleClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pRoleDBClone ) );

			IServer *pPlayerSvrClone = NULL;
			m_pPlayerServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &pPlayerSvrClone ) );
			/*
			 * Create a player node and add it into list
			 */
			IPlayer *pPlayer = new CGamePlayer( pPlayerSvrClone, pRoleDBClone, index );
			
			m_thePlayers[index] = pPlayer;
		}

		/*
		 * Initialize a helper thread for network
		 */
		if ( NULL == m_hHelperThread )
		{
			unsigned int threadID = 0;
			
			m_hHelperThread = (HANDLE)::_beginthreadex(0,
				0, 
				HelperThreadFunction,
				( void * )this,
				0, 
				&threadID );
			
			if ( m_hHelperThread == INVALID_HANDLE_VALUE )
			{
				return false;
			}
		}
	}

	return ( bDBRoleOK & bPlayerServerOK );
}

void CPlayerManager::CleanNetwork()
{
	if ( m_pDBRoleClient )
	{
		m_pDBRoleClient->Shutdown();

		m_pDBRoleClient->Cleanup();

		SAFE_RELEASE( m_pDBRoleClient );
	}

	if ( m_pPlayerServer )
	{
/*		CCriticalSection::Owner locker( m_csUsedClientList );
		
		LIST::iterator it;
		for ( it = m_usedClientNode.begin(); it != m_usedClientNode.end(); it ++ )
		{
			size_t index = *it;

			m_pPlayerServer->ShutdownClient( index );
		}
*/		
		m_pPlayerServer->CloseService();

		m_pPlayerServer->Cleanup();
		
		SAFE_RELEASE( m_pPlayerServer );
	}

	CCriticalSection::Owner lock( m_csSystemAction );

	/*
     * Repeat until stack is empty
	 */
	while ( !m_freeClientNode.empty() )
	{
		m_freeClientNode.pop();
	}

	CCriticalSection::Owner locker( m_csUsedClientList );

	m_usedClientNode.erase( m_usedClientNode.begin(), m_usedClientNode.end() );

	PLAYER_MAP::iterator it;
	for ( it = m_thePlayers.begin(); it != m_thePlayers.end(); it ++ )
	{
		IPlayer *pPlayer = ( IPlayer * )( ( *it ).second );

		ASSERT( pPlayer );

		pPlayer->Destroy( NULL );

		SAFE_DELETE( pPlayer );
	}

	m_thePlayers.erase( m_thePlayers.begin(), m_thePlayers.end() );
}

unsigned int __stdcall CPlayerManager::HelperThreadFunction( void *pV )
{
	int result = 0;

	CPlayerManager* pThis = ( CPlayerManager * )pV;
   
	if ( pThis )
	{
		try
		{
			result = pThis->Helper();
		}
		catch( ... )
		{
		}
	}

	return result;
}

int CPlayerManager::Helper()
{
	while ( WAIT_OBJECT_0 != ::WaitForSingleObject( m_hQuitEvent, 0L ) )
	{
		/*
		 * Get data from role-database
		 */
		size_t dataLength = 0;
		const void *pData = m_pDBRoleClient->GetPackFromServer( dataLength );

		while( pData && dataLength )
		{
			KBlock *pBlock = ( KBlock * )( pData );

			UINT uID = pBlock->ulIdentity;

			if ( uID < m_theEnviroment.lnPlayerMaxCount )
			{
				IPlayer *pPlayer = m_thePlayers[uID];

				ASSERT( pPlayer );
				
				pPlayer->AppendData( pData );
			}

			pData = m_pDBRoleClient->GetPackFromServer( dataLength );
		}

		Sleep( 1 );
	}

	return 0;
}

bool CPlayerManager::AnalyzePlayerRequire( size_t index, const void *pData, size_t datalength )
{
	ASSERT( pData && datalength );

	BYTE cProtocol = *( const BYTE * )pData;

	const BYTE *pBuffer = ( const BYTE * )pData + 1;

	ASSERT( pBuffer );

	switch ( cProtocol )
	{
	case c2s_login:
		{
			KLoginAccountInfo *pLAI = ( KLoginAccountInfo * )pBuffer;
		
			m_thePlayers[index]->Create( pLAI->Account, pLAI->Password );
			m_thePlayers[index]->DispatchTask( CGamePlayer::enumLogin );
		}

		break;

	default:
		break;
	}
	
	return true;
}