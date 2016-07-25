#include "Stdafx.h"
#include "GameServer.h"
#include "IPlayer.h"
#include "GamePlayer.h"

#include "msg_define.h"

#include <process.h>

#include "AccountLoginDef.h"

#include "Macro.h"
#include "SmartClient.h"

using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::_tstring;

CBuffer::Allocator						CGameServer::m_theGlobalAllocator( 1024 * 96, 200 );

CCriticalSection						CGameServer::m_csMapIDAction;
CGameServer::stdMapIDConvert			CGameServer::m_theMapIDConvert;

CEvent									CGameServer::m_shQuitEvent( NULL, true, false, NULL/*"GS_QUIT_EVENT"*/ );
CEvent									CGameServer::m_shStartupManagerThreadEvent( NULL, false, false, NULL/*"GS_MANAGER_EVENT"*/ );

HANDLE									CGameServer::m_shManagerThread = NULL;

CGameServer::stdGameSvr					CGameServer::m_theGameServers;
CCriticalSection						CGameServer::m_csGameSvrAction;

/*
 * CGamePlayer Global Function
 */
bool CGameServer::SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers )
{
	return CGameServer::m_theGlobalAllocator.ReSet( bufferSize, maxFreeBuffers );
}

LONG CGameServer::m_slnIdentityCounts = 0L;

/*
 * A generator of guid
 */
static void GenGuid( GUID *guid )
{
	ASSERT( guid );

	static char szData[9] = { 0 };
	static DWORD dwBase = 0;

	if ( FAILED( ::CoCreateGuid( guid ) ) )
	{
		if ( ( dwBase >> 31 ) & 0x1 )
		{
			dwBase = 0;
		}

		/*
		 * Make a 128 bits cipher
		 */
		guid->Data1 = dwBase++;
		guid->Data2 = ( DWORD )rand() & 0xFFFF;
		guid->Data3 = ( ( DWORD )rand() >> 16 ) & 0xFFFF ;

		sprintf( ( char * )szData, "%d%d", ( ( DWORD )rand() + dwBase++ ), ( DWORD )rand() );
		memcpy( guid->Data4, szData, 8 );		
	}
}

/*
 * class CGameServer
 */
CGameServer::CGameServer( IServer *pGameSvrServer,
						 IClient *pAccountClient,
						 UINT nIdentityID /* = -1 */ )
				: m_lnIdentityID( nIdentityID )
				, m_pGameSvrServer( pGameSvrServer )
//				, m_pAccountClient( pAccountClient )
				, m_nServerIP_Internet( 0 )
				, m_nServerIP_Intraner( 0 )
				, m_nServerPort( 0 )
				, m_dwCapability( -1 )

{
	LONG lnID = ::InterlockedExchangeAdd( &m_slnIdentityCounts, 1 );

	m_lnIdentityID = ( ( UINT )( -1 ) == m_lnIdentityID ) ? lnID : m_lnIdentityID;
}

CGameServer::~CGameServer()
{
//	SAFE_RELEASE( m_pGameSvrServer );
//	SAFE_RELEASE( m_pAccountClient );

	m_thePackager.Empty();

	::InterlockedExchangeAdd( &m_slnIdentityCounts, -1 );
}

bool CGameServer::Create()
{
	ZeroMemory( m_theProcessProtocolFun, sizeof( m_theProcessProtocolFun ) );
// Multi Fixed By MrChuCong@gmail.com
	m_theProcessProtocolFun[c2s_registeraccount]  = &CGameServer::_RegisterAccount;
	m_theProcessProtocolFun[c2s_entergame] = &CGameServer::_NotifyEnterGame;
	m_theProcessProtocolFun[c2s_leavegame] = &CGameServer::_NotifyLeaveGame;
	m_theProcessProtocolFun[c2s_permitplayerlogin] = &CGameServer::_NotifyPlayerLogin;
	m_theProcessProtocolFun[c2s_updatemapinfo] = &CGameServer::_UpdateMapID;
	m_theProcessProtocolFun[c2s_updategameserverinfo] = &CGameServer::_UpdateGameSvrInfo;
	m_theProcessProtocolFun[c2s_requestsvrip] = &CGameServer::_RequestSvrIP;

	/*
	 * Query gameserver information
	 */
	_QueryGameSvrInfo();

	return _QueryMapID();
}

