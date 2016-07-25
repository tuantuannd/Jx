#include "Stdafx.h"
#include "GamePlayer.h"

#include "..\..\Network\S3AccServer\AccountLoginDef.h"
#include "..\..\s3client\login\LoginDef.h"

#include "..\..\RoleDBManager\kroledbheader.h"
#include "S3DBInterface.h"
#include "KProtocolDef.h"
#include "KProtocol.h"

#include "Utils.h"
#include "Macro.h"
#include "Exception.h"
#include "Buffer.h"

using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::Trace;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CPackager;

OnlineGameLib::Win32::CBuffer::Allocator CGamePlayer::m_theGlobalAllocator( 1024 * 64, 20 );

LONG CGamePlayer::m_slnIdentityCounts = 0L;
const int CGamePlayer::s_nRoleListCount = 3;

/*
 * CGamePlayer Global Function
 */
bool CGamePlayer::SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers )
{
	return CGamePlayer::m_theGlobalAllocator.ReSet( bufferSize, maxFreeBuffers );
}


/*
 * CGamePlayer::CTask
 */
CGamePlayer::CTask::CTask()
{
	ASSERT( FALSE );
}

CGamePlayer::CTask::~CTask()
{
	CCriticalSection::Owner	lock( m_csTask );

	stdVector::iterator theIterator;

	for ( theIterator = m_stdCommand.begin(); theIterator != m_stdCommand.end(); theIterator ++ )
	{
		ICommand *pCmd = reinterpret_cast< ICommand * >( *theIterator );

		SAFE_DELETE( pCmd );
	}

	m_stdCommand.clear();
}

CGamePlayer::CTask::CTask( CGamePlayer *pReceiver, UINT nTaskID )
			: m_pReceiver( pReceiver )
			, m_indexCmd( 0 )
			, m_nTaskProgID( nTaskID )
{
}

size_t CGamePlayer::CTask::AddCmd( Action pFun )
{
	CCriticalSection::Owner	lock( m_csTask );

	/*
	 * Convert a status to the other status
	 */

	/*
	 * Generate a command and push it into the task queue
	 */
	ICommand *pCmd = new CTaskCommand< CGamePlayer >( m_pReceiver, pFun );

	m_stdCommand.push_back( pCmd );
	
	size_t id = m_stdCommand.size();

	return id;
}

UINT CGamePlayer::CTask::Execute()
{
	CCriticalSection::Owner	lock( m_csTask );

	if ( m_indexCmd < m_stdCommand.size() )
	{
		ICommand *pCmd = m_stdCommand[ m_indexCmd ];

		ASSERT( pCmd );

		UINT nResult = pCmd->Execute();

		switch ( nResult )
		{
		case enumCompleted:
			Reset();

		case enumError:
		case enumNone:

			return nResult;
			break;

		case enumRepeat:

			return m_nTaskProgID;
			break;

		case enumToNextTask:
		default:
			break;
		}

		m_indexCmd ++;
		return m_nTaskProgID;
	}	

	Reset();	
	return enumCompleted;
}

/*
 * CGamePlayer class
 */
CGamePlayer::CDataQueue::CDataQueue( size_t bufferSize /*= sizeof( DQ )*/, size_t maxFreeBuffers /*= 3*/ )
				: m_theDQAllocator( bufferSize, maxFreeBuffers )
{

}

CGamePlayer::CDataQueue::~CDataQueue()
{
	Empty();
}

void CGamePlayer::CDataQueue::Empty()
{
	CCriticalSection::Owner locker( m_csQueue );

	stdDataMap::iterator it;
	for ( it = m_theData.begin(); it != m_theData.end(); it ++ )
	{
		CBuffer *pBuffer = (*it).second;

		SAFE_RELEASE( pBuffer );
	}

	m_theData.erase( m_theData.begin(), m_theData.end() );
}

void CGamePlayer::CDataQueue::Clear( LONG lnID )
{
	CCriticalSection::Owner locker( m_csQueue );
	
	stdDataMap::iterator it;
	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		CBuffer *pBuffer = (*it).second;

		SAFE_RELEASE( pBuffer );

		m_theData.erase( it );
	}
}

