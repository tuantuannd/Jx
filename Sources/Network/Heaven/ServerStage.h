/********************************************************************
	created:	2003/03/20
	file base:	ServerStage
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_SERVERSTAGE_H__
#define __INCLUDE_SERVERSTAGE_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

/*
 * for disable some STL warning
 */
#pragma warning(disable:4786)

#include "stack"
#include "list"

using namespace std;

#if _MSC_VER > 1020			// if VC++ version is > 4.2
	using namespace std;	// std c++ libs implemented in std
#endif

#include "Interface\HeavenInterface.h"
#include "Interface\IServer.h"

#include "IOCPServer\SocketServer.h"
#include "IOCPServer\CriticalSection.h"
#include "IOCPServer\tstring.h"

/*
 * Classes defined in other files...
 */
namespace OnlineGameLib {
namespace Win32 {

	class CIOCompletionPort;
	class CIOBuffer;

} // End of namespace OnlineGameLib
} // End of namespace Win32

typedef stack< int >	std_STACK;
typedef list< int >		std_LIST;

#define MAX_CLIENT_CANBELINKED	200

#define MAX_SOCKET_KEEPINPOOL	MAX_CLIENT_CANBELINKED
#define MAX_BUFFER_KEEPINPOOL	500
#define BUFFER_SIZE				1024 * 64
#define EXTEND_BUFFER_SIZE		BUFFER_SIZE

/*
	Antetype:	QueryServerInterface	

	Return:		S_OK : success create object
				E_OUTOFMEMORY : create faild

	Purpose:	Get server interface

				CoInitialize( NULL ); 
				...
				QueryServerInterface( ... );
				...
				CoUninitialize();

	Created:	2003/03/19
*/

STDAPI QueryServerInterface
		(
			REFIID	riid,
			void	**ppv
		);

/*
 * class CGameServerWorkerThread 
 */
class CGameServerWorkerThread : 
	public OnlineGameLib::Win32::CSocketServer::WorkerThread
{
public:

	friend class CClientManager;
	
	explicit CGameServerWorkerThread( 
				OnlineGameLib::Win32::CIOCompletionPort &iocp,
				CClientManager	&refClientManager);
	
	virtual ~CGameServerWorkerThread();
	
private:
	
	virtual void ReadCompleted(
				OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
				OnlineGameLib::Win32::CIOBuffer *pBuffer);
	
	virtual void OnBeginProcessing();	
	virtual void OnEndProcessing();
	
	virtual void OnError( const OnlineGameLib::Win32::_tstring &message );

	CClientManager	&m_refClientManager;
	
	/*
	 * No copies do not implement
	 */
	CGameServerWorkerThread( const CGameServerWorkerThread &rhs );
	CGameServerWorkerThread &operator=( const CGameServerWorkerThread &rhs ); 
};

/*
 * class CGameServer
 */
class CGameServer : public OnlineGameLib::Win32::CSocketServer
{
public:

	friend class CClientManager;
	
	CGameServer(
		CClientManager	&refClientManager,
		unsigned long addressToListenOn,
		unsigned short portToListenOn,
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024,
		size_t numThreads = 0);
	
	CGameServer(
		CClientManager	&refClientManager,
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024,
		size_t numThreads = 0);
	
	virtual ~CGameServer();
	
private:
	
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

	CClientManager	&m_refClientManager;
	
	// No copies do not implement
	CGameServer( const CGameServer &rhs );
	CGameServer &operator=( const CGameServer &rhs );
};

/*
 * struct tagClientContext
 */
typedef struct tagClientContext
{
	/*
	 * Socket
	 */
	OnlineGameLib::Win32::CSocketServer::Socket		*pSocket;

	/*
	 * Read
	 */
	OnlineGameLib::Win32::CCriticalSection			csReadAction;
	
	OnlineGameLib::Win32::CIOBuffer					*pRecvBuffer;
	OnlineGameLib::Win32::CIOBuffer					*pReadBuffer;

	/*
	 * Write
	 */
	OnlineGameLib::Win32::CCriticalSection			csWriteAction;

	OnlineGameLib::Win32::CIOBuffer					*pWriteBuffer;
	
}CLIENT_CONTEXT, NEAR *PCLIENT_CONTEXT, FAR *LPCLIENT_CONTEXT;

/*
 * class CClientManager
 */
class CClientManager : public IServer, private OnlineGameLib::Win32::CIOBuffer::Allocator
{
public:

	/*
	 * Inherit and public the IServer interface
	 */

	STDMETHOD( Startup )( );
	STDMETHOD( Cleanup )( );

	STDMETHOD( OpenService )(
			const unsigned long		&ulnAddressToListenOn,
			const unsigned short	&usnPortToListenOn );
	
	STDMETHOD( CloseService )();

	STDMETHOD( RegisterMsgFilter )( CALLBACK_SERVER_EVENT pfnEventNotify );

	STDMETHOD( PreparePackSink )( );
	STDMETHOD( PackDataToClient )
		(
			const unsigned long &ulnClientID,
			const void * const	pData,
			const size_t		&datalength );

	STDMETHOD( SendPackToClient )( );

	STDMETHOD_( const void *, GetPackFromClient )
		(
			const unsigned long &ulnClientID,
			size_t				&datalength );

	STDMETHOD( ShutdownClient )( const unsigned long &ulnClientID );

	STDMETHOD_( size_t, GetClientCount )( );

	/*
     *  IUnknown COM Interface Methods
     */
    STDMETHOD ( QueryInterface ) ( REFIID riid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ( );
    STDMETHOD_( ULONG, Release ) ( );
	
public:
	CClientManager( size_t maxFreeBuffers, size_t bufferSize = 1024 );

	virtual ~CClientManager();
	
	unsigned long AddClient( OnlineGameLib::Win32::CSocketServer::Socket *pSocket );
	unsigned long DelClient( OnlineGameLib::Win32::CSocketServer::Socket *pSocket );

	void RecvFromClient(
			const unsigned long &ulnID,
			const char *pData, 
			size_t dataLength );
	
protected:
	
	size_t GetMinimumMessageSize() const;	
	size_t GetMessageSize( const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const;
	
	OnlineGameLib::Win32::CIOBuffer *ProcessDataStream(
				const unsigned long &ulnID,
				OnlineGameLib::Win32::CIOBuffer *pBuffer) const;
	
	void ProcessCommand(
				const unsigned long &ulnID,
				const OnlineGameLib::Win32::CIOBuffer *pBuffer) const;
	
private:
	
	LONG	m_lRefCount;

	OnlineGameLib::Win32::CCriticalSection	m_csSystemAction;

	CLIENT_CONTEXT	m_clientContext[MAX_CLIENT_CANBELINKED];

	std_STACK			m_freeClientStack;
	std_LIST			m_usedClientStack;

	friend class CGameServer;

	CGameServer			m_theGameServer;
	
	CALLBACK_SERVER_EVENT	m_pfnCallBackServerEvent;

};

#endif // __INCLUDE_SERVERSTAGE_H__