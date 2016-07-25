#include "stdafx.h"
#include "Intercessor.h"

#include "AccountLoginDef.h"
#include "LoginDef.h"

#include "KProtocolDef.h"
#include "KProtocol.h"

#include "Macro.h"
#include "Buffer.h"

#include "SmartClient.h"

using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CPackager;

OnlineGameLib::Win32::CLibrary	CIntercessor::m_theHeavenLib( "heaven.dll" );
OnlineGameLib::Win32::CLibrary	CIntercessor::m_theRainbowLib( "rainbow.dll" );

CIntercessor::CIntercessor( unsigned long lnMaxPlayerCount, CNetwork &theNetworkConfig )
		: m_hwndViewer( NULL )
//		, m_pAccSvrClient( NULL )
		, m_pDBRoleClient( NULL )
		, m_pGameSvrServer( NULL )
		, m_pPlayerServer( NULL )
		, m_hWorkingThread( INVALID_HANDLE_VALUE )
		, m_hHelperThread( INVALID_HANDLE_VALUE )
		, m_hQuitEvent( NULL, true, false, NULL /*"PM_Quit_Event"*/ )
		, m_hStartupWoringThreadEvent( NULL, false, false, NULL /*"PM_WORKING_EVENT"*/ )
		, m_hStartupHelperThreadEvent( NULL, false, false, NULL /*"PM_HELPER_EVENT"*/ )
		, m_theNetworkConfig( theNetworkConfig )
		, m_lnMaxPlayerCount( lnMaxPlayerCount )
{

}

CIntercessor::~CIntercessor()
{

}

bool CIntercessor::Create( HWND hwndViewer /* NULL */ )
{
	m_hwndViewer = hwndViewer;
	
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

//	m_pAccSvrClient = m_theNetworkConfig.CreateAccSvrClient( m_theNetworkConfig.GetAccSvrIP(), m_theNetworkConfig.GetAccSvrPort() );

//	ASSERT( m_pAccSvrClient );

//	m_pAccSvrClient->RegisterMsgFilter( ( void * )this, AccountEventNotify );

	return StartupNetwork();
}

void CIntercessor::Destroy()
{
	m_hQuitEvent.Set();

	m_hStartupWoringThreadEvent.Set();
	m_hStartupHelperThreadEvent.Set();

	SAFE_CLOSEHANDLE( m_hWorkingThread );
	SAFE_CLOSEHANDLE( m_hHelperThread );

//	SAFE_RELEASE( m_pAccSvrClient );

	CleanNetwork();
}
/*
void __stdcall CIntercessor::AccountEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType )
{
	CIntercessor *pPlayerManager = reinterpret_cast< CIntercessor * >( lpParam );

	ASSERT( pPlayerManager );

	try
	{
		pPlayerManager->_AccountEventNotify( ulnEventType );
	}
	catch(...)
	{
		TRACE( "CIntercessor::AccountEventNotify exception!" );
	}
}

void CIntercessor::_AccountEventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		::PostMessage( m_hwndViewer, WM_SERVER_STATUS, ACCOUNTSERVER_NOTIFY, CONNECTED );

		break;

	case enumServerConnectClose:

		::PostMessage( m_hwndViewer, WM_SERVER_STATUS, ACCOUNTSERVER_NOTIFY, DICONNECTED );

		break;
	}
}	
*/
void __stdcall CIntercessor::DBRoleEventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType )
{
	CIntercessor *pPlayerManager = reinterpret_cast< CIntercessor * >( lpParam );

	ASSERT( pPlayerManager );

	try
	{
		pPlayerManager->_DBRoleEventNotify( ulnEventType );
	}
	catch(...)
	{
		TRACE( "CIntercessor::DBRoleEventNotify exception!" );
	}
}

void CIntercessor::_DBRoleEventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		::PostMessage( m_hwndViewer, WM_SERVER_STATUS, DBROLESERVER_NOTIFY, CONNECTED );

		break;

	case enumServerConnectClose:

		::PostMessage( m_hwndViewer, WM_SERVER_STATUS, DBROLESERVER_NOTIFY, DICONNECTED );

		break;
	}
}

void __stdcall CIntercessor::PlayerEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CIntercessor *pPlayerManager = reinterpret_cast< CIntercessor * >( lpParam );

	ASSERT( pPlayerManager );

	try
	{
		pPlayerManager->_PlayerEventNotify( ulnID, ulnEventType );
	}
	catch(...)
	{
		TRACE( "CIntercessor::PlayerEventNotify exception!" );
	}
}

