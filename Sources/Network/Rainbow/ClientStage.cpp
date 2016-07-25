#include "ClientStage.h"
#include <assert.h>

#include "ESClient\Utils.h"
#include "ESClient\Win32Exception.h"
#include "ESClient\Socket.h"
#include "ESClient\Exception.h"

#include "Protocol\Protocol.h"

/*
 * Using directives
 */
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::CWin32Exception;
using OnlineGameLib::Win32::CSocket;
using OnlineGameLib::Win32::DumpData;
using OnlineGameLib::Win32::CCriticalSection;

CGameClient::CGameClient(
			CConnectManager	&refConnectManager,
			const OnlineGameLib::Win32::_tstring &addressToConnectServer,
			unsigned short portToConnectServer,
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */
			)
		  : OnlineGameLib::Win32::CSocketClient(addressToConnectServer, portToConnectServer, maxFreeBuffers, bufferSize)
		  , m_theConnectManager(refConnectManager)
{

}

CGameClient::CGameClient(
			CConnectManager	&refConnectManager,
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */
			)
		: OnlineGameLib::Win32::CSocketClient(maxFreeBuffers, bufferSize)
		, m_theConnectManager(refConnectManager)
{
}

CGameClient::~CGameClient()
{	
	try
	{
		ReleaseBuffers();
	}
	catch(...)
	{
	}
}

SOCKET CGameClient::CreateConnectionSocket( 
					  const OnlineGameLib::Win32::_tstring &addressToConnectServer,
					  unsigned short port)
{
	SOCKET s = ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, 0 );
	
	if ( INVALID_SOCKET == s )
	{
		throw CWin32Exception( _T("CSocket::CreateListeningSocket()"), ::WSAGetLastError() );
	}
	
	CSocket connectionSocket( s );
	
	CSocket::InternetAddress localAddress( addressToConnectServer, port );
	
	connectionSocket.Connect( localAddress );
	
	return connectionSocket.Detatch();	
}

void CGameClient::OnStartConnections()
{
	/*
	 * OnStartConnections
	 */
	m_theConnectManager.ServerConnected();

	Output( _T("OnStartConnections") );
}

void CGameClient::OnStopConnections()
{
	/*
	 * OnStopConnections
	 */
	m_theConnectManager.ServerDisConnected();

	Output( _T("OnStopConnections") );
}

void CGameClient::OnShutdownInitiated()
{
	Output( _T("OnShutdownInitiated") );
}

void CGameClient::OnShutdownComplete()
{
	Output( _T("OnShutdownComplete") );
}

void CGameClient::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

void CGameClient::OnConnect()
{
	Output( _T("OnConnect") );
}

void CGameClient::OnClose()
{
	Output( _T("OnClose") );
}