bool CGameServer::Destroy()
{
	{
		CCriticalSection::Owner lock( m_csAITS );
		
		stdAccountAttachIn::iterator it;
		
		for ( it = m_theAccountInThisServer.begin(); it != m_theAccountInThisServer.end(); it ++ )
		{
			string sAccountName = ( *it ).first;
			
			FreezeMoney( sAccountName.c_str(), 0);
		}
	}

	m_dwCapability	= 0;
	m_nServerPort	= 0;
	m_nServerIP_Intraner = 0;
	m_nServerIP_Internet = 0;
	m_sServerIPAddr_Internet = "";
	m_sServerIPAddr_Intraner = "";
	
	{
		CCriticalSection::Owner locker( m_csMapIDAction );

		stdMapIDConvert::iterator itM2C;

		for ( itM2C = m_theMapIDConvert.begin(); itM2C != m_theMapIDConvert.end(); itM2C ++ )
		{
			stdServerList &sl = ( *itM2C ).second;

			if ( !sl.empty() )
			{
				sl.remove( this );
			}
		}
	}

	return true;
}

void __stdcall CGameServer::GameSvrEventNotify( LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CGameServer::LPNI pNI = reinterpret_cast< CGameServer::NI * >( lpParam );

	ASSERT( pNI );

	CCriticalSection::Owner locker( CGameServer::m_csGameSvrAction );

	switch ( ulnEventType )
	{
	case enumClientConnectCreate:
		{
			IGServer *pGServer = new CGameServer( pNI->pServer, pNI->pClient, ulnID );

			ASSERT( pGServer );

			pGServer->Create();

			std::pair< CGameServer::stdGameSvr::iterator, bool > result = 
				CGameServer::m_theGameServers.insert( CGameServer::stdGameSvr::value_type( ulnID, pGServer ) );
			
			if ( result.second && pNI->hwndContainer && ::IsWindow( pNI->hwndContainer ) )
			{
				::PostMessage( pNI->hwndContainer, WM_GAMESERVER_EXCHANGE, ADD_GAMESERVER_ACTION, ulnID );
			}
		}
		break;

	case enumClientConnectClose:
		{
			stdGameSvr::iterator it;

			if ( CGameServer::m_theGameServers.end() != 
				( it = CGameServer::m_theGameServers.find( ulnID ) ) )
			{
				if ( pNI->hwndContainer && ::IsWindow( pNI->hwndContainer ) )
				{
					::PostMessage( pNI->hwndContainer, WM_GAMESERVER_EXCHANGE, DEL_GAMESERVER_ACTION, ulnID );
				}

				IGServer *pGServer = ( *it ).second;

				ASSERT( pGServer );

				CGameServer::m_theGameServers.erase( it );

				pGServer->Destroy();
				
				SAFE_DELETE( pGServer );
			}
		}
		break;
	}
}

