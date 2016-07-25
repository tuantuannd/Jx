#include "stdafx.h"
#include "ClientNode.h"

#include <process.h>
#include <iostream>

#include "IDBRoleServer.h"

#include "Macro.h"

using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CBuffer;

HANDLE CClientNode::m_hThread = NULL;
CEvent CClientNode::m_hQuitEvent( NULL, true, false );

CCriticalSection	CClientNode::m_csCL;

CClientNode::stdMap	CClientNode::m_theClientMap;


CClientNode::CDataQueue::CDataQueue( size_t bufferSize /*= 1024 * 64*/, size_t maxFreeBuffers /*= 160*/ )
						: m_theDQAllocator( bufferSize, maxFreeBuffers )
{
}

CClientNode::CDataQueue::~CDataQueue()
{
}

bool CClientNode::CDataQueue::AddData( const BYTE *pData, size_t datalength )
{
	CBuffer *pBuffer = m_theDQAllocator.Allocate();

	pBuffer->AddData( pData, datalength );

	{
		CCriticalSection::Owner lock( m_csQueue );

		m_theData.push_back( pBuffer );
	}

	return true;
}

CBuffer *CClientNode::CDataQueue::Get()
{
	{
		CCriticalSection::Owner lock( m_csQueue );

		if ( !m_theData.empty() )
		{
			CBuffer *pBuffer = m_theData.front();

			pBuffer->AddRef();

			m_theData.pop_front();

			pBuffer->Release();

			return pBuffer;
		}
	}

	return NULL;
}

CClientNode::CClientNode( IServer *pServer, size_t id )
			: m_nIndentity( id ) 
			, m_theAllocator( 1024 * 64, 3 )
			, m_pServer( pServer )
{
	ZeroMemory( m_theProcessArray, sizeof( m_theProcessArray ) );

	m_theProcessArray[c2s_roleserver_getrolelist] = _QueryRoleList;
	m_theProcessArray[c2s_roleserver_createroleinfo] = _CreateRole;
	m_theProcessArray[c2s_roleserver_saveroleinfo] = _SaveRoleInfo;
	m_theProcessArray[c2s_roleserver_deleteplayer] = _DelRole;
	m_theProcessArray[c2s_roleserver_getroleinfo] = _GetRoleInfo;
}

CClientNode::~CClientNode()
{
	SAFE_RELEASE( m_pServer );
}

CClientNode *CClientNode::AddNode( IServer *pServer, size_t id )
{
	CCriticalSection::Owner lock( CClientNode::m_csCL );

	IServer *pCloneServer = NULL;
	pServer->QueryInterface( IID_IIOCPServer, ( void ** )&pCloneServer );

	CClientNode *pNode = new CClientNode( pCloneServer, id );
		
	CClientNode::m_theClientMap.insert( stdMap::value_type( id, pNode ) );

	return pNode;
}

void CClientNode::DelNode( size_t id )
{
	stdMap::iterator it;

	if ( CClientNode::m_theClientMap.end() != ( it = CClientNode::m_theClientMap.find( id ) ) )
	{
		CCriticalSection::Owner lock( CClientNode::m_csCL );

		CClientNode *pNode = ( *it ).second;
	
		CClientNode::m_theClientMap.erase( id );

		SAFE_DELETE( pNode );
	}
}

