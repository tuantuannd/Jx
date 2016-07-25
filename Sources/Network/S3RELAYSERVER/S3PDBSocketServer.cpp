//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketServer.cpp	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBSocketServer.h"

S3PDBSocketServer::S3PDBSocketServer( SOCKET s, S3PDBSocketPool* pPool ):
m_piRunSignal( NULL ),
m_hServer( NULL ),
m_dwServerThreadId( 0 )
{
	m_Socket = s;
	m_piRunSignal = new int;
	*m_piRunSignal = 0;

	m_pSocketPool = pPool;
	CreateEnablePoolEvent();
}

S3PDBSocketServer::S3PDBSocketServer( int iPort, S3PDBSocketPool* pPool ):
m_Socket( INVALID_SOCKET ),
m_piRunSignal( NULL ),
m_hServer( NULL ),
m_dwServerThreadId( 0 )
{
	m_iPort = iPort;

	m_piRunSignal = new int;
	*m_piRunSignal = 0;

	m_pSocketPool = pPool;
	CreateEnablePoolEvent();
}

S3PDBSocketServer::~S3PDBSocketServer()
{
	if ( NULL != m_piRunSignal )
	{
		delete m_piRunSignal;
		m_piRunSignal = NULL;
	}
}

DWORD WINAPI S3PDBSocketServer::StartServer( LPVOID lpParam )
{
	DWORD dwRet = 0;

	if ( NULL == lpParam )
	{
		dwRet = 0;
		return dwRet;
	}

	_LPDBSOCKETSERVERPARAM lpServerParam =
		( _LPDBSOCKETSERVERPARAM )lpParam;

	if ( ( NULL == lpServerParam->piRunSignal )
		|| ( NULL == lpServerParam->pSocketPool ) )
	{
		dwRet = 0;
		return dwRet;
	}
	S3PDBSocketPool* pSocketPool = lpServerParam->pSocketPool;

	SOCKET s = lpServerParam->serverSocket;

	int iResult = 0;
	IBYTE recvBuf[def_UDPSIZE];

	while( 0 != *( lpServerParam->piRunSignal ) )
	{
		SOCKADDR_IN fromAddr;
		memset( recvBuf, 0, def_UDPSIZE );
		int iFromLen = sizeof( fromAddr );
		iResult = recvfrom( s,
			recvBuf,
			def_UDPSIZE,
			0,
			( SOCKADDR* )( &fromAddr ),
			&iFromLen );

		if ( SOCKET_ERROR == iResult )
		{
			// Reserved code section
			continue;
		}
		else if ( 0 == iResult )
		{
			// Reserved code section
			continue;
		}
		else
		{
			S3PDBSocketPool::_DBSOCKETPOOLDATAPACKET poolData;
			memcpy( poolData.dataBuf, recvBuf, iResult );
			poolData.dwSize = iResult;
			poolData.dwFromIP = fromAddr.sin_addr.S_un.S_addr;
			poolData.iFromPort = htons( fromAddr.sin_port );
			pSocketPool->PushDataPacket( &poolData );
		}
	}
	return dwRet;
}

HANDLE S3PDBSocketServer::Start()
{
	HANDLE hRet = NULL;
	if ( ( NULL != m_hServer )
		|| ( NULL == m_piRunSignal )
		|| ( NULL == m_pSocketPool )
		|| ( NULL == m_hEnablePool ) )
	{
		hRet = NULL;
		return hRet;
	}

	// The follow code is used to initialize
	// the environment for Windows OS platform
	// and should be removed or changed when
	// replanted to other platforms.
	//------>BEGIN
	WORD wVersionRequired = MAKEWORD(1,1);
	WSADATA WSAdata;
	if ( 0 != WSAStartup( wVersionRequired, &WSAdata ) )
	{
		hRet = NULL;
		return hRet;
	}
	//<------END

	if ( INVALID_SOCKET == m_Socket )
	{
		if ( FALSE == CreateSocket() )
		{
			hRet = NULL;
			return hRet;
		}
	}
	
	// Start up server thread
	//------>BEGIN
	if ( NULL != m_pSocketPool )
	{
		m_pSocketPool->Init( m_Socket, m_hEnablePool );
		m_pSocketPool->Start();
	}

	*m_piRunSignal = 1;
	m_ServerParam.piRunSignal = m_piRunSignal;
	m_ServerParam.pSocketPool = m_pSocketPool;
	m_ServerParam.serverSocket = m_Socket;
	
	m_hServer =
		KPICreateThread( S3PDBSocketServer::StartServer, &m_ServerParam, &m_dwServerThreadId );
	
	hRet = m_hServer;
	//<------END

	return hRet;
}

