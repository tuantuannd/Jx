#include "Stdafx.h"
#include "GamePlayer.h"

#include "AccountLoginDef.h"
#include "LoginDef.h"
#include "S3DBInterface.h"

#include "KProtocol.h"
#include "KProtocolDef.h"

#include "Utils.h"
#include "Macro.h"
#include "Exception.h"
#include "Buffer.h"

#include "SmartClient.h"

using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::Trace;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::net_aton;

CBuffer::Allocator	CGamePlayer::m_theGlobalAllocator( 1024 * 64, 1000 );

LONG				CGamePlayer::m_slnIdentityCounts = 0;
LONG				CGamePlayer::m_lnWorkingCounts = 0;

const int			CGamePlayer::s_nRoleListCount	 = 3;
const int			CGamePlayer::s_nLoginTimeoutTimer	 = 60 * 1000;
const int			CGamePlayer::s_nProcessTimeoutTimer	 = 200 * 1000;

CCriticalSection		CGamePlayer::m_csMapSP;
CGamePlayer::stdMapSP	CGamePlayer::m_sthePlayerTable;

CPlayerCreator						CGamePlayer::m_thePlayerCreator;

//IClient * CGamePlayer::m_pAccSvrClient = NULL;
IServer * CGamePlayer::m_pPlayerServer = NULL;
IClient * CGamePlayer::m_pDBRoleClient = NULL;

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
	//CCriticalSection::Owner	lock( m_csTask );

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
	//CCriticalSection::Owner	lock( m_csTask );

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
	//CCriticalSection::Owner	lock( m_csTask );

	if ( m_indexCmd < m_stdCommand.size() )
	{
		ICommand *pCmd = m_stdCommand[ m_indexCmd ];

		ASSERT( pCmd );

		UINT nResult = pCmd->Execute();

		switch ( nResult )
		{
		case enumSelAddDelRole:
		case enumLoginCreateRole:
		case enumLoginDeleteRole:
		case enumLoginSelectRole:
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
CGamePlayer::CDataQueue::CDataQueue( size_t bufferSize /*= 1024 * 64*/, size_t maxFreeBuffers /*= 1*/ )
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
		LONG id = ( *it ).first;
		
		CBuffer *pBuffer = (*it).second;

		SAFE_RELEASE( pBuffer );
	}

	m_theData.erase( m_theData.begin(), m_theData.end() );
}

bool CGamePlayer::CDataQueue::AddData( LONG lnID, const BYTE *pData, size_t datalength )
{
	bool ok = false;

	ASSERT( pData && datalength );

	CBuffer *pBuffer = m_theDQAllocator.Allocate();

	ASSERT( pBuffer );

	pBuffer->AddData( pData, datalength );

	{
		CCriticalSection::Owner locker( m_csQueue );

		pBuffer->AddRef();

		stdDataMap::iterator it;
		if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
		{
			CBuffer *pTemp = ( *it ).second;
			
			SAFE_RELEASE( pTemp );
			
			m_theData.erase( it );
		}

		std::pair< stdDataMap::iterator, bool > result = 
			m_theData.insert( stdDataMap::value_type( lnID, pBuffer ) );

		if ( !( ok = result.second ) )
		{
			SAFE_RELEASE( pBuffer );
		}
	}

	SAFE_RELEASE( pBuffer );

	return ok;
}

CBuffer *CGamePlayer::CDataQueue::Attach( LONG lnID )
{
	CCriticalSection::Owner locker( m_csQueue );

	stdDataMap::iterator it;

	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		CBuffer *pBuffer = ( *it ).second;

		m_theData.erase( it );

		return pBuffer;
	}

	return NULL;
}

void CGamePlayer::CDataQueue::Detach( LONG lnID )
{
//	Clear( lnID );
}

void CGamePlayer::CDataQueue::Clear( LONG lnID )
{
	CCriticalSection::Owner locker( m_csQueue );
	
	stdDataMap::iterator it;
	if ( m_theData.end() != ( it = m_theData.find( lnID ) ) )
	{
		CBuffer *pBuffer = ( *it ).second;

		SAFE_RELEASE( pBuffer );

		m_theData.erase( it );
	}
}

/*
 * CGamePlayer class
 */
CGamePlayer::CGamePlayer( UINT nIdentityID /*  = ( UINT )( -1 ) */ )
				: m_lnIdentityID( nIdentityID )
				, m_theLoginTask( this, enumLogin )
				, m_theSelAddDelTask( this, enumSelAddDelRole )
				, m_theLoginCreateRoleTask( this, enumLoginCreateRole )
				, m_theLoginSelectRoleTask( this, enumLoginSelectRole )
				, m_theLoginDeleteRoleTask( this, enumLoginDeleteRole )
				, m_theSafeCloseTask( this, enumSafeClose )
				, m_nCurrentTaskID( 0 )
				, m_nAttachServerID( -1 )
				, m_bActiveStatus( false )
				, m_dwTaskBeginTimer( 0 )
				, m_dwTaskTotalTimer( 0 )
				, m_bAutoUnlockAccount( false )
                , m_bUseSuperPassword( false )
				, m_nExtPoint(-1)
{
	SetCurrentTask( enumNone );

	LONG lnID = ::InterlockedExchangeAdd( &m_slnIdentityCounts, 1 );

	m_lnIdentityID = ( ( UINT )( -1 ) == m_lnIdentityID ) ? lnID : m_lnIdentityID;

	InitTaskProcessor();
}

