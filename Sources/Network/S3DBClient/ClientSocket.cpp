// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "S3DBClient.h"
#include "S3PDBSocketParser.h"
#include "ClientSocket.h"
#include "objbase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientSocket* CClientSocket::m_pInstance = NULL;

CClientSocket::CClientSocket():
//m_s( INVALID_SOCKET ),
m_hServer( NULL ),
m_pS( NULL )
{
	memset( &m_serverAddr, 0, sizeof( struct sockaddr_in ) );
}

CClientSocket::~CClientSocket()
{
}

CClientSocket* CClientSocket::Instance( short* psiPort/* = NULL */ )
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new CClientSocket;
	}

	if ( NULL != m_pInstance )
	{
		if ( ( !( m_pInstance->HasValidSocket() ) )
			&& ( NULL != psiPort ) )
		{
			if ( m_pInstance->InitWinSocket() )
			{
				m_pInstance->CreateSocket( *psiPort );
			}
		}
	}

	return m_pInstance;
}

CClientSocket* CClientSocket::GetInstance()
{
	return m_pInstance;
}

void CClientSocket::ReleaseInstance()
{
	if ( NULL != m_pInstance )
	{
		m_pInstance->ReleaseSocket();
		m_pInstance->UninitWinSocket();
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

BOOL CClientSocket::HasValidSocket()
{
	BOOL bRet = FALSE;
	if ( NULL != m_pS )
	{
		bRet = TRUE;
	}
	/*
	if ( INVALID_SOCKET != m_s )
	{
		bRet = TRUE;
	}
	*/
	return bRet;
}

BOOL CClientSocket::CreateSocket(short siPort)
{
	BOOL bRet = FALSE;
	try
	{
		m_pS = new CGameClient( 10, 1024 );
		if ( NULL != m_pS )
		{
			bRet = TRUE;
		}
	}
	catch ( ... )
	{
		bRet = FALSE;
	}
	/*
	m_s = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( INVALID_SOCKET == m_s )
	{
		bRet = FALSE;
	}
	else
	{
		struct sockaddr_in local;
		local.sin_family = AF_INET;
		local.sin_port = htons( siPort );
		local.sin_addr.s_addr = INADDR_ANY;
		if ( SOCKET_ERROR ==
			bind( m_s, ( SOCKADDR* )&local, sizeof( local ) ) )
		{
			closesocket( m_s );	// Asserts that m_s can be close rightly. 
			m_s = INVALID_SOCKET;
			bRet = FALSE;
		}
	}
	*/
	return bRet;
}

BOOL CClientSocket::ReleaseSocket()
{
	BOOL bRet = TRUE;
	if ( NULL != m_pS )
	{
		try
		{
			m_pS->StopConnections();
		}
		catch ( ... )
		{
			bRet = FALSE;
		}
	}
	/*
	if ( 0 == closesocket( m_s ) )
	{
		bRet = TRUE;
	}
	*/
	return bRet;
}

BOOL CClientSocket::InitWinSocket()
{
	BOOL bRet = TRUE;
	WSADATA wsd;
	if ( 0 != WSAStartup( MAKEWORD(1,1), &wsd ) )
	{
		bRet = FALSE;
	}
	return bRet;
}

BOOL CClientSocket::UninitWinSocket()
{
	BOOL bRet = TRUE;
	if ( 0 != WSACleanup() )
	{
		bRet = FALSE;
	}
	return bRet;
}

BOOL CClientSocket::Start()
{
	BOOL bRet = FALSE;
	if ( NULL != m_pS )
	{
		try
		{
			m_pS->Start();
			bRet = TRUE;
		}
		catch ( ... )
		{
			bRet = FALSE;
		}
	}
	/*
	if ( ( HasValidSocket() )
		&& ( NULL == m_hServer ) )
	{
		m_serverParam.s = m_s;
		m_serverParam.pS = m_pS;
		DWORD dwThreadId;
		m_hServer =
			CreateThread( 0, 0, CClientSocket::RunServer, &m_serverParam, 0, &dwThreadId );
		bRet = TRUE;
	}*/
	return bRet;
}

BOOL CClientSocket::Stop()
{
	BOOL bRet = TRUE;
	if ( TRUE == TerminateThread( m_hServer, 0 ) )
	{
		bRet = CloseHandle( m_hServer );
		if ( TRUE == bRet )
		{
			m_hServer = NULL;
		}
	}
	return bRet;
}

BOOL CClientSocket::RemoteQuery( _PDBLOGINSTRUCT pParam,
								CClientSocket::LOGINCALLBACK LoginCallbackFun,
								DWORD dwCustomParam,
								DWORD dwMilliseconds )
{
	BOOL bRet = FALSE;
	if ( NULL != pParam )
	{
		IBYTE iTag = 0;
		unsigned long ulSendSize =
			pParam->Size + sizeof( iTag ) + sizeof( GUID );
		IBYTE* pSendBuf = new IBYTE[ulSendSize];
		if ( NULL != pSendBuf )
		{
			GUID msgId = GUID_NULL;
			::CoCreateGuid( &msgId );
			if ( GUID_NULL != msgId )
			{
				pSendBuf[0] = iTag;
				memcpy( &( pSendBuf[1] ), pParam, pParam->Size/*sizeof( _DBLOGINSTRUCT )*/ );
				memcpy( &( pSendBuf[1+pParam->Size] ), &msgId, sizeof( GUID ) );
				DWORD dwSize = pParam->Size + sizeof( GUID );
				memcpy( &( pSendBuf[1] ), &dwSize, sizeof( pParam->Size ) );

				unsigned long ulEncodedSize;
				if ( TRUE == S3PDBSocketParser::Encode( pSendBuf, ulSendSize,
					NULL, ulEncodedSize ) )
				{
					IBYTE* pEncodedBuf = new IBYTE[ulEncodedSize];
					if ( NULL != pEncodedBuf )
					{
						S3PDBSocketParser::Encode( pSendBuf, ulSendSize,
							pEncodedBuf, ulEncodedSize );
						try
						{
							m_pS->Write( pEncodedBuf, ulEncodedSize );	//douhao 调用刘鹏的socket发送
							
							DWORD dwResult =
								WaitForSingleObject( m_pS->m_hHasRecvdata, dwMilliseconds );
							if ( WAIT_OBJECT_0 == dwResult )
							{
								ResetEvent( m_pS->m_hHasRecvdata );
								S3PDBSocketParser parser;
								if ( TRUE == parser.Parse( ( char* )( m_pS->m_pRecvBuf ),
									m_pS->m_dwRecvSize ) )
								{
									GUID revMsgId;
									memcpy( &revMsgId,
										&( parser.m_lpData[def_DBUSERNAME_MAX_LEN+def_DBPASSWORD_MAX_LEN] ),
										sizeof( GUID ) );
									if ( revMsgId == msgId )
									{
										if ( NULL != LoginCallbackFun )
										{
											LoginCallbackFun( ( DWORD )( m_pS->m_pRecvBuf ),
												dwCustomParam );
										}
										bRet = TRUE;
									}
								}
							}
						}
						catch ( ... )
						{
							bRet = FALSE;
						}
						delete []pEncodedBuf;
						pEncodedBuf = NULL;
					}
				}
				delete []pSendBuf;
				pSendBuf = NULL;
			}
		}
	}
	return bRet;
}

BOOL CClientSocket::SetServerAddr( LPCTSTR lpszIPAddr, short siPort )
{
	BOOL bRet = FALSE;
	if ( NULL != lpszIPAddr )
	{
		DWORD dwServerAddr = inet_addr( lpszIPAddr );
		if ( INADDR_NONE != dwServerAddr )
		{
			bRet = SetServerAddr( dwServerAddr, siPort );
		}
	}
	return bRet;
}

BOOL CClientSocket::SetServerAddr( DWORD dwIPAddr, short siPort )
{
	BOOL bRet = TRUE;
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons( siPort );
	m_serverAddr.sin_addr.s_addr = dwIPAddr;
	return bRet;
}

static DWORD WINAPI gRecvfrom( LPVOID lParam )
{
	DWORD dwRet = 0;
	if ( NULL != lParam )
	{
		CClientSocket::_PRECVPARAM pParam =
			( CClientSocket::_PRECVPARAM )lParam;
		pParam->iResult = recvfrom( pParam->s, pParam->pRecvBuf, pParam->iRecvBufSize,
			0, ( struct sockaddr* )( &pParam->from ), &( pParam->fromlen ) );
	}
	return dwRet;
}
//-------------------------------------------------------------------------
// Recvfrom:
// Return 0 : successful
//        1 : time out
//        2 : general error
//        3 : socket has been closed gracefully
//        4 : socket error
//-------------------------------------------------------------------------
DWORD CClientSocket::Recvfrom( char FAR* buf, int len, DWORD dwMilliseconds )
{
	DWORD dwRet = 0;
	DWORD dwThreadID;
	m_recvParam.from = m_serverAddr;
	m_recvParam.fromlen = sizeof( m_recvParam.from );
	m_recvParam.pRecvBuf = buf;
	m_recvParam.iRecvBufSize = len;
	m_recvParam.s = m_s;	// Not used
	m_recvParam.pS = m_pS;
	HANDLE hThread = CreateThread( 0, 0, gRecvfrom, &m_recvParam, 0, &dwThreadID );
	if ( NULL != hThread )
	{
		DWORD dwResult = WaitForSingleObject( hThread, dwMilliseconds );
		if ( WAIT_OBJECT_0 == dwResult )
		{
			if (  0 == m_recvParam.iResult )
			{
				dwRet = 3; // socket has been closed gracefully
			}
			else if ( SOCKET_ERROR == m_recvParam.iResult )
			{
				dwRet = 4; // socket error
			}
			else
			{
				dwRet = 0; // successful
			}
		}
		else if ( WAIT_TIMEOUT == dwResult )
		{
			dwRet = 1; // time out
		}
		else// if ( WAIT_ABANDONED == dwResult )
		{
			dwRet = 2; // wrong
		}

		CloseHandle( hThread );
	}
	return dwRet;
}

BOOL CClientSocket::IncludeSomeBitValue( DWORD dwMatrix, DWORD dwSomeBitValue )
{
	BOOL bRet = FALSE;
	if ( ( dwMatrix & dwSomeBitValue ) == dwSomeBitValue )
	{
		bRet = TRUE;
	}
	return bRet;
}

BOOL CClientSocket::Connect( LPCTSTR lpszAddr, short siPort )
{
	BOOL bRet = FALSE;
	if ( NULL != m_pS )
	{
		try
		{
			m_pS->ConnectTo( lpszAddr, siPort );
			m_pS->StartConnections();
			bRet = TRUE;
		}
		catch ( ... )
		{
			bRet = FALSE;
		}
	}
	return bRet;
}