/********************************************************************
	created:	2003/02/14
	file base:	SocketServer
	file ext:	h
	author:		liupeng
	
	purpose:	Build
*********************************************************************/
#ifndef __INCLUDE_SOCKETSERVER_H__
#define __INCLUDE_SOCKETSERVER_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include "UsesWinsock.h"
#include "Thread.h"
#include "CriticalSection.h"
#include "IOCompletionPort.h"
#include "IOBuffer.h"
#include "ManualResetEvent.h"
#include "NodeList.h"
#include "OpaqueUserData.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {
	
struct tagSockAddrIn
{
public:
	tagSockAddrIn() { Clear(); }
	tagSockAddrIn( const tagSockAddrIn& sin ) { Copy( sin ); }

	~tagSockAddrIn() { }

	tagSockAddrIn& Copy( const tagSockAddrIn& sin );
	void	Clear() { memset( &sockAddrIn, 0, sizeof( sockAddrIn ) ); }
	bool	IsEqual( const tagSockAddrIn& sin );
	bool	IsGreater( const tagSockAddrIn& sin );
	bool	IsLower( const tagSockAddrIn& pm );
	bool	IsNull() const { return ( ( sockAddrIn.sin_addr.s_addr == 0L ) && ( sockAddrIn.sin_port==0 ) ); }

	ULONG	GetIPAddr() const { return sockAddrIn.sin_addr.s_addr; }
	short	GetPort() const { return sockAddrIn.sin_port; }

	tagSockAddrIn& operator=( const tagSockAddrIn& sin ) { return Copy( sin ); }
	bool	operator==( const tagSockAddrIn& sin ) { return IsEqual( sin ); }
	bool	operator!=( const tagSockAddrIn& sin ) { return !IsEqual( sin ); }
	bool	operator<( const tagSockAddrIn& sin )  { return IsLower( sin ); }
	bool	operator>( const tagSockAddrIn& sin )  { return IsGreater( sin ); }
	bool	operator<=( const tagSockAddrIn& sin ) { return !IsGreater( sin ); }
	bool	operator>=( const tagSockAddrIn& sin ) { return !IsLower( sin ); }
	operator LPSOCKADDR() { return (LPSOCKADDR)(&sockAddrIn); }

	size_t	Size() const { return sizeof( sockAddrIn ); }
	void	SetAddr( SOCKADDR_IN* psin ) { memcpy( &sockAddrIn, psin, Size() ); }

	SOCKADDR_IN sockAddrIn;
};

/*
 * CSocketServer
 */
class CSocketServer : protected CThread, private CUsesWinsock, private CIOBuffer::Allocator
{
public:

	class Socket;
	class WorkerThread;

	friend class Socket;

	virtual ~CSocketServer();

	void Open( 
			unsigned long addressToListenOn,
			unsigned short portToListenOn
			);

	using CThread::Start;

	void StartAcceptingConnections();
	void StopAcceptingConnections();

	void InitiateShutdown();
	void WaitForShutdownToComplete();

protected:
	
	CSocketServer(
			unsigned long addressToListenOn,
			unsigned short portToListenOn,
			size_t maxFreeSockets,
			size_t maxFreeBuffers,
			size_t bufferSize = 1024,
			size_t numThreads = 0);
	
	CSocketServer(
			size_t maxFreeSockets,
			size_t maxFreeBuffers,
			size_t bufferSize = 1024,
			size_t numThreads = 0);

	void ReleaseSockets();	
	void ReleaseBuffers();
	
	/*
	 * Virtual function has different access specifier to base class
	 */
	virtual int Run();
	
private:
	
	/*
	 * Override this to create your worker thread
	 */	
	virtual WorkerThread *CreateWorkerThread( CIOCompletionPort &iocp ) = 0;
	
	/*
	 * Override this to create the listening socket of your choice
	 */	
	virtual SOCKET CreateListeningSocket( 
				unsigned long address,
				unsigned short port) = 0;
	
	/*
	 * Interface for derived classes to receive state change notifications...
	 */	
	virtual void OnStartAcceptingConnections() {}
	virtual void OnStopAcceptingConnections() {}
	virtual void OnShutdownInitiated() {}
	virtual void OnShutdownComplete() {}
	