bool CGameServer::Begin( IServer *pGameSvrServer )
{
	/*
	 * Startup a manager thread
	 */
	DWORD dwThreadID = 0;

	m_shManagerThread = ::CreateThread( NULL, 
				0, 
				ManagerThreadFunction, 
				( void * )pGameSvrServer,
				0, 
				&dwThreadID );
			
	if ( m_shManagerThread == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	m_shStartupManagerThreadEvent.Set();

	return true;
}

void CGameServer::End()
{
	m_shQuitEvent.Set();

	m_shStartupManagerThreadEvent.Set();

	if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_shManagerThread, 5000 ) )
	{
		::TerminateThread( m_shManagerThread, 0 );
	}

	SAFE_CLOSEHANDLE( m_shManagerThread );


	/*
	 * MapID & GameServer
	 */
	{
		CCriticalSection::Owner locker( m_csMapIDAction );

		stdMapIDConvert::iterator itM2C;

		for ( itM2C = m_theMapIDConvert.begin(); itM2C != m_theMapIDConvert.end(); itM2C ++ )
		{
			stdServerList &SL = ( *itM2C ).second;

			SL.clear();
		}

		m_theMapIDConvert.erase( m_theMapIDConvert.begin(), m_theMapIDConvert.end() );				
	}

	/*
	 * Clear gameserver information
	 */
	{
		CCriticalSection::Owner locker( m_csGameSvrAction );

		m_theGameServers.erase( m_theGameServers.begin(), m_theGameServers.end() );
	}
}

DWORD WINAPI CGameServer::ManagerThreadFunction( void *pParam )
{
	IServer *pGameSvrServer = ( IServer * )pParam;

	ASSERT( pGameSvrServer );

	m_shStartupManagerThreadEvent.Wait();

	stdGameSvr::iterator it;

	while ( !m_shQuitEvent.Wait( 0 ) )
	{
		CCriticalSection::Owner locker( CGameServer::m_csGameSvrAction );

		for ( it = CGameServer::m_theGameServers.begin(); 
			it != CGameServer::m_theGameServers.end(); 
			it ++ )
		{
			UINT nlnID = ( *it ).first;

			size_t datalength = 0;

			const void *pData = pGameSvrServer->GetPackFromClient( nlnID, datalength );

			if ( 0 == datalength || NULL == pData )
			{
				continue;
			}

			IGServer *pGServer = ( *it ).second;

			if ( pGServer )
			{
				pGServer->AnalyzeRequire( pData, datalength );
			}			
		}

		::Sleep( 1 );
	}

	SAFE_RELEASE( pGameSvrServer );

	return 0;
}

bool CGameServer::AnalyzeRequire( const void *pData, size_t datalength )
{
	bool ok = true;

	BYTE cProtocol = CPackager::Peek( pData );
	
	if ( cProtocol < g_nGlobalProtocolType )
	{
		return LargePackProcess( cProtocol, pData, datalength );
	}
	else if ( cProtocol > g_nGlobalProtocolType )
	{
		return SmallPackProcess( cProtocol, pData, datalength );
	}

	return false;
}

bool CGameServer::LargePackProcess( BYTE cProtocol, const void *pData, size_t datalength )
{
	return true;
}

bool CGameServer::SmallPackProcess( BYTE cProtocol, const void *pData, size_t datalength )
{
	if ( cProtocol < c2s_end && m_theProcessProtocolFun[cProtocol] )
	{
		return ( this->*m_theProcessProtocolFun[cProtocol] )( ( const void * )pData, datalength );
	}

	return false;
}

bool CGameServer::DispatchTask( UINT nTask, const void *pData, size_t datalength, WORD nData )
{
	bool ok = true;

	switch ( nTask )
	{
	case enumSyncRoleInfo:

		ok = _SyncRoleInfo( pData, datalength, nData);
		
		break;
		
	case enumPlayerLogicLogout:

		ok = _PlayerLogicLogout( pData, datalength );

		break;

	case enumTaskProtocol:
	default:		
		break;
	}

	return true;
}

bool CGameServer::_QueryMapID()
{
	tagQueryMapInfo qmi;

	qmi.cProtocol = s2c_querymapinfo;

	m_pGameSvrServer->SendData( m_lnIdentityID, ( const void * )&qmi, sizeof( tagQueryMapInfo ) );

	return true;
}

bool CGameServer::_QueryGameSvrInfo()
{
	tagQueryGameSvrInfo qgsi;

	qgsi.cProtocol = s2c_querygameserverinfo;

	m_pGameSvrServer->SendData( m_lnIdentityID, ( const void * )&qgsi, sizeof( tagQueryGameSvrInfo ) );

	return true;
}

