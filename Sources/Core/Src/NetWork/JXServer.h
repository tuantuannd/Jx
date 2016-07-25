/********************************************************************
	created:	2003/02/24
	file base:	JXServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_JXSERVER_H__
#define __INCLUDE_JXSERVER_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

#include "IOCPServer\SocketServer.h"
#include "IOCPServer\CriticalSection.h"

#include "stack"
#include "list"

using namespace std;

#if _MSC_VER > 1020			// if VC++ version is > 4.2
	using namespace std;	// std c++ libs implemented in std
#endif

typedef stack< unsigned long > STACK;
typedef list< unsigned long > LIST;

/*
 * Macro define
 */
#define MAX_CLIENT_CANBELINKED	200

#define MAX_SOCKET_KEEPINPOOL	MAX_CLIENT_CANBELINKED
#define MAX_BUFFER_KEEPINPOOL	1000
#define BUFFER_SIZE				1024 * 64
#define EXTEND_BUFFER_SIZE		BUFFER_SIZE

/*
 * If you install this function, You can get some event that 
 * it happend on all whole client. example: a connect come from client,
 * a client close the current link, etc.
 */
#define CLIENT_CONNECT_CREATE	1
#define CLIENT_CONNECT_CLOSE	2

typedef VOID ( CALLBACK *CALLBACK_SERVER_EVENT )( 
				const unsigned long ulnID,
				DWORD dwEventType /*CLIENT_CONNECT_CREATE | CLIENT_CONNECT_CLOSE*/ );

/*
 * struct tagClientContext
 */
typedef struct tagClientContext
{
	OnlineGameLib::Win32::CSocketServer::Socket		*pSocket;

	OnlineGameLib::Win32::tagSockAddrIn				SocketAddrIn;
	
	OnlineGameLib::Win32::CCriticalSection			csReadAction;
	OnlineGameLib::Win32::CCriticalSection			csWriteAction;
	
	OnlineGameLib::Win32::CIOBuffer					*pRecvBuffer;
	OnlineGameLib::Win32::CIOBuffer					*pReadBuffer;

	OnlineGameLib::Win32::CIOBuffer					*pWriteBuffer;
	
}CLIENT_CONTEXT, NEAR *PCLIENT_CONTEXT, FAR *LPCLIENT_CONTEXT;


/*
 * class CClientManager
 */
class CClientManager : private OnlineGameLib::Win32::CIOBuffer::Allocator
{
public:
	CClientManager( size_t maxFreeBuffers, size_t bufferSize = 1024);

	~CClientManager();

	unsigned long AddClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
		OnlineGameLib::Win32::CIOBuffer *pAddress );
	unsigned long DelClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket );

	void SendToClient(
		const unsigned long ulnID, 
		const char *pData,
		size_t dataLength );
	
	const char *GetFromClient(
		const unsigned long ulnID,
		size_t &dataLength );
	
	void RecvFromClient(
		const unsigned long ulnID, 
		const char *pData,
		size_t dataLength );
	
	size_t GetClientCount();
	ULONG	GetClientAddrIP( const unsigned long &ulnID );

	void ShutdownClient( const unsigned long ulnID );

	void PrepareToSend();
	void BeginToSend();

	void InstallCallBack( CALLBACK_SERVER_EVENT pfn ) { m_pfnCallBackServerEvent = pfn; };

protected:
	
	size_t GetMinimumMessageSize() const;	
	size_t GetMessageSize( const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const;
	
	OnlineGameLib::Win32::CIOBuffer *ProcessDataStream(
				const unsigned long ulnID,
				OnlineGameLib::Win32::CIOBuffer *pBuffer) const;
	
	void ProcessCommand(
				const unsigned long ulnID,
				const OnlineGameLib::Win32::CIOBuffer *pBuffer) const;
	
private:
	/*
	 * Private data
	 */
	OnlineGameLib::Win32::CCriticalSection	m_csAction;
	OnlineGameLib::Win32::CCriticalSection	m_csWriteExchange;
	
	CLIENT_CONTEXT	m_clientContext[MAX_CLIENT_CANBELINKED];

	STACK			m_freeClientStack;
	LIST			m_usedClientStack;

	CALLBACK_SERVER_EVENT	m_pfnCallBackServerEvent;
};

/*
 * Public interface for the third user
 */
#define JX_SERVER_	// NULL

/*
 * Create and destory a server to accpet some required client
 */
bool ServerStartup( 
		JX_SERVER_
		unsigned long ulnAddressToListenOn,
		unsigned short usnPortToListenOn,
		bool bStartAtOnce
		);
void ServerCleanup( JX_SERVER_ );

void InstallCallBack( JX_SERVER_ CALLBACK_SERVER_EVENT pfn );

/*
 * For control the server status
 */
#define SERVER(opt) Server_##opt();

void Server_Start();
void Server_Pause();
void Server_Begin();
void Server_End();

extern inline unsigned long AddClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
		OnlineGameLib::Win32::CIOBuffer *pAddress);
extern inline unsigned long DelClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket );

extern inline size_t GetClientCount();
extern inline ULONG	GetClientAddrIP( const unsigned long &ulnID );

/*
 * Server usage
 */
extern inline void RecvFromClient( 
		JX_SERVER_ 
		const unsigned long ulnID,
		const char *pData, 
		size_t dataLength );

extern inline const char *GetFromClient(
	const unsigned long ulnID, 
	size_t &dataLength );
 
extern inline void SendToClient( 
		JX_SERVER_ 
		const unsigned long ulnID,
		void *pData, 
		size_t dataLength );

void ShutdownClient( 
		JX_SERVER_ 
		const unsigned long ulnID );


#endif // __INCLUDE_JXSERVER_H__