BOOL S3PDBSocketServer::CreateSocket()
{
	BOOL bRet = FALSE;

	if ( INVALID_SOCKET != m_Socket )
	{
		bRet = FALSE;
		return bRet;
	}

	m_Socket =
		socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( INVALID_SOCKET == m_Socket )
	{
		bRet = FALSE;
		return bRet;
	}
	
	SOCKADDR_IN local;
	local.sin_family = AF_INET;
	local.sin_port = htons( ( short )( m_iPort ) );
	//local.sin_port = htons( ( short )( KPIHash( "AccountPort", 1000, 7000 ) ) );// Not Completed...
	local.sin_addr.s_addr = htonl( INADDR_ANY );
	if ( bind( m_Socket, ( SOCKADDR* )( &local ), sizeof( local ) )
		== SOCKET_ERROR )
	{
		closesocket( m_Socket );
		m_Socket = INVALID_SOCKET;
		bRet = FALSE;
		return bRet;
	}

	bRet = TRUE;

	return bRet;
}

BOOL S3PDBSocketServer::ReleaseSocket()
{
	BOOL bRet = FALSE;
	if ( INVALID_SOCKET != m_Socket )
	{
		if ( 0 == closesocket( m_Socket ) )
		{
			m_Socket = INVALID_SOCKET;
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL S3PDBSocketServer::Stop()
{
	BOOL bRet = FALSE;

	if ( NULL == m_hServer )
	{
		bRet = FALSE;
		return bRet;
	}

	if ( NULL != m_pSocketPool )
	{
		if ( FALSE == m_pSocketPool->Stop() )
		{
			bRet = FALSE;
			return bRet;
		}
	}

	*m_piRunSignal = 0;

	DWORD dwResult = KPIWaitForSingleObject( m_hServer, 3000 );
	if ( 0 == dwResult )
	{
		bRet = FALSE;
		return bRet;
	}
	else if ( 1 == dwResult )	// The thread has ended.
	{
		m_hServer = NULL;
		bRet = ReleaseSocket();
	}
	else if ( 2 == dwResult )	// Time out.
	{
		if ( TRUE == KPITerminateThread( m_hServer, 0 ) )
		{
			m_hServer = NULL;
			bRet = ReleaseSocket();
		}
		else
		{
			bRet = FALSE;
			return bRet;
		}
	}

	if ( TRUE == bRet )
	{
		WSACleanup();	// Needed on Windows OS platforms.
	}
	return bRet;
}

BOOL S3PDBSocketServer::SendUDP( SOCKET s,
								DWORD dwTargetIP,
								int iTargetPort,
								IBYTE buf[def_UDPSIZE],
								DWORD dwSize )
{
	BOOL bRet = TRUE;

	SOCKADDR_IN targetAddr;
	targetAddr.sin_family = AF_INET;
	targetAddr.sin_port = htons( ( short )( iTargetPort ) );
	targetAddr.sin_addr.s_addr = dwTargetIP;
	int iResult = sendto( s,
		buf,
		dwSize,
		0,
		( SOCKADDR* )( &targetAddr ),
		sizeof( targetAddr ) );
	if ( SOCKET_ERROR == iResult )
	{
		bRet = FALSE;
	}

	return bRet;
}

void S3PDBSocketServer::CreateEnablePoolEvent()
{
	m_hEnablePool = KPICreateEvent( NULL, TRUE, TRUE, def_ENABLEPOOLEVENTNAME );
	if ( NULL != m_hEnablePool )
	{
		KPIResetEvent( m_hEnablePool );
	}
}

SOCKET S3PDBSocketServer::CreateSocket( int iPort )
{
	SOCKET socketRet = INVALID_SOCKET;

	socketRet =
		socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( INVALID_SOCKET == socketRet )
	{
		return socketRet;
	}
	
	SOCKADDR_IN local;
	local.sin_family = AF_INET;
	local.sin_port = htons( ( short )( iPort ) );
	local.sin_addr.s_addr = htonl( INADDR_ANY );
	if ( bind( socketRet, ( SOCKADDR* )( &local ), sizeof( local ) )
		== SOCKET_ERROR )
	{
		closesocket( socketRet );
		socketRet = INVALID_SOCKET;
		return socketRet;
	}

	return socketRet;
}