bool CGameServer::_UpdateMapID( const void *pData, size_t datalength )
{
	tagUpdateMapID *pUMI = ( tagUpdateMapID * )pData;

	int  nMapCount = pUMI->cMapCount;
	BYTE *pMapID = pUMI->szMapID;

	while ( --nMapCount >= 0 )
	{
		RegisterServer( pMapID[nMapCount], ( IGServer * )this );
	}

	return true;
}

bool CGameServer::_UpdateGameSvrInfo( const void *pData, size_t datalength )
{
	ASSERT( sizeof( tagGameSvrInfo ) == datalength );

	tagGameSvrInfo *pGSI = ( tagGameSvrInfo * )pData;
	
	m_dwCapability = pGSI->wCapability;

	m_nServerPort	= pGSI->nPort;

	m_nServerIP_Internet = pGSI->nIPAddr_Internet;
	m_nServerIP_Intraner = pGSI->nIPAddr_Intraner;

	m_sServerIPAddr_Intraner = OnlineGameLib::Win32::net_ntoa( m_nServerIP_Intraner );
	m_sServerIPAddr_Internet = OnlineGameLib::Win32::net_ntoa( m_nServerIP_Internet );

	return true;
}

bool CGameServer::_RequestSvrIP( const void *pData, size_t datalength )
{
	tagRequestSvrIp *pRSI = ( tagRequestSvrIp * )pData;

	ASSERT( sizeof( tagRequestSvrIp ) == datalength );

	DWORD dwIP = 0;
	IGServer *pGServer = CGameServer::QueryServer( pRSI->dwMapID );

	if ( pGServer )
	{
		dwIP = pGServer->GetIP( pRSI->cIPType );
	}

	tagNotifySvrIp nsi;

	nsi.cProtocol = s2c_notifysvrip;

	nsi.pckgID = pRSI->pckgID;
	nsi.dwMapID = pRSI->dwMapID;
	nsi.cIPType = pRSI->cIPType;
	nsi.dwSvrIP = dwIP;

	m_pGameSvrServer->SendData( m_lnIdentityID, ( const void * )&nsi, sizeof( tagNotifySvrIp ) );

	return true;
}

bool CGameServer::_NotifyPlayerLogin( const void *pData, size_t datalength )
{
	tagPermitPlayerLogin *pPPL = ( tagPermitPlayerLogin * )pData;

	ASSERT( sizeof( tagPermitPlayerLogin ) == datalength );

	bool ok = false;

	IPlayer *pPlayer = CGamePlayer::Get( ( const char * )( pPPL->szRoleName ) );

	if ( pPlayer )
	{
		tagNotifyPlayerLogin npl;
		
		npl.cProtocol = s2c_notifyplayerlogin;
		
		int nMinLen = strlen( ( const char * )pPPL->szRoleName );
		nMinLen = nMinLen > sizeof( npl.szRoleName ) ? sizeof( npl.szRoleName ) : nMinLen;
		memcpy( ( char * )npl.szRoleName, ( const char * )pPPL->szRoleName, nMinLen );
		npl.szRoleName[nMinLen] = '\0';

		memcpy( &( npl.guid ), &( pPPL->guid ), sizeof( GUID ) );
		
		npl.bPermit = pPPL->bPermit; 

		npl.nIPAddr = m_nServerIP_Internet;
		npl.nPort	= m_nServerPort;

		ok = pPlayer->AppendData( CGamePlayer::enumOwnerPlayer, ( const void * )&npl, sizeof( tagNotifyPlayerLogin ) );
	}

	return ok;
}

bool CGameServer::_RegisterAccount( const void *pData, size_t datalength )
{
	ASSERT( pData && datalength );

	tagRegisterAccount *pRA = ( tagRegisterAccount * )pData;

	if ( sizeof( tagRegisterAccount ) != datalength )
	{
		return false;
	}

	AttatchAccountToGameServer( ( const char * )pRA->szAccountName );

	return true;
}

