#include "GameServer.h"
#include "GameServerWorkerThread.h"
//#ifdef def_MSSQLSERVER_DB
//[ Include in .\..\IPCPServer
#include "..\NetWork\IOCPServer\Utils.h"
#include "..\NetWork\IOCPServer\Win32Exception.h"
#include "..\NetWork\IOCPServer\Socket.h"
//]

//[ Include in .\..\Protocol
#include "..\Protocol\Protocol.h"
//]

/*
 * Using directives
 */
using OnlineGameLib::Win32::CIOCompletionPort;
using OnlineGameLib::Win32::CIOBuffer;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CWin32Exception;
using OnlineGameLib::Win32::CSocket;

CGameServer::CGameServer(
		   const std::string &welcomeMessage,
		   unsigned long addressToListenOn,
		   unsigned short portToListenOn,
		   size_t maxFreeSockets,
		   size_t maxFreeBuffers,
		   size_t bufferSize /* = 1024 */,
		   size_t numThreads /* = 0 */)
	   : OnlineGameLib::Win32::CSocketServer(addressToListenOn, portToListenOn, maxFreeSockets, maxFreeBuffers, bufferSize, numThreads),
		 m_welcomeMessage(welcomeMessage)
{
}

CGameServer::CGameServer(
		const std::string &welcomeMessage,
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize /* = 1024 */,
		size_t numThreads /* = 0 */)
	   : OnlineGameLib::Win32::CSocketServer(maxFreeSockets, maxFreeBuffers, bufferSize, numThreads),
		 m_welcomeMessage(welcomeMessage)
{
}


CGameServer::~CGameServer()
{
	/*
	 * If we want to be informed of any buffers or sockets being destroyed at destruction 
	 * time then we need to release these resources now, whilst we, the derived class,
	 * still exists. Once our destructor exits and the base destructor takes over we wont 
	 * get any more notifications...
	 */
	
	try
	{
		ReleaseSockets();
		ReleaseBuffers();
	}
	catch(...)
	{
	}
}

SOCKET CGameServer::CreateListeningSocket( unsigned long address, unsigned short port )
{
	SOCKET s = ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED );
	
	if ( INVALID_SOCKET == s )
	{
		throw CWin32Exception( _T("CSocket::CreateListeningSocket()"), ::WSAGetLastError() );
	}
	
	CSocket listeningSocket( s );
	
	CSocket::InternetAddress localAddress( address, port );
	
	listeningSocket.Bind( localAddress );
	
	listeningSocket.Listen( 5 );
	
	return listeningSocket.Detatch();
}

OnlineGameLib::Win32::CSocketServer::WorkerThread *CGameServer::CreateWorkerThread( 
					OnlineGameLib::Win32::CIOCompletionPort &iocp )
{
	Output( _T("CreateWorkerThread") );
	
	return new CGameServerWorkerThread( iocp );
}

void CGameServer::OnStartAcceptingConnections()
{
	Output( _T("OnStartAcceptingConnections") );
}

void CGameServer::OnStopAcceptingConnections()
{
	Output( _T("OnStopAcceptingConnections") );
}
      
void CGameServer::OnShutdownInitiated()
{
	Output( _T("OnShutdownInitiated") );
}
      
void CGameServer::OnShutdownComplete()
{
	Output( _T("OnShutdownComplete") );
}

void CGameServer::OnConnectionEstablished(
					OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
					OnlineGameLib::Win32::CIOBuffer * pAddress )
{
	Output( _T("OnConnectionEstablished") );
	// Not completed...
	//------>BEGIN
	//<------END
	pSocket->Read();
}

void CGameServer::OnConnectionClosed( Socket * pSocket )
{
	Output( _T("OnConnectionClosed") );
	// Not completed...
	//------>BEGIN
	//<------END
}

void CGameServer::OnConnectionCreated()
{
	Output( _T("OnConnectionCreated") );
}

void CGameServer::OnConnectionDestroyed()
{
	Output( _T("OnConnectionDestroyed") );
}

void CGameServer::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

void CGameServer::PreWrite( 
			OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
			OnlineGameLib::Win32::CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength )
{
	/*
	 * TODO : You can change protocol that it is used to split package
	 */
	if ( pBuffer && dataLength > 0 )
	{
		PACK_HEADER ph = {0};
		
		ph.cPackBeginFlag = PACK_BEGIN_FLAG;
		ph.cPackEndFlag = PACK_END_FLAG;
		
		ph.wDataLen = dataLength;
		ph.wCRCData = MAKE_CRC_DATE( PACK_BEGIN_FLAG, PACK_END_FLAG, dataLength );

		pBuffer->AddData( reinterpret_cast<const char *>(&ph), PACK_HEADER_LEN );
	}
}
//#endif