void CIntercessor::_PlayerEventNotify( const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	//CCriticalSection::Owner locker( m_csPlayerAction );

	switch ( ulnEventType )
	{
	case enumClientConnectCreate:
		{
			IPlayer *pPlayer = m_thePlayers[ulnID];
			
			ASSERT( pPlayer );

			pPlayer->Active();
		}
		break;

	case enumClientConnectClose:
		{
			PLAYER_MAP::iterator it;
			
			if ( m_thePlayers.end() != ( it = m_thePlayers.find( ulnID ) ) )
			{
				IPlayer *pPlayer = m_thePlayers[ulnID];
				
				ASSERT( pPlayer );

				pPlayer->Inactive();
			}
		}
		break;
	}
}

bool CIntercessor::StartupNetwork()
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
		pClientFactory->SetEnvironment( 1024 * 1024 );
		
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
/*
		IServer *pCloneGameServer = NULL;
		m_pGameSvrServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &pCloneGameServer ) );
		
		IClient *pCloneAcc = NULL;
		m_pAccSvrClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pCloneAcc ) );
*/
		static CGameServer::NI ni;
		ni.pServer = m_pGameSvrServer;
		ni.pClient = NULL; //m_pAccSvrClient;
		ni.hwndContainer = m_hwndViewer;
		
		m_pGameSvrServer->RegisterMsgFilter( reinterpret_cast< void * >( &ni ), CGameServer::GameSvrEventNotify );

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
//		IClient *pCloneAcc = NULL;
//		m_pAccSvrClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pCloneAcc ) );
		
		IServer *pClonePlayer = NULL;
		m_pPlayerServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &pClonePlayer ) );
		
		IClient *pCloneDBRole = NULL;
		m_pDBRoleClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pCloneDBRole ) );
		
		ASSERT( /* pCloneAcc && */ pClonePlayer && pCloneDBRole );
			
		CGamePlayer::ATTACH_NETWORK( NULL/* pCloneAcc */, pClonePlayer, pCloneDBRole );

		/*
		 * Create new player
		 */
		{
			CCriticalSection::Owner lock( m_csPlayerAction );
			
			/*
			 * Initialize all of player
			 */
			for ( size_t index = 0; index < m_lnMaxPlayerCount; index ++ )
			{
				/*
				* Create a player node and add it into list
				*/
				IPlayer *pPlayer = new CGamePlayer( index );
				
				ASSERT( pPlayer );
				
				m_thePlayers[index] = pPlayer;
			}
		}

		IServer *pCloneGameServer = NULL;
		m_pGameSvrServer->QueryInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &pCloneGameServer ) );

		CGameServer::Begin( pCloneGameServer );

		m_hStartupWoringThreadEvent.Set();
		m_hStartupHelperThreadEvent.Set();

	}
	
	return ( bPlayerServerSucceed && bDBRoleServerSucceed && bGameSvrServerSucceed );
}

void CIntercessor::CleanNetwork()
{
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
			
			SAFE_DELETE( pPlayer );
		}
		
		m_thePlayers.erase( m_thePlayers.begin(), m_thePlayers.end() );
	}

	CGameServer::End();
	CGamePlayer::DETACH_NETWORK();

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
}

DWORD WINAPI CIntercessor::WorkingThreadFunction( void *pV )
{
	CIntercessor *pThis = reinterpret_cast< CIntercessor * >( pV );

	ASSERT( pThis );

	try
	{
		pThis->Working();
	}
	catch(...)
	{
		::MessageBox( NULL, "Run a working thread is failed!", "CIntercessor class", MB_OK );
	}

	return 0;
}

int	CIntercessor::Working()
{
//	::CoInitialize( NULL );
	
	m_hStartupWoringThreadEvent.Wait();
	
	while ( !m_hQuitEvent.Wait( 0 ) )
	{
		/*
		 * For some alive player
		 */
		{
			CCriticalSection::Owner locker( m_csPlayerAction );
			
			for ( size_t index = 0; index < m_lnMaxPlayerCount; index ++ )
			{		
				IPlayer *pPlayer = m_thePlayers[index];
				
				ASSERT( pPlayer );	

				if ( pPlayer && pPlayer->IsActive() )
				{
					/*
					 * Get data from player
					 */
					size_t datalength = 0;
					
					const void *pData = m_pPlayerServer->GetPackFromClient( index, datalength );
					
					if ( pData && datalength )
					{
						AnalyzePlayerRequire( index, pData, datalength );
					}
					
					/*
					* Execute work
					*/
					pPlayer->Run();
				}
			}
		}

		::Sleep( 1 );
	}

//	::CoUninitialize();

	return 0;
}