bool CGameServer::_NotifyEnterGame( const void *pData, size_t datalength )
{
	ASSERT( pData && datalength );

	tagEnterGame *pEG = ( tagEnterGame * )pData;

	if ( sizeof( tagEnterGame ) != datalength )
	{
		return false;
	}

	BYTE *pAccountName = ( BYTE * )( pEG->szAccountName );

	PushAccount( ( const char * )pAccountName );

	return true;
}

bool CGameServer::_NotifyLeaveGame( const void *pData, size_t datalength )
{
	ASSERT( pData && datalength );
	
	tagLeaveGame *pLG = ( tagLeaveGame * )pData;

	ASSERT( sizeof( tagLeaveGame ) == datalength );

	BYTE *pAccountName = ( BYTE * )( pLG->szAccountName );
	
	return PopAccount( ( const char * )pAccountName, ( pLG->cCmdType != HOLDACC_LEAVEGAME ), pLG->nExtPoint);
}

bool CGameServer::PushAccount( const char *pAccountName )
{
	ASSERT( pAccountName );

	return ConsumeMoney( pAccountName );
}

bool CGameServer::PopAccount( const char *pAccountName, bool bUnlockAccount, WORD nExtPoint )
{
	ASSERT( pAccountName );

	if ( HaveAccountInGameServer( pAccountName ) )
	{
		if ( bUnlockAccount )
		{
			FreezeMoney( pAccountName, nExtPoint);
		}

		DetachAccountFromGameServer( pAccountName );
		
		return true;
	}

	return false;
}

bool CGameServer::Attach( const char *pAccountName )
{
	return AttatchAccountToGameServer( pAccountName );
}

bool CGameServer::AttatchAccountToGameServer( const char *pAccountName )
{
	if ( !pAccountName || !pAccountName[0] )
	{
		return false;
	}

	if ( _NAME_LEN > strlen( pAccountName ) )
	{
		CCriticalSection::Owner lock( m_csAITS );

		std::pair< stdAccountAttachIn::iterator, bool > result = 
			m_theAccountInThisServer.insert( stdAccountAttachIn::value_type( pAccountName, 
			reinterpret_cast< void * >( this ) ) );

		return result.second;
	}

	return false;
}

bool CGameServer::HaveAccountInGameServer( const char *pAccountName )
{
	if ( !pAccountName || !pAccountName[0] )
	{
		return false;
	}

	if ( _NAME_LEN > strlen( pAccountName ) )
	{
		CCriticalSection::Owner lock( m_csAITS );

		stdAccountAttachIn::iterator it;

		if ( m_theAccountInThisServer.end() != 
			( it = m_theAccountInThisServer.find( pAccountName ) ) )
		{
			return true;
		}
	}

	return false;
}

bool CGameServer::DetachAccountFromGameServer( const char *pAccountName )
{
	if ( !pAccountName || !pAccountName[0] )
	{
		return false;
	}

	if ( _NAME_LEN > strlen( pAccountName ) )
	{
		CCriticalSection::Owner lock( m_csAITS );

		stdAccountAttachIn::iterator it;

		if ( m_theAccountInThisServer.end() != 
			( it = m_theAccountInThisServer.find( pAccountName ) ) )
		{
			CGameServer *pThis = reinterpret_cast< CGameServer * >( ( *it ).second );

			ASSERT( pThis && pThis == this );
			
			m_theAccountInThisServer.erase( it );

			return true;
		}
	}

	return false;
}