bool CGamePlayer::CDataQueue::Set( LONG lnID, WPARAM wParam, LPARAM lParam )
{
	CCriticalSection::Owner locker( m_csQueue );

	CBuffer *pBuffer = m_theDQAllocator.Allocate();

	/*
	 * Format this buffer
	 */
	LPDQ lpDQ = reinterpret_cast< LPDQ >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	lpDQ->wParam = wParam;
	lpDQ->lParam = lParam;

	m_theData.insert( stdDataMap::value_type( lnID, pBuffer ) );

	return true;
}

bool CGamePlayer::CDataQueue::Peek( LONG lnID, LPDQ lpDQ )
{
	ASSERT( lpDQ );

	CCriticalSection::Owner locker( m_csQueue );
	
	stdDataMap::iterator it;
	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		CBuffer *pBuffer = (*it).second;

		const DQ *lpSrcDQ = ( const DQ * )( pBuffer->GetBuffer() );

		memcpy( lpDQ, lpSrcDQ, sizeof( DQ ) );

		return true;
	}

	return false;
}

bool CGamePlayer::CDataQueue::Get( LONG lnID, LPDQ lpDQ )
{
	ASSERT( lpDQ );

	CCriticalSection::Owner locker( m_csQueue );
	
	stdDataMap::iterator it;
	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		CBuffer *pBuffer = (*it).second;

		const DQ *lpSrcDQ = ( const DQ * )( pBuffer->GetBuffer() );

		memcpy( lpDQ, lpSrcDQ, sizeof( DQ ) );

		/*
		 * Clear the buffer and delete it from map
		 */
		SAFE_RELEASE( pBuffer );

		m_theData.erase( it );

		return true;
	}

	return false;
}

/*
 * CGamePlayer class
 */
CGamePlayer::CGamePlayer( IClient *pAccSvrClient, 
						 IServer *pPlayerServer, 
						 IClient *pDBRoleClient, 
						 UINT nIdentityID /*  = ( UINT )( -1 ) */ )

				: m_lnIdentityID( nIdentityID )
				, m_theLoginTask( this, enumLogin )
				, m_theLogoutTask( this, enumLogOut )
				, m_theExchangeReginTask( this, enumExchangeRegin )
				, m_dwAccountID( ( DWORD )-1 )
				, m_pAccSvrClient( pAccSvrClient )
				, m_pPlayerServer( pPlayerServer )
				, m_pDBRoleClient( pDBRoleClient )
{
	SetCurrentTask( enumNone );

	LONG lnID = ::InterlockedExchangeAdd( &m_slnIdentityCounts, 1 );

	m_lnIdentityID = ( ( UINT )( -1 ) == m_lnIdentityID ) ? lnID : m_lnIdentityID;

	InitTaskProcessor();
}

CGamePlayer::~CGamePlayer()
{
	SAFE_RELEASE( m_pAccSvrClient );
	SAFE_RELEASE( m_pPlayerServer );
	SAFE_RELEASE( m_pDBRoleClient );

	::InterlockedExchangeAdd( &m_slnIdentityCounts, -1 );
}

bool CGamePlayer::DispatchTask( UINT nTaskID )
{
	if ( IsWorking() )
	{
		/*
		 * This player is processing a special tasks
		 */

		return false;
	}

	SetCurrentTask( nTaskID );

	return true;	
}

bool CGamePlayer::IsWorking()
{
	return ( GetCurrentTask() != enumNone );
}

int CGamePlayer::Run()
{
	LONG lnNextTask = enumNone;

	switch ( GetCurrentTask() )
	{
	case enumLogin:
		lnNextTask = m_theLoginTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumLogOut:
		lnNextTask = m_theLogoutTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumExchangeRegin:
		lnNextTask = m_theExchangeReginTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumCompleted:
		lnNextTask = TaskCompleted() ? enumNone : enumCompleted;
		SetCurrentTask( lnNextTask );
		break;

	case enumError:
		SetCurrentTask( enumNone );

	case enumNone:
	default:
		break;
	}	

	return true;
}

