/********************************************************************
	created:	2003/03/24
	file base:	ClientStage
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_CLIENTSTAGE_H__
#define __INCLUDE_CLIENTSTAGE_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include "ESClient\SocketClient.h"
#include "ESClient\tstring.h"

#include "Interface\RainbowInterface.h"
#include "Interface\IClient.h"

#define MAX_BUFFER_KEEPINPOOL	30
#define BUFFER_SIZE				1024
#define EXTEND_BUFFER_SIZE		BUFFER_SIZE * 64

/*
 * class CGameClient
 */
class CGameClient : public OnlineGameLib::Win32::CSocketClient
{
public:

	friend class CConnectManager;

	CGameClient(
		CConnectManager		&refConnectManager,
		const OnlineGameLib::Win32::_tstring &addressToConnectServer,
		unsigned short portToConnectServer,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024
		);

	CGameClient(
		CConnectManager		&refConnectManager,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024
		);	
	
	virtual ~CGameClient();

private:

	virtual SOCKET CreateConnectionSocket( 
			const OnlineGameLib::Win32::_tstring &addressToConnectServer,
			unsigned short port);

	virtual void OnStartConnections();
	virtual void OnStopConnections();
	virtual void OnShutdownInitiated();
	virtual void OnShutdownComplete();
	
	virtual void OnConnect();
	virtual void OnClose();
	virtual void ReadCompleted( OnlineGameLib::Win32::CIOBuffer *pBuffer );

	virtual void OnError( const OnlineGameLib::Win32::_tstring &message );
	
	virtual void PreWrite( 
			OnlineGameLib::Win32::CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength );

	CConnectManager		&m_theConnectManager;

	// No copies do not implement
	CGameClient( const CGameClient &rhs );
	CGameClient &operator=( const CGameClient &rhs );

};

/*
 * class CConnectManager
 */
class CConnectManager : public IClient, private OnlineGameLib::Win32::CIOBuffer::Allocator
{
public:

	STDMETHOD( Startup )( );
	STDMETHOD( Cleanup )( );

	STDMETHOD( ConnectTo )( 
		const char * const &pAddressToConnectServer,
		unsigned short usPortToConnectServer );

	STDMETHOD( RegisterMsgFilter )( CALLBACK_CLIENT_EVENT pfnEventNotify );

	STDMETHOD( SendPackToServer )( 
		const void * const pData,
		const size_t &datalength );

	STDMETHOD_( const void *, GetPackFromServer )( size_t &datalength );

	STDMETHOD( Shutdown )( );
	
	/*
     *  IUnknown COM Interface Methods
     */
    STDMETHOD ( QueryInterface ) ( REFIID riid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ( );
    STDMETHOD_( ULONG, Release ) ( );

public:
	
	CConnectManager( size_t maxFreeBuffers, size_t bufferSize = 1024);

	virtual ~CConnectManager();

	void ServerConnected();
	void ServerDisConnected();
	
	void RecvFromServer( const char *pData, size_t dataLength );

protected:
	
	size_t GetMinimumMessageSize() const;	
	size_t GetMessageSize( const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const;
	
	OnlineGameLib::Win32::CIOBuffer *ProcessDataStream(
				OnlineGameLib::Win32::CIOBuffer *pBuffer);
	
	void ProcessCommand( const OnlineGameLib::Win32::CIOBuffer *pBuffer);

private:

	LONG m_lRefCount;

	CALLBACK_CLIENT_EVENT m_pfnCallbackClientEvent;

	OnlineGameLib::Win32::CCriticalSection m_csReadAction;
	
	OnlineGameLib::Win32::CIOBuffer		*m_pRecvBuffer;
	OnlineGameLib::Win32::CIOBuffer		*m_pReadBuffer;
	
	friend class CGameClient;

	CGameClient			m_theGameClient;
	
};

#endif // __INCLUDE_CLIENTSTAGE_H__