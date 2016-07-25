#include "stdafx.h"
#include "PlayerManager.h"

#include "..\..\Network\S3AccServer\AccountLoginDef.h"
#include "..\..\s3client\login\LoginDef.h"

#include "..\..\RoleDBManager\kroledbheader.h"

#include "KProtocolDef.h"
#include "KProtocol.h"

#include "Macro.h"
#include "Buffer.h"

using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CPackager;

OnlineGameLib::Win32::CLibrary	CPlayerManager::m_theHeavenLib( "heaven.dll" );
OnlineGameLib::Win32::CLibrary	CPlayerManager::m_theRainbowLib( "rainbow.dll" );

CPlayerManager::CPlayerManager( unsigned long lnMaxPlayerCount, CNetwork &theNetworkConfig )
	: m_pAccSvrClient( NULL )
		, m_pDBRoleClient( NULL )
		, m_pGameSvrServer( NULL )
		, m_pPlayerServer( NULL )
		, m_hWorkingThread( INVALID_HANDLE_VALUE )
		, m_hHelperThread( INVALID_HANDLE_VALUE )
		, m_hQuitEvent( NULL, true, false, "PM_Quit_Event" )
		, m_hStartupWoringThreadEvent( NULL, false, false, "PM_WORKING_EVENT" )
		, m_hStartupHelperThreadEvent( NULL, false, false, "PM_HELPER_EVENT" )
		, m_theNetworkConfig( theNetworkConfig )
		, m_lnMaxPlayerCount( lnMaxPlayerCount )
{

}

CPlayerManager::~CPlayerManager()
{

}

bool CPlayerManager::Create()
{
	/*
	 * Start a working thread
	 */
	DWORD dwThreadID = 0;

	m_hWorkingThread = ::CreateThread( NULL, 
		0, 
		WorkingThreadFunction, 
		( void * )this, 
		0, 
		&dwThreadID );
			
	if ( m_hWorkingThread == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	/*
	 * Startup a helper thread
	 */
	dwThreadID = 0;

	m_hHelperThread = ::CreateThread( NULL, 
		0, 
		HelperThreadFunction, 
		( void * )this,
		0, 
		&dwThreadID );
			
	if ( m_hHelperThread == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	m_pAccSvrClient = m_theNetworkConfig.CreateAccSvrClient( m_theNetworkConfig.GetAccSvrIP(), m_theNetworkConfig.GetAccSvrPort() );

	ASSERT( m_pAccSvrClient );

	m_pAccSvrClient->RegisterMsgFilter( ( void * )this, AccountEventNotify );

	return StartupNetwork();
}

void CPlayerManager::Destroy()
{
	m_hStartupWoringThreadEvent.Set();
	m_hStartupHelperThreadEvent.Set();

	m_hQuitEvent.Set();

	SAFE_CLOSEHANDLE( m_hWorkingThread );
	SAFE_CLOSEHANDLE( m_hHelperThread );

	SAFE_RELEASE( m_pAccSvrClient );

	CleanNetwork();
}

void __stdcall CPlayerManager::AccountEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType )
{

}

void __stdcall CPlayerManager::DBRoleEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType )
{

}

void __stdcall CPlayerManager::PlayerEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CPlayerManager *pPlayerManager = reinterpret_cast< CPlayerManager * >( lpParam );

	ASSERT( pPlayerManager );

	try
	{
		pPlayerManager->_PlayerEventNotify( ulnID, ulnEventType );
	}
	catch(...)
	{}
}

void CPlayerManager::_PlayerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CCriticalSection::Owner locker( m_csUsedClientList );

	switch ( ulnEventType )
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

void __stdcall CPlayerManager::GameSvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CPlayerManager *pPlayerManager = reinterpret_cast< CPlayerManager * >( lpParam );

	ASSERT( pPlayerManager );

	try
	{
		pPlayerManager->_GameServerEventNotify( ulnID, ulnEventType );
	}
	catch(...)
	{}
}

void CPlayerManager::_GameServerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CCriticalSection::Owner locker( m_csGameSvrAction );

	switch ( ulnEventType )
	{
	case enumClientConnectCreate:

		break;

	case enumClientConnectClose:

		break;
	}
}

