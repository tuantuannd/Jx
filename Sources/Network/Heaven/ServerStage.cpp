#include "ServerStage.h"

#include "IOCPServer\Exception.h"
#include "IOCPServer\Utils.h"
#include "IOCPServer\ManualResetEvent.h"
#include "IOCPServer\Win32Exception.h"
#include "IOCPServer\Socket.h"

#include "Protocol\Protocol.h"

/*
 * Using directives
 */
using OnlineGameLib::Win32::CWin32Exception;
using OnlineGameLib::Win32::CSocket;
	
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::GetTimeStamp;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::DumpData;
using OnlineGameLib::Win32::Trace2File;

/*
 * Global variable
 */

//[ for trace
#ifdef  NETWORK_DEBUG
//{
	static DWORD gsc_dwPackID = 0;
//}
#endif
//]

/*
 * Get server interface
 */
STDAPI QueryServerInterface
		(
			REFIID	riid,
			void	**ppv
		)
{
	HRESULT hr = E_NOINTERFACE;

	if ( IID_IIOCPServer == riid )
	{
		CClientManager *pObject = new CClientManager( MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE );

		*ppv = reinterpret_cast< void * > ( dynamic_cast< IServer * >( pObject ) );

		if ( *ppv )
		{
			reinterpret_cast< IUnknown * >( *ppv )->AddRef();

			hr = S_OK;
		}
	}

	if ( FAILED( hr ) )
	{
		*ppv = NULL;
	}

	return ( HRESULT )( hr );
}

/*
 * CGameServerWorkerThread implement
 */
CGameServerWorkerThread::CGameServerWorkerThread(
				OnlineGameLib::Win32::CIOCompletionPort &iocp,
				CClientManager	&refClientManager)
		   : OnlineGameLib::Win32::CSocketServer::WorkerThread(iocp)
		   , m_refClientManager(refClientManager)
{
}

CGameServerWorkerThread::~CGameServerWorkerThread()
{
}

void CGameServerWorkerThread::OnBeginProcessing()
{
//	Output( _T("OnBeginProcessing") );
}

void CGameServerWorkerThread::ReadCompleted(
					OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
					OnlineGameLib::Win32::CIOBuffer *pBuffer)
{
	try
	{
		unsigned long ulnID = pSocket->GetUserData();

		const BYTE *pPackData = pBuffer->GetBuffer();
		const size_t used = pBuffer->GetUsed();

		m_refClientManager.RecvFromClient( ulnID, reinterpret_cast<const char*>( pPackData ), used );

		pBuffer->Empty();
		
		pSocket->Read( pBuffer );
	}
	catch( const CException &e )
	{
		Output( _T("ReadCompleted - Exception - ") + e.GetWhere() + _T(" - ") + e.GetMessage() );

		pSocket->Shutdown();
	}
	catch(...)
	{
		Output( _T("ReadCompleted - Unexpected exception") );

		pSocket->Shutdown();
	}
}

void CGameServerWorkerThread::OnEndProcessing()
{
//	Output( _T("OnEndProcessing") );
}

void CGameServerWorkerThread::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

CGameServer::CGameServer(
			CClientManager	&refClientManager,		   	
			unsigned long addressToListenOn,
			unsigned short portToListenOn,
			size_t maxFreeSockets,
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */,
			size_t numThreads /* = 0 */)
	   : OnlineGameLib::Win32::CSocketServer(addressToListenOn, portToListenOn, maxFreeSockets, maxFreeBuffers, bufferSize, numThreads)
	   , m_refClientManager( refClientManager )
{
}

CGameServer::CGameServer(
		CClientManager	&refClientManager,
		size_t maxFreeSockets,
		size_t maxFreeBuffers,
		size_t bufferSize /* = 1024 */,
		size_t numThreads /* = 0 */)
	   : OnlineGameLib::Win32::CSocketServer(maxFreeSockets, maxFreeBuffers, bufferSize, numThreads)
	   , m_refClientManager( refClientManager )
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
		throw CWin32Exception( _T("CSocket::CreateListeningSocket()"), ::WSAGetLastError() );
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
	Output( _T("CreateWorkerThread") );
	
	return new CGameServerWorkerThread( iocp, m_refClientManager );
}

