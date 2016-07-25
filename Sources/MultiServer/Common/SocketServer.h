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
#include "..\heaven\interface\IServer.h"
/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {


/*
 * CSocketServer
 */
class CSocketServer : 
		   protected CThread, 
		   private CUsesWinsock, 
		   protected CIOBuffer::Allocator
{
public:

	class Socket;
	friend class Socket;
	
	virtual ~CSocketServer();
	
	using CThread::Start;
	
	void StartAcceptingConnections();
	void StopAcceptingConnections();
	
	void InitiateShutdown();
	
	void WaitForShutdownToComplete();

protected:
	
	class WorkerThread;
	friend class WorkerThread;

	CSocketServer(	
			size_t maxFreeSockets,
			size_t maxFreeBuffers,
			size_t bufferSize = 1024,
			size_t numThreads = 0,
			bool useSequenceNumbers = true,
			bool postZeroByteReads = false);
	
	void Open( unsigned long addressToListenOn, unsigned short portToListenOn );
	
	static void SetServerDataPtr( Socket *pSocket, void *pData );
	static void *GetServerDataPtr( const Socket *pSocket );
	
	static void WriteCompleted( Socket *pSocket );
	
	static CSocketServer &GetServer( Socket *pSocket );

	/*
	 * Thread virtual function
	 */
	virtual int Run();

	enum enumConnectionErrorSource
	{
		enumZeroByteReadError,
		enumReadError,
		enumWriteError
	};

private:

	class UserData;
	class ServerData;
	
	/*
	 * Override this to create your worker thread
	 */	
	virtual WorkerThread *CreateWorkerThread( CIOCompletionPort &iocp );
	
	/*
	 * Override this to create the listening socket of your choice
	 */	
	virtual SOCKET CreateListeningSocket( unsigned long address, 
							unsigned short port );
	
	/*
	 * Interface for derived classes to receive state change notifications...
	 */	
	virtual void OnStartAcceptingConnections() {}
	virtual void OnStopAcceptingConnections() {}
	virtual void OnShutdownInitiated() {}
	virtual void OnShutdownComplete() {}
	
	virtual void OnConnectionCreated() {}
	
	virtual void OnConnectionEstablished( Socket *pSocket, CIOBuffer *pAddress ) = 0;
	
	virtual void OnConnectionClientClose( Socket * /*pSocket*/ ) {}
	
	virtual void OnConnectionReset( Socket * /*pSocket*/ ) {}
	
	virtual bool OnConnectionClosing( Socket * /*pSocket*/ ) { return false; }	
	virtual void OnConnectionClosed( Socket * /*pSocket*/ ) {}
	
	virtual void OnConnectionDestroyed() {}
	
	virtual void OnConnectionError( enumConnectionErrorSource source,
							Socket *pSocket,
							CIOBuffer *pBuffer,
							DWORD lastError );
	
	virtual void OnError( const _tstring &message );
	
	virtual void OnBufferCreated() {}
	virtual void OnBufferAllocated() {}
	virtual void OnBufferReleased() {}
	
	virtual void OnThreadCreated() {}
	virtual void OnThreadDestroyed() {}
	
	virtual bool FilterSocketShutdown( Socket *pSocket, int how );	
	virtual bool FilterSocketClose( Socket *pSocket );
	
	virtual void ReadCompleted( Socket *pSocket, CIOBuffer *pBuffer ) = 0;	
	virtual void WriteCompleted( Socket *pSocket, CIOBuffer *pBuffer );
	
	void ReleaseSockets();
	
	Socket *AllocateSocket( SOCKET theSocket );
	
	void ReleaseSocket( Socket *pSocket );
	
	void DestroySocket( Socket *pSocket );
	
	enum enumIO_Operation
	{ 
		enumIO_Zero_Byte_Read_Request,
		enumIO_Zero_Byte_Read_Completed,
		enumIO_Read_Request, 
		enumIO_Read_Completed, 
		enumIO_Write_Request, 
		enumIO_Write_Completed,
	};
	
	void PostIoOperation( Socket *pSocket, 
				CIOBuffer *pBuffer, 
				enumIO_Operation operation );
	
	const size_t m_numThreads;
	
	CCriticalSection m_listManipulationSection;
	
	typedef OnlineGameLib::TNodeList< Socket > SocketList;
	
	SocketList m_activeList;
	SocketList m_freeList;
	
	SOCKET m_listeningSocket;
	
	CIOCompletionPort m_iocp;
	
	CManualResetEvent m_shutdownEvent;
	
	CManualResetEvent m_acceptConnectionsEvent;
	
	unsigned long m_address;
	unsigned short m_port;
	
	const size_t m_maxFreeSockets;
	
	const bool m_useSequenceNumbers;
	
	const bool m_postZeroByteReads;
	
	/*
	 * No copies do not implement
	 */
	CSocketServer( const CSocketServer &rhs );
	CSocketServer &operator=( const CSocketServer &rhs );

};

/*
 * CSocketServer::UserData
 */
class CSocketServer::UserData : public COpaqueUserData
{
   /*
    * UserData is a shim class that allows Socket to inherit from two 
    * COpaqueUserData bases without ambiguity
	*/
};

/*
 * CSocketServer::ServerData
 */
class CSocketServer::ServerData : public COpaqueUserData
{
   /*
    * ServerData is a shim class that allows Socket to inherit from two 
    * COpaqueUserData bases without ambiguity
	*/
};


/*
 * CSocketServer::Socket
 */
class CSocketServer::Socket : 
		   public CNodeList::Node, 
		   public CSocketServer::UserData, 
		   protected CSocketServer::ServerData
{
public:

	friend class CSocketServer;
	friend class CSocketServer::WorkerThread;
	
	using UserData::SetUserData;
	using UserData::GetUserData;
	using UserData::SetUserPtr;
	using UserData::GetUserPtr;

	bool Read( CIOBuffer *pBuffer = 0, bool throwOnFailure = false );
	
	bool Write( const char *pData, 
		size_t dataLength, 
		bool throwOnFailure = false );
	
	bool Write( const BYTE *pData, 
		size_t dataLength,
		bool throwOnFailure = false );
	
	bool Write( CIOBuffer *pBuffer,
		bool throwOnFailure = false );

	void AddRef();
	void Release();
	
	void Shutdown( int how = SD_BOTH );
	
	void Close();
	void AbortiveClose();
	
	bool IsConnected( int how = SD_BOTH ) const;

	void SetMessageCallback(IMessageProcess* pIM)
	{ m_pIM = pIM; }

	IMessageProcess* GetMessageCallback() const
	{ return m_pIM; }

private:

	Socket( CSocketServer &server,                                 
		SOCKET socket,
		bool useSequenceNumbers );
	
	~Socket();
	
	void Attach( SOCKET socket );
	
	void *GetServerDataPtr() const
	{
		return ServerData::GetUserPtr();
	}
	
	void SetServerDataPtr( void *pData )
	{
		ServerData::SetUserPtr( pData );
	}
	
	void WriteCompleted();
	bool WritePending();
	
	void OnClientClose();
	void OnConnectionReset();
	
	bool IsValid();
	
	void InternalClose();
	
	void OnConnectionError( CSocketServer::enumConnectionErrorSource source,
		CIOBuffer *pBuffer,
		DWORD lastError);
	
	CCriticalSection m_crit;

	CCriticalSection m_critWriteQueue;
	
	CSocketServer &m_server;
	SOCKET m_socket;
	
	long m_ref;
	
	long m_outstandingWrites;
	
	/*
	   * We could store all of these 1 bit flags 
	   * in with the outstanding write count..
	   */
	
	bool m_readShutdown;       
	bool m_writeShutdown;
	bool m_closing;
	long m_clientClosed;       
	IMessageProcess* m_pIM;
	
	enum enumSequenceType
	{
		enumReadSequenceNo,
		enumWriteSequenceNo,
		enumFilteredReadSequenceNo,
		enumFilteredWriteSequenceNo,
		enumMaxSequenceNo
	};
	
	long GetSequenceNumber( enumSequenceType type );
	
	CIOBuffer *GetNextBuffer( CIOBuffer *pBuffer = 0 );
	
	struct SequenceData
	{
		explicit SequenceData( CCriticalSection &section );
		
		void Reset();
		
		long m_numbers[4];
		
		CIOBuffer::InOrderBufferList m_outOfSequenceWrites;
	};
	
	SequenceData *m_pSequenceData;
	
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
public :
	
	WorkerThread( CSocketServer &server, CIOCompletionPort &iocp );
	
	virtual ~WorkerThread();
	
	void InitiateShutdown();
	
	void WaitForShutdownToComplete();

protected:
	
	/*
	 * Thread virtual function
	 */
	virtual int Run();
	
	virtual void HandleOperation( Socket *pSocket,
				CIOBuffer *pBuffer,
				DWORD dwIoSize,
				bool weClosedSocket );

private:

	void ZeroByteRead( Socket *pSocket, CIOBuffer *pBuffer ) const;
	
	void Read( Socket *pSocket, CIOBuffer *pBuffer ) const;
	void Write( Socket *pSocket, CIOBuffer *pBuffer ) const;
	
	CSocketServer		&m_server;
	CIOCompletionPort	&m_iocp;
	
	/*
	 * No copies do not implement
	 */
	WorkerThread( const WorkerThread &rhs );
	WorkerThread &operator=( const WorkerThread &rhs );
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif // __INCLUDE_SOCKETSERVER_H__