bool CGameServer::ConsumeMoney( const char *pAccountName )
{
	if ( !pAccountName || !pAccountName[0] )
	{
		return false;
	}

	if ( _NAME_LEN <= strlen( pAccountName ) )
	{
		return false;
	}

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();
	
	BYTE *pContext = const_cast< BYTE * >( pBuffer->GetBuffer() );
	
	const size_t len = sizeof( KAccountUser ) + 1;
	
	KAccountUser user;
	
	user.Size = sizeof( KAccountUser );
	user.Type = AccountUser;
	user.Version = ACCOUNT_CURRENT_VERSION;

	size_t length = strlen( pAccountName );
	length = ( length > LOGIN_USER_ACCOUNT_MAX_LEN ) ? LOGIN_USER_ACCOUNT_MAX_LEN : length;
	memcpy( user.Account, pAccountName, length );
	user.Account[length] = '\0';
	
	*pContext = c2s_gamelogin;
	memcpy( pContext + 1, &user, sizeof( KAccountUser ) );
	
	g_theSmartClient.Send( pContext, len );
	//m_pAccountClient->SendPackToServer( pContext, len );
	
	SAFE_RELEASE( pBuffer );

	return true;
}

bool CGameServer::FreezeMoney( const char *pAccountName, WORD nExtPoint )
{
	if ( !pAccountName || !pAccountName[0] )
	{
		return false;
	}

	if ( _NAME_LEN <= strlen( pAccountName ) )
	{
		return false;
	}

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();

	BYTE *pData = const_cast< BYTE * >( pBuffer->GetBuffer() );

	const size_t datalength = sizeof( KAccountUserLogout ) + 1;

	KAccountUserLogout user;

	user.Size = sizeof( KAccountUserLogout );
	user.Type = AccountUserLogout;
	user.Version = ACCOUNT_CURRENT_VERSION;
	user.nExtPoint = nExtPoint;

	size_t length = strlen( pAccountName );
	length = ( length > LOGIN_USER_ACCOUNT_MAX_LEN ) ? LOGIN_USER_ACCOUNT_MAX_LEN : length;
	memcpy( user.Account, pAccountName, length );
	user.Account[length] = '\0';

	*pData = c2s_accountlogout;
	memcpy( pData + 1, &user, sizeof( KAccountUserLogout ) );

	g_theSmartClient.Send( ( const void * )pData, datalength );
	//m_pAccountClient->SendPackToServer( ( const void * )pData, datalength );

	SAFE_RELEASE( pBuffer );

	return true;
}

bool CGameServer::_PlayerLogicLogout( const void *pData, size_t datalength )
{
	tagLogicLogout ll;

	if ( sizeof( ll.szRoleName ) < datalength )
	{
		return false;
	}

	ll.cProtocol = s2c_logiclogout;
	memcpy( ll.szRoleName, pData, datalength );

	m_pGameSvrServer->SendData( m_lnIdentityID, ( const void * )&ll, sizeof( tagLogicLogout ) );

	return true;
}

bool CGameServer::_SyncRoleInfo( const void *pData, size_t datalength, WORD nData )
{
	/*
	 * Send role data to the game server
	 */
	tagGuidableInfo ginfo;

	ginfo.cProtocol = s2c_syncgamesvr_roleinfo_cipher;

	GenGuid( &( ginfo.guid ) );

	ginfo.nExtPoint = nData;
	ginfo.nChangePoint = 0;
	
	ginfo.datalength = datalength;

	m_thePackager.AddData( s2c_syncgamesvr_roleinfo_cipher, ( const char * )&ginfo, sizeof( tagGuidableInfo ) );
	m_thePackager.AddData( s2c_syncgamesvr_roleinfo_cipher, ( const char * )pData, datalength );

	/*
	 * Begin to split this buffer
	 */
	CBuffer *pBuffer = m_thePackager.GetHeadPack( s2c_syncgamesvr_roleinfo_cipher );

	while ( pBuffer )
	{
		m_pGameSvrServer->SendData( m_lnIdentityID, pBuffer->GetBuffer(), pBuffer->GetUsed() );

		/*
		 * Your must relase this buffer
		 */
		SAFE_RELEASE( pBuffer );

		/*
		 * Get next package
		 */
		pBuffer = m_thePackager.GetNextPack( s2c_syncgamesvr_roleinfo_cipher );		
	}
	
	SAFE_RELEASE( pBuffer );

	m_thePackager.DelData( s2c_syncgamesvr_roleinfo_cipher );

	return true;
}

