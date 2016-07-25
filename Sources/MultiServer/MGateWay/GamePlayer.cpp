#include "StdAfx.h"
#include "GamePlayer.h"

#include "..\..\s3client\login\LoginDef.h"
#include "..\..\core\src\KProtocol.h"

#include "Macro.h"
#include "define.h"
#include "ICommand.h"

#include "KRoleDBHeader.h"
#include "KRoleBlockProcess.h"

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

static const size_t g_scNetworkBlockSize = 1000;
OnlineGameLib::Win32::CBuffer::Allocator CGamePlayer::m_theGlobalAllocator( 1024, 10 );

LONG CGamePlayer::m_slnIdentityCounts = 0L;

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
	CCriticalSection::Owner	lock( m_cs );

	stdVector::iterator theIterator;

	for ( theIterator = m_stdCommand.begin(); theIterator != m_stdCommand.end(); theIterator ++ )
	{
		ICommand *pCmd = reinterpret_cast< ICommand* >( *theIterator );

		SAFE_DELETE( pCmd );
	}

	m_stdCommand.clear();
}

CGamePlayer::CTask::CTask( CGamePlayer *pReceiver, UINT nTaskID, UINT nNextTaskID /*enumComplete*/ )
			: m_pReceiver( pReceiver )
			, m_indexCmd( 0 )
			, m_nTaskProgID( nTaskID )
			, m_nNextTaskID( nNextTaskID )
{
}

size_t CGamePlayer::CTask::AddCmd( Action pFun, UINT unFaildValue /*enumNone*/ )
{
	CCriticalSection::Owner	lock( m_cs );

	/*
	 * Convert a status to the other status
	 */
	UINT unFaildTaskID = unFaildValue;

	switch ( unFaildValue )
	{
	case enumRepeat:
		unFaildTaskID = m_nTaskProgID;
		break;

	default:
		unFaildTaskID = unFaildValue;
		break;
	}

	/*
	 * Generate a command and push it into the task queue
	 */
	ICommand *pCmd = new CTaskCommand< CGamePlayer >( m_pReceiver, pFun, unFaildTaskID );

	m_stdCommand.push_back( reinterpret_cast< void* >( pCmd ) );
	
	size_t id = m_stdCommand.size();

	return id;
}

UINT CGamePlayer::CTask::Execute()
{
	CCriticalSection::Owner	lock( m_cs );

	if ( m_indexCmd < m_stdCommand.size() )
	{
		ICommand *pCmd = reinterpret_cast< ICommand * >( m_stdCommand[m_indexCmd] );

		ASSERT( pCmd );

		if ( !pCmd->Execute() )
		{
			return pCmd->GetFaildValue();
		}

		m_indexCmd ++;
		return m_nTaskProgID;
	}	

	Reset();
	
	return m_nNextTaskID;
}

/*
 * CGamePlayer class
 */
CGamePlayer::CDataQueue::~CDataQueue()
{
	CCriticalSection::Owner locker( m_csQueue );

	m_theData.erase( m_theData.begin(), m_theData.end() );
}

void CGamePlayer::CDataQueue::Empty()
{
	CCriticalSection::Owner locker( m_csQueue );

	m_theData.erase( m_theData.begin(), m_theData.end() );
}

size_t CGamePlayer::CDataQueue::Push( LONG lnID, const void *pData )
{
	CCriticalSection::Owner locker( m_csQueue );

	m_theData.insert( stdMap::value_type( lnID, pData ) );
	//m_theData[lnID] = pData;

	return m_theData.size();
}

const void *CGamePlayer::CDataQueue::Top( LONG lnID )
{
	CCriticalSection::Owner locker( m_csQueue );

	stdMap::iterator it;

	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		return m_theData[lnID];
	}

	return NULL;
}

size_t CGamePlayer::CDataQueue::Pop( LONG lnID )
{
	CCriticalSection::Owner locker( m_csQueue );

	size_t size = m_theData.size();

	stdMap::iterator it;

	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		m_theData.erase( lnID );

		size = m_theData.size();
	}

	return size;
}

/*
 * CGamePlayer class
 */