CGamePlayer::~CGamePlayer()
{
	{
		CCriticalSection::Owner locker( CGamePlayer::m_csMapSP );

		m_sthePlayerTable.erase( m_sthePlayerTable.begin(), m_sthePlayerTable.end() );
	}

	::InterlockedExchangeAdd( &m_slnIdentityCounts, -1 );
}

bool CGamePlayer::Active()
{
	SetCurrentTask( enumNone );

	m_nAttachServerID = -1;

	m_bActiveStatus = true;

	m_dwTaskBeginTimer = ::GetTickCount();
	m_dwTaskTotalTimer = s_nLoginTimeoutTimer;

	::InterlockedExchangeAdd( &m_lnWorkingCounts, 1 );

	return true;
}

bool CGamePlayer::Inactive()
{
	m_bActiveStatus = false;

	SetCurrentTask( enumNone );

	Del( m_sRoleName.c_str() );	

	m_dwTaskBeginTimer = 0;
	m_dwTaskTotalTimer = s_nLoginTimeoutTimer;

	if ( m_bAutoUnlockAccount )
	{
		_UnlockAccount();

		m_bAutoUnlockAccount = false;
	}

	_ClearTaskQueue();

/*	IGServer *pGServer = CGameServer::GetServer( m_nAttachServerID );

	if ( pGServer )
	{
		pGServer->DispatchTask( CGameServer::enumPlayerLogicLogout, m_sRoleName.c_str(), m_sRoleName.size() );
	}
*/
	m_nAttachServerID = -1;
	
	/*
	* Clear this role info
	*/
	m_sAccountName	= "";
	m_sPassword		= "";
    m_sSuperPassword = "";
    m_sDelRoleName   = "";
    m_bUseSuperPassword = false;

	m_sRoleName		= "";

	::InterlockedExchangeAdd( &m_lnWorkingCounts, -1 );

	return true;
}

UINT CGamePlayer::SafeClose()
{
	ASSERT( FALSE );
	
	return enumToNextTask;
}

bool CGamePlayer::_UnlockAccount()
{
	const char *pAccountName = m_sAccountName.c_str();

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
	user.nExtPoint = 0;

	size_t length = strlen( pAccountName );
	length = ( length > LOGIN_USER_ACCOUNT_MAX_LEN ) ? LOGIN_USER_ACCOUNT_MAX_LEN : length;
	memcpy( user.Account, pAccountName, length );
	user.Account[length] = '\0';

	*pData = c2s_accountlogout;
	memcpy( pData + 1, &user, sizeof( KAccountUserLogout ) );

	g_theSmartClient.Send( ( const void * )pData, datalength );
//	m_pAccSvrClient->SendPackToServer( ( const void * )pData, datalength );

	SAFE_RELEASE( pBuffer );

	return true;	
}

void CGamePlayer::ATTACH_NETWORK( IClient *pAccSvrClient, 
				IServer *pPlayerServer, 
				IClient	*pDBRoleClient )
{
	ASSERT( m_slnIdentityCounts == 0 );

//	m_pAccSvrClient = pAccSvrClient;
	m_pPlayerServer = pPlayerServer;
	m_pDBRoleClient = pDBRoleClient;
}

void CGamePlayer::DETACH_NETWORK()
{
	ASSERT( m_slnIdentityCounts == 0 );

//	SAFE_RELEASE( m_pAccSvrClient );
	SAFE_RELEASE( m_pPlayerServer );
	SAFE_RELEASE( m_pDBRoleClient );
}

bool CGamePlayer::DispatchTask( UINT nTaskID )
{
	/*
	 * This player is processing a special tasks
	 */	
/*
	if ( IsWorking() )
	{
		return false;
	}
*/
	m_theLoginTask.Reset();

	m_theSelAddDelTask.Reset();

	m_theLoginCreateRoleTask.Reset();
	m_theLoginSelectRoleTask.Reset();
	m_theLoginDeleteRoleTask.Reset();

	SetCurrentTask( nTaskID );

	return true;	
}

bool CGamePlayer::IsWorking()
{
	return ( GetCurrentTask() != enumNone );
}

bool CGamePlayer::Run()
{
	LONG lnNextTask = enumNone;

	if ( m_bActiveStatus && m_dwTaskBeginTimer )
	{
		DWORD dwCurTimer = ::GetTickCount();

		if ( dwCurTimer - m_dwTaskBeginTimer > m_dwTaskTotalTimer )
		{
			m_pPlayerServer->ShutdownClient( m_lnIdentityID );

			return false;
		}
	}

	switch ( GetCurrentTask() )
	{
	case enumNone:
		return true;
		break;

	case enumLogin:
		lnNextTask = m_theLoginTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumSelAddDelRole:
		lnNextTask = m_theSelAddDelTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumLoginCreateRole:
		lnNextTask = m_theLoginCreateRoleTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumLoginSelectRole:
		lnNextTask = m_theLoginSelectRoleTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumLoginDeleteRole:
		lnNextTask = m_theLoginDeleteRoleTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumSafeClose:
		lnNextTask = m_theSafeCloseTask.Execute();
		SetCurrentTask( lnNextTask );
		break;

	case enumCompleted:
		lnNextTask = TaskCompleted() ? enumNone : enumCompleted;
		SetCurrentTask( lnNextTask );
		break;

	case enumError:
		SetCurrentTask( enumNone );
		break;

	default:
		break;
	}	

	return true;
}

