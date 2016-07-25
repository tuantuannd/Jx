#include "stdafx.h"
#include "GameServer.h"
#include "GameServerWorkerThread.h"

//[ Include in .\..\IPCPServer
#include "IOCPServer\Utils.h"
#include "IOCPServer\Win32Exception.h"
#include "IOCPServer\Socket.h"
//]

//[ Include in .\..\Protocol
#include "Protocol\Protocol.h"
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
		   unsigned long addressToListenOn,
		   unsigned short portToListenOn,
		   size_t maxFreeSockets,
		   size_t maxFreeBuffers,
		   size_t bufferSize /* = 1024 */,
		   size_t numThreads /* = 0 */)
	   : OnlineGameLib::Win32::CSocketServer(
							addressToListenOn, 
							portToListenOn,
							maxFreeSockets,
							maxFreeBuffers, 
							bufferSize, 
							numThreads)
{
}

CGameServer::CGameServer(
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize /* = 1024 */,
		size_t numThreads /* = 0 */)
	   : OnlineGameLib::Win32::CSocketServer(
							maxFreeSockets, 
							maxFreeBuffers, 
							bufferSize, 
							numThreads)
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
		throw CWin32Exception( _T("CGameServer::CreateListeningSocket()"), ::WSAGetLastError() );
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
	Output( _T("CGameServer::CreateWorkerThread") );
	
	return new CGameServerWorkerThread( iocp );
}

void CGameServer::OnConnectionEstablished(
					OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
					OnlineGameLib::Win32::CIOBuffer * pAddress )
{
	Output( _T("CGameServer::OnConnectionEstablished") );
	
	pSocket->Read();
}

void CGameServer::OnConnectionClosed( Socket * pSocket )
{
	Output( _T("CGameServer::OnConnectionClosed") );	
}

void CGameServer::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("CGameServer::OnError - ") + message );
}

void CGameServer::PreWrite( 
			OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
			OnlineGameLib::Win32::CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength )
{

}