CGamePlayer::CGamePlayer( IServer *pPlayerSender, IClient *pRoleDBSender, LONG nIdentityID /*-1*/ )
			: m_lnIdentityID( nIdentityID )
			, m_theLoginTask( this, enumLogin )
			, m_theLogoutTask( this, enumLogOut )
			, m_theExchangeReginTask( this, enumExchangeRegin )
			, m_pRoleDBSender( pRoleDBSender )
			, m_pPlayerServer( pPlayerSender )
{
	ASSERT( m_pRoleDBSender );

	/*
	 * Resize block buffer
	 */
	m_sBlockBuffer.resize( g_scNetworkBlockSize );

	SetCurrentTask( enumNone );

	LONG lnID = ::InterlockedExchangeAdd( &m_slnIdentityCounts, 1 );

	m_lnIdentityID = ( m_lnIdentityID < 0 ) ? lnID : m_lnIdentityID;

	InitEventProcessor();

	InitTaskProcessor();
}

CGamePlayer::~CGamePlayer()
{
	SAFE_RELEASE( m_pRoleDBSender );
	
	::InterlockedExchangeAdd( &m_slnIdentityCounts, -1 );
}

bool CGamePlayer::DispatchTask( LONG lnTaskID )
{
	if ( IsWorking() )
	{
		/*
		 * This player is processing a special tasks
		 */

		return false;
	}

	SetCurrentTask( lnTaskID );

	return true;
}

void CGamePlayer::InitTaskProcessor()
{
	m_theLoginTask.AddCmd( &CGamePlayer::QueryAccPwd );
	m_theLoginTask.AddCmd( &CGamePlayer::VerifyAccount );
	m_theLoginTask.AddCmd( &CGamePlayer::QueryRoleList );
	m_theLoginTask.AddCmd( &CGamePlayer::GetRoleListInfo, enumRepeat );
	m_theLoginTask.AddCmd( &CGamePlayer::QueryRoleInfo );
	m_theLoginTask.AddCmd( &CGamePlayer::ProcessRoleInfo, enumRepeat );
}

int CGamePlayer::Run()
{
	(this->*EventProcessor[enumWorkEvent])();

	return 0;
}

bool CGamePlayer::AppendData( const void *pData )
{
	TProcessData *pBlock = m_theRoleStreamVein.Block_IN( reinterpret_cast< const KBlock * >( pData ) );

	if ( pBlock )
	{
		switch ( pBlock->nProtoId )
		{
		case PROTOL_GETROLELIST:
			m_theDataQueue.Push( PROTOL_GETROLELIST, reinterpret_cast< const void * >( pBlock->pDataBuffer ) );
			break;

		case PROTOL_LOADROLE:
			m_theDataQueue.Push( PROTOL_LOADROLE, reinterpret_cast< const void * >( pBlock->pDataBuffer ) );
			break;

		default:			
			break;
		}
	}

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

bool CGamePlayer::Destroy( const char * const pPassword )
{
	_tstring sPassword;

	sPassword = pPassword ? pPassword : "";

	if ( m_sPassword.compare( sPassword ) )
	{
		return false;
	}

	return true;
}

void CGamePlayer::InitEventProcessor()
{
	ZeroMemory( EventProcessor, sizeof(EventProcessor) );

	EventProcessor[enumQuitEvent] = Quit;
	EventProcessor[enumWorkEvent] = Work;
}

bool CGamePlayer::Quit()
{
	return true;
}

bool CGamePlayer::Work()
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

	case enumComplete:
		lnNextTask = TaskCompleted() ? enumNone : enumComplete;
		SetCurrentTask( lnNextTask );
		break;

	case enumNone:
	default:
		break;
	}	

	return true;
}

bool CGamePlayer::TaskCompleted()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::TaskCompleted" );

	m_theDataQueue.Empty();
	
	/*
	 * Clear some role data
	 */
	m_theRoleStreamVein.ClearINStream( ( size_t )( -1 ) );
	
	return true;
}

/*
 * Task function
 */
bool CGamePlayer::QueryAccPwd()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::QueryAccPwd -> Begin" );

	Trace( ToString( m_lnIdentityID ), "CGamePlayer::QueryAccPwd -> End" );

	return true;
}