void CGameServer::OnStartAcceptingConnections()
{
	Output( _T("OnStartAcceptingConnections") );
}

void CGameServer::OnStopAcceptingConnections()
{
	Output( _T("OnStopAcceptingConnections") );
}
      
void CGameServer::OnShutdownInitiated()
{
	Output( _T("OnShutdownInitiated") );
}
      
void CGameServer::OnShutdownComplete()
{
	Output( _T("OnShutdownComplete") );
}

void CGameServer::OnConnectionEstablished(
					OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
					OnlineGameLib::Win32::CIOBuffer * pAddress )
{
	Output( _T("OnConnectionEstablished") );
	
	/*
	 * Add Client
	 */
	m_refClientManager.AddClient( pSocket );

	pSocket->Read();
}

void CGameServer::OnConnectionClosed( 
					OnlineGameLib::Win32::CSocketServer::Socket * pSocket )
{
	/*
	 * Delete client
	 */
	m_refClientManager.DelClient( pSocket );

	Output( _T("OnConnectionClosed") );	
}

void CGameServer::OnConnectionCreated()
{
	Output( _T("OnConnectionCreated") );
}

void CGameServer::OnConnectionDestroyed()
{
	Output( _T("OnConnectionDestroyed") );
}

void CGameServer::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

void CGameServer::PreWrite( 
			OnlineGameLib::Win32::CSocketServer::Socket *pSocket, 
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

		pBuffer->AddData( reinterpret_cast<const char *>( &ph ), PACK_HEADER_LEN );
	}
}