bool CGamePlayer::AppendData( UINT nOwner, const void *pData, size_t dataLength )
{
	if ( nOwner >= enumOwnerTotal || !m_bActiveStatus )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );
	
	if ( cProtocol < g_nGlobalProtocolType )
	{
		return LargePackProcess( nOwner, pData, dataLength );
	}
	else if ( cProtocol > g_nGlobalProtocolType )
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
		
		return DispatchTaskForDBRole( pData, dataLength );
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
			CBuffer *pBuffer = m_thePackager.PackUp( pData, dataLength );
			
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

	m_theDataQueue[enumOwnerAccSvr].AddData( cProtocol, ( const BYTE * )pData, dataLength );

	return true;
}

bool CGamePlayer::DispatchTaskForDBRole( const void *pData, size_t dataLength )
{
	if ( NULL == pData || 0 == dataLength )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );

	m_theDataQueue[enumOwnerRoleSvr].AddData( cProtocol, ( const BYTE * )pData, dataLength );

	return true;
}

bool CGamePlayer::DispatchTaskForPlayer( const void *pData, size_t dataLength )
{
	if ( NULL == pData || 0 == dataLength )
	{
		return false;
	}

	BYTE cProtocol = CPackager::Peek( pData );

	m_theDataQueue[enumOwnerPlayer].AddData( cProtocol, ( const BYTE * )pData, dataLength );

	return true;
}

void CGamePlayer::_ClearTaskQueue()
{
	for ( int i=0; i<enumOwnerTotal; i++ )
	{
		m_theDataQueue[i].Empty();
	}
	
	m_thePackager.Empty();
}

void CGamePlayer::InitTaskProcessor()
{
	/*
	 * Login main task
	 */
	m_theLoginTask.AddCmd( &CGamePlayer::WaitForAccPwd );
	m_theLoginTask.AddCmd( &CGamePlayer::QueryAccPwd );
	m_theLoginTask.AddCmd( &CGamePlayer::VerifyAccount );
	m_theLoginTask.AddCmd( &CGamePlayer::QueryRoleList );
	m_theLoginTask.AddCmd( &CGamePlayer::ProcessRoleList );

	/*
	 * Login branch task
	 */
	{
		/*
		 * m_theSelAddDelTask::SelAddDelRole
		 *
		 * switch( result )
		 * case m_theLoginCreateRoleTask
		 * case m_theLoginDeleteRoleTask
		 * case m_theLoginSelectRoleTask
		 */
		m_theSelAddDelTask.AddCmd( &CGamePlayer::SelAddDelRole );
		m_theSelAddDelTask.AddCmd( &CGamePlayer::QueryAccPwd );
		m_theSelAddDelTask.AddCmd( &CGamePlayer::DelRole_WaitForVerify );

		/*
		 * m_theLoginCreateRoleTask::WaitForCreateResult
		 *
		 * successed : m_theLoginCreateRoleTask::ProcessRoleInfo
		 * failed	 : m_theSelAddDelTask::SelAddDelRole
		 */
		m_theLoginCreateRoleTask.AddCmd( &CGamePlayer::WaitForCreateResult ); 
		m_theLoginCreateRoleTask.AddCmd( &CGamePlayer::ProcessRoleInfo );
		m_theLoginCreateRoleTask.AddCmd( &CGamePlayer::WaitForGameSvrPermit );

		m_theLoginDeleteRoleTask.AddCmd( &CGamePlayer::WaitForDeleteResult );

		m_theLoginSelectRoleTask.AddCmd( &CGamePlayer::ProcessRoleInfo );
		m_theLoginSelectRoleTask.AddCmd( &CGamePlayer::WaitForGameSvrPermit );
	}

	m_theSafeCloseTask.AddCmd( &CGamePlayer::SafeClose );

	/*
	 * Logout
	 */

	m_nExtPoint = -1;
}

bool CGamePlayer::TaskCompleted()
{
	Trace( ToString( m_lnIdentityID ), "CGamePlayer::TaskCompleted" );

	/*
	 * Clear some data
	 */
	
	return true;
}

UINT CGamePlayer::WaitForAccPwd()
{
#ifdef	CONSOLE_DEBUG
	cprintf( "CGamePlayer::WaitForAccPwd...\n" );
#endif

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerPlayer].Attach( c2s_login );
	 
	if ( pRetBuffer )
	{
		KLoginAccountInfo* pLAI = ( KLoginAccountInfo * )( pRetBuffer->GetBuffer() + 1/* size of a protocol byte */ );

		UINT nNextTask = enumError;

		const char *pAccName	= pLAI->Account;
		const char *pAccPwd		= pLAI->Password.szPassword;//ganlei

#ifdef	CONSOLE_DEBUG
	cprintf( "CGamePlayer::WaitForAccPwd [AccountName:%s]\n", pAccName );
#endif
            
        // modify by Freeway Chen in 2003.7.1
        if (pLAI)
        {
            int nCheckProtocolVersion = true;

            #ifdef USE_KPROTOCOL_VERSION
             nCheckProtocolVersion = (pLAI->ProtocolVersion == KPROTOCOL_VERSION);
            #endif

            if (nCheckProtocolVersion)
            {
                // 如果协议版本相同，继续判断账号
                if (pAccName[0]) 
		        {
			        nNextTask = enumToNextTask;
			        
			        m_sAccountName	= pAccName;
			        m_sPassword		= pAccPwd ? pAccPwd : "";
                    m_bUseSuperPassword = false;
		        }
            }
            else
            {
                // 如果协议版本不同，就提示用户出错，需要升级到新版本
      		    UINT nQueryResult = LOGIN_A_LOGIN | LOGIN_R_INVALID_PROTOCOLVERSION;

                _VerifyAccount_ToPlayer(nQueryResult, 0);

                // 是否需要玩家断开? 需要进一步确认
    			//m_pPlayerServer->ShutdownClient( m_lnIdentityID ); // 断开玩家
            }
        }

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerPlayer].Detach( c2s_login );

		return nNextTask;
	}

	return enumRepeat;
}