bool CGamePlayer::VerifyAccount()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::VerifyAccount -> Begin" );

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
	lai.Param = LOGIN_A_LOGIN | LOGIN_R_SUCCESS; /* LOGIN_R_FAILED | LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR */

	lai.Size = sizeof( lai );	

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();

	BYTE *pData = const_cast< BYTE * >( pBuffer->GetBuffer() );

	BYTE bProtocol = s2c_login;
	size_t sizeProtocol = sizeof( BYTE );
	memcpy( pData, &bProtocol, sizeProtocol );
	memcpy( pData + sizeProtocol, ( BYTE * )( &lai ), sizeof( lai ) );

	pBuffer->Use( lai.Size + sizeof( BYTE )/* protocol id */ );

	m_pPlayerServer->PreparePackSink();
	
	m_pPlayerServer->PackDataToClient( m_lnIdentityID, pBuffer->GetBuffer(), pBuffer->GetUsed() );
	
	m_pPlayerServer->SendPackToClient();

	pBuffer->Release();

	Trace( ToString( m_lnIdentityID ), "CGamePlayer::VerifyAccount -> End" );

	return true;
}

bool CGamePlayer::QueryRoleList()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::QueryRoleList -> Begin" );

	const size_t lenAN = m_sAccountName.length();
	const size_t lenPL = lenAN + sizeof( TStreamData );

	/*
	 * Require to get rolelist from the role-db server.
	 */
	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();

	TStreamData *pPlayerList = reinterpret_cast< TStreamData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	ASSERT( pPlayerList );

	pPlayerList->nProtoId = PROTOL_GETROLELIST;
	pPlayerList->nDataLen = lenAN + 1;
	pPlayerList->ulNetId = 0;
	pPlayerList->ulIdentity = m_lnIdentityID;

	pPlayerList->pDataBuffer[0] = enumRoleListCount;
	memcpy( &pPlayerList->pDataBuffer[1], m_sAccountName.c_str(), lenAN );

	pBuffer->Use( lenPL );

	/*
	 * Send this information to role database server
	 */	
	KRoleBlockProcess theRoleBlocks( pPlayerList, lenPL );

	for ( size_t block = 0; block < theRoleBlocks.GetBlockCount( g_scNetworkBlockSize ); block ++ )
	{
		KBlock *pBlock = reinterpret_cast< KBlock * >( const_cast< char * >( m_sBlockBuffer.c_str() ) );

		size_t sizeBlock = theRoleBlocks.GetBlockPtr( pBlock, block, g_scNetworkBlockSize );

		m_pRoleDBSender->SendPackToServer( pBlock, sizeBlock );
	}

	pBuffer->Release();

	Trace( ToString( m_lnIdentityID ), "CGamePlayer::QueryRoleList -> End" );

	return true;
}

bool CGamePlayer::GetRoleListInfo()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::GetRoleListInfo -> Begin" );

	bool ok = false;

	const void *pData = NULL;

	if ( NULL != ( pData = m_theDataQueue.Top( PROTOL_GETROLELIST ) ) )
	{
		int nRoleCount = *( char * )pData;
		const S3DBI_RoleBaseInfo *pRoleList = ( const S3DBI_RoleBaseInfo * )( ( const char * )pData + 1/* sizeof( char ) */ );
		
		const size_t datalength = sizeof( S3DBI_RoleBaseInfo ) * nRoleCount;

		if ( nRoleCount > 0 && nRoleCount <= enumRoleListCount && pRoleList )
		{
			ZeroMemory ( m_theRoleDBList, sizeof( S3DBI_RoleBaseInfo ) * enumRoleListCount );
			
			for ( int no = 0; no < nRoleCount; no ++ )
			{
				memcpy( &m_theRoleDBList[no], &pRoleList[no], sizeof( S3DBI_RoleBaseInfo ) );

				Trace( ToString( m_lnIdentityID ), m_theRoleDBList[no].szRoleName );
			}
			
			/*
			 * Send this info to player
			 */
			KRoleBlockProcess theRoleBlocks( ( const TStreamData * )pData, datalength );
			
			for ( size_t block = 0; block < theRoleBlocks.GetBlockCount( g_scNetworkBlockSize ); block ++ )
			{
				m_pPlayerServer->PreparePackSink();

				KBlock *pBlock = reinterpret_cast< KBlock * >( const_cast< char * >( m_sBlockBuffer.c_str() ) );
				
				size_t sizeBlock = theRoleBlocks.GetBlockPtr( pBlock, block, g_scNetworkBlockSize );
				
				m_pPlayerServer->PackDataToClient( m_lnIdentityID, pBlock, sizeBlock );

				m_pPlayerServer->SendPackToClient();
			}

			ok = true;
		}

		m_theDataQueue.Pop( PROTOL_GETROLELIST );
	}

	Trace( ToString( m_lnIdentityID ), "CGamePlayer::GetRoleListInfo -> End" );

	return ok;
}