bool CGameServer::RegisterServer( UINT nID, IGServer *pGServer )
{
	ASSERT( pGServer );

	CCriticalSection::Owner locker( m_csMapIDAction );

	stdMapIDConvert::iterator it;

	/*
	 * Append this sever information into table
	 */
	if ( m_theMapIDConvert.end() != ( it = m_theMapIDConvert.find( nID ) ) )
	{
		stdServerList& sl = ( *it ).second;

		sl.push_back( pGServer );
	}
	else
	{
		/*
		 * Insert this server information into table
		 */
		stdServerList sl;

		sl.push_back( pGServer );
		
		std::pair< stdMapIDConvert::iterator, bool > result =
			m_theMapIDConvert.insert( stdMapIDConvert::value_type( nID, sl ) );

		return result.second;
	}

	return true;
}

IGServer *CGameServer::QueryServer( UINT nMapID )
{
	CCriticalSection::Owner locker( m_csMapIDAction );

	stdMapIDConvert::iterator it;

	if ( m_theMapIDConvert.end() != ( it = m_theMapIDConvert.find( nMapID ) ) )
	{
		stdServerList& sl = ( *it ).second;

		/*
		 * TODO : Don't get the server when it can't carry anyone
		 */
		if ( !sl.empty() )
		{
			IGServer *pGServer = NULL;

			stdServerList::iterator it;
			for ( it = sl.begin(); it != sl.end(); it ++ )
			{
				pGServer = ( IGServer * )( *it );

				ASSERT( pGServer );

				if ( NULL == pGServer )
				{
					continue;
				}
				
				if ( pGServer->GetContent() < pGServer->GetCapability() )
				{
					return pGServer;
				}
			}
		}
	}

	return NULL;
}

IGServer *CGameServer::GetServer( size_t nID )
{
	CCriticalSection::Owner locker( CGameServer::m_csGameSvrAction );
	
	stdGameSvr::iterator it;

	if ( CGameServer::m_theGameServers.end() != 
		( it = CGameServer::m_theGameServers.find( nID ) ) )
	{
		IGServer *pGServer = ( *it ).second;

		ASSERT( pGServer );

		return pGServer;
	}

	return NULL;
}

size_t CGameServer::GetContent()
{
	CCriticalSection::Owner lock( m_csAITS );
		
	return m_theAccountInThisServer.size();
}

void CGameServer::SendToAll( const char *pText, int nLength, UINT uOption )
{
	stdGameSvr::iterator it;

	CCriticalSection::Owner locker( CGameServer::m_csGameSvrAction );

	for ( it = CGameServer::m_theGameServers.begin(); 
		it != CGameServer::m_theGameServers.end(); 
		it ++ )
		{
			IGServer *pGServer = ( *it ).second;

			if ( pGServer )
			{
				pGServer->SendText( pText, nLength, uOption );
			}
		}
}

void CGameServer::SendText( const char *pText, int nLength, UINT uOption )
{
	if ( !pText || 0 == nLength || !m_pGameSvrServer )
	{
		return;
	}

	tagGatewayBroadCast gbc;

	gbc.cProtocol = s2c_gateway_broadcast;
	gbc.uCmdType = uOption;

	int nLen = 0;
	if ( sizeof( gbc.szData ) > nLength )
	{
		strcpy( gbc.szData, pText );

		nLen = nLength;
	}

	gbc.szData[nLen] = '\0';

	m_pGameSvrServer->SendData( m_lnIdentityID, ( const void * )&gbc, sizeof( tagGatewayBroadCast ) );
}