bool CGamePlayer::AppendData( UINT nOwner, const void *pData, size_t dataLength )
{
	if ( nOwner >= enumOwnerTotal )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );
	
	if ( cProtocol < s2c_micropackbegin )
	{
		return LargePackProcess( nOwner, pData, dataLength );
	}
	else if ( cProtocol > s2c_micropackbegin )
	{
		return SmallPackProcess( nOwner, pData, dataLength );
	}

	return true;
}

bool CGamePlayer::SmallPackProcess( UINT nOwner, const void *pData, size_t dataLength )
{
	switch ( nOwner )
	{
	case enumOwnerAccSvr:

		return DispatchTaskForAccount( pData, dataLength );
		break;
		
	case enumOwnerRoleSvr:
		
		return DispatchTaskForDBRole( pData ,dataLength );
		break;
		
	case enumOwnerPlayer:
		
		return DispatchTaskForPlayer( pData, dataLength );
		break;

	default:
		break;
	}

	return false;
}

bool CGamePlayer::LargePackProcess( UINT nOwner, const void *pData, size_t dataLength )
{
	switch ( nOwner )
	{
	case enumOwnerAccSvr:

		ASSERT( FALSE );
		
		break;

	case enumOwnerRoleSvr:
		{	
			bool ok = true;
			CBuffer *pBuffer = m_thePackReceiver.PackUp( pData, dataLength );
			
			if ( pBuffer )
			{
				ok = DispatchTaskForDBRole( pBuffer->GetBuffer(), pBuffer->GetUsed() );
				
				SAFE_RELEASE( pBuffer );
			}
			
			return ok;
		}
		break;

	case enumOwnerPlayer:
		
		ASSERT( FALSE );
		break;

	default:
		break;
	}

	return false;
}

bool CGamePlayer::DispatchTaskForAccount( const void *pData, size_t dataLength )
{
	if ( NULL == pData || 0 == dataLength )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );

	m_theDataQueue[enumOwnerAccSvr].Set( cProtocol, ( WPARAM )pData, dataLength );

	return true;
}

bool CGamePlayer::DispatchTaskForDBRole( const void *pData, size_t dataLength )
{
	if ( NULL == pData || 0 == dataLength )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );

	m_theDataQueue[enumOwnerRoleSvr].Set( cProtocol, ( WPARAM )pData, dataLength );

	return true;
}

bool CGamePlayer::DispatchTaskForPlayer( const void *pData, size_t dataLength )
{
	if ( NULL == pData || 0 == dataLength )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );

	m_theDataQueue[enumOwnerPlayer].Set( cProtocol, ( WPARAM )pData, dataLength );

	return true;
}

bool CGamePlayer::Create( const char * const pName, const char * const pPassword )
{
	if ( NULL == pName )
	{
		return false;
	}

	m_sAccountName = pName;
	m_sPassword = pPassword ? pPassword : "";

	return true;
}

bool CGamePlayer::Destroy( const char * const /*pPassword*/ )
{
	SetCurrentTask( enumNone );

	m_sAccountName.empty();
	m_sPassword.empty();
	
	m_dwAccountID = ( DWORD )-1;

	for ( int i=0; i<enumOwnerTotal; i++ )
	{
		m_theDataQueue[i].Empty();
	}

	m_thePackReceiver.Empty();

	return true;
}

void CGamePlayer::InitTaskProcessor()
{
	m_theLoginTask.AddCmd( &CGamePlayer::QueryAccPwd );
	m_theLoginTask.AddCmd( &CGamePlayer::VerifyAccount );
	m_theLoginTask.AddCmd( &CGamePlayer::QueryRoleList );
	m_theLoginTask.AddCmd( &CGamePlayer::ProcessRoleList );
	m_theLoginTask.AddCmd( &CGamePlayer::SelectRole );
	m_theLoginTask.AddCmd( &CGamePlayer::ProcessRoleInfo );	
}

bool CGamePlayer::TaskCompleted()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::TaskCompleted" );

	/*
	 * Clear some data
	 */
	
	return true;
}

