#include "stdafx.h"
#include "ClientNode.h"

#include <process.h>
#include <iostream>

#include "IDBRoleServer.h"

#include "Macro.h"

#include "../../../../Headers/KGmProtocol.h"
#include "RoleNameFilter.h"

using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CBuffer;

HANDLE CClientNode::m_hThread = NULL;
CEvent CClientNode::m_hQuitEvent( NULL, true, false );

extern UINT g_nDBEngineLoop;

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
// Multi Fixed By mrChuCong@gmail.com
	m_theProcessArray[c2s_roleserver_getrolelist] = &CClientNode::_QueryRoleList;
	m_theProcessArray[c2s_roleserver_createroleinfo] = &CClientNode::_CreateRole;
	m_theProcessArray[c2s_roleserver_saveroleinfo] = &CClientNode::_SaveRoleInfo;
	m_theProcessArray[c2s_roleserver_deleteplayer] = &CClientNode::_DelRole;
	m_theProcessArray[c2s_roleserver_getroleinfo] = &CClientNode::_GetRoleInfo;
//	m_theProcessArray[c2s_extend] = &CClientNode::_RelayExtend;
	m_theProcessArray[c2s_gamestatistic] = &CClientNode::_GetGameStat;
	m_theProcessArray[c2s_roleserver_lock] = &CClientNode::_LockOrUnlockRole;
}

CClientNode::~CClientNode()
{
	UnlockAllRole(m_nIndentity);
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
	if ( CClientNode::m_hThread == NULL )
	{
		unsigned int threadID = 0;
		CClientNode::m_hQuitEvent.Reset();		
		CClientNode::m_hThread = (HANDLE)::_beginthreadex(0,
			0,
			ThreadFunction,
			( void * )pServer,
			0,
			&threadID );
		
		if ( CClientNode::m_hThread == NULL )
		{
			return false;
		}
	}

	return true;
}