DWORD WINAPI CIntercessor::HelperThreadFunction( void *pV )
{
	CIntercessor *pThis = reinterpret_cast< CIntercessor * >( pV );

	ASSERT( pThis );

	try
	{
		pThis->Helper();
	}
	catch(...)
	{
		::MessageBox( NULL, "Startup a helper thread is failed!", "CIntercessor class", MB_OK );
	}

	return 0;
}

int	CIntercessor::Helper()
{
	m_hStartupHelperThreadEvent.Wait();
	
	static const DWORD dwTimeLimit = 1000 * 10;
	DWORD dwStart = ::GetTickCount();
	DWORD dwEnd = dwStart;

	while ( !m_hQuitEvent.Wait( 0 ) )
	{
		/*
		 * ping account server
		 */
		if ( ( ( ( dwEnd = ::GetTickCount() ) - dwStart ) > dwTimeLimit ) ||
			( dwStart > dwEnd ) )
		{
			dwStart = dwEnd;

			PING_COMMAND pc;
			pc.ProtocolType = c2s_ping;
			pc.m_dwTime		= dwStart;

			g_theSmartClient.Send( &pc, sizeof( PING_COMMAND ) );
//			m_pAccSvrClient->SendPackToServer( &pc, sizeof( PING_COMMAND ) );
		}

		/*
		 * Get data from account-server
		 */
		size_t dataLength = 0;
		const void *pData = g_theSmartClient.Recv( dataLength );
//		const void *pData = m_pAccSvrClient->GetPackFromServer( dataLength );

		while( pData && dataLength )
		{
			ASSERT( CPackager::Peek( pData ) > g_nGlobalProtocolType );

			if ( s2c_accountlogin == *( const char * )pData )
			{
				const KAccountHead *pAUR = ( KAccountHead * )( ( ( const char * )pData ) + 1 );
				
				UINT uID = pAUR->Operate;
				
				if ( uID < m_lnMaxPlayerCount )
				{
					IPlayer *pPlayer = m_thePlayers[uID];
					
					ASSERT( pPlayer );
					
					pPlayer->AppendData( CGamePlayer::enumOwnerAccSvr, pData, dataLength );
				}
			}
			
			/*
			 * ping account server
			 */
			if ( ( ( ( dwEnd = ::GetTickCount() ) - dwStart ) > dwTimeLimit ) ||
				( dwStart > dwEnd ) )
			{
				dwStart = dwEnd;
				
				PING_COMMAND pc;
				pc.ProtocolType = c2s_ping;
				pc.m_dwTime		= dwStart;
				
				g_theSmartClient.Send( &pc, sizeof( PING_COMMAND ) );
//				m_pAccSvrClient->SendPackToServer( &pc, sizeof( PING_COMMAND ) );
			}

			pData = g_theSmartClient.Recv( dataLength );
//			pData = m_pAccSvrClient->GetPackFromServer( dataLength );
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

			if ( cProtocol < g_nGlobalProtocolType )
			{
				uID = *( const unsigned long * )( ( const char * )pData + 2 );
			}
			else // cProtocol > g_nGlobalProtocolType
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
			
			/*
			 * ping account server
			 */
			if ( ( ( ( dwEnd = ::GetTickCount() ) - dwStart ) > dwTimeLimit ) ||
				( dwStart > dwEnd ) )
			{
				dwStart = dwEnd;
				
				PING_COMMAND pc;
				pc.ProtocolType = c2s_ping;
				pc.m_dwTime		= dwStart;

				g_theSmartClient.Send( &pc, sizeof( PING_COMMAND ) );
//				m_pAccSvrClient->SendPackToServer( &pc, sizeof( PING_COMMAND ) );
			}

			pData = m_pDBRoleClient->GetPackFromServer( dataLength );
		}
		
		::Sleep( 1 );
	}

	return 0;
}

bool CIntercessor::AnalyzePlayerRequire( size_t index, const void *pData, size_t datalength )
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

			IPlayer *pPlayer = m_thePlayers[index];

			if ( pLAI && pPlayer )
			{				
				pPlayer->AppendData( CGamePlayer::enumOwnerPlayer, pData, datalength );

				pPlayer->DispatchTask( CGamePlayer::enumLogin );
			}
		}

		break;
		
	case c2s_dbplayerselect:
	case c2s_newplayer:
	case c2s_roleserver_deleteplayer:
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

size_t CIntercessor::GetGameServerCount()
{
	if ( m_pGameSvrServer )
	{
		return m_pGameSvrServer->GetClientCount();
	}

	return 0;
}

const char *CIntercessor::GetGameServerInfo( const unsigned long &ulnID )
{
	if ( m_pGameSvrServer )
	{
		return m_pGameSvrServer->GetClientInfo( ulnID );
	}

	return NULL;
}