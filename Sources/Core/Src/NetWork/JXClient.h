/********************************************************************
	created:	2003/02/26
	file base:	JXClient
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_JXCLIENT_H__
#define __INCLUDE_JXCLIENT_H__

//[ Include in .\..\ESClient
#include "ESClient\SocketClient.h"
#include "ESClient\tstring.h"
//]

#define MAX_BUFFER_KEEPINPOOL	30
#define BUFFER_SIZE				1024
#define EXTEND_BUFFER_SIZE		BUFFER_SIZE * 64

#define SERVER_CONNECT_CREATE	1
#define SERVER_CONNECT_CLOSE	2

typedef VOID ( CALLBACK *CALLBACK_CLIENT_EVENT )( 
				DWORD dwEventType /*SERVER_CONNECT_CREATE | SERVER_CONNECT_CLOSE*/ );

/*
 * class CConnectManager
 */
class CConnectManager : private OnlineGameLib::Win32::CIOBuffer::Allocator
{
public:
	CConnectManager( size_t maxFreeBuffers, size_t bufferSize = 1024);

	~CConnectManager();
	
	void ConnectServer();
	void DisConnetServer();

	void RecvFromServer( const char *pData, size_t dataLength );
	const char *GetFromClient( size_t &dataLength );

	void InstallCallBack( CALLBACK_CLIENT_EVENT pfn ) { m_pfnCallbackClientEvent = pfn; };

protected:
	
	size_t GetMinimumMessageSize() const;	
	size_t GetMessageSize( const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const;
	
	OnlineGameLib::Win32::CIOBuffer *ProcessDataStream(
				OnlineGameLib::Win32::CIOBuffer *pBuffer);
	
	void ProcessCommand( const OnlineGameLib::Win32::CIOBuffer *pBuffer);

private:

	CALLBACK_CLIENT_EVENT m_pfnCallbackClientEvent;

	OnlineGameLib::Win32::CCriticalSection m_csReadAction;
	
	OnlineGameLib::Win32::CIOBuffer		*m_pRecvBuffer;
	OnlineGameLib::Win32::CIOBuffer		*m_pReadBuffer;

};

/*
 * Public interface for the third user
 */
#define JX_CLIENT_	// NULL

/*
 * Create and destory a server to accpet some required client
 */
bool ClientStartup( 
		JX_CLIENT_
		);
void ClientCleanup( JX_CLIENT_ );

bool ConnectTo( 
		JX_CLIENT_
		const OnlineGameLib::Win32::_tstring &addressToConnectServer,
		unsigned short portToConnectServer );

void InstallCallBack( JX_CLIENT_ CALLBACK_CLIENT_EVENT pfn );

void ConnectServer( JX_CLIENT_ );
void DisConnetServer( JX_CLIENT_ );

/*
 * For control the Client status
 */
#define CLIENT(opt) Client_##opt();

void Client_Start();
void Client_Pause();
void Client_Begin();
void Client_End();

/*
 * Client usage
 */
extern inline void RecvFromServer( 
		JX_CLIENT_ 
		const char *pData, 
		size_t dataLength );

extern inline const char *GetFromServer( size_t &dataLength );

extern inline void SendToServer( 
		JX_CLIENT_ 
		void *pData, 
		size_t dataLength );

void Shutdown( JX_CLIENT_ );

#endif //__INCLUDE_JXCLIENT_H__