UINT CGamePlayer::QueryAccPwd()
{
#ifdef	CONSOLE_DEBUG
	cprintf( "CGamePlayer::QueryAccPwd...\n" );
#endif

/*	if ( !m_pAccSvrClient )
	{
		return enumError;
	}
*/
	m_dwTaskTotalTimer = s_nProcessTimeoutTimer;

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();

	BYTE *pData = const_cast< BYTE * >( pBuffer->GetBuffer() );

	const size_t datalength = sizeof( KAccountUserLoginInfo ) + 1;

	KAccountUserLoginInfo userlogin;
	userlogin.Size = sizeof( KAccountUserLoginInfo );

    if (m_bUseSuperPassword)
    {
    	userlogin.Type = AccountUserVerify;
    }
    else
    {
    	userlogin.Type = AccountUserLoginInfo;
    }

	userlogin.Version = ACCOUNT_CURRENT_VERSION;
	userlogin.Operate = m_lnIdentityID;

	size_t nMinLen = sizeof( userlogin.Account );
	nMinLen = m_sAccountName.size() > nMinLen ? nMinLen: m_sAccountName.size();
	memcpy( userlogin.Account, m_sAccountName.c_str(), nMinLen );
	userlogin.Account[nMinLen] = '\0';

    if (m_bUseSuperPassword)
    {
	    nMinLen = sizeof( userlogin.Password );
	    nMinLen = m_sSuperPassword.size() > nMinLen ? nMinLen: m_sSuperPassword.size();	
	    memcpy( userlogin.Password, m_sSuperPassword.c_str(), nMinLen );
	    userlogin.Password[nMinLen] = '\0';
    }
    else
    {
	    nMinLen = sizeof( userlogin.Password );
	    nMinLen = m_sPassword.size() > nMinLen ? nMinLen: m_sPassword.size();	
	    memcpy( userlogin.Password, m_sPassword.c_str(), nMinLen );
	    userlogin.Password[nMinLen] = '\0';
    }

	*pData = c2s_accountlogin;
	memcpy( pData + 1, &userlogin, sizeof( KAccountUserLoginInfo ) );

	m_bAutoUnlockAccount = true;
	m_theDataQueue[enumOwnerAccSvr].Empty();

	g_theSmartClient.Send( ( const void * )pData, datalength );
//	m_pAccSvrClient->SendPackToServer( ( const void * )pData, datalength );

	SAFE_RELEASE( pBuffer );

	return enumToNextTask;
}

UINT CGamePlayer::VerifyAccount()
{
#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount...\n" );
#endif
	
	m_nExtPoint = -1;

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerAccSvr].Attach( s2c_accountlogin );

	if ( pRetBuffer )
	{
		KAccountUserReturnExt* pReturn = ( KAccountUserReturnExt * )( pRetBuffer->GetBuffer() + 1/* size of a protocol byte */ );

		int nResult = pReturn->nReturn;

		UINT nQueryResult = LOGIN_A_LOGIN;
		UINT nNextTask = enumError;

		switch ( nResult )
		{
		case ACTION_SUCCESS:		/* Login Success ! */

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount Successful!\n" );
#endif
			// pReturn->Account;
			nQueryResult |= LOGIN_R_SUCCESS;

			nNextTask = enumToNextTask;

			m_nExtPoint = pReturn->nExtPoint;

			break;

		case E_ACCOUNT_OR_PASSWORD:	/* Login Failed ! --- Name, Password */

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount Failed! [name or pwd error]\n" );
#endif
			nQueryResult |= LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR;

			m_bAutoUnlockAccount = false;

			break;

		case E_ACCOUNT_EXIST:		/* Login Failed ! --- Already login */

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount Failed! [Already login]\n" );
#endif

			nQueryResult |= LOGIN_R_ACCOUNT_EXIST;

			m_bAutoUnlockAccount = false;

			break;

		case E_ACCOUNT_FREEZE:		/* Login Failed ! --- Account Freeze */

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount Failed! [Account Freeze]\n" );
#endif

			nQueryResult |= LOGIN_R_FREEZE;

			m_bAutoUnlockAccount = false;

			break;

		case E_ACCOUNT_NODEPOSIT:	/* Login Failed ! --- No Money */

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount Failed! [No Money]\n" );
#endif

			nQueryResult |= LOGIN_R_TIMEOUT;

			m_bAutoUnlockAccount = false;

			break;

		default:					/* Login Failed ! */

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::VerifyAccount Failed!\n" );
#endif

			nQueryResult |= LOGIN_R_FAILED;

			m_bAutoUnlockAccount = false;

			break;

		}
		
		/*
		 * Notify the result to player
		 */
		_VerifyAccount_ToPlayer( nQueryResult, pReturn->nLeftTime );

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerAccSvr].Detach( s2c_accountlogin );

		return nNextTask;
	}

	return enumRepeat;
}