bool CPlayerManager::StartupNetwork()
{
	bool bPlayerServerSucceed = false;
	bool bDBRoleServerSucceed = false;
	bool bGameSvrServerSucceed = false;
	
	/*
	 * There is connectted the heaven by the rainbow
	 */

	/*
	 * For the db-role server
	 */
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( m_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	
	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 10, 1024 * 64 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pDBRoleClient ) );
		
		SAFE_RELEASE( pClientFactory );
	}

	if ( m_pDBRoleClient )
	{
		m_pDBRoleClient->Startup();

		m_pDBRoleClient->RegisterMsgFilter( reinterpret_cast< void * >( this ), DBRoleEventNotify );

		if ( SUCCEEDED( m_pDBRoleClient->ConnectTo( m_theNetworkConfig.GetRoleSvrIP(), m_theNetworkConfig.GetRoleSvrPort() ) ) )
		{
			bDBRoleServerSucceed = true;
		}
	}

	/*
	 * We open the heaven to wait for the rainbow
	 */
	pfnCreateServerInterface pServerFactroyFun = 
		( pfnCreateServerInterface )( m_theHeavenLib.GetProcAddress( _T( "CreateInterface" ) ) );

	IServerFactory *pServerFactory = NULL;

	if ( pServerFactroyFun && 
		SUCCEEDED( pServerFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( m_lnMaxPlayerCount, 10, 1000, 1024 * 8 );

		/*
		 * For player
		 */
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pPlayerServer ) );

		/*
		 * For gameserver
		 */
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pGameSvrServer ) );
		
		pServerFactory->Release();		
	}

	if ( m_pGameSvrServer )
	{
		m_pGameSvrServer->Startup();

		m_pGameSvrServer->RegisterMsgFilter( reinterpret_cast< void * >( this ), GameSvrEventNotify );

		if ( SUCCEEDED( m_pGameSvrServer->OpenService( INADDR_ANY, m_theNetworkConfig.GetGameSvrOpenPort() ) ) )
		{
			bGameSvrServerSucceed = true;
		}
	}

	if ( m_pPlayerServer )
	{
		m_pPlayerServer->Startup();

		m_pPlayerServer->RegisterMsgFilter( reinterpret_cast< void * >( this ), PlayerEventNotify );

		if ( SUCCEEDED( m_pPlayerServer->OpenService( INADDR_ANY, m_theNetworkConfig.GetClientOpenPort() ) ) )
		{
			bPlayerServerSucceed = true;			
		}
	}

	if ( bPlayerServerSucceed && 
			bDBRoleServerSucceed && 
			bGameSvrServerSucceed )
	{
		CCriticalSection::Owner lock( m_csPlayerAction );

		/*
		 * Initialize all of player
		 */
		for ( size_t index = 0; index < m_lnMaxPlayerCount; index ++ )
		{
			IClient *pCloneAcc = NULL;
			m_pAccSvrClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pCloneAcc ) );

			IServer *pClonePlayer = NULL;
			m_pPlayerServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &pClonePlayer ) );

			IClient *pCloneDBRole = NULL;
			m_pDBRoleClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pCloneDBRole ) );

			ASSERT( pCloneAcc && pClonePlayer );

			/*
			 * Create a player node and add it into list
			 */
			IPlayer *pPlayer = new CGamePlayer( pCloneAcc, pClonePlayer, pCloneDBRole, index );

			ASSERT( pPlayer );
			
			m_thePlayers[index] = pPlayer;
		}

		m_hStartupWoringThreadEvent.Set();
		m_hStartupHelperThreadEvent.Set();

	}
	
	return ( bPlayerServerSucceed && bDBRoleServerSucceed && bGameSvrServerSucceed );
}

void CPlayerManager::CleanNetwork()
{
	/*
	 * Disconnect network and relase this resource
	 */
	if ( m_pPlayerServer )
	{
		m_pPlayerServer->CloseService();

		m_pPlayerServer->Cleanup();
		
		SAFE_RELEASE( m_pPlayerServer );
	}

	if ( m_pDBRoleClient )
	{
		m_pDBRoleClient->Cleanup();

		SAFE_RELEASE( m_pDBRoleClient );
	}

	if ( m_pGameSvrServer )
	{
		m_pGameSvrServer->CloseService();

		m_pGameSvrServer->Cleanup();

		SAFE_RELEASE( m_pGameSvrServer );
	}

	/*
	 * Clear client node
	 */
	{
		CCriticalSection::Owner locker( m_csUsedClientList );
		
		m_usedClientNode.erase( m_usedClientNode.begin(), m_usedClientNode.end() );
	}
	
	/*
	 * Clear player info
	 */
	{
		CCriticalSection::Owner locker( m_csPlayerAction );
		
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
	
	/*
	 * MAPID_MAPTO_GAMESVR
	 */
	{
		CCriticalSection::Owner locker( m_csMI2GSAction );

		m_theMapIDMapToGameSvr.erase( m_theMapIDMapToGameSvr.begin(), 
			m_theMapIDMapToGameSvr.end() );
	}

	/*
	 * PLAYERID_MAPTO_GAMESVR
	 */
	{
		CCriticalSection::Owner locker( m_csPI2GSAction );

		m_thePlayerIDMapToGameSvr.erase( m_thePlayerIDMapToGameSvr.begin(), 
			m_thePlayerIDMapToGameSvr.end() );
	}
}

DWORD WINAPI CPlayerManager::WorkingThreadFunction( void *pV )
{
	CPlayerManager *pThis = reinterpret_cast< CPlayerManager * >( pV );

	ASSERT( pThis );

	try
	{
		pThis->Working();
	}
	catch(...)
	{
		::MessageBox( NULL, "Startup a working thread is failed!", "CPlayerManager class", MB_OK );
	}

	return 0;
}

int	CPlayerManager::Working()
{
	m_hStartupWoringThreadEvent.Wait();

	while ( !m_hQuitEvent.Wait( 0 ) )
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

		::Sleep( 1 );
	}

	return 0;
}

