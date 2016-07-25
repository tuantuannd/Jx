//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketPool.cpp	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBSocketPool.h"
#include "regexpr2.h"

using namespace std;
using namespace regex;
BOOL S3PDBSocketPool::m_bAccDBIsLocked = FALSE;

static DWORD g_dwCount = 1; // Testing code

static BOOL CALLBACK FreePoolData( DWORD data )
{
	BOOL bRet = FALSE;
	S3PDBSocketPool::_LPDBSOCKETPOOLDATAPACKET lpData =
		( S3PDBSocketPool::_LPDBSOCKETPOOLDATAPACKET )data;
	if ( NULL != lpData )
	{
		delete lpData;
		lpData = NULL;
		bRet = TRUE;
	}
	return bRet;
}

S3PDBSocketPool::S3PDBSocketPool():
m_hProcessor( NULL ),
m_dwProcessorThreadId( 0 ),
m_piRunSignal( NULL ),
m_hEnable( NULL ),
m_Socket( INVALID_SOCKET )
{
	m_piRunSignal = new int;
	*m_piRunSignal = 0;

	m_UDPs.NeedFreeData( FreePoolData );

	memset( &m_PoolParam, 0, sizeof( _DBSOCKETPOOLPARAM ) );
}

S3PDBSocketPool::~S3PDBSocketPool()
{
	if ( NULL != m_piRunSignal )
	{
		delete m_piRunSignal;
		m_piRunSignal = NULL;
	}
}

static HANDLE g_hPoolMutex = NULL;	// Testing code
void S3PDBSocketPool::PushDataPacket(_LPDBSOCKETPOOLDATAPACKET lpData)
{
	// Testing code
	//------>BEGIN
	if ( NULL == g_hPoolMutex )
	{
		g_hPoolMutex =
			KPICreateMutex( NULL, FALSE, "Socket pool's mutex" );
	}
	if ( NULL != g_hPoolMutex )
	{
		DWORD dwWaitResult;
		dwWaitResult =
			KPIWaitForSingleObject( g_hPoolMutex, INFINITE );
		if ( 1 == dwWaitResult )
		{
			__try
			{
	//<------END

	//m_UDPs.push_back( *lpData );
	_LPDBSOCKETPOOLDATAPACKET lpNewData = new _DBSOCKETPOOLDATAPACKET;
	if ( NULL != lpNewData )
	{
		*lpNewData = *lpData;
		m_UDPs.AddTail( ( DWORD )( lpNewData ) );
		if ( NULL != m_hEnable )
		{
			KPISetEvent( m_hEnable );
		}
	}

	// Testing code
	//------>BEGIN
			}
			__finally
			{ 
                // Release ownership of the mutex object.
                if ( ! KPIReleaseMutex( g_hPoolMutex ) )
				{ 
                    // Deal with error.
                }
			}
		}
	}
	//<------END
}

BOOL S3PDBSocketPool::PopDataPacket( _DBSOCKETPOOLDATAPACKET& data )
{
	BOOL bRet = FALSE;
	// Testing code
	//------>BEGIN
	if ( NULL == g_hPoolMutex )
	{
		g_hPoolMutex =
			KPICreateMutex( NULL, FALSE, "Socket pool's mutex" );
	}

	if ( NULL != g_hPoolMutex )
	{
		DWORD dwWaitResult;
		dwWaitResult =
			KPIWaitForSingleObject( g_hPoolMutex, INFINITE );
		if ( 1 == dwWaitResult )
		{
			__try
			{
	//<------END


	if ( !( m_UDPs.IsEmpty() ) )
	{
		DWORD headData = NULL;
		if ( TRUE == m_UDPs.GetHead( headData ) )
		{
			data = *( ( _LPDBSOCKETPOOLDATAPACKET )headData );
			m_UDPs.RemoveHead();
		}

		if ( ( m_UDPs.IsEmpty() )
			&& ( NULL != m_hEnable ) )
		{
			KPIResetEvent( m_hEnable );
		}
		bRet = TRUE;
	}
	else if ( NULL != m_hEnable )
	{
		KPIResetEvent( m_hEnable );
	}

	// Testing code
	//------>BEGIN
			}
			__finally
			{ 
                // Release ownership of the mutex object.
                if ( ! KPIReleaseMutex( g_hPoolMutex ) )
				{ 
                    // Deal with error.
                }
			}
		}
	}
	//<------END

	return bRet;
}

static DWORD WINAPI ProcessData( LPVOID lpParam )
{
	DWORD dwRet = 0;
	if ( NULL == lpParam )
	{
		dwRet = 0;
		return dwRet;
	}
	S3PDBSocketPool::_LPDBSOCKETPOOLPARAM lpPoolParam =
		( S3PDBSocketPool::_LPDBSOCKETPOOLPARAM )lpParam;
	if ( ( NULL == lpPoolParam->piRunSignal )
		|| ( NULL == lpPoolParam->hEnableEvent )
		|| ( NULL == lpPoolParam->pPool ) )
	{
		dwRet = 0;
		return dwRet;
	}

	S3PDBSocketPool* pPool = lpPoolParam->pPool;

	while ( 0 != *( lpPoolParam->piRunSignal ) )
	{
		DWORD dwResult =
			KPIWaitForSingleObject( lpPoolParam->hEnableEvent, INFINITE );
		if ( dwResult == 1 )
		{
			S3PDBSocketPool::_DBSOCKETPOOLDATAPACKET data;
			
			// Processes data
			//------>BEGIN
			if ( TRUE == pPool->PopDataPacket( data ) )
			{
				BOOL bResult = pPool->Process( &data );

				// Testing code
				//------>BEGIN
				if ( bResult )
					cout << "\r\n" << g_dwCount << " Successful";
				else
					cout << "\r\n" << g_dwCount << " Failed";
				g_dwCount++;
				//<------END
			}
			//<------END
		}
	}

	return dwRet;
}