STDMETHODIMP CClientManager::Startup()
{
	HRESULT hr = E_FAIL;

	try
	{
		m_theGameServer.Start();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("CClientManager::Startup Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("CClientManager::Startup - Unexpected exception") );
	}
	
	return hr;
}

STDMETHODIMP CClientManager::Cleanup()
{
	HRESULT hr = E_FAIL;

	try
	{		
		m_theGameServer.StopAcceptingConnections();
		
		m_theGameServer.WaitForShutdownToComplete();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("CClientManager::Cleanup Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("CClientManager::Cleanup - Unexpected exception") );
	}

	return hr;
}

STDMETHODIMP CClientManager::OpenService(
		const unsigned long		&ulnAddressToListenOn,
		const unsigned short	&usnPortToListenOn )
{
	HRESULT hr = E_FAIL;

	try
	{
		m_theGameServer.Open( ulnAddressToListenOn, usnPortToListenOn );
		
		m_theGameServer.StartAcceptingConnections();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("CClientManager::OpenService Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("CClientManager::OpenService - Unexpected exception") );
	}	

	return hr;
}

STDMETHODIMP CClientManager::CloseService()
{
	HRESULT hr = E_FAIL;

	try
	{
		m_theGameServer.StopAcceptingConnections();

		hr = S_OK;
	}
	catch( const CException &e )
	{
		Output( _T("CClientManager::CloseService Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		Output( _T("CClientManager::CloseService - Unexpected exception") );
	}	

	return hr;
}

STDMETHODIMP CClientManager::RegisterMsgFilter( CALLBACK_SERVER_EVENT pfnEventNotify )
{
	m_pfnCallBackServerEvent = pfnEventNotify;

	return S_OK;
}

STDMETHODIMP CClientManager::PreparePackSink( )
{
//	CCriticalSection::Owner lock( m_csSystemAction );

	return S_OK;
}

STDMETHODIMP CClientManager::PackDataToClient(
		const unsigned long &ulnClientID,
		const void * const	pData,
		const size_t		&datalength )
{
	HRESULT hr = E_FAIL;

	if ( ulnClientID >= 0 && ulnClientID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
						m_clientContext[ulnClientID].pSocket;

		if ( pSocket )
		{
			CCriticalSection::Owner	locker( m_clientContext[ulnClientID].csWriteAction );
			
			try
			{
				m_clientContext[ulnClientID].pWriteBuffer->AddData( static_cast<const char * const>( pData ), datalength );

				hr = S_OK;
			}
			catch( const CException &e )
			{
				Output( _T("CClientManager::PackDataToClient Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );

				pSocket->Shutdown();
			}
			catch(...)
			{
				Output( _T("CClientManager::PackDataToClient - Unexpected exception") );

				pSocket->Shutdown();
			}
		}
	}	

	return hr;
}

STDMETHODIMP CClientManager::SendPackToClient( )
{
	CCriticalSection::Owner lock( m_csSystemAction );

	HRESULT hr = E_FAIL;

	for ( std_LIST::iterator i = m_usedClientStack.begin(); i != m_usedClientStack.end(); i ++ )
	{
		int nID = *i;

		CCriticalSection::Owner	locker( m_clientContext[nID].csWriteAction );

		const BYTE * pPackData = m_clientContext[nID].pWriteBuffer->GetBuffer();
		const size_t used = m_clientContext[nID].pWriteBuffer->GetUsed();

		if ( used > 0 )
		{
		//[
#ifdef  NETWORK_DEBUG
//{
			static const DWORD sc_dwFlag = 0xABCD0000; // End of stream
			static const int sc_nFlagLen = sizeof(DWORD) * 2;
			
			DWORD dwEOSFlag[2];
			dwEOSFlag[0] = gsc_dwPackID ++;
			dwEOSFlag[1] = sc_dwFlag | ( 0xFFFF & used );
			
			m_clientContext[nID].pWriteBuffer->AddData( reinterpret_cast<const char *>( &dwEOSFlag ), sc_nFlagLen );
			
			m_clientContext[nID].pSocket->Write( reinterpret_cast<const char *>( pPackData ), used + sc_nFlagLen );
			
/*			#undef NETWORK_DEBUG_TRACE2FILE
			#ifdef NETWORK_DEBUG_TRACE2FILE
			//{
				Trace2File( _T("Package[ID:") + ToString( gsc_dwPackID - 1 ) + _T("]\n") + DumpData( pPackData, used, 50 ) );
			//}
			#else
			//{
				Output( _T("NO.[ID:") +
					ToString( gsc_dwPackID - 1 ) +
					_T("]Pack - Len:") +
					ToString( used )
					);
			//}
			#endif // NETWORK_DEBUG_TRACE_MININFO
*/
//}
#else	// NETWORK_DEBUG
//{
			m_clientContext[nID].pSocket->Write( reinterpret_cast<const char *>( pPackData ), used );
//}
#endif  // NETWORK_DEBUG
		//]

/*			Output(
				_T("ID:") + 
				ToString(nID) + 
				_T(" - Package[length:") + 
				ToString( used ) + 
				_T("]\n") + 
				DumpData( pPackData, used, 50 ) 
				);
*/
		}

		m_clientContext[nID].pWriteBuffer->Empty();

		hr = S_OK;
	}
	
	return hr;
}

STDMETHODIMP_( const void * ) CClientManager::GetPackFromClient(
			const unsigned long &ulnClientID,
			size_t				&datalength )
{
	if ( ulnClientID >= 0 && ulnClientID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
			m_clientContext[ulnClientID].pSocket;
		if ( pSocket )
		{
			CCriticalSection::Owner	locker( m_clientContext[ulnClientID].csReadAction );
			
			m_clientContext[ulnClientID].pReadBuffer->Empty();

			m_clientContext[ulnClientID].pRecvBuffer = ProcessDataStream(
										ulnClientID,
										m_clientContext[ulnClientID].pRecvBuffer );

			const BYTE *pPackData = m_clientContext[ulnClientID].pReadBuffer->GetBuffer();
			datalength = m_clientContext[ulnClientID].pReadBuffer->GetUsed();
			 
			return reinterpret_cast<const char*>( pPackData );
		}
	}

	datalength = 0;

	return NULL;
}

STDMETHODIMP CClientManager::ShutdownClient( const unsigned long &ulnClientID )
{
	CCriticalSection::Owner lock( m_csSystemAction );

	if ( ulnClientID >= 0 && ulnClientID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
						m_clientContext[ulnClientID].pSocket;

		if ( pSocket )
		{
			pSocket->Close();
			pSocket = NULL;

			m_clientContext[ulnClientID].pReadBuffer->Empty();
			m_clientContext[ulnClientID].pRecvBuffer->Empty();

			m_clientContext[ulnClientID].pWriteBuffer->Empty();
		}
	}

	return S_OK;
}

STDMETHODIMP_( size_t ) CClientManager::GetClientCount( )
{
	CCriticalSection::Owner lock( m_csSystemAction );
	
	return m_usedClientStack.size();
}

STDMETHODIMP CClientManager::QueryInterface( REFIID riid, void** ppv )
{
	/*
	 * By definition all COM objects support the IUnknown interface
	 */
	if( riid == IID_IUnknown )
	{
		AddRef();

		*ppv = dynamic_cast< IUnknown * >( this );
	}
	else if ( riid == IID_IIOCPServer )
	{
		AddRef();

		*ppv = dynamic_cast< IServer * >( this );	
	}
	else
	{
		*ppv = NULL;

		return E_NOINTERFACE;
	}

	return S_OK;
}
    
STDMETHODIMP_(ULONG) CClientManager::AddRef()
{
	return InterlockedIncrement( &m_lRefCount );
}

STDMETHODIMP_(ULONG) CClientManager::Release()
{
	if ( InterlockedDecrement( &m_lRefCount ) > 0 )
	{
		return m_lRefCount;
	}

	delete this;
	return 0L;
}

CClientManager::CClientManager( size_t maxFreeBuffers, size_t bufferSize /*1024*/ )
	: OnlineGameLib::Win32::CIOBuffer::Allocator( bufferSize, maxFreeBuffers )
	, m_lRefCount( 0L )
	, m_theGameServer( *this, MAX_SOCKET_KEEPINPOOL, MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE )
	, m_pfnCallBackServerEvent( NULL )
{
	int i=0;

	for ( i=0; i<MAX_CLIENT_CANBELINKED; i++ )
	{
		m_freeClientStack.push( i );
	}

	for ( i=0; i<MAX_CLIENT_CANBELINKED; i++ )
	{
		m_clientContext[i].pSocket = NULL;

		m_clientContext[i].pRecvBuffer = Allocate();
		m_clientContext[i].pReadBuffer = Allocate();

		m_clientContext[i].pWriteBuffer = Allocate();
	}	

}

CClientManager::~CClientManager()
{
	CCriticalSection::Owner lock( m_csSystemAction );
		
	/*
     * Repeat until stack is empty
	 */
	while ( !m_freeClientStack.empty() )
	{
		m_freeClientStack.pop();
	}

	m_usedClientStack.erase( m_usedClientStack.begin(), m_usedClientStack.end() );

	for ( int i=0; i<MAX_CLIENT_CANBELINKED; i++ )
	{
		m_clientContext[i].pReadBuffer->Release();
		m_clientContext[i].pRecvBuffer->Release();

		m_clientContext[i].pWriteBuffer->Release();
	}	

	try
	{
		Flush();
	}
	catch(...)
	{
		/*
		 * Some error was found, what are you doing now?
		 */
	}
}

unsigned long CClientManager::AddClient( 
			OnlineGameLib::Win32::CSocketServer::Socket *pSocket )
{
	CCriticalSection::Owner lock( m_csSystemAction );

	unsigned long index = -1;

	if ( !m_freeClientStack.empty() && pSocket )
	{
		/*
		 * Get ID from the free table
		 */
		index = m_freeClientStack.top();
		m_freeClientStack.pop();

		/*
		 * Set ID into the used table
		 */
		m_usedClientStack.push_back( index );

		/*
		 * Store information
		 */
		pSocket->SetUserData( index );
		m_clientContext[index].pSocket = pSocket;
		
		if ( m_pfnCallBackServerEvent )
		{
			m_pfnCallBackServerEvent( index, CLIENT_CONNECT_CREATE );
		}

		Output( GetTimeStamp() + _T("Add a client [ID:") + 
			ToString( index ) + _T( "] current total is " ) +
			ToString( m_usedClientStack.size() )
			);
	}

	return index;
}

unsigned long CClientManager::DelClient(
				OnlineGameLib::Win32::CSocketServer::Socket *pSocket )
{
	CCriticalSection::Owner lock( m_csSystemAction );

	unsigned long index = -1;

	if ( !m_usedClientStack.empty() && pSocket )
	{
		index = pSocket->GetUserData();
		m_clientContext[index].pSocket = NULL;

		m_clientContext[index].pReadBuffer->Empty();
		m_clientContext[index].pRecvBuffer->Empty();
		m_clientContext[index].pWriteBuffer->Empty();

		/*
		 * Call back ID into the free table
		 */
		m_freeClientStack.push( index );
		
		/*
		 * Erase ID from the used table
		 */
		m_usedClientStack.remove( index );

		if ( m_pfnCallBackServerEvent )
		{
			m_pfnCallBackServerEvent( index, CLIENT_CONNECT_CLOSE );
		}

		Output( GetTimeStamp() + _T("Del a client [ID:") + 
			ToString( index ) + _T( "] current total is " ) + 
			ToString( m_usedClientStack.size() ) );
	}

	return index;
}

void CClientManager::RecvFromClient(
		   const unsigned long &ulnID,
		   const char *pData,
		   size_t dataLength )
{
	if ( ulnID >= 0 && ulnID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
			m_clientContext[ulnID].pSocket;

		if ( pSocket )
		{
			CCriticalSection::Owner	locker( m_clientContext[ulnID].csReadAction );

			m_clientContext[ulnID].pRecvBuffer->AddData( pData, dataLength );			
		}
	}	
}

void CClientManager::ProcessCommand(
				   const unsigned long &ulnID,
				   const OnlineGameLib::Win32::CIOBuffer *pBuffer) const
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
		 * TODO : Process data receive from client
		 */
		
		//Output( _T("Get a package[length:") + ToString( wDataLen ) + _T("]\n") + DumpData( pData, datalength, 50 ) );

		m_clientContext[ulnID].pReadBuffer->AddData( pData, datalength );				
	}
	else
	{
		Output( "found error and close this socket!" );
		
		m_clientContext[ulnID].pSocket->Close();
	}
}

size_t CClientManager::GetMinimumMessageSize() const
{
	/*
	 * The smallest possible package we accept is pack-header
	 * once we have this many bytes we can start with try and work out
	 * what we have...
	 */
	
	return PACK_HEADER_LEN;
}

size_t CClientManager::GetMessageSize(
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

OnlineGameLib::Win32::CIOBuffer *CClientManager::ProcessDataStream(
				const unsigned long &ulnID,
				OnlineGameLib::Win32::CIOBuffer *pBuffer) const
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
				Output( "found error and close this socket!" );
	
				m_clientContext[ulnID].pSocket->Close();
				
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
			ProcessCommand( ulnID, pBuffer );
			
			pBuffer->Empty();
		}
		else if ( used > messageSize )
		{
			Output( _T("Got message plus extra data") );
			/*
			 * we have a message, plus some more data
			 * 
			 * allocate a new buffer, copy the extra data into it and try again...
			 */				
			OnlineGameLib::Win32::CIOBuffer *pMessage = 
				pBuffer->SplitBuffer( messageSize );
			
			ProcessCommand( ulnID, pMessage );
			
			pMessage->Release();			

		}
	}

	/*
	 * Reissue a read into the same buffer to collect more data
	 */
	return pBuffer;
}
