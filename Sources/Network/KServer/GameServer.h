/********************************************************************
	created:	2003/02/15
	file base:	GameServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_ONLINEGAME_GAMESERVER_H__
#define __INCLUDE_ONLINEGAME_GAMESERVER_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

//[ Include in .\..\IPCPServer
#include "IOCPServer\SocketServer.h"
#include "IOCPServer\tstring.h"
//]

/*
 * Classes defined in other files...
 */
namespace OnlineGameLib {
namespace Win32 {

//[ Include in .\..\IPCPServer
class CIOCompletionPort;
//]

} // End of namespace OnlineGameLib
} // End of namespace Win32

class CGameServer : public OnlineGameLib::Win32::CSocketServer
{
public :
	
	CGameServer(
		unsigned long addressToListenOn,
		unsigned short portToListenOn,
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024,
		size_t numThreads = 0);
	
	CGameServer(
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024,
		size_t numThreads = 0);
	
	virtual ~CGameServer();
	
private :
	
	virtual WorkerThread *CreateWorkerThread(
		OnlineGameLib::Win32::CIOCompletionPort &iocp);
	
	virtual SOCKET CreateListeningSocket(
		unsigned long address,
		unsigned short port);
	
	virtual void OnStartAcceptingConnections();
	virtual void OnStopAcceptingConnections();
	virtual void OnShutdownInitiated();
	virtual void OnShutdownComplete();
	
	virtual void OnConnectionCreated();
	virtual void OnConnectionEstablished(
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
		OnlineGameLib::Win32::CIOBuffer *pAddress);
	
	virtual void OnConnectionClosed( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket );
	
	virtual void OnConnectionDestroyed();
	
	virtual void OnError( const OnlineGameLib::Win32::_tstring &message );
	
	virtual void PreWrite( 
			OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
			OnlineGameLib::Win32::CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength );
	
	// No copies do not implement
	CGameServer( const CGameServer &rhs );
	CGameServer &operator=( const CGameServer &rhs );
};

#endif //__INCLUDE_ONLINEGAME_GAMESERVER_H__