DWORD WINAPI CPlayerManager::HelperThreadFunction( void *pV )
{
	CPlayerManager *pThis = reinterpret_cast< CPlayerManager * >( pV );

	ASSERT( pThis );

	try
	{
		pThis->Helper();
	}
	catch(...)
	{
		::MessageBox( NULL, "Startup a helper thread is failed!", "CPlayerManager class", MB_OK );
	}

	return 0;
}

int	CPlayerManager::Helper()
{
	m_hStartupHelperThreadEvent.Wait();

	while ( !m_hQuitEvent.Wait( 0 ) )
	{
		/*
		 * Get data from role-database
		 */
		size_t dataLength = 0;
		const void *pData = m_pAccSvrClient->GetPackFromServer( dataLength );

		while( pData && dataLength )
		{
			ASSERT( CPackager::Peek( pData ) > s2c_micropackbegin );
			
			KAccountHead *pAUR = ( KAccountHead * )( ( ( char * )pData ) + 1 );

			UINT uID = pAUR->Operate;
			
			if ( uID < m_lnMaxPlayerCount )
			{
				IPlayer *pPlayer = m_thePlayers[uID];

				ASSERT( pPlayer );
				
				pPlayer->AppendData( CGamePlayer::enumOwnerAccSvr, pData, dataLength );
			}

			pData = m_pAccSvrClient->GetPackFromServer( dataLength );
		}
		
		/*
		 * Get data from role-database
		 */
		dataLength = 0;
		pData = m_pDBRoleClient->GetPackFromServer( dataLength );

		while( pData && dataLength )
		{
			BYTE cProtocol = CPackager::Peek( pData );

			UINT uID = ( UINT )( -1 );

			if ( cProtocol < s2c_micropackbegin )
			{
				uID = *( const unsigned long * )( ( const char * )pData + 2 );
			}
			else // cProtocol > s2c_micropackbegin
			{
				TProcessData *pPD = ( TProcessData * )( pData );

				uID = pPD->ulIdentity;
			}

			if ( uID < m_lnMaxPlayerCount )
			{
				IPlayer *pPlayer = m_thePlayers[uID];

				ASSERT( pPlayer );
				
				pPlayer->AppendData( CGamePlayer::enumOwnerRoleSvr, pData, dataLength );
			}

			pData = m_pDBRoleClient->GetPackFromServer( dataLength );
		}
		
		::Sleep( 1 );
	}

	return 0;
}

bool CPlayerManager::AnalyzePlayerRequire( size_t index, const void *pData, size_t datalength )
{
	ASSERT( pData && datalength );

	BYTE cProtocol = *( const BYTE * )pData;

	switch ( cProtocol )
	{
	case c2s_login:
		{
			const BYTE *pBuffer = ( const BYTE * )pData + 1;

			KLoginAccountInfo *pLAI = ( KLoginAccountInfo * )pBuffer;

			ASSERT( pLAI );
		
			if ( m_thePlayers[index]->Create( pLAI->Account, pLAI->Password ) )
			{
				m_thePlayers[index]->DispatchTask( CGamePlayer::enumLogin );
			}
		}

		break;
		
	case c2s_dbplayerselect:
		{
			if ( index < m_lnMaxPlayerCount )
			{
				IPlayer *pPlayer = m_thePlayers[index];
				
				ASSERT( pPlayer );
				
				pPlayer->AppendData( CGamePlayer::enumOwnerPlayer, pData, datalength );
			}
		}		
		break;

	default:
		break;
	}
	
	return true;
}