bool CClientNode::Start( IServer *pServer )
{
	unsigned int threadID = 0;
	
	CClientNode::m_hThread = (HANDLE)::_beginthreadex(0,
		0,
		ThreadFunction,
		( void * )pServer,
		0,
		&threadID );
	
	if ( CClientNode::m_hThread == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	return true;
}

void CClientNode::End()
{
	CClientNode::m_hQuitEvent.Set();

	DWORD result = ::WaitForSingleObject( CClientNode::m_hThread, 50000 );

	if ( result == WAIT_TIMEOUT )
	{
		::TerminateThread( CClientNode::m_hThread, ( DWORD )( -2 ) );
	}

	if ( CClientNode::m_hThread != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( CClientNode::m_hThread );
		CClientNode::m_hThread = INVALID_HANDLE_VALUE;
	}

	/*
	 * Save all
	 */
}

unsigned int __stdcall CClientNode::ThreadFunction( void *pV )
{
	IServer *pServer = reinterpret_cast< IServer * >( pV );

	ASSERT( pServer );

	try
	{
		while ( !CClientNode::m_hQuitEvent.Wait( 0 ) )
		{
			{
				CCriticalSection::Owner lock( CClientNode::m_csCL );

				CClientNode::stdMap::iterator it;

				for ( it = CClientNode::m_theClientMap.begin();
					it != CClientNode::m_theClientMap.end();
					it ++ )
					{
						CClientNode *pNode = ( CClientNode * )( ( *it ).second );

						ASSERT( pNode );

						pNode->Process();
					}
			}
		
			::Sleep( 1 );
		}
	}
	catch(...)
	{
		::MessageBox( NULL, "CClientNode::ThreadFunction was error!", "Warning", MB_OK );
	}

	return 0L;
}

void CClientNode::AppendData( const void *pData, size_t datalength )
{
	if ( pData && datalength )
	{
		BYTE cProtocol = CPackager::Peek( pData );
		
		if ( cProtocol < s2c_micropackbegin )
		{
			LargePackProcess( pData, datalength );
		}
		else if ( cProtocol > s2c_micropackbegin )
		{
			SmallPackProcess( pData, datalength );
		}
		else
		{
			ASSERT( FALSE && "Error!" );
		}
	}
}

void CClientNode::SmallPackProcess( const void *pData, size_t dataLength )
{
	BYTE cProtocol = CPackager::Peek( pData );
	
	ASSERT( cProtocol >= 0 && cProtocol < s2c_end );

	m_theDataQueue.AddData( ( const BYTE * )pData, dataLength );
}

void CClientNode::LargePackProcess( const void *pData, size_t dataLength )
{
	ASSERT( pData && dataLength );

	CBuffer *pBuffer = m_theRecv.PackUp( pData, dataLength );

	if ( pBuffer )
	{
		m_theDataQueue.AddData( pBuffer->GetBuffer(), pBuffer->GetUsed() );
		
		SAFE_RELEASE( pBuffer );
	}
}

void CClientNode::Process()
{
	CBuffer *pBuffer = m_theDataQueue.Get();

	if ( pBuffer )
	{
		const BYTE *pData = pBuffer->GetBuffer();
		const size_t dataLength = pBuffer->GetUsed();

		BYTE cProtocol = CPackager::Peek( pData );

		if ( cProtocol < c2s_end && m_theProcessArray[cProtocol] )
		{
			( this->*m_theProcessArray[cProtocol] )( ( const void * )pData, dataLength );
		}
	}

	SAFE_RELEASE( pBuffer );
}

void CClientNode::_QueryRoleList( const void *pData, size_t dataLength )
{
	static char szAccountName[_NAME_LENGTH];

	ASSERT( m_pServer && pData && dataLength );

#ifdef CONSOLE_DEBUG
	cout << "_QueryRoleList::Begin" << endl;
#endif

	TProcessData *pPlayerList = ( TProcessData * )pData;

	int nRoleListCount = pPlayerList->pDataBuffer[0];
	int nLen = pPlayerList->nDataLen;
	unsigned long ulIdentity = pPlayerList->ulIdentity;

	if ( nLen <= 1 || nLen >= _NAME_LENGTH )
	{
#ifdef CONSOLE_DEBUG
		cout << "_QueryRoleList::Name is invalid" << endl;
#endif

		return;
	}

	memcpy( szAccountName, ( const char * )( &pPlayerList->pDataBuffer[1] ), nLen - 1 );
	szAccountName[nLen - 1] = '\0';

	/*
	 * Database
	 */
	S3DBI_RoleBaseInfo DBI[4];
	static const size_t s_nStructSize = sizeof( S3DBI_RoleBaseInfo );

#ifdef CONSOLE_DEBUG
	cout << "_QueryRoleList::GetRoleListOfAccount " << szAccountName << endl;
#endif

	int nCount = GetRoleListOfAccount( szAccountName, &DBI[0], nRoleListCount );

	CBuffer *pBuffer = m_theAllocator.Allocate();

	TProcessData *pListData = reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	pListData->nProtoId = s2c_roleserver_getrolelist_result;
	pListData->ulIdentity = ulIdentity;

	int nDataLen = nCount * s_nStructSize + 1;
	pListData->nDataLen = nDataLen;
	
	pListData->pDataBuffer[0] = nCount;
	memcpy( &pListData->pDataBuffer[1], &DBI[0], nDataLen );

	int nUsedLen = sizeof( TProcessData ) - 1 + nDataLen;
	pBuffer->Use( nUsedLen );

	m_pServer->SendData( m_nIndentity, ( const void * )pListData, nUsedLen );

	SAFE_RELEASE( pBuffer );	

#ifdef CONSOLE_DEBUG
	cout << "_QueryRoleList::end" << endl;
#endif
}
void CClientNode::_CreateRole( const void *pData, size_t dataLength )
{
	ASSERT( m_pServer && pData && dataLength );

#ifdef CONSOLE_DEBUG
	cout << "_CreateRole::Begin" << endl;
#endif

	TProcessData *pPD = ( TProcessData * )pData;
	
	int nResult = SaveRoleInfo( &pPD->pDataBuffer[1], NULL, TRUE );

	TProcessData Info;

	Info.nProtoId = s2c_roleserver_createrole_result;
	Info.ulIdentity = pPD->ulIdentity;
	Info.nDataLen = 1;
	Info.pDataBuffer[0] = ( nResult == 1 ) ? 1 : -1;

	m_pServer->SendData( m_nIndentity, ( const void * )&Info, sizeof( Info ) );

#ifdef CONSOLE_DEBUG
	cout << "_CreateRole::End" << endl;
#endif
}
void CClientNode::_SaveRoleInfo( const void *pData, size_t dataLength )
{
	ASSERT( m_pServer && pData && dataLength );
#ifdef CONSOLE_DEBUG
	cout << "_SaveRoleInfo::Begin" << endl;
#endif

	TProcessData *pPD = ( TProcessData * )pData;
	
	int nResult = SaveRoleInfo( &pPD->pDataBuffer[0], NULL, FALSE );

	if ( pPD->ulIdentity >= 0 )
	{
		TProcessData Info;
		
		Info.nProtoId = s2c_roleserver_saverole_result;
		Info.ulIdentity = pPD->ulIdentity;
		Info.nDataLen = 1;
		Info.pDataBuffer[0] = ( nResult == 1 ) ? 1 : -1;
		
		m_pServer->SendData( m_nIndentity, ( const void * )&Info, sizeof( Info ) );
	}

#ifdef CONSOLE_DEBUG
	cout << "_SaveRoleInfo::End" << endl;
#endif
}
void CClientNode::_DelRole( const void *pData, size_t dataLength )
{
	ASSERT( m_pServer && pData && dataLength );

#ifdef CONSOLE_DEBUG
	cout << "_DelRole::Begin" << endl;
#endif

	TProcessData *pRoleInfo = ( TProcessData * )pData;

	static char szRoleName[_NAME_LENGTH];

	int nDataLen = pRoleInfo->nDataLen;
	nDataLen = ( nDataLen > _NAME_LENGTH ) ? _NAME_LENGTH : nDataLen;

	int result = -1;

	if ( nDataLen > 0 )
	{
		memcpy( szRoleName, &pRoleInfo->pDataBuffer[0], nDataLen );
		szRoleName[nDataLen] = '\0';

#ifdef CONSOLE_DEBUG
		cout << "_DelRole::DeleteRole " << szRoleName << endl;
#endif

		if ( DeleteRole( szRoleName ) )
		{
			result = 1;
		}
		else
		{
			result = -1;
		}
	}

	if ( pRoleInfo->ulIdentity >= 0 )
	{
		TProcessData Info;
		
		Info.nProtoId = s2c_roleserver_deleterole_result;
		Info.ulIdentity = pRoleInfo->ulIdentity;
		Info.nDataLen = 1;
		Info.pDataBuffer[0] = result;
		
		m_pServer->SendData( m_nIndentity, ( const void * )&Info, sizeof( Info ) );	
	}

#ifdef CONSOLE_DEBUG	
	cout << "_DelRole::End" << endl;
#endif
}
void CClientNode::_GetRoleInfo( const void *pData, size_t dataLength )
{
	ASSERT( m_pServer && pData && dataLength );

#ifdef CONSOLE_DEBUG
	cout << "_GetRoleInfo::Begin" << endl;
#endif

	TProcessData *pRoleInfo = ( TProcessData * )pData;

	static char szRoleName[_NAME_LENGTH];

	int nDataLen = pRoleInfo->nDataLen;
	nDataLen = ( nDataLen > _NAME_LENGTH ) ? _NAME_LENGTH : nDataLen;

	CBuffer *pBuffer = m_theAllocator.Allocate();
		
	TProcessData *pRoleData = reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	int nUsedLength = sizeof( TProcessData );

	size_t nIdentity = pRoleInfo->ulIdentity;
	
	pRoleData->nProtoId = s2c_roleserver_getroleinfo_result;
	pRoleData->pDataBuffer[0] = -1;
	pRoleData->nDataLen = 1;	
	pRoleData->ulIdentity = nIdentity;

	if ( nDataLen > 0 )
	{
		memcpy( szRoleName, &pRoleInfo->pDataBuffer[0], nDataLen );
		szRoleName[nDataLen] = '\0';
		
		int result = -1;

#ifdef CONSOLE_DEBUG
		cout << "_GetRoleInfo::GetRoleInfo " << szRoleName << endl;
#endif

		int nRoleInfoLen = 0;
		GetRoleInfo( &pRoleData->pDataBuffer[1], szRoleName, nRoleInfoLen );

		if ( nRoleInfoLen > 0 )
		{
			pRoleData->pDataBuffer[0] = 1;
			pRoleData->nDataLen = nRoleInfoLen + 1;

			nUsedLength = sizeof( TProcessData ) + nRoleInfoLen;
		}
	}

	pBuffer->Use( nUsedLength );

	m_theSend.AddData( s2c_roleserver_getroleinfo_result, pBuffer->GetBuffer(), nUsedLength, nIdentity );

	CBuffer *pPack = m_theSend.GetHeadPack( s2c_roleserver_getroleinfo_result );

	while ( pPack )
	{
		m_pServer->SendData( m_nIndentity, ( const void * )pPack->GetBuffer(), pPack->GetUsed() );

		SAFE_RELEASE( pPack );

		pPack = m_theSend.GetNextPack( s2c_roleserver_getroleinfo_result );
	}

	m_theSend.DelData( s2c_roleserver_getroleinfo_result );

	SAFE_RELEASE( pPack );

	SAFE_RELEASE( pBuffer );

#ifdef CONSOLE_DEBUG
	cout << "_GetRoleInfo::End" << endl;
#endif
}