bool CGamePlayer::_VerifyAccount_ToPlayer( UINT nQueryResult, unsigned long nLeftTime)
{
	KLoginAccountInfo lai;

	ZeroMemory( &lai, sizeof( KLoginAccountInfo ) );

	/*
	 * Account
	 */
	size_t used = sizeof( lai.Account );
	used = ( used > m_sAccountName.length() ) ? m_sAccountName.length() : ( used - 1 );

	memcpy( lai.Account, m_sAccountName.c_str(), used );
	lai.Account[used] = '\0';

	/*
	 * Password
	 */
	used = sizeof( lai.Password.szPassword );//ganlei
	used = ( used > m_sPassword.length() ) ? m_sPassword.length() : ( used - 1 );

	memcpy( lai.Password.szPassword, m_sPassword.c_str(), used );//ganlei
	lai.Password.szPassword[used] = '\0';//ganlei

	lai.nLeftTime = nLeftTime;

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

	m_pPlayerServer->SendData( m_lnIdentityID, pData, ( sizeof( KLoginAccountInfo ) + sizeof( BYTE )/* protocol id */ ) );

	pBuffer->Release();

	return true;
}

UINT CGamePlayer::QueryRoleList()
{
#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::QueryRoleList...\n" );
#endif

	const size_t lenAN = strlen( m_sAccountName.c_str() );
	const size_t lenPL = lenAN + sizeof( TProcessData );
	
	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();	

	TProcessData *pPlayerList = reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	ASSERT( pPlayerList );

	pPlayerList->nProtoId = c2s_roleserver_getrolelist;
	pPlayerList->nDataLen = lenAN + 1;
	pPlayerList->ulIdentity = m_lnIdentityID;

	pPlayerList->pDataBuffer[0] = s_nRoleListCount;

	memcpy( &pPlayerList->pDataBuffer[1], m_sAccountName.c_str(), lenAN );
	pPlayerList->pDataBuffer[lenAN + 1] = '\0';

	m_theDataQueue[enumOwnerRoleSvr].Empty();

	m_pDBRoleClient->SendPackToServer( ( const void * )pPlayerList, lenPL );

	SAFE_RELEASE( pBuffer );	

	return enumToNextTask;
}

UINT CGamePlayer::ProcessRoleList()
{
#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::ProcessRoleList...\n" );
#endif

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerRoleSvr].Attach( s2c_roleserver_getrolelist_result );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

		const TProcessData *pPD = ( const TProcessData * )pRetBuffer->GetBuffer();

		int nRoleCount = CPackager::Peek( ( const char * )pPD->pDataBuffer );

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::ProcessRoleList find %d role in list\n", nRoleCount );
#endif

		if ( nRoleCount >= 0 )
		{
			m_theDataQueue[enumOwnerPlayer].Empty();

			m_pPlayerServer->SendData( m_lnIdentityID, pRetBuffer->GetBuffer(), pRetBuffer->GetUsed() );

			nNextTask = enumSelAddDelRole;
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerRoleSvr].Detach( s2c_roleserver_getrolelist_result );

		return nNextTask;
	}

	return enumRepeat;
}

UINT CGamePlayer::SelAddDelRole()
{
	CBuffer *pRetBuffer = NULL;

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::SelAddDelRole...\n" );
#endif

	/*
	 * Select a role
	 */
	pRetBuffer = m_theDataQueue[enumOwnerPlayer].Attach( c2s_dbplayerselect );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

		const tagDBSelPlayer *pDSPC = ( const tagDBSelPlayer * )pRetBuffer->GetBuffer();

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::SelAddDelRole Select a role from list\n" );
#endif

		if ( _QueryRoleInfo_ToDBRole( pDSPC->szRoleName ) )
		{
			nNextTask = enumLoginSelectRole;
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerPlayer].Detach( c2s_dbplayerselect );

		return nNextTask;
	}

	/*
	 * Create a role
	 */
	pRetBuffer = m_theDataQueue[enumOwnerPlayer].Attach( c2s_newplayer );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

		const TProcessData *pCRPD = ( const TProcessData * )pRetBuffer->GetBuffer();

		const NEW_PLAYER_COMMAND *pNPC = ( const NEW_PLAYER_COMMAND * )( pCRPD->pDataBuffer );

		ASSERT( pNPC );
		
		char szRoleName[NAME_LEN];
		size_t namelen = pRetBuffer->GetUsed() - sizeof( TProcessData );
		
		namelen = ( namelen < 31 ) ? namelen : 31;

		memcpy( szRoleName, pNPC->m_szName, namelen );
		szRoleName[namelen] = '\0';

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::SelAddDelRole Create a role [Name : %s]\n", szRoleName );
#endif
		m_theDataQueue[enumOwnerRoleSvr].Empty();

		if ( _CreateNewPlayer_ToDBRole( ( const char * )szRoleName, 
					pNPC->m_btRoleNo, 
					pNPC->m_btSeries,
					pNPC->m_NativePlaceId ) )
		{
			nNextTask = enumLoginCreateRole;
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerPlayer].Detach( c2s_newplayer );

		return nNextTask;
	}

	/*
	 * Delete a role
	 */
	pRetBuffer = m_theDataQueue[enumOwnerPlayer].Attach( c2s_roleserver_deleteplayer );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::SelAddDelRole Del a role in list\n" );