UINT CGamePlayer::QueryAccPwd()
{
	if ( !m_pAccSvrClient )
	{
		return enumError;
	}

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();

	BYTE *pData = const_cast< BYTE * >( pBuffer->GetBuffer() );

	const size_t datalength = sizeof( KAccountUserLoginInfo ) + 1;

	KAccountUserLoginInfo userlogin;
	userlogin.Size = sizeof( KAccountUserLoginInfo );

	userlogin.Type = AccountUserLoginInfo;
	userlogin.Version = ACCOUNT_CURRENT_VERSION;
	userlogin.Operate = m_lnIdentityID;

	strcpy( userlogin.Account, m_sAccountName.c_str() );
	strcpy( userlogin.Password, m_sPassword.c_str() );

	*pData = c2s_accountlogin;
	memcpy( pData + 1, &userlogin, sizeof( KAccountUserLoginInfo ) );

	pBuffer->Use( datalength );

	m_pAccSvrClient->SendPackToServer( ( const void * )pData, datalength );

	SAFE_RELEASE( pBuffer );

	return enumToNextTask;
}

UINT CGamePlayer::VerifyAccount()
{
	CDataQueue::DQ dq;

	if ( m_theDataQueue[enumOwnerAccSvr].Get( s2c_accountlogin, &dq ) )
	{
		const void *pData = ( const void * )( dq.wParam );

		KAccountUserLoginReturn* pReturn = ( KAccountUserLoginReturn * )( ( ( char * )pData ) + 1/* size of a protocol byte */ );

		int nResult = pReturn->nReturn;

		UINT nQueryResult = LOGIN_A_LOGIN;
		UINT nNextTask = enumError;

		switch ( nResult )
		{
		case ACTION_SUCCESS:		/* Login Success ! */

			m_dwAccountID = pReturn->AccountID;

			nQueryResult |= LOGIN_R_SUCCESS;

			nNextTask = enumToNextTask;

			break;

		case E_ACCOUNT_OR_PASSWORD:	/* Login Failed ! --- Name, Password */

			nQueryResult |= LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR;

			break;

		case E_ACCOUNT_EXIST:		/* Login Failed ! --- Already login */

			nQueryResult |= LOGIN_R_ACCOUNT_EXIST;

			break;

		case E_ACCOUNT_NODEPOSIT:	/* Login Failed ! --- No Money */

			nQueryResult |= LOGIN_R_TIMEOUT;

			break;

		default:					/* Login Failed ! */

			nQueryResult |= LOGIN_R_FAILED;

			break;

		}
		
		/*
		 * Notify the result to player
		 */
		_VerifyAccount_ToPlayer( nQueryResult );

		return nNextTask;
	}

	return enumRepeat;
}

bool CGamePlayer::_VerifyAccount_ToPlayer( UINT nQueryResult )
{
	KLoginAccountInfo lai;

	ZeroMemory( &lai, sizeof( KLoginAccountInfo ) );

	/*
	 * Account
	 */
	size_t used = sizeof( lai.Account );
	used = ( used > m_sAccountName.length() ) ? m_sAccountName.length() : used;

	memcpy( lai.Account, m_sAccountName.c_str(), used );

	/*
	 * Password
	 */
	used = sizeof( lai.Password );
	used = ( used > m_sPassword.length() ) ? m_sPassword.length() : used;

	memcpy( lai.Password, m_sPassword.c_str(), used );

	/*
	 * The other info
	 */
	lai.Param = nQueryResult;

	lai.Size = sizeof( lai );	

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();

	BYTE *pData = const_cast< BYTE * >( pBuffer->GetBuffer() );

	BYTE bProtocol = s2c_login;
	size_t sizeProtocol = sizeof( BYTE );
	memcpy( pData, &bProtocol, sizeProtocol );
	memcpy( pData + sizeProtocol, ( BYTE * )( &lai ), sizeof( lai ) );

	pBuffer->Use( lai.Size + sizeof( BYTE )/* protocol id */ );

	m_pPlayerServer->SendData( m_lnIdentityID, pBuffer->GetBuffer(), pBuffer->GetUsed() );

	pBuffer->Release();

	return true;
}

