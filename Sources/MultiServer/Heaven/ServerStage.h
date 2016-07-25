/********************************************************************
	created:	2003/04/15
	file base:	ServerStage
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_SERVERSTAGE_H__
#define __INCLUDE_SERVERSTAGE_H__

#pragma warning(disable : 4786)  // identifier was truncated to '255' characters 
                                 // in the debug information

#include "SocketServer.h"
#include "tstring.h"
#include "Event.h"
#include ".\Interface\IServer.h"

#include <map>
#include <stack>
#include <list>
#include <queue>

/*
 * class CIOCPServer
 */
class CIOCPServer : public IServer, public OnlineGameLib::Win32::CSocketServer
{
public:

	/*
	 * Inherit and public the IServer interface
	 */

	STDMETHOD( Startup )( );
	STDMETHOD( Cleanup )( );

	STDMETHOD( OpenService )( const unsigned long &ulnAddressToListenOn,
			const unsigned short &usnPortToListenOn );
	
	STDMETHOD( CloseService )();

	STDMETHOD( RegisterMsgFilter )( LPVOID lpParam, 
		CALLBACK_SERVER_EVENT pfnEventNotify );
	STDMETHOD( RegisterMsgFilter )( const unsigned long ulnClientID, 
		IMessageProcess* pfnMsgNotify );

	STDMETHOD( PreparePackSink )( );
	STDMETHOD( PackDataToClient )( const unsigned long &ulnClientID,
			const void * const	pData,
			const size_t		&datalength );

	STDMETHOD( SendPackToClient )( const unsigned long &ulnClientID /* -1 */ );
	
	STDMETHOD( SendData )( const unsigned long &ulnClientID,
			const void * const	pData,
			const size_t		&datalength );

	STDMETHOD_( const void *, GetPackFromClient )( 
			const unsigned long &ulnClientID,
			size_t				&datalength );

	STDMETHOD( ShutdownClient )( const unsigned long &ulnClientID );

	STDMETHOD_( size_t, GetClientCount )( );

	STDMETHOD_( const char *, GetClientInfo )( const unsigned long &ulnClientID );

	/*
     *  IUnknown COM Interface Methods
     */
    STDMETHOD ( QueryInterface ) ( REFIID riid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ( );
    STDMETHOD_( ULONG, Release ) ( );
	
public:

	CIOCPServer(
		size_t nPlayerMaxCount,
		size_t nPrecision,
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t maxFreeBuffers_Cache,
		size_t bufferSize_Cache = 8192,
		size_t bufferSize = 1024,
		size_t numThreads = 0 );

	virtual ~CIOCPServer();
	
protected:

	virtual void OnConnectionEstablished( Socket *pSocket,
         OnlineGameLib::Win32::CIOBuffer *pAddress );
	
	virtual bool OnConnectionClosing( Socket *pSocket );
	virtual void OnConnectionClosed( Socket *pSocket );
	
	virtual void ReadCompleted( Socket *pSocket,
		OnlineGameLib::Win32::CIOBuffer *pBuffer );

protected:

	size_t GetMinimumMessageSize();

	size_t GetMessageSize( const OnlineGameLib::Win32::CIOBuffer *pBuffer );

	OnlineGameLib::Win32::CIOBuffer *ProcessDataStream( Socket *pSocket,
				OnlineGameLib::Win32::CIOBuffer *pBuffer);

	void ProcessMessage( Socket *pSocket,
				OnlineGameLib::Win32::CIOBuffer *pBuffer);

	void ProcessCommand( Socket *pSocket,
				const OnlineGameLib::Win32::CIOBuffer *pBuffer);
	
private:
	
	LONG	m_lRefCount;

	OnlineGameLib::Win32::CIOBuffer::Allocator	m_theCacheAllocator;

	const size_t m_nPlayerMaxCount;
	const size_t m_nPrecision;
	
	const size_t m_nNetworkBufferMaxLen;

	LPVOID					m_lpCallBackParam;
	CALLBACK_SERVER_EVENT	m_pfnCallBackServerEvent;
	
	/*
	 * Client information
	 */
	typedef struct tagClientNode
	{
		OnlineGameLib::Win32::CSocketServer::Socket		*pSocket;
		
		OnlineGameLib::Win32::CCriticalSection			csReadAction;
		
		OnlineGameLib::Win32::CIOBuffer					*pRecvBuffer;
		OnlineGameLib::Win32::CIOBuffer					*pReadBuffer;
		
		OnlineGameLib::Win32::CCriticalSection			csWriteAction;
		
		OnlineGameLib::Win32::CIOBuffer					*pWriteBuffer;

        unsigned    uKeyMode;
		unsigned	uServerKey;
		unsigned	uClientKey;

	}CLIENT_NODE, NEAR *PCLIENT_NODE, FAR *LPCLIENT_NODE;

	typedef std::map< size_t, LPCLIENT_NODE > CLIENT_MANAGER;

	OnlineGameLib::Win32::CCriticalSection	m_csCM;

	CLIENT_MANAGER							m_theClientManager;

	/*
	 * ATTENTION :
	 *
	 * Convert stack to queue, because of a specific usage in the S3Server
	 * But stack can decrease hit the target of memory cache in system
	 *
	 * Modify on 2002.5.14
	 */
	typedef std::queue< size_t >	QUEUE;
	//typedef std::stack< size_t >	STACK;
	typedef std::list< size_t >		LIST;

	OnlineGameLib::Win32::CCriticalSection	m_csFCN;

	//STACK			m_freeClientNode;
	QUEUE			m_freeClientNode;

	/*
	 * use m_csUCN same as m_csCM
	 */
	//OnlineGameLib::Win32::CCriticalSection	m_csUCN;

	LIST			m_usedClientNode;

	/*
	 * Helper
	 */

	class CTaskQueue
	{
	public:
		
		~CTaskQueue();

		void Push( void *pItem );
		void *Pop();

	private:

		OnlineGameLib::Win32::CCriticalSection m_cs;

		typedef std::stack< void * >	TASK_STACK;

		TASK_STACK		m_theTaskStack;

	};

	bool _SendDataEx( LPCLIENT_NODE pNode,
			const void * const	pData,
			const size_t		&datalength );

	/*
	 * Helper for process user login & logout
	 */
	CTaskQueue	m_theAddClientQueue;
	CTaskQueue	m_theDelClientQueue;

	OnlineGameLib::Win32::CEvent	m_hQuitHelper;

	HANDLE							m_hHelperThread;

	static unsigned int __stdcall HelperThreadFunction( void *pParam );
	
	unsigned int _HelperThreadFunction();

	bool _HelperAddClient();
	bool _HelperDelClient();
	
};

/*
 * class CServerFactory
 */
class CServerFactory : public IServerFactory
{
public:

	STDMETHOD( SetEnvironment )
		(
			const size_t &nPlayerMaxCount,
			const size_t &nPrecision,
			const size_t &maxFreeBuffers_Cache,
			const size_t &bufferSize_Cache
		);

	STDMETHOD( CreateServerInterface )
		(
			REFIID riid, 
			void** ppv
		);

	/*
     *  IUnknown COM Interface Methods
     */
    STDMETHOD ( QueryInterface ) ( REFIID riid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ( );
    STDMETHOD_( ULONG, Release ) ( );

public:

	CServerFactory();
	virtual ~CServerFactory();

private:

	long			m_lRefCount;

	size_t m_nPlayerMaxCount;
	size_t m_nPrecision;

	size_t m_maxFreeBuffers_Cache;
	size_t m_bufferSize_Cache;
};

#endif // __INCLUDE_SERVERSTAGE_H__