HANDLE S3PDBSocketPool::Start()
{
	HANDLE hRet = NULL;
	if ( ( NULL != m_hProcessor )
		|| ( NULL == m_hEnable )
		|| ( NULL == m_piRunSignal )
		|| ( INVALID_SOCKET == m_Socket ) )
	{
		hRet = NULL;
		return hRet;
	}

	*m_piRunSignal = 1;

	m_PoolParam.hEnableEvent = m_hEnable;
	m_PoolParam.piRunSignal = m_piRunSignal;
	m_PoolParam.pPool = this;

	m_hProcessor =
		KPICreateThread( ProcessData, &m_PoolParam, &m_dwProcessorThreadId );

	hRet = m_hProcessor;

	return hRet;
}

BOOL S3PDBSocketPool::Stop()
{
	BOOL bRet = FALSE;

	if ( ( NULL == m_hProcessor )
		|| ( NULL == m_hEnable ) )
	{
		bRet = FALSE;
		return bRet;
	}

	*m_piRunSignal = 0;
	KPISetEvent( m_hEnable );

	DWORD dwResult = KPIWaitForSingleObject( m_hProcessor, 3000 );
	if ( 0 == dwResult )
	{
		bRet = FALSE;
	}
	else if ( 1 == dwResult )
	{
		bRet = TRUE;
	}
	else if ( 2 == dwResult )	// Time out.
	{
		if ( TRUE == KPITerminateThread( m_hProcessor, 0 ) )
		{
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

void S3PDBSocketPool::Init( SOCKET s, HANDLE hEnableEvent )
{
	m_Socket = s;
	m_hEnable = hEnableEvent;
}

BOOL S3PDBSocketPool::ProMsgWhenLocked( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		IBYTE feedBuf[def_UDPSIZE];
		memset( feedBuf, 0, def_UDPSIZE );
		feedBuf[0] = LOGIN_R_TAG;
		DWORD dwSize = sizeof( KLoginAccountInfo );
		memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
		DWORD dwParam = LOGIN_R_LOGIN;
		
		if ( ( LOGIN_ACCOUNT_MAX_LEN + 2 + LOGIN_PASSWORD_MAX_LEN + 2 )
			== lpcParsedData->m_dwSize )
		{
			S3PAccount account;
			char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
			szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
			char szPassword[LOGIN_PASSWORD_MAX_LEN+2];
			szPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
			DWORD dwOffset = 0;
			memcpy( szAccount,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_ACCOUNT_MAX_LEN );
			dwOffset += LOGIN_ACCOUNT_MAX_LEN + 2;
			memcpy( szPassword,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_PASSWORD_MAX_LEN );
			dwOffset += LOGIN_PASSWORD_MAX_LEN + 2;
			
			dwParam |= LOGIN_R_ACCDBISLOCKED;
			
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( KLoginStructHead )],
				szAccount, LOGIN_ACCOUNT_MAX_LEN + 2 );
			memcpy( &feedBuf[1+sizeof( KLoginStructHead )+LOGIN_ACCOUNT_MAX_LEN+2],
				szPassword, LOGIN_PASSWORD_MAX_LEN + 2 );
			unsigned long ulSize = dwSize + 1;
			unsigned long ulEncodedSize;
			if ( TRUE == S3PDBSocketParser::Encode( feedBuf, ulSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulSize, pEncodedBuf, ulEncodedSize );
					bRet =
						SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProAddAccount( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	assert( NULL != lpcParsedData );
	BOOL bRet = FALSE;

	IBYTE feedBuf[1+KLOGINSTRUCTHEADSIZE];
	feedBuf[0] = LOGIN_R_TAG;
	DWORD dwSize = KLOGINSTRUCTHEADSIZE;
	memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
	DWORD dwParam = LOGIN_R_NEWACCOUNT;

	if ( ( LOGIN_ACCOUNT_MAX_LEN + 2
		+ LOGIN_PASSWORD_MAX_LEN + 2
		+ LOGIN_REALNAME_MAX_LEN + 2 )
		== lpcParsedData->m_dwSize )
	{	
		char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
		szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
		char szPassword[LOGIN_PASSWORD_MAX_LEN+2];
		szPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
		char szRealName[LOGIN_REALNAME_MAX_LEN+2];
		szRealName[LOGIN_REALNAME_MAX_LEN] = 0;
		DWORD dwOffset = 0;
		memcpy( szAccount,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_ACCOUNT_MAX_LEN + 2 );
		dwOffset += LOGIN_ACCOUNT_MAX_LEN + 2;
		memcpy( szPassword,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_PASSWORD_MAX_LEN + 2 );
		dwOffset += LOGIN_PASSWORD_MAX_LEN + 2;
		memcpy( szRealName,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_REALNAME_MAX_LEN + 2 );

		int iResult = S3PAccount::AddAccount( szAccount, szPassword, szRealName );
		
		switch ( iResult )
		{
		case 1:
			dwParam |= LOGIN_R_SUCCESS;
			break;
		case -4:
			dwParam |= LOGIN_R_ACCOUNT_EXIST;
			break;
		default:
			dwParam |= LOGIN_R_FAILED;
			break;
		}
		memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );

		//bRet = SendUDP( dwTargetIP, iTargetPort, feedBuf, 1 + KLOGINSTRUCTHEADSIZE );
		unsigned long ulEncodedSize;
		if ( TRUE == S3PDBSocketParser::Encode( feedBuf,
			1 + KLOGINSTRUCTHEADSIZE, NULL, ulEncodedSize ) )
		{
			IBYTE* pEncodedFeedBuf = new IBYTE[ulEncodedSize];
			if ( NULL != pEncodedFeedBuf )
			{
				S3PDBSocketParser::Encode( feedBuf, 1 + KLOGINSTRUCTHEADSIZE,
					pEncodedFeedBuf, ulEncodedSize );
				bRet =
					SendUDP( dwTargetIP, iTargetPort,
					pEncodedFeedBuf, ulEncodedSize );
				delete []pEncodedFeedBuf;
				pEncodedFeedBuf = NULL;
			}
		}
	}
	else
	{
		dwParam |= LOGIN_R_FAILED;
		memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
		bRet =
			SendUDP( dwTargetIP, iTargetPort, feedBuf, 1 + KLOGINSTRUCTHEADSIZE );
	}

	return bRet;
}

BOOL S3PDBSocketPool::ProLoginFromGame( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		IBYTE feedBuf[def_UDPSIZE];
		memset( feedBuf, 0, def_UDPSIZE );
		feedBuf[0] = LOGIN_R_TAG;
		DWORD dwSize = sizeof( KLoginAccountInfo );
		memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
		DWORD dwParam = LOGIN_R_LOGIN;

		if ( ( LOGIN_ACCOUNT_MAX_LEN + 2 + LOGIN_PASSWORD_MAX_LEN + 2 )
			== lpcParsedData->m_dwSize )
		{
			S3PAccount account;
			char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
			szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
			char szPassword[LOGIN_PASSWORD_MAX_LEN+2];
			szPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
			DWORD dwOffset = 0;
			memcpy( szAccount,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_ACCOUNT_MAX_LEN );
			dwOffset += LOGIN_ACCOUNT_MAX_LEN + 2;
			memcpy( szPassword,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_PASSWORD_MAX_LEN );
			dwOffset += LOGIN_PASSWORD_MAX_LEN + 2;
			int iResult = S3PAccount::Login( szAccount, szPassword );
			if ( 1 == iResult )
			{
				dwParam |= LOGIN_R_SUCCESS;
			}
			else if ( -4 == iResult )
			{
				dwParam |= LOGIN_R_NOTREGISTERED;
			}
			else
			{
				dwParam |= LOGIN_R_FAILED;
			}

			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( KLoginStructHead )],
				szAccount, LOGIN_ACCOUNT_MAX_LEN + 2 );
			memcpy( &feedBuf[1+sizeof( KLoginStructHead )+LOGIN_ACCOUNT_MAX_LEN+2],
				szPassword, LOGIN_PASSWORD_MAX_LEN + 2 );
			unsigned long ulSize = dwSize + 1;
			unsigned long ulEncodedSize;
			if ( TRUE == S3PDBSocketParser::Encode( feedBuf, ulSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulSize, pEncodedBuf, ulEncodedSize );
					bRet =
						SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProLogoutFromGame( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	assert( NULL != lpcParsedData );
	BOOL bRet = FALSE;

	IBYTE feedBuf[def_UDPSIZE];
	memset( feedBuf, 0, def_UDPSIZE );
	feedBuf[0] = LOGIN_R_TAG;
	DWORD dwSize = sizeof( KLoginAccountInfo );
	memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
	DWORD dwParam = LOGIN_R_LOGOUT;

	if ( ( LOGIN_ACCOUNT_MAX_LEN + 2 + LOGIN_PASSWORD_MAX_LEN + 2 + sizeof( DWORD ) )
		== lpcParsedData->m_dwSize )
	{
		S3PAccount account;

		char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
		szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
		char szPassword[LOGIN_PASSWORD_MAX_LEN+2];
		szPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
		DWORD dwGameID = 0;
		DWORD dwOffset = 0;
		memcpy( szAccount,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_ACCOUNT_MAX_LEN );
		dwOffset += LOGIN_ACCOUNT_MAX_LEN + 2;
		memcpy( szPassword,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_PASSWORD_MAX_LEN );
		dwOffset += LOGIN_PASSWORD_MAX_LEN + 2;
		memcpy( &dwGameID,
			lpcParsedData->m_lpData + dwOffset,
			sizeof( DWORD ) );
		dwOffset += sizeof( DWORD );

		int iResult = S3PAccount::Logout( szAccount, szPassword, dwGameID );
		switch ( iResult )
		{
		case 1:
			dwParam |= LOGIN_R_SUCCESS;
			break;
		default:
			dwParam |= LOGIN_R_FAILED;
			break;
		}
		memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
		memcpy( &feedBuf[1+sizeof( KLoginStructHead )],
			szAccount, LOGIN_ACCOUNT_MAX_LEN + 2 );
		memcpy( &feedBuf[1+sizeof( KLoginStructHead )+LOGIN_ACCOUNT_MAX_LEN+2],
			szPassword, LOGIN_PASSWORD_MAX_LEN + 2 );
		unsigned long ulSize = sizeof( KLoginAccountInfo ) + 1;
		unsigned long ulEncodedSize;
		if ( TRUE == S3PDBSocketParser::Encode( feedBuf, ulSize, NULL, ulEncodedSize ) )
		{
			IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
			if ( NULL != pEncodedBuf )
			{
				S3PDBSocketParser::Encode( feedBuf, ulSize, pEncodedBuf, ulEncodedSize );
				bRet =
					SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
				delete []pEncodedBuf;
				pEncodedBuf = NULL;
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProReportFromGame( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	assert( NULL != lpcParsedData );
	BOOL bRet = FALSE;
	IBYTE feedBuf[def_UDPSIZE];
	memset( feedBuf, 0, def_UDPSIZE );
	feedBuf[0] = LOGIN_R_TAG;
	DWORD dwSize = sizeof( KLoginAccountInfo );
	memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
	DWORD dwParam = LOGIN_R_REPORT;

	if ( ( LOGIN_ACCOUNT_MAX_LEN + 2 + LOGIN_PASSWORD_MAX_LEN + 2 + sizeof( DWORD ) )
		== lpcParsedData->m_dwSize )
	{
		S3PAccount account;

		char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
		szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
		char szPassword[LOGIN_PASSWORD_MAX_LEN+2];
		szPassword[LOGIN_PASSWORD_MAX_LEN] = 0; 
		DWORD dwGameID = 0;
		DWORD dwOffset = 0;
		memcpy( szAccount,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_ACCOUNT_MAX_LEN );
		dwOffset += LOGIN_ACCOUNT_MAX_LEN + 2;
		memcpy( szPassword,
			lpcParsedData->m_lpData + dwOffset,
			LOGIN_PASSWORD_MAX_LEN );
		dwOffset += LOGIN_PASSWORD_MAX_LEN + 2;
		memcpy( &dwGameID,
			&( lpcParsedData->m_lpData[LOGIN_ACCOUNT_MAX_LEN+LOGIN_PASSWORD_MAX_LEN] ),
			sizeof( DWORD ) );
		dwOffset += sizeof( DWORD );

		int iResult = S3PAccount::Report( szAccount, szPassword, dwGameID );
		switch ( iResult )
		{
		case 1:
			dwParam |= LOGIN_R_SUCCESS;
			break;
		case -3:
			dwParam |= LOGIN_R_BEDISCONNECTED;
			break;
		case -4:
			dwParam |= LOGIN_R_NOTREGISTERED;
			break;
		default:
			dwParam |= LOGIN_R_FAILED;
			break;
		}
		memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
		memcpy( &feedBuf[1+sizeof( KLoginStructHead )],
			szAccount, LOGIN_ACCOUNT_MAX_LEN + 2 );
		memcpy( &feedBuf[1+sizeof( KLoginStructHead )+LOGIN_ACCOUNT_MAX_LEN+2],
			szPassword, LOGIN_PASSWORD_MAX_LEN + 2 );
		unsigned long ulSize = sizeof( KLoginAccountInfo ) + 1;
		unsigned long ulEncodedSize;
		if ( TRUE == S3PDBSocketParser::Encode( feedBuf, ulSize, NULL, ulEncodedSize ) )
		{
			IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
			if ( NULL != pEncodedBuf )
			{
				S3PDBSocketParser::Encode( feedBuf, ulSize, pEncodedBuf, ulEncodedSize );
				bRet =
					SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
				delete []pEncodedBuf;
				pEncodedBuf = NULL;
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProQueryGameserverList( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		IBYTE feedBuf[def_UDPSIZE*10];
		memset( feedBuf, 0, def_UDPSIZE * 10 );
		feedBuf[0] = LOGIN_R_TAG;
		DWORD dwSize = sizeof( KLoginAccountInfo );
		memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
		DWORD dwParam = LOGIN_R_LOGIN;
		
		if ( ( LOGIN_ACCOUNT_MAX_LEN + 2 + LOGIN_PASSWORD_MAX_LEN + 2 )
			== lpcParsedData->m_dwSize )
		{
			char szAccount[LOGIN_ACCOUNT_MAX_LEN + 2 ];
			szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
			char szPassword[LOGIN_PASSWORD_MAX_LEN+1];
			szPassword[LOGIN_PASSWORD_MAX_LEN] = 0; 
			memcpy( szAccount,
				lpcParsedData->m_lpData,
				LOGIN_ACCOUNT_MAX_LEN );
			memcpy( szPassword,
				&( lpcParsedData->m_lpData[LOGIN_ACCOUNT_MAX_LEN] ),
				LOGIN_PASSWORD_MAX_LEN );		
			S3PAccount account;
			DWORD dwSizeTemp = 0;
			IBYTE* pFeedBuf = feedBuf;
			IBYTE* pLastFeedBuf = NULL;
			int iResult =
				account.QueryGameserverList( szAccount, szPassword, NULL, dwSizeTemp );
			BOOL bFixedMemory = TRUE;
			if ( 1 == iResult )
			{
				pFeedBuf = new IBYTE[dwSizeTemp+dwSize+1];
				if ( NULL != pFeedBuf )
				{
					memcpy( pFeedBuf, feedBuf, dwSize + 1 );
					dwSize += dwSizeTemp;
					memcpy( &( pFeedBuf[1] ), &dwSize, sizeof( DWORD ) );
					account.QueryGameserverList( szAccount, szPassword,
						&( pFeedBuf[dwSize+1] ), dwSizeTemp );
					bFixedMemory = FALSE;
				}
				else
				{
					pFeedBuf = feedBuf;
					dwParam |= LOGIN_R_FAILED;
				}
			}
			else
			{
				dwParam |= LOGIN_R_FAILED;
			}

			memcpy( &pFeedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			memcpy( &pFeedBuf[1+sizeof( KLoginStructHead )],
				szAccount, LOGIN_ACCOUNT_MAX_LEN + 2 );
			memcpy( &pFeedBuf[1+sizeof( KLoginStructHead )+LOGIN_ACCOUNT_MAX_LEN+2],
				szPassword, LOGIN_PASSWORD_MAX_LEN + 2 );
			unsigned long ulSize = dwSize + 1;
			unsigned long ulEncodedSize;
			if ( TRUE == S3PDBSocketParser::Encode( feedBuf, ulSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulSize, pEncodedBuf, ulEncodedSize );
					bRet =
						SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}

			if ( ( !bFixedMemory ) && ( NULL != pFeedBuf ) )
			{
				delete []pFeedBuf;
				pFeedBuf = NULL;
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProBeginGame( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		IBYTE feedBuf[def_UDPSIZE];
		memset( feedBuf, 0, def_UDPSIZE );
		feedBuf[0] = LOGIN_R_TAG;
		DWORD dwSize = sizeof( KLoginAccountInfo );
		memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
		DWORD dwParam = LOGIN_R_BEGINGAME;

		if ( ( LOGIN_ACCOUNT_MAX_LEN + 2 + LOGIN_PASSWORD_MAX_LEN + 2 + sizeof( DWORD ) )
			== lpcParsedData->m_dwSize )
		{
			S3PAccount account;
			char szAccount[LOGIN_ACCOUNT_MAX_LEN+2];
			szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
			char szPassword[LOGIN_PASSWORD_MAX_LEN+2];
			szPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
			DWORD dwGameID = 0;
			DWORD dwOffset = 0;
			memcpy( szAccount,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_ACCOUNT_MAX_LEN );
			dwOffset += LOGIN_ACCOUNT_MAX_LEN + 2;
			memcpy( szPassword,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_PASSWORD_MAX_LEN );
			dwOffset += LOGIN_PASSWORD_MAX_LEN + 2;
			memcpy( &dwGameID,
				lpcParsedData->m_lpData + dwOffset,
				sizeof( DWORD ) );
			dwOffset += sizeof( DWORD );

			int iResult = S3PAccount::Login( szAccount, szPassword, dwGameID );
			if ( 1 == iResult )
			{
				dwParam |= LOGIN_R_SUCCESS;
			}
			else if ( -4 == iResult )
			{
				dwParam |= LOGIN_R_NOTREGISTERED;
			}
			else
			{
				dwParam |= LOGIN_R_FAILED;
			}

			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( KLoginStructHead )],
				szAccount, LOGIN_ACCOUNT_MAX_LEN + 2 );
			memcpy( &feedBuf[1+sizeof( KLoginStructHead )+LOGIN_ACCOUNT_MAX_LEN+2],
				szPassword, LOGIN_PASSWORD_MAX_LEN + 2 );
			unsigned long ulSize = dwSize + 1;
			unsigned long ulEncodedSize;
			if ( TRUE == S3PDBSocketParser::Encode( feedBuf, ulSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulSize, pEncodedBuf, ulEncodedSize );
					bRet =
						SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBLogin( DWORD dwTargetIP, INT iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT ) - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+1];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+1];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			struct sockaddr_in targetAddr;
			targetAddr.sin_family = AF_INET;
			targetAddr.sin_addr.s_addr = dwTargetIP;
			targetAddr.sin_port = htons( ( short )iTargetPort );
			GUID msgID;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			memcpy( szPassword,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN] ),
				def_DBPASSWORD_MAX_LEN );
			memcpy( &msgID,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN + def_DBPASSWORD_MAX_LEN] ),
				sizeof( GUID ) );
			char* lpszTargetIP = inet_ntoa( targetAddr.sin_addr );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_LOGIN;

			int iResult =
				S3PAccount::DBLogin( szUserName, szPassword, lpszTargetIP );
			if ( 1 == iResult )			// Successful
			{
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else if ( -4 == iResult )	// User has logged in
			{
				dwParam |= DBLOGIN_R_HASLOGGEDIN;
			}
			else						// Failed to logged in
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBLogout( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT ) - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+1];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+1];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			GUID msgID;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			memcpy( szPassword,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN] ),
				def_DBPASSWORD_MAX_LEN );
			memcpy( &msgID,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN + def_DBPASSWORD_MAX_LEN] ),
				sizeof( GUID ) );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_LOGOUT;

			int iResult =
				S3PAccount::DBLogout( szUserName, szPassword );
			if ( 1 == iResult )			// Successful
			{
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else						// Failed to logged out
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBQueryUserList( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT ) - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+1];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+1];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			GUID msgID;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			memcpy( szPassword,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN] ),
				def_DBPASSWORD_MAX_LEN );
			memcpy( &msgID,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN + def_DBPASSWORD_MAX_LEN] ),
				sizeof( GUID ) );

			IBYTE feedBuf[def_UDPSIZE];
			IBYTE* pLastFeedBuf = NULL;
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_QUERYUSERLIST;
			DWORD dwUserListSize = 0;
			int iResult =
				S3PAccount::DBQueryUserList( szUserName,
				szPassword, NULL, dwUserListSize );
			BOOL bFeedBufIsFixed = TRUE;
			if ( 1 == iResult )
			{
				if ( 0 == dwUserListSize )
				{
					pLastFeedBuf = feedBuf;
					dwParam |= DBLOGIN_R_SUCCESS;
				}
				else
				{
					pLastFeedBuf =
						new IBYTE[1+sizeof( _DBLOGINSTRUCT )+sizeof( GUID )+dwUserListSize];
					if ( NULL != pLastFeedBuf )
					{
						bFeedBufIsFixed = FALSE;
						dwSize += dwUserListSize;
						memcpy( pLastFeedBuf, feedBuf, 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID ) );
						memcpy( &pLastFeedBuf[1], &dwSize, sizeof( DWORD ) );
						dwParam |= DBLOGIN_R_SUCCESS;
						S3PAccount::DBQueryUserList( szUserName, szPassword,
							&( pLastFeedBuf[1+sizeof( _DBLOGINSTRUCT )+sizeof( GUID )] ),
							dwUserListSize );
					}
					else
					{
						pLastFeedBuf = feedBuf;
						dwParam |= DBLOGIN_R_FAILED;
					}
				}
			}
			else
			{
				pLastFeedBuf = feedBuf;
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &pLastFeedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );

			unsigned long ulFeedSize =
				1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID ) + dwUserListSize;
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( pLastFeedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( pLastFeedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
			if ( !bFeedBufIsFixed )
			{
				delete []pLastFeedBuf;
				pLastFeedBuf = NULL;
			}
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBLock( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT ) - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+1];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+1];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			GUID msgID;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			memcpy( szPassword,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN] ),
				def_DBPASSWORD_MAX_LEN );
			memcpy( &msgID,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN + def_DBPASSWORD_MAX_LEN] ),
				sizeof( GUID ) );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_LOCKDATABASE;

			int iResult =
				S3PAccount::DBLock( szUserName, szPassword );
			if ( 1 == iResult )			// Successful
			{
				m_bAccDBIsLocked = TRUE;
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else						// Failed to lock account database
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBActivate( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT ) - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+1];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+1];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			GUID msgID;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			memcpy( szPassword,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN] ),
				def_DBPASSWORD_MAX_LEN );
			memcpy( &msgID,
				&( lpcParsedData->m_lpData[def_DBUSERNAME_MAX_LEN + def_DBPASSWORD_MAX_LEN] ),
				sizeof( GUID ) );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_ACTIVATEDATABASE;

			int iResult =
				S3PAccount::DBActivate( szUserName, szPassword );
			if ( 1 == iResult )			// Successful
			{
				m_bAccDBIsLocked = TRUE;
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else						// Failed to activate account database
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBAddUser( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT )
			 + def_DBUSERNAME_MAX_LEN + def_DBPASSWORD_MAX_LEN
			 + sizeof( short int )
			 - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+2];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+2];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			char szNewUserName[def_DBUSERNAME_MAX_LEN+2];
			szNewUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szNewUserPassword[def_DBPASSWORD_MAX_LEN+2];
			szNewUserPassword[def_DBPASSWORD_MAX_LEN] = 0;
			short int siNewUserPriority = 0;
			GUID msgID;
			DWORD dwOffset = 0;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			dwOffset += def_DBUSERNAME_MAX_LEN;
			memcpy( szPassword,
				lpcParsedData->m_lpData + dwOffset,
				def_DBPASSWORD_MAX_LEN );
			dwOffset += def_DBPASSWORD_MAX_LEN;
			memcpy( szNewUserName,
				lpcParsedData->m_lpData + dwOffset,
				def_DBUSERNAME_MAX_LEN );
			dwOffset += def_DBUSERNAME_MAX_LEN;
			memcpy( szNewUserPassword,
				lpcParsedData->m_lpData + dwOffset,
				def_DBPASSWORD_MAX_LEN );
			dwOffset += def_DBPASSWORD_MAX_LEN;
			memcpy( &siNewUserPriority,
				lpcParsedData->m_lpData + dwOffset,
				sizeof( short int ) );
			dwOffset += sizeof( short int );
			memcpy( &msgID,
				lpcParsedData->m_lpData + dwOffset,
				sizeof( GUID ) );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_ADDDBUSER;

			int iResult =
				S3PAccount::DBAddUser( szUserName, szPassword,
				szNewUserName, szNewUserPassword, siNewUserPriority );
			if ( 1 == iResult )			// Successful
			{
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else if ( -6 == iResult )
			{
				dwParam |= DBLOGIN_R_HASNOENOUGHPRIORITY;
			}
			else if ( -7 == iResult )
			{
				dwParam |= DBLOGIN_R_USERHASEXISTED;
			}
			else						// Failed to activate account database
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBDeleteUser( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT )
			 + def_DBUSERNAME_MAX_LEN
			 - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+2];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+2];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			char szSelUserName[def_DBUSERNAME_MAX_LEN+2];
			szSelUserName[def_DBUSERNAME_MAX_LEN] = 0;
			GUID msgID;
			DWORD dwOffset = 0;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			dwOffset += def_DBUSERNAME_MAX_LEN;
			memcpy( szPassword,
				lpcParsedData->m_lpData + dwOffset,
				def_DBPASSWORD_MAX_LEN );
			dwOffset += def_DBPASSWORD_MAX_LEN;
			memcpy( szSelUserName,
				lpcParsedData->m_lpData + dwOffset,
				def_DBUSERNAME_MAX_LEN );
			dwOffset += def_DBUSERNAME_MAX_LEN;
			memcpy( &msgID,
				lpcParsedData->m_lpData + dwOffset,
				sizeof( GUID ) );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_DELDBUSER;
			int iResult =
				S3PAccount::DBDeleteUser( szUserName, szPassword, szSelUserName );
			if ( 1 == iResult )			// Successful
			{
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else if ( -4 == iResult )
			{
				dwParam |= DBLOGIN_R_HASNOENOUGHPRIORITY;
			}
			else						// Failed to activate account database
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::ProDBCreateAccount( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpcParsedData )
	{
		if ( ( sizeof( GUID ) + sizeof( _DBLOGINSTRUCT )
			 + LOGIN_REALNAME_MAX_LEN
			 + LOGIN_PASSWORD_MAX_LEN
			 + LOGIN_ACCOUNT_MAX_LEN
			 - sizeof( KLoginStructHead ) )
			== lpcParsedData->m_dwSize )
		{
			char szUserName[def_DBUSERNAME_MAX_LEN+2];
			szUserName[def_DBUSERNAME_MAX_LEN] = 0;
			char szPassword[def_DBPASSWORD_MAX_LEN+2];
			szPassword[def_DBPASSWORD_MAX_LEN] = 0;
			char szAccRealName[LOGIN_REALNAME_MAX_LEN+2];
			szAccRealName[LOGIN_REALNAME_MAX_LEN] = 0;
			char szAccPassword[LOGIN_PASSWORD_MAX_LEN+2];
			szAccPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
			char szAccName[LOGIN_ACCOUNT_MAX_LEN+2];
			szAccName[LOGIN_ACCOUNT_MAX_LEN] = 0;
			GUID msgID;
			DWORD dwOffset = 0;
			memcpy( szUserName,
				lpcParsedData->m_lpData,
				def_DBUSERNAME_MAX_LEN );
			dwOffset += def_DBUSERNAME_MAX_LEN;
			memcpy( szPassword,
				lpcParsedData->m_lpData + dwOffset,
				def_DBPASSWORD_MAX_LEN );
			dwOffset += def_DBPASSWORD_MAX_LEN;
			memcpy( szAccRealName,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_REALNAME_MAX_LEN );
			dwOffset += LOGIN_REALNAME_MAX_LEN;
			memcpy( szAccPassword,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_PASSWORD_MAX_LEN );
			dwOffset += LOGIN_PASSWORD_MAX_LEN;
			memcpy( szAccName,
				lpcParsedData->m_lpData + dwOffset,
				LOGIN_ACCOUNT_MAX_LEN );
			dwOffset += LOGIN_ACCOUNT_MAX_LEN;
			memcpy( &msgID,
				lpcParsedData->m_lpData + dwOffset,
				sizeof( GUID ) );
			IBYTE feedBuf[def_UDPSIZE];
			feedBuf[0] = LOGIN_R_TAG;
			DWORD dwSize = sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			memcpy( &feedBuf[1], &dwSize, sizeof( DWORD ) );
			memcpy( &feedBuf[1+sizeof( DWORD )*2],
				&szUserName, def_DBUSERNAME_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( DWORD )*2+def_DBUSERNAME_MAX_LEN],
				&szPassword, def_DBPASSWORD_MAX_LEN );
			memcpy( &feedBuf[1+sizeof( _DBLOGINSTRUCT )], &msgID, sizeof( GUID ) );
			DWORD dwParam = DBLOGIN_R_CREATEACCOUNT;
			int iResult =
				S3PAccount::DBCreateAccount( szUserName, szPassword,
				szAccRealName, szAccPassword, szAccName );
			if ( 1 == iResult )
			{
				dwParam |= DBLOGIN_R_SUCCESS;
			}
			else if ( -6 == iResult )
			{
				dwParam |= DBLOGIN_R_HASNOENOUGHPRIORITY;
			}
			else
			{
				dwParam |= DBLOGIN_R_FAILED;
			}
			memcpy( &feedBuf[1+sizeof( DWORD )], &dwParam, sizeof( DWORD ) );
			unsigned long ulFeedSize = 1 + sizeof( _DBLOGINSTRUCT ) + sizeof( GUID );
			unsigned long ulEncodedSize;
			if ( TRUE ==
				S3PDBSocketParser::Encode( feedBuf, ulFeedSize, NULL, ulEncodedSize ) )
			{
				IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
				if ( NULL != pEncodedBuf )
				{
					S3PDBSocketParser::Encode( feedBuf, ulFeedSize, pEncodedBuf, ulEncodedSize );
					bRet = SendUDP( dwTargetIP, iTargetPort, pEncodedBuf, ulEncodedSize );
					delete []pEncodedBuf;
					pEncodedBuf = NULL;
				}
			}
		}
		else
		{
			// No operation here
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::Distribute( DWORD dwTargetIP, int iTargetPort, _LPCPARSEDDATAPACKET lpcParsedData )
{
	assert( NULL != lpcParsedData );
	BOOL bRet = FALSE;
	
	switch( lpcParsedData->m_dwParam )
	{
	case LOGIN_A_LOGIN:
		if ( m_bAccDBIsLocked )
			bRet = ProMsgWhenLocked( dwTargetIP, iTargetPort, lpcParsedData );
		else
			bRet = ProLoginFromGame( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case LOGIN_A_NEWACCOUNT:
		if ( m_bAccDBIsLocked )
			bRet = ProMsgWhenLocked( dwTargetIP, iTargetPort, lpcParsedData );
		else
			bRet = ProAddAccount( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case LOGIN_A_REPORT:
		if ( m_bAccDBIsLocked )
			bRet = ProMsgWhenLocked( dwTargetIP, iTargetPort, lpcParsedData );
		else
			bRet = ProReportFromGame( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case LOGIN_A_LOGOUT:
		if ( m_bAccDBIsLocked )
			bRet = ProMsgWhenLocked( dwTargetIP, iTargetPort, lpcParsedData );
		else
			bRet = ProLogoutFromGame( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case LOGIN_A_SERVERLIST:
		if ( m_bAccDBIsLocked )
			bRet = ProMsgWhenLocked( dwTargetIP, iTargetPort, lpcParsedData );
		else
			bRet = ProQueryGameserverList( dwTargetIP, iTargetPort, lpcParsedData );
		bRet = FALSE;
		break;
	case LOGIN_A_BEGINGAME:
		if ( m_bAccDBIsLocked )
			bRet = ProMsgWhenLocked( dwTargetIP, iTargetPort, lpcParsedData );
		else
			bRet = ProBeginGame( dwTargetIP, iTargetPort, lpcParsedData );
		bRet = FALSE;
		break;
	case DBLOGIN_A_LOGIN:
		bRet = ProDBLogin( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_LOGOUT:
		bRet = ProDBLogout( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_QUERYUSERLIST:
		bRet = ProDBQueryUserList( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_LOCKDATABASE:
		bRet = ProDBLock( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_ACTIVATEDATABASE:
		bRet = ProDBActivate( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_ADDDBUSER:
		bRet = ProDBAddUser( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_DELDBUSER:
		bRet = ProDBDeleteUser( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	case DBLOGIN_A_CREATEACCOUNT:
		bRet = ProDBCreateAccount( dwTargetIP, iTargetPort, lpcParsedData );
		break;
	default:
		break;
	}

	return bRet;
}

BOOL S3PDBSocketPool::Process( _LPDBSOCKETPOOLDATAPACKET lpData )
{
	BOOL bRet = FALSE;
	if ( NULL != lpData )
	{
		DWORD dwTargetIP = lpData->dwFromIP;
		int iTargetPort = lpData->iFromPort;

		S3PDBSocketParser parser;
		if ( TRUE == parser.Parse( lpData->dataBuf, lpData->dwSize ) )
		{
			bRet = Distribute( dwTargetIP, iTargetPort, &parser );
		}
	}
	return bRet;
}

BOOL S3PDBSocketPool::SendUDP( DWORD dwTargetIP, int iTargetPort, IBYTE* pBuf, DWORD dwSize )
{
	BOOL bRet = FALSE;

	SOCKADDR_IN targetAddr;
	targetAddr.sin_family = AF_INET;
	targetAddr.sin_port = htons( ( short )( iTargetPort ) );
	targetAddr.sin_addr.s_addr = dwTargetIP;
	int iResult = sendto( m_Socket,
		pBuf,
		dwSize,
		0,
		( SOCKADDR* )( &targetAddr ),
		sizeof( targetAddr ) );
	if ( SOCKET_ERROR == iResult )
	{
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
	}

	return bRet;
}

BOOL S3PDBSocketPool::SendError( DWORD dwTargetIP, int iTargetPort )
{
	BOOL bRet = FALSE;
	IBYTE sendBuf[KLOGINSTRUCTHEADSIZE+1];
	sendBuf[0] = LOGIN_R_TAG;
	DWORD dwSize = KLOGINSTRUCTHEADSIZE;
	DWORD dwParam = LOGIN_R_FAILED;
	memcpy( &( sendBuf[1] ), &dwSize, sizeof( DWORD ) );
	memcpy( &( sendBuf[1+sizeof( DWORD )] ), &dwParam, sizeof( DWORD ) );

	bRet = SendUDP( dwTargetIP, iTargetPort, sendBuf, KLOGINSTRUCTHEADSIZE + 1 );
	
	return bRet;
}