UINT CGamePlayer::QueryRoleList()
{
	const size_t lenAN = strlen( m_sAccountName.c_str() );
	const size_t lenPL = lenAN + sizeof( TStreamData );
	
	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();	

	TStreamData *pPlayerList = reinterpret_cast< TStreamData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	ASSERT( pPlayerList );

	pPlayerList->nProtoId = c2s_roleserver_getrolelist;
	pPlayerList->nDataLen = lenAN + 1;
	pPlayerList->ulIdentity = m_lnIdentityID;

	pPlayerList->pDataBuffer[0] = s_nRoleListCount;
	memcpy( &pPlayerList->pDataBuffer[1], m_sAccountName.c_str(), lenAN );

	pBuffer->Use( lenPL );

	m_pDBRoleClient->SendPackToServer( ( const void * )pPlayerList, lenPL );

	SAFE_RELEASE( pBuffer );	

	return enumToNextTask;
}

UINT CGamePlayer::ProcessRoleList()
{
	CDataQueue::DQ dq;

	if ( m_theDataQueue[enumOwnerRoleSvr].Get( s2c_roleserver_getrolelist_result, &dq ) )
	{
		UINT nNextTask = enumError;

		const void *pData = ( const void * )( dq.wParam );

		const TProcessData *pPD = ( const TProcessData * )pData;

		int nRoleCount = *( char * )( pPD->pDataBuffer );

		// const S3DBI_RoleBaseInfo *pRoleList = ( const S3DBI_RoleBaseInfo * )( ( const char * )( pPA->pDataBuffer ) + 1/* sizeof( char ) */ );
		// const size_t datalength = sizeof( S3DBI_RoleBaseInfo ) * nRoleCount;
			
		if ( nRoleCount > 0 )
		{
			m_pPlayerServer->SendData( m_lnIdentityID, pData, dq.lParam );

			nNextTask = enumToNextTask;
		}

		return nNextTask;
	}

	return enumRepeat;
}

UINT CGamePlayer::SelectRole()
{
	CDataQueue::DQ dq;

	if ( m_theDataQueue[enumOwnerPlayer].Get( c2s_dbplayerselect, &dq ) )
	{
		UINT nNextTask = enumError;

		const void *pData = ( const void * )( dq.wParam );

		const tagDBSelPlayer *pDSPC = ( const tagDBSelPlayer * )pData;

		_QueryRoleInfo_ToDBRole( pDSPC->szRoleName );

		nNextTask = enumToNextTask;

		return nNextTask;
	}

	return enumRepeat;
}

bool CGamePlayer::_QueryRoleInfo_ToDBRole( const char *pRoleName )
{
	if ( NULL == pRoleName || '\0' == pRoleName[0] )
	{
		return false;
	}

	const size_t lenRN = strlen( pRoleName );
	const size_t lenRI = sizeof( TStreamData ) - 1 + lenRN;

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();
			
	TStreamData *pRoleInfo = reinterpret_cast< TStreamData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );
	
	ASSERT( pRoleInfo );
	
	pRoleInfo->nProtoId = c2s_roleserver_getroleinfo;
	pRoleInfo->nDataLen = lenRN;
	pRoleInfo->ulIdentity = m_lnIdentityID;
	memcpy( &pRoleInfo->pDataBuffer[0], pRoleName, lenRN );
	
	pBuffer->Use( lenRI );
	
	m_pDBRoleClient->SendPackToServer( ( const void * )pRoleInfo, lenRI );
	
	SAFE_RELEASE( pBuffer );

	return true;
}

UINT CGamePlayer::ProcessRoleInfo()
{
	CDataQueue::DQ dq;

	if ( m_theDataQueue[enumOwnerRoleSvr].Get( s2c_roleserver_getroleinfo_result, &dq ) )
	{
		UINT nNextTask = enumError;

		const void *pData = ( const void * )( dq.wParam );

		TProcessData *pPA = ( TProcessData * )( pData );
		
		ASSERT( m_lnIdentityID == pPA->ulIdentity );

		const TRoleData *pRoleData = ( const TRoleData * )( ( const char * )( pPA->pDataBuffer ) );

		::MessageBox( NULL, pRoleData->BaseInfo.szName, "Info", MB_OK );

		if ( _SyncRoleInfo_ToGameServer( ( const void * )pRoleData, pPA->nDataLen ) )
		{
			nNextTask = enumToNextTask;
		}

		return nNextTask;
	}

	return enumRepeat;
}

bool CGamePlayer::_SyncRoleInfo_ToGameServer( const void *pData, size_t dataLength )
{
	

	return true;
}