	virtual void OnConnectionCreated() {}
	
	virtual void OnConnectionEstablished(
					Socket *pSocket,
					CIOBuffer *pAddress) = 0;
	
	virtual void OnConnectionClosed( Socket * /*pSocket*/ ) {}
	
	virtual void OnConnectionDestroyed() {}
	
	virtual void OnError( const _tstring &message );
	
	virtual void OnBufferCreated() {}
	virtual void OnBufferAllocated() {}
	virtual void OnBufferReleased() {}
	virtual void OnBufferDestroyed() {}
	
	Socket *AllocateSocket( SOCKET theSocket );	
	void ReleaseSocket( Socket *pSocket );	
	void DestroySocket( Socket *pSocket );
	
	void PostAbortiveClose( Socket *pSocket );
	
	void Read( Socket *pSocket, CIOBuffer *pBuffer );	
	void Write( Socket *pSocket, const char *pData, size_t dataLength, bool thenShutdown );	
	void Write( Socket *pSocket, CIOBuffer *pBuffer, bool thenShutdown );
	
	virtual void PreWrite( 
			Socket *pSocket, 
			CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength ) {};
	
	const size_t m_numThreads;
	
	CCriticalSection m_listManipulationSection;
	
	typedef OnlineGameLib::TNodeList<Socket> SocketList;
	
	SocketList m_activeList;
	SocketList m_freeList;
	
	SOCKET m_listeningSocket;
	
	CIOCompletionPort m_iocp;
	
	CManualResetEvent m_shutdownEvent;	
	CManualResetEvent m_acceptConnectionsEvent;
	
	unsigned long m_address;
	unsigned short m_port;
	
	const size_t m_maxFreeSockets;
	
	/*
	 * No copies do not implement
	 */
	CSocketServer( const CSocketServer &rhs );
	CSocketServer &operator=( const CSocketServer &rhs );
};

/*
 * CSocketServer::Socket
 */
class CSocketServer::Socket : public CNodeList::Node, public COpaqueUserData
{
public :
	
	friend class CSocketServer;
	friend class CSocketServer::WorkerThread;
	
	void Read( CIOBuffer *pBuffer = 0 );	
	void Write( const char *pData, size_t dataLength, bool thenShutdown = false );	
	void Write( CIOBuffer *pBuffer, bool thenShutdown = false );
	
	void AddRef();
	void Release();
	
	void Shutdown( int how = SD_BOTH );	
	void Close();	
	void AbortiveClose();
	
private:
	
	Socket( CSocketServer &server, SOCKET socket );	
	~Socket();
	
	void Attach( SOCKET socket );
	
	CSocketServer &m_server;
	SOCKET m_socket;
	
	long m_ref;
	
	/*
	 * No copies do not implement
	 */
	Socket( const Socket &rhs );
	Socket &operator=( const Socket &rhs );
};

/*
 * CSocketServer::WorkerThread
 */
class CSocketServer::WorkerThread : public CThread
{
public:
	
	virtual ~WorkerThread() {}
	
	void InitiateShutdown();	
	void WaitForShutdownToComplete();
	
protected:
	
	explicit WorkerThread( CIOCompletionPort &iocp );
	
	/*
	 * Virtual function has different access specifier to base class
	 */
	virtual int Run();
	
private:
	
	virtual void OnBeginProcessing() {}
	virtual void OnEndProcessing() {}
	
	virtual void OnError( const _tstring &message );
	
	void Read( Socket *pSocket, CIOBuffer *pBuffer ) const;	
	virtual void ReadCompleted( Socket *pSocket, CIOBuffer *pBuffer ) = 0;
	
	void Write( Socket *pSocket, CIOBuffer *pBuffer ) const;	
	virtual void WriteCompleted( Socket *pSocket, CIOBuffer *pBuffer );
	
	void AbortiveClose( Socket *pSocket );
	
	CIOCompletionPort &m_iocp;
	
	/*
	 * No copies do not implement
	 */
	WorkerThread( const WorkerThread &rhs );
	WorkerThread &operator=( const WorkerThread &rhs );
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif // __INCLUDE_SOCKETSERVER_H__