#endif
		m_theDataQueue[enumOwnerRoleSvr].Empty();

		if ( _DeleteRole_ToDBRole( pRetBuffer->GetBuffer(), pRetBuffer->GetUsed() ) )
		{
            nNextTask = enumToNextTask;
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerPlayer].Detach( c2s_roleserver_deleteplayer );

		if ( enumToNextTask != nNextTask )
		{
			tagNewDelRoleResponse ndrr;
			ndrr.cProtocol = s2c_rolenewdelresponse;
			ndrr.szRoleName[0] = '\0';
			ndrr.bSucceeded = false;

			m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&ndrr, sizeof( tagNewDelRoleResponse ) );

			nNextTask = enumSelAddDelRole;
		}

		return nNextTask;
	}

	return enumRepeat;
}

UINT CGamePlayer::DelRole_WaitForVerify()
{
#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::DelRole_WaitForVerify...\n" );
#endif
	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerAccSvr].Attach( s2c_accountlogin );
    
    if (pRetBuffer)
    {
		UINT nNextTask = enumError;

		KAccountUserReturnExt* pReturn = ( KAccountUserReturnExt * )( pRetBuffer->GetBuffer() + 1/* size of a protocol byte */ );

		int nResult = pReturn->nReturn;

		if (nResult == ACTION_SUCCESS)
        {
            const char *pRoleName = m_sDelRoleName.c_str();
	        
            const size_t lenRN = strlen( pRoleName );
	        const size_t lenRI = sizeof( TProcessData ) - 1 + lenRN;
	        
	        CBuffer *pBuffer = m_theGlobalAllocator.Allocate();
	        
	        TProcessData *pRoleInfo = reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );
	        
	        ASSERT( pRoleInfo );
	        
	        pRoleInfo->nProtoId = c2s_roleserver_deleteplayer;
	        pRoleInfo->nDataLen = lenRN;
	        pRoleInfo->ulIdentity = m_lnIdentityID;
	        
	        memcpy( &pRoleInfo->pDataBuffer[0], pRoleName, lenRN );
	        pRoleInfo->pDataBuffer[lenRN] = '\0';
	        
	        m_pDBRoleClient->SendPackToServer( ( const void * )pRoleInfo, lenRI );
	        
	        SAFE_RELEASE( pBuffer );
        
            nNextTask = enumLoginDeleteRole;
        }
        else
        {
            tagNewDelRoleResponse ndrr;
            ndrr.cProtocol = s2c_rolenewdelresponse;
            ndrr.szRoleName[0] = '\0';
            ndrr.bSucceeded = false;

            m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&ndrr, sizeof( tagNewDelRoleResponse ) );

            nNextTask = enumSelAddDelRole;  // 表示这次的动作完成，进入下一个等待流程
        }

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerAccSvr].Detach( s2c_accountlogin );

        return nNextTask;
    }
	return enumRepeat;


}


UINT CGamePlayer::WaitForCreateResult()
{
#ifdef	CONSOLE_DEBUG
	_cprintf( "CGamePlayer::WaitForCreateResult...\n" );
#endif

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerRoleSvr].Attach( s2c_roleserver_createrole_result );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

		TProcessData *pPD = ( TProcessData * )pRetBuffer->GetBuffer();

		ASSERT( pPD );

		char cResult = pPD->pDataBuffer[0];

		tagNewDelRoleResponse ndrr;
		ndrr.cProtocol = s2c_rolenewdelresponse;
		ndrr.szRoleName[0] = '\0';
		ndrr.bSucceeded = false;

#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::WaitForCreateResult %s\n", ( 1 == cResult ) ? "Successful" : "Failed" );
#endif

		switch ( cResult )
		{
		case 1:	// Successed

			ndrr.bSucceeded = true;

			m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&ndrr, sizeof( tagNewDelRoleResponse ) );

			nNextTask = enumToNextTask;
			break;

		//case -1:	// Failed
		//case -2:	// Have exist
		default:

			ndrr.bSucceeded = false;

			m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&ndrr, sizeof( tagNewDelRoleResponse ) );
			
			nNextTask = enumSelAddDelRole;
			break;
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerRoleSvr].Detach( s2c_roleserver_createrole_result );

		return nNextTask;
	}

	return enumRepeat;
}

UINT CGamePlayer::WaitForDeleteResult()
{
#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::WaitForDeleteResult\n" );
#endif

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerRoleSvr].Attach( s2c_roleserver_deleterole_result );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

		TProcessData *pPD = ( TProcessData * )pRetBuffer->GetBuffer();

		ASSERT( pPD );

		char cResult = pPD->pDataBuffer[0];

		tagNewDelRoleResponse ndrr;
		ndrr.cProtocol = s2c_rolenewdelresponse;
		ndrr.szRoleName[0] = '\0';
		ndrr.bSucceeded = false;	