void CGameClient::ReadCompleted( OnlineGameLib::Win32::CIOBuffer *pBuffer )
{
	try
	{	
		const BYTE *pPackData = pBuffer->GetBuffer();
		const size_t used = pBuffer->GetUsed();

		m_theConnectManager.RecvFromServer( reinterpret_cast<const char*>( pPackData ), used );
		
		pBuffer->Empty();
	}
	catch(const CException &e)
	{
		Output( _T("ReadCompleted - Exception - ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
		StopConnections();
	}
	catch(...)
	{
		Output( _T("ReadCompleted - Unexpected exception") );
		StopConnections();
	}
}


void CGameClient::PreWrite( 
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

/*
 * class CConnectManager
 */
STDMETHODIMP CConnectManager::Startup()
{
	HRESULT hr = E_FAIL;

	try
	{
		m_theGameClient.Start();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("CConnectManager::Startup Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("CConnectManager::Startup - Unexpected exception") );
	}
	
	return hr;	
}

STDMETHODIMP CConnectManager::Cleanup()
{
	HRESULT hr = E_FAIL;

	try
	{
		m_theGameClient.StopConnections();
		
		m_theGameClient.WaitForShutdownToComplete();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("CConnectManager::Cleanup Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("CConnectManager::Cleanup Unexpected exception") );
	}

	return hr;
}

STDMETHODIMP CConnectManager::ConnectTo(
				const char * const &pAddressToConnectServer, 
				unsigned short usPortToConnectServer )
{
	HRESULT hr = E_FAIL;

	try
	{
		m_theGameClient.ConnectTo( pAddressToConnectServer, usPortToConnectServer );
		
		m_theGameClient.StartConnections();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("Unexpected exception") );
	}
	
	return hr;
}

STDMETHODIMP CConnectManager::RegisterMsgFilter( 
				CALLBACK_CLIENT_EVENT pfnEventNotify )
{
	m_pfnCallbackClientEvent = pfnEventNotify;

	return S_OK;
}

STDMETHODIMP CConnectManager::SendPackToServer(
				const void * const pData,
				const size_t &datalength )
{
	m_theGameClient.Write( reinterpret_cast< const char* const >( pData ), datalength );

	return S_OK;
}

STDMETHODIMP_( const void * ) CConnectManager::GetPackFromServer( size_t &datalength )
{
	CCriticalSection::Owner locker( m_csReadAction );

	m_pReadBuffer->Empty();

	m_pRecvBuffer = ProcessDataStream( m_pRecvBuffer );

	const BYTE *pPackData = m_pReadBuffer->GetBuffer();
	datalength = m_pReadBuffer->GetUsed();	
	
	//[
#ifdef NETWORK_DEBUG	

	if ( datalength > 0 )
	{
		static const int sc_nFlagLen = sizeof(DWORD) * 2 + PACK_HEADER_LEN + sizeof(BYTE);

		assert( datalength > sc_nFlagLen && "Get error buffer from server!" );
		datalength -= sc_nFlagLen;
	}

#endif //NETWORK_DEBUG
	//]	

	return reinterpret_cast<const char*>( pPackData );
}

STDMETHODIMP CConnectManager::Shutdown()
{
	m_theGameClient.StopConnections();

	return S_OK;
}

STDMETHODIMP CConnectManager::QueryInterface( REFIID riid, void** ppv )
{
	/*
	 * By definition all COM objects support the IUnknown interface
	 */
	if( riid == IID_IUnknown )
	{
		AddRef();

		*ppv = dynamic_cast< IUnknown * >( this );
	}
	else if ( riid == IID_IESClient )
	{
		AddRef();

		*ppv = dynamic_cast< IClient * >( this );	
	}
	else
	{
		*ppv = NULL;

		return E_NOINTERFACE;
	}

	return S_OK;
}
    
STDMETHODIMP_(ULONG) CConnectManager::AddRef()
{
	return InterlockedIncrement( &m_lRefCount );
}

STDMETHODIMP_(ULONG) CConnectManager::Release()
{
	if ( InterlockedDecrement( &m_lRefCount ) > 0 )
	{
		return m_lRefCount;
	}

	delete this;
	return 0L;
}

CConnectManager::CConnectManager(
			size_t maxFreeBuffers, 
			size_t bufferSize /*1024*/ )
		: OnlineGameLib::Win32::CIOBuffer::Allocator(bufferSize, maxFreeBuffers)
		, m_lRefCount( 0L )
		, m_pfnCallbackClientEvent(NULL)
		, m_theGameClient( *this, MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE )
{
	m_pReadBuffer = Allocate();
	m_pRecvBuffer = Allocate();
}

CConnectManager::~CConnectManager()
{
	m_pReadBuffer->Release();
	m_pRecvBuffer->Release();

	try
	{
		Flush();
	}
	catch(...)
	{
	}
}

void CConnectManager::ServerConnected()
{
	if ( m_pfnCallbackClientEvent )
	{
		m_pfnCallbackClientEvent( SERVER_CONNECT_CREATE	);
	}
}

void CConnectManager::ServerDisConnected()
{
	if ( m_pfnCallbackClientEvent )
	{
		m_pfnCallbackClientEvent( SERVER_CONNECT_CLOSE );
	}
}

void CConnectManager::RecvFromServer( const char *pData, size_t dataLength )
{
	CCriticalSection::Owner locker( m_csReadAction );
	
	m_pRecvBuffer->AddData( pData, dataLength );
}

void CConnectManager::ProcessCommand( const OnlineGameLib::Win32::CIOBuffer *pBuffer)
{
	const BYTE *pPackData = pBuffer->GetBuffer();
	const size_t used = pBuffer->GetUsed();

	bool ok = false;

	WORD wDataLen = 0;
	PACK_HEADER ph = {0};

	if ( used > PACK_HEADER_LEN )
	{
		memcpy( &ph, pPackData, PACK_HEADER_LEN );

		wDataLen = ph.wDataLen;

		ok = true;
	}
	
	if ( ok )
	{
		const BYTE *pData = pPackData + PACK_HEADER_LEN;
		const size_t datalength = wDataLen;

		/*
		 * TODO : Process data receive from server
		 */
		
		//Output( _T("Get a package[length:") + ToString( wDataLen ) + _T("]\n") + DumpData( pData, datalength, 50 ) );

		m_pReadBuffer->AddData( pData, datalength );
		
	}
	else
	{
		Output( "found error and close this connection!" );
		
		Shutdown();
	}
}

size_t CConnectManager::GetMinimumMessageSize() const
{
	/*
	 * The smallest possible package we accept is pack-header
	 * once we have this many bytes we can start with try and work out
	 * what we have...
	 */
	
	return PACK_HEADER_LEN;
}

size_t CConnectManager::GetMessageSize( 
			const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const
{
	const BYTE *pData = pBuffer->GetBuffer();	
	const size_t used = pBuffer->GetUsed();
	
	PACK_HEADER ph = {0};

	/*
	 * First, verify the flag of a message
	 */
	if ( used > PACK_HEADER_LEN )
	{
		memcpy( &ph, pData, PACK_HEADER_LEN );

		if ( PACK_BEGIN_FLAG == ph.cPackBeginFlag && 
			 PACK_END_FLAG == ph.cPackEndFlag )
		{
			WORD wCRC = MAKE_CRC_DATE( PACK_BEGIN_FLAG, PACK_END_FLAG, ph.wDataLen );

			if ( ph.wCRCData == wCRC )
			{
				return ph.wDataLen + PACK_HEADER_LEN;
			}
		}		
	}
	
	return 0;
}

OnlineGameLib::Win32::CIOBuffer *CConnectManager::ProcessDataStream( 
						OnlineGameLib::Win32::CIOBuffer *pBuffer)
{
	const size_t used = pBuffer->GetUsed();
	
	if ( used >= GetMinimumMessageSize() )
	{
		const size_t messageSize = GetMessageSize( pBuffer );
		
		if ( messageSize == 0 )
		{
			/*
			 * havent got a complete message yet.
			
			 * we null terminate our messages in the buffer, so we need to reserve
			 * a byte of the buffer for this purpose...
			 */
			
			if ( used == ( pBuffer->GetSize() - 1 ) )
			{
				Output( _T("Too much data!") );
				
				/*
				 * Write this message and then shutdown the sending side of the socket.
				 */
				Output( "found error and close this connection!" );
	
				Shutdown();
				
				/*
				 * throw the rubbish away
				 */
				pBuffer->Empty();					
			}
		}
		else if ( used == messageSize )
		{
			Output( _T("Got a complete message and begin to process it") );
			/*
			 * we have a whole, distinct, message
			 */
			
			ProcessCommand( pBuffer );
			
			pBuffer->Empty();
		}
		else if (used > messageSize)
		{
			Output(_T("Got message plus extra data"));
			/*
			 * we have a message, plus some more data
			 * 
			 * allocate a new buffer, copy the extra data into it and try again...
			 */
			
			OnlineGameLib::Win32::CIOBuffer *pMessage = pBuffer->SplitBuffer( messageSize );
			
			ProcessCommand( pMessage );
			
			pMessage->Release();
			
		}
	}
	
	/*
	 * Reissue a read into the same buffer to collect more data
	 */
	return pBuffer;
}