bool CGamePlayer::QueryRoleInfo()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::QueryRoleInfo -> Begin" );

	enum enumProcessRoleInfo
	{
		enumCreateRole,
		enumQueryRoleInfo,
		enumError
	};
	
	ZeroMemory( &m_theRoleInfomation, sizeof( tagRoleInfomation ) );
	
	// TEMPORAL
	m_theRoleInfomation.nIndexInList = 0;
	
	UINT nProcessor = enumQueryRoleInfo;
	
	switch ( nProcessor )
	{
	case enumQueryRoleInfo:
		{
			/*
			 * Query player information
			 */
			
			// TEMPORAL
			_tstring	 sRoleName = m_theRoleDBList[ m_theRoleInfomation.nIndexInList ].szRoleName;
			
			const size_t lenRN = sRoleName.length();
			const size_t lenRI = sizeof( TStreamData ) - 1 + lenRN;
			
			CBuffer *pBuffer = m_theGlobalAllocator.Allocate();
			
			TStreamData *pRoleInfo = reinterpret_cast< TStreamData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );
			
			ASSERT( pRoleInfo );
			
			pRoleInfo->nProtoId = PROTOL_LOADROLE;
			pRoleInfo->nDataLen = lenRN;
			pRoleInfo->ulNetId = 0;
			pRoleInfo->ulIdentity = m_lnIdentityID;
			memcpy( &pRoleInfo->pDataBuffer[0], sRoleName.c_str(), lenRN );

			pBuffer->Use( lenRI );
		
			/*
			 * Send this information to role database server
			 */	
			KRoleBlockProcess theRoleInfo( pRoleInfo, lenRI );
			
			for ( size_t block = 0; block < theRoleInfo.GetBlockCount( g_scNetworkBlockSize ); block ++ )
			{
				KBlock *pBlock = reinterpret_cast< KBlock * >( const_cast< char * >( m_sBlockBuffer.c_str() ) );
				
				size_t sizeBlock = theRoleInfo.GetBlockPtr( pBlock, block, g_scNetworkBlockSize );
				
				m_pRoleDBSender->SendPackToServer( pBlock, sizeBlock );				
			}
			
			pBuffer->Release();

		}
		
		break;
	}

	Trace( ToString( m_lnIdentityID ), "CGamePlayer::QueryRoleInfo -> End" );

	return true;
}

bool CGamePlayer::ProcessRoleInfo()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::ProcessRoleInfo -> Begin" );

	bool ok = false;

	const void *pData = NULL;

	if ( NULL != ( pData = m_theDataQueue.Top( PROTOL_LOADROLE ) ) )
	{
		const TRoleData *pRoleData = ( const TRoleData * )( ( const char * )( pData ) );

		if ( pRoleData )
		{
			memcpy( &m_theRoleInfomation.theRoleData, pRoleData, sizeof( TRoleData ) );

			Trace( ToString( m_lnIdentityID ), m_theRoleInfomation.theRoleData.BaseInfo.caccname );

			ok = true;
		}

		m_theDataQueue.Pop( PROTOL_LOADROLE );
	}

	Trace( ToString( m_lnIdentityID ), "CGamePlayer::ProcessRoleInfo -> End" );

	return ok;
}