#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::WaitForDeleteResult %s\n", ( 1 == cResult ) ? "Successful" : "Failed" );
#endif

		switch ( cResult )
		{
		case 1:	// Successed

			ndrr.bSucceeded = true;

			m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&ndrr, sizeof( tagNewDelRoleResponse ) );

			nNextTask = enumSelAddDelRole;
			break;

		//case -1:	// Failed
		//case -2:	// Have exist
		default:

			ndrr.bSucceeded = false;

			m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&ndrr, sizeof( tagNewDelRoleResponse ) );

			nNextTask = enumSelAddDelRole;
			break;
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerRoleSvr].Detach( s2c_roleserver_deleterole_result );

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
	const size_t lenRI = sizeof( TProcessData ) - 1 + lenRN;

	CBuffer *pBuffer = m_theGlobalAllocator.Allocate();
			
	TProcessData *pRoleInfo = reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );
	
	ASSERT( pRoleInfo );
	
	pRoleInfo->nProtoId = c2s_roleserver_getroleinfo;
	pRoleInfo->nDataLen = lenRN;
	pRoleInfo->ulIdentity = m_lnIdentityID;

	memcpy( &pRoleInfo->pDataBuffer[0], pRoleName, lenRN );
	pRoleInfo->pDataBuffer[lenRN] = '\0';
	
	m_pDBRoleClient->SendPackToServer( ( const void * )pRoleInfo, lenRI );
	
	SAFE_RELEASE( pBuffer );

	return true;
}

bool CGamePlayer::_CreateNewPlayer_ToDBRole( const char *pRoleName, 
						int nRoleSex /* male or female */, 
						int nRoleClass,
						unsigned short nMapID )
{
	if ( NULL == pRoleName || '\0' == pRoleName[0] )
	{
		return false;
	}

	size_t datalength = 0;

	CPlayerCreator::ROLEPARAM	RP;

	int nMinLen = strlen( pRoleName );
	nMinLen = nMinLen > sizeof( RP.szName ) ? sizeof( RP.szName ) : nMinLen;
	memcpy( RP.szName, pRoleName, nMinLen );
	RP.szName[nMinLen] = '\0';

	nMinLen = m_sAccountName.size();
	nMinLen = nMinLen > sizeof( RP.szAccName ) ? sizeof( RP.szAccName ) : nMinLen;
	memcpy( RP.szAccName, m_sAccountName.c_str(), nMinLen );
	RP.szAccName[nMinLen] = '\0';

	RP.nSex = nRoleSex;
	RP.nSeries = nRoleClass;
	RP.nMapID = nMapID;

	const TRoleData *pRoleData = m_thePlayerCreator.GetRoleData( datalength, &RP );

	if ( pRoleData && datalength )
	{
		CBuffer *pData = m_theGlobalAllocator.Allocate();
		
		TProcessData *pPD = reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pData->GetBuffer() ) );

		pPD->nProtoId = c2s_roleserver_createroleinfo;
		pPD->ulIdentity = m_lnIdentityID;

		pPD->pDataBuffer[0] = 1; // Create successed
		pPD->nDataLen = datalength + 1 /* sizeof( pPD->pDataBuffer[0] ) */;
		memcpy( &pPD->pDataBuffer[1], pRoleData, datalength );

		/*
		 * Send a role-info to role dbserver
		 */
		size_t nRoleInfoLength = datalength + sizeof( TProcessData );

		m_thePackager.AddData( c2s_roleserver_createroleinfo,
			( const BYTE * )pPD,
			nRoleInfoLength );

		/*
		 * For create a role
		 */
		m_theDataQueue[enumOwnerRoleSvr].AddData( s2c_roleserver_getroleinfo_result, 
			( const BYTE * )pPD,
			nRoleInfoLength );

		SAFE_RELEASE( pData );

		CBuffer *pPack = m_thePackager.GetHeadPack( c2s_roleserver_createroleinfo );

		while ( pPack )
		{
			m_pDBRoleClient->SendPackToServer( pPack->GetBuffer(), pPack->GetUsed() );

			SAFE_RELEASE( pPack );

			pPack = m_thePackager.GetNextPack( c2s_roleserver_createroleinfo );
		}

		SAFE_RELEASE( pPack );
		
		m_thePackager.DelData( c2s_roleserver_createroleinfo );

		return true;
	}

	return false;
}

UINT CGamePlayer::_DeleteRole_ToDBRole( const void *pData, size_t dataLength )
{
	tagDBDelPlayer *pDBDP = ( tagDBDelPlayer * )pData;

	if ( NULL == pData || 0 == dataLength )
	{
		return false;
	}

	const char *pAccountName = pDBDP->szAccountName;
	const char *pPassword = pDBDP->Password.szPassword;//ganlei
	const char *pRoleName = pDBDP->szRoleName;

	if ( !pAccountName || !pPassword || !pRoleName )
	{
		return false;
	}

    
	if (m_sAccountName.compare( pAccountName ) != 0)
    {
        return false;
    }

    m_sSuperPassword = pPassword ? pPassword : "";
    m_sDelRoleName   = pRoleName ? pRoleName : ""; 
    m_bUseSuperPassword = true;
    
	return true;
}