void CClientNode::End()
{
	CClientNode::m_hQuitEvent.Set();

	if ( CClientNode::m_hThread != NULL )
	{
		DWORD result = ::WaitForSingleObject( CClientNode::m_hThread, 50000 );
		
		if ( result == WAIT_TIMEOUT )
		{
			::TerminateThread( CClientNode::m_hThread, ( DWORD )( -2 ) );
		}
		
		if ( CClientNode::m_hThread != NULL )
		{
			::CloseHandle( CClientNode::m_hThread );
			CClientNode::m_hThread = NULL;
		}
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

			if ( ++ g_nDBEngineLoop & 0x80000000 )
			{
				g_nDBEngineLoop = 0;
			}

			if ( g_nDBEngineLoop & 0x1 )
			{
				::Sleep( 1 );
			}
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
	char szAccountName[_NAME_LENGTH];

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
	const size_t s_nStructSize = sizeof( S3DBI_RoleBaseInfo );

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

	int nResult = 0;

	{{
	extern CRoleNameFilter g_fltRoleName;

	TRoleData* pRoleData = (TRoleData*)(pPD->pDataBuffer + 1);

	if (pRoleData->BaseInfo.szName[0])
	{
		for (size_t pos = sizeof(pRoleData->BaseInfo.szName) - 1; pos >= 1; pos--)
		{
			if (!pRoleData->BaseInfo.szName[pos])
				break;
		}


size_t pos; // Fixed By MrChucong@gmail.com
		if (pos >= 1)
		{
			if (g_fltRoleName.IsTextPass(pRoleData->BaseInfo.szName))
				nResult = SaveRoleInfo( &pPD->pDataBuffer[1], NULL, TRUE );
			else
				nResult = -1;
		}
	}
	}}

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
	
	int nResult = 0;
	TRoleData* pRole = (TRoleData*)(&pPD->pDataBuffer[0]);
	char szName[32];
	int len = strlen( pRole->BaseInfo.szName );
	ASSERT( len > 0 );
	len = len > 31 ? 31 : len;
	memcpy( szName, pRole->BaseInfo.szName, len );
	szName[len] = '\0';

	if (IsRoleLockBySelf(szName))
	{
		nResult = SaveRoleInfo( &pPD->pDataBuffer[0], NULL, FALSE );
		if (pPD->bLeave)
		{
			UnlockRoleSelf(szName);
		}
	}
	else
		nResult = 0;

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

	char szRoleName[_NAME_LENGTH];

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

//for Relay System and GM [wxb 2003-7-22]
void CClientNode::_RelayExtend( const void *pData, size_t dataLength )
{
	ASSERT(c2s_extend == *((unsigned char *)pData));
	char* pNewData = ((char *)pData) + 1;
	switch (*((WORD*)pNewData))
	{
	case MAKEWORD(pf_gamemaster, gm_c2s_getrole):
		{
			ASSERT(dataLength == 1 + sizeof(GM_GET_ROLE_DATA_COMMAND));
			GM_GET_ROLE_DATA_COMMAND* pGMData = (GM_GET_ROLE_DATA_COMMAND*)pNewData;
			int nResultBuflen = 0;
			switch(pGMData->wGetID)
			{
			case gm_role_entergame_position:
				{
					ASSERT(0 == pGMData->wLength);
					char* pReturnBuffer = (char*)_alloca(1 + sizeof(GM_GET_ROLE_DATA_SYNC) + sizeof(GM_ROLE_DATA_SUB_ENTER_POS));

					*pReturnBuffer = (char)s2c_extend;

					GM_GET_ROLE_DATA_SYNC* pGMReturn = (GM_GET_ROLE_DATA_SYNC*)(pReturnBuffer + 1);
					strncpy(pGMReturn->AccountName, pGMData->AccountName, sizeof(pGMData->AccountName));
					pGMReturn->ProtocolFamily = pf_gamemaster;
					pGMReturn->ProtocolType = gm_s2c_getrole;
					pGMReturn->wGetID = gm_role_entergame_position;
					pGMReturn->wLength = sizeof(GM_ROLE_DATA_SUB_ENTER_POS);

					GetRoleInfoForGM(pGMData->wGetID, (char*)(pGMReturn + 1), pGMData->AccountName, nResultBuflen);
					ASSERT(pGMReturn->wLength == nResultBuflen);
					m_pServer->SendData( m_nIndentity, pReturnBuffer, 1 + sizeof(GM_GET_ROLE_DATA_SYNC) + sizeof(GM_ROLE_DATA_SUB_ENTER_POS) );	
				}
				break;
			default:
				ASSERT(0);
				break;
			}
		}
		break;

	case MAKEWORD(pf_gamemaster, gm_c2s_setrole):
		{
			ASSERT(dataLength >= 1 + sizeof(GM_SET_ROLE_DATA_COMMAND));
			GM_SET_ROLE_DATA_COMMAND* pGMData = (GM_SET_ROLE_DATA_COMMAND*)pNewData;
			SetRoleInfoForGM(pGMData->wSetID, (char*)(pGMData + 1), pGMData->AccountName, pGMData->wLength);
		}
		break;
	default:
		ASSERT(0);
		break;
	}
}

void CClientNode::_GetRoleInfo( const void *pData, size_t dataLength )
{
	ASSERT( m_pServer && pData && dataLength );

#ifdef CONSOLE_DEBUG
	cout << "_GetRoleInfo::Begin" << endl;
#endif

	TProcessData *pRoleInfo = ( TProcessData * )pData;

	char szRoleName[_NAME_LENGTH];

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
		
		if (!IsRoleLock(szRoleName))
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

void CClientNode::_GetGameStat( const void *pData, size_t dataLength )
{//发送游戏统计数据(By Fellow,2003.7.22)
	ASSERT( m_pServer && pData && dataLength );

#ifdef CONSOLE_DEBUG
	cout << "_GetGameStat::Begin" << endl;
#endif

	//收到的数据包，只有控制字没有数据
	TProcessData *pProData = ( TProcessData * )pData;
	size_t nIdentity = pProData->ulIdentity;

	//需要发送的数据包
	CBuffer *pBuffer = m_theAllocator.Allocate();
	TProcessData *pStatData = 
		reinterpret_cast< TProcessData * >( const_cast< BYTE * >( pBuffer->GetBuffer() ) );

	int nDataLen = sizeof(TGAME_STAT_DATA);
	
	pStatData->nProtoId = s2c_gamestatistic;
	pStatData->ulIdentity = nIdentity;

	pStatData->nDataLen = nDataLen;
		//取得统计数据
	TGAME_STAT_DATA* myGameStatData = (TGAME_STAT_DATA*)pStatData->pDataBuffer;
	GetGameStat(myGameStatData);

	int nUsedLen = sizeof( TProcessData ) - 1 + nDataLen;
	pBuffer->Use( nUsedLen );
	//发送
	
	m_theSend.AddData( s2c_gamestatistic_bigpackage, pBuffer->GetBuffer(), nUsedLen );

	CBuffer *pPack = m_theSend.GetHeadPack( s2c_gamestatistic_bigpackage );

	while ( pPack )
	{
		m_pServer->SendData( m_nIndentity, ( const void * )pPack->GetBuffer(), pPack->GetUsed() );

		SAFE_RELEASE( pPack );

		pPack = m_theSend.GetNextPack( s2c_gamestatistic_bigpackage );
	}

	m_theSend.DelData( s2c_gamestatistic_bigpackage );

	SAFE_RELEASE( pPack );

	SAFE_RELEASE( pBuffer );
	
#ifdef CONSOLE_DEBUG
	cout << "_GetGameStat::End" << endl;
#endif
}

/////////////////////////////////////////////////////////////////////////////////

CClientNode::stdRoleLockMap CClientNode::m_csRoleLock;
CCriticalSection	CClientNode::m_csCR;

void CClientNode::_LockOrUnlockRole( const void *pData, size_t dataLength )	//强行加解锁
{
	CCriticalSection::Owner lock( CClientNode::m_csCR );
	if (pData && dataLength == sizeof(tagRoleEnterGame))
	{
		tagRoleEnterGame* pRole = (tagRoleEnterGame*)pData;
		char szRole[32];
		strncpy(szRole, pRole->Name, 31);
		szRole[31] = 0;
		bool bLock = pRole->bLock;

		if (szRole && szRole[0] != 0)
		{
			if (bLock)
				m_csRoleLock[szRole] = m_nIndentity;
			else
			{
				stdRoleLockMap::iterator it = m_csRoleLock.find(szRole);
				if (it != m_csRoleLock.end())
					m_csRoleLock.erase(it);
			}
		}
	}
}

bool CClientNode::IsRoleLock(char* szRole)
{
	CCriticalSection::Owner lock( CClientNode::m_csCR );
	if (szRole && szRole[0] != 0)
	{
		stdRoleLockMap::iterator it = m_csRoleLock.find(szRole);
		if (it != m_csRoleLock.end() && it->second != -1)
			return true;
	}
	return false;
}

bool CClientNode::IsRoleLockBySelf(char* szRole)
{
	CCriticalSection::Owner lock( CClientNode::m_csCR );
	if (szRole && szRole[0] != 0)
	{
		stdRoleLockMap::iterator it = m_csRoleLock.find(szRole);
		if (it != m_csRoleLock.end() && it->second == m_nIndentity)
			return true;
	}
	return false;
}

bool CClientNode::UnlockRoleSelf(char* szRole)
{
	CCriticalSection::Owner lock( CClientNode::m_csCR );
	if (szRole && szRole[0] != 0)
	{
		stdRoleLockMap::iterator it = m_csRoleLock.find(szRole);
		if (it != m_csRoleLock.end() && it->second == m_nIndentity)
		{
			m_csRoleLock.erase(it);
			return true;
		}
	}
	return false;
}

void CClientNode::UnlockAllRole(size_t ID)
{
	CCriticalSection::Owner lock( CClientNode::m_csCR );
	stdRoleLockMap::iterator it = m_csRoleLock.begin();
	while (it != m_csRoleLock.end())
	{
		if (it->second == ID)
			it->second = -1;
		it++;
	}
}

/////////////////////////////////////////////////////////////////////////////////