UINT CGamePlayer::ProcessRoleInfo()
{
#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::ProcessRoleInfo...\n" );
#endif

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerRoleSvr].Attach( s2c_roleserver_getroleinfo_result );

	if ( pRetBuffer )
	{
		UINT nNextTask = enumError;

		TProcessData *pPA = ( TProcessData * )pRetBuffer->GetBuffer();
		
		ASSERT( m_lnIdentityID == pPA->ulIdentity );

		char cResult = pPA->pDataBuffer[0];

#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::ProcessRoleInfo Send info to gameserver\n" );
#endif
		m_nAttachServerID = -1;

		if ( 1 == cResult && _SyncRoleInfo_ToGameServer( ( const void * )( &pPA->pDataBuffer[1] ), pPA->nDataLen-1 ) )
		{
			nNextTask = enumToNextTask;
		}
		else
		{
			tagNotifyPlayerLogin npl;

			memset( &npl, 0, sizeof( tagNotifyPlayerLogin ) );

			npl.cProtocol = s2c_notifyplayerlogin;

			m_pPlayerServer->SendData( m_lnIdentityID, ( const void * )&npl, sizeof( tagNotifyPlayerLogin ) );
		}

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerRoleSvr].Detach( s2c_roleserver_getroleinfo_result );

		return nNextTask;
	}

	return enumRepeat;
}

bool CGamePlayer::_SyncRoleInfo_ToGameServer( const void *pData, size_t dataLength )
{
	bool ok = false;

	ASSERT( pData );
	
	const TRoleData *pRoleData = ( const TRoleData * )( pData );

	IGServer *pGServer = NULL;
	if (pRoleData->BaseInfo.cUseRevive)
		pGServer = CGameServer::QueryServer( pRoleData->BaseInfo.irevivalid );
	else
		pGServer = CGameServer::QueryServer( pRoleData->BaseInfo.ientergameid );

	if ( pGServer )
	{
		ASSERT( pRoleData->BaseInfo.szName[0] != '\0' );

		CGamePlayer::Add( ( const char * )pRoleData->BaseInfo.szName,
				( IPlayer * )this );
		
		pGServer->Attach( m_sAccountName.c_str() );

		m_nAttachServerID = pGServer->GetID();

		m_theDataQueue[enumOwnerPlayer].Empty();

		ok = pGServer->DispatchTask( CGameServer::enumSyncRoleInfo, pData, dataLength, max(m_nExtPoint, 0));
		m_nExtPoint = -1;	//用完就清掉
	}

	return ok;
}

UINT CGamePlayer::WaitForGameSvrPermit()
{
#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::WaitForGameSvrPermit...\n" );
#endif

	CBuffer *pRetBuffer = m_theDataQueue[enumOwnerPlayer].Attach( s2c_notifyplayerlogin );

	if ( pRetBuffer && m_pPlayerServer )
	{
		UINT nNextTask = enumError;

		tagNotifyPlayerLogin *pNPL = ( tagNotifyPlayerLogin * )pRetBuffer->GetBuffer();

#ifdef	CONSOLE_DEBUG
		_cprintf( "CGamePlayer::WaitForGameSvrPermit Notify player to login gameserver\n" );
#endif

		if ( pNPL->bPermit )
		{
			m_bAutoUnlockAccount = false;

			nNextTask = enumToNextTask;
		}

		m_pPlayerServer->SendData( m_lnIdentityID, pRetBuffer->GetBuffer(), pRetBuffer->GetUsed() );

		SAFE_RELEASE( pRetBuffer );
		m_theDataQueue[enumOwnerPlayer].Detach( s2c_notifyplayerlogin );

		return nNextTask;
	}

	return enumRepeat;
}

bool CGamePlayer::Attach( const char *pRoleName )
{
	if ( pRoleName && pRoleName[0] )
	{
		m_sRoleName = pRoleName;

		return true;
	}

	return false;
}

bool CGamePlayer::Add( const char *pRoleName, IPlayer *pPlayer )
{
	if ( NULL == pRoleName || NULL == pPlayer || !pRoleName[0] )
	{
		ASSERT( FALSE );

		return false;
	}

	if ( pPlayer )
	{
		CCriticalSection::Owner locker( CGamePlayer::m_csMapSP );

		std::pair< stdMapSP::iterator, bool > result = 
			m_sthePlayerTable.insert( stdMapSP::value_type( pRoleName, pPlayer ) );

		if ( result.second )
		{
			return pPlayer->Attach( pRoleName );
		}
	}

	return false;
}

bool CGamePlayer::Del( const char *pRoleName )
{
	if ( !pRoleName || !pRoleName[0] )
	{
		return false;
	}

	{
		CCriticalSection::Owner locker( CGamePlayer::m_csMapSP );
		
		stdMapSP::iterator it;
		
		if ( m_sthePlayerTable.end() != ( it = m_sthePlayerTable.find( pRoleName ) ) )
		{
			IPlayer *pPlayer = ( IPlayer * )( ( *it ).second );
			
			ASSERT( pPlayer );
			
			m_sthePlayerTable.erase( it );
			
			return true;
		}
	}

	return false;
}

IPlayer *CGamePlayer::Get( const char *pRoleName )
{
	if ( !pRoleName )
	{
		return NULL;
	}

	CCriticalSection::Owner locker( CGamePlayer::m_csMapSP );

	stdMapSP::iterator it;

	if ( m_sthePlayerTable.end() != ( it = m_sthePlayerTable.find( pRoleName ) ) )
	{
		return ( IPlayer * )( ( *it ).second );
	}

	return NULL;
}