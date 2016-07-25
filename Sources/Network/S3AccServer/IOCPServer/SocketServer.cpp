#include "SocketServer.h"
#include "IOCompletionPort.h"
#include "Win32Exception.h"
#include "Utils.h"
#include "SystemInfo.h"
#include "..\protocol\protocol.h"

#include <vector>

#pragma comment(lib, "ws2_32.lib")

/*
 * Link options and warning
 */
#pragma message( "NOTE : --------------------OnlineGameLib [Server] : Announcement--------------------" )
#pragma message( "NOTE : Developing a truly scalable Winsock Server using IO Completion Ports(IOCP)" )
#pragma message( "NOTE : But this module depend on the microsoft platform" )
#pragma message( "NOTE : Requirements :" )
#pragma message( "NOTE :    * Windows NT/2000/XP: Included in Windows NT 4.0 and later." )
#pragma message( "NOTE :    * Windows 95/98/Me: Unsupported." )
#pragma message( "NOTE :" )
#pragma message( "NOTE : liupeng xishanju.zhuhai.china 2003.1" )
#pragma message( "NOTE : -----------------------------------------------------------------------------" )

using std::vector;

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * Local enums environment
 */
enum IO_Operation 
{ 
	IO_Read_Request, 
	IO_Read_Completed, 
	IO_Write_Request, 
	IO_Write_Completed,
	IO_Close
};

/*
 * Static helper methods
 */
static size_t CalculateNumberOfThreads( size_t numThreads );

static size_t CalculateNumberOfThreads( size_t numThreads )
{
	if ( 0 == numThreads )
	{
		CSystemInfo systemInfo;
		
		numThreads = systemInfo.dwNumberOfProcessors * 2;
	}
	
	return numThreads;
}

/*
 * Copy
 */
tagSockAddrIn& tagSockAddrIn::Copy( const tagSockAddrIn& sin )
{
	memcpy( &this->sockAddrIn, &sin.sockAddrIn, Size() );

	return *this;
}

/*
 * IsEqual
 */
bool tagSockAddrIn::IsEqual( const tagSockAddrIn& sin )
{
	/*
	 * Is it Equal? - ignore 'sin_zero'
	 */
	return ( memcmp( &this->sockAddrIn, &sin.sockAddrIn, Size()-sizeof( sockAddrIn.sin_zero ) ) == 0 );
}

/*
 * IsGreater
 */
bool tagSockAddrIn::IsGreater( const tagSockAddrIn& sin )
{
	/*
	 * Is it Greater? - ignore 'sin_zero'
	 */
	return ( memcmp( &this->sockAddrIn, &sin.sockAddrIn, Size()-sizeof( sockAddrIn.sin_zero ) ) > 0 );
}

/*
 * IsLower
 */
bool tagSockAddrIn::IsLower( const tagSockAddrIn& sin )
{
	/*
	 * Is it Lower? - ignore 'sin_zero'
	 */
	return ( memcmp( &this->sockAddrIn, &sin.sockAddrIn, Size()-sizeof( sockAddrIn.sin_zero ) ) < 0 );
}

/*
 * CSocketServer 
 */
CSocketServer::CSocketServer(
		   unsigned long addressToListenOn,
		   unsigned short portToListenOn,
		   size_t maxFreeSockets,
		   size_t maxFreeBuffers,
		   size_t bufferSize /* = 1024 */,
		   size_t numThreads /* = 0 */)
		: CIOBuffer::Allocator(bufferSize, maxFreeBuffers),
		  m_numThreads(CalculateNumberOfThreads( numThreads )),
		  m_listeningSocket(INVALID_SOCKET),
		  m_iocp(0),
		  m_address(addressToListenOn),
		  m_port(portToListenOn),
		  m_maxFreeSockets(maxFreeSockets)
{
}

CSocketServer::CSocketServer(
			size_t maxFreeSockets,
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */,
			size_t numThreads /* = 0 */)
		: CIOBuffer::Allocator(bufferSize, maxFreeBuffers),
		  m_numThreads(CalculateNumberOfThreads( numThreads )),
		  m_listeningSocket(INVALID_SOCKET),
		  m_iocp(0),
		  m_maxFreeSockets(maxFreeSockets)
{
}

void CSocketServer::Open( 
			unsigned long addressToListenOn,
			unsigned short portToListenOn
			)
{
	m_address = addressToListenOn;
	m_port = portToListenOn;

	Start();
}

CSocketServer::~CSocketServer()
{
	try
	{
		ReleaseSockets();
	}
	catch(...)
	{
	}
}

void CSocketServer::ReleaseSockets()
{
	CCriticalSection::Owner lock( m_listManipulationSection );
	
	Socket *pSocket = m_activeList.Head();
	
	while ( pSocket )
	{
		Socket *pNext = SocketList::Next( pSocket );
		
		pSocket->Close();
		
		pSocket = pNext;
	}
	
	while ( m_activeList.Head() )
	{
		ReleaseSocket( m_activeList.Head() );
	}
	
	while ( m_freeList.Head() )
	{
		DestroySocket( m_freeList.PopNode() );
	}
	
	if ( m_freeList.Count() + m_freeList.Count() != 0 )
	{
		/*
		 * call to unqualified virtual function
		 */
		OnError( _T("CSocketServer::ReleaseSockets() - Leaked sockets") );
	}
}

void CSocketServer::ReleaseBuffers()
{
	Flush();
}

void CSocketServer::StartAcceptingConnections()
{
	if ( INVALID_SOCKET == m_listeningSocket )
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnStartAcceptingConnections();
		
		/*
		 * call to unqualified virtual function
		 */
		m_listeningSocket = CreateListeningSocket( m_address, m_port );
		
		m_acceptConnectionsEvent.Set();
	}
}

void CSocketServer::StopAcceptingConnections()
{
	if ( INVALID_SOCKET != m_listeningSocket )
	{
		m_acceptConnectionsEvent.Reset();
		
		if ( 0 != ::closesocket( m_listeningSocket ) )
		{
			/*
			 * Call to unqualified virtual function
			 */
			OnError( _T("CSocketServer::StopAcceptingConnections() - closesocket - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
		}
		
		m_listeningSocket = INVALID_SOCKET;
		
		/*
		 * Call to unqualified virtual function
		 */
		OnStopAcceptingConnections();
	}
}

void CSocketServer::InitiateShutdown()
{
	/*
	 * Signal that the dispatch thread should shut down all worker threads and then exit
	 */
	
	StopAcceptingConnections();
	
	m_shutdownEvent.Set();
	
	/*
	 * Call to unqualified virtual function
	 */
	OnShutdownInitiated();
}

void CSocketServer::WaitForShutdownToComplete()
{
	/*
	 * If we havent already started a shut down, do so...
	 */
	
	InitiateShutdown();
	
	Wait();
}

int CSocketServer::Run()
{
	try
	{
		vector<WorkerThread *> workers;
		
		workers.reserve( m_numThreads );
		
		for ( size_t i = 0; i < m_numThreads; ++i )
		{
			/*
			 * Call to unqualified virtual function
			 */
			WorkerThread *pThread = CreateWorkerThread( m_iocp ); 
			
			workers.push_back( pThread );
			
			pThread->Start();
		}
		
		HANDLE handlesToWaitFor[2];
		
		handlesToWaitFor[0] = m_shutdownEvent.GetEvent();
		handlesToWaitFor[1] = m_acceptConnectionsEvent.GetEvent();
		
		while ( !m_shutdownEvent.Wait( 0 ) )
		{
			DWORD waitResult = ::WaitForMultipleObjects( 2, handlesToWaitFor, false, INFINITE );
			
			if ( waitResult == WAIT_OBJECT_0 )
			{
				/*
				 * Time to shutdown
				 */
				break;
			}
			else if ( waitResult == WAIT_OBJECT_0 + 1 )
			{
				/*
				 * accept connections
				 */
				
				while ( !m_shutdownEvent.Wait( 0 ) && m_acceptConnectionsEvent.Wait( 0 ) )
				{
					CIOBuffer *pAddress = Allocate();
					
					int addressSize = (int)pAddress->GetSize();
					
					SOCKET acceptedSocket = ::WSAAccept(
									m_listeningSocket, 
									reinterpret_cast<sockaddr *>(const_cast<BYTE *>( pAddress->GetBuffer() ) ), 
									&addressSize, 
									0, 
									0);
					
					pAddress->Use( addressSize );
					
					if ( acceptedSocket != INVALID_SOCKET )
					{
						Socket *pSocket = AllocateSocket( acceptedSocket );
						
						/*
						 * Call to unqualified virtual function
						 */
						OnConnectionEstablished( pSocket, pAddress );
					}
					else if ( m_acceptConnectionsEvent.Wait( 0 ) )
					{
						/*
						 * Call to unqualified virtual function
						 */
						OnError( _T("CSocketServer::Run() - WSAAccept:") + GetLastErrorMessage( ::WSAGetLastError() ) );
					}
					
					pAddress->Release();
				}
			}
			else
			{
				/*
				 * Call to unqualified virtual function
				 */
				OnError( _T("CSocketServer::Run() - WaitForMultipleObjects: ") + GetLastErrorMessage( ::GetLastError() ) );
			}
		}
		
		for ( i = 0; i < m_numThreads; ++i )
		{
			workers[i]->InitiateShutdown();
		}  
		
		for ( i = 0; i < m_numThreads; ++i )
		{
			workers[i]->WaitForShutdownToComplete();
			
			delete workers[i];
			
			workers[i] = 0;
		}  
	}
	catch( const CException &e )
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnError( _T("CSocketServer::Run() - Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnError( _T("CSocketServer::Run() - Unexpected exception") );
	}
	
	/*
	 * Call to unqualified virtual function
	 */
	OnShutdownComplete();
	
	return 0;
}

CSocketServer::Socket *CSocketServer::AllocateSocket( SOCKET theSocket )
{
	CCriticalSection::Owner lock( m_listManipulationSection );
	
	Socket *pSocket = 0;
	
	if ( !m_freeList.Empty() )
	{
		pSocket = m_freeList.PopNode();
		
		pSocket->Attach( theSocket );
		
		pSocket->AddRef();
	}
	else
	{
		pSocket = new Socket( *this, theSocket );
		
		/*
		 * Call to unqualified virtual function
		 */
		OnConnectionCreated();
	}
	
	m_activeList.PushNode( pSocket );
	
	/*
	 * suspicious cast
	 */
	m_iocp.AssociateDevice( reinterpret_cast<HANDLE>( theSocket ), (ULONG_PTR)pSocket );
	
	return pSocket;
}

void CSocketServer::ReleaseSocket( Socket *pSocket )
{
	if ( !pSocket )
	{
		throw CException( _T("CSocketServer::ReleaseSocket()"), _T("pSocket is null") );
	}
	
	CCriticalSection::Owner lock( m_listManipulationSection );
	
	pSocket->RemoveFromList();
	
	if ( m_maxFreeSockets == 0 || m_freeList.Count() < m_maxFreeSockets )
	{
		m_freeList.PushNode( pSocket );
	}
	else
	{
		DestroySocket( pSocket );
	}
}

void CSocketServer::DestroySocket( Socket *pSocket )
{
   delete pSocket;

   /*
    * Call to unqualified virtual function
	*/
   OnConnectionDestroyed();
}

void CSocketServer::PostAbortiveClose( Socket *pSocket )
{
	CIOBuffer *pBuffer = Allocate();
	
	pBuffer->SetUserData( IO_Close );
	
	pSocket->AddRef();
	
	m_iocp.PostStatus( (ULONG_PTR)pSocket, 0, pBuffer );
}


void CSocketServer::Read( Socket *pSocket, CIOBuffer *pBuffer )
{
	/*
	 * Post a read request to the iocp so that the actual socket read gets performed by
	 * one of our IO threads...
	 */
	
	if ( !pBuffer )
	{
		pBuffer = Allocate();
	}
	else
	{
		pBuffer->AddRef();
	}
	
	pBuffer->SetUserData( IO_Read_Request );
	
	pSocket->AddRef();
	
	m_iocp.PostStatus( (ULONG_PTR)pSocket, 0, pBuffer );
}

void CSocketServer::Write(
				   Socket *pSocket,
				   const char *pData,
				   size_t dataLength, 
				   bool thenShutdown)
{
	if ( !pSocket || !pData || dataLength <= 0 )
	{
		return;
	}
   /*
    * Post a write request to the iocp so that the actual socket write gets performed by
	* one of our IO threads...
	*/

   CIOBuffer *pBuffer = Allocate();
   
   /*
    * Call to unqualified virtual function
    */
#ifdef NETWORK_DEBUG
//{
   PreWrite( pSocket, pBuffer, pData, dataLength + PACK_HEADER_LEN + sizeof(BYTE) );
//}
#else
//{
   PreWrite( pSocket, pBuffer, pData, dataLength );
//}
#endif // NETWORK_DEBUG

   pBuffer->AddData( pData, dataLength );
   
#ifdef NETWORK_DEBUG
//{
	const BYTE *pPackData = pBuffer->GetBuffer();

	PACK_HEADER ph = {0};

	memcpy( (BYTE *)&ph, pPackData, PACK_HEADER_LEN );

	pBuffer->AddData( (BYTE *)&ph, PACK_HEADER_LEN );
	pBuffer->AddData( 0xAA );
//}
#endif // NETWORK_DEBUG

   pBuffer->SetUserData( IO_Write_Request );

   pSocket->AddRef();

   m_iocp.PostStatus( (ULONG_PTR)pSocket, thenShutdown, pBuffer );
}

void CSocketServer::Write(
				   Socket *pSocket,
				   CIOBuffer *pBuffer, 
				   bool thenShutdown)
{
   /*
    * Post a write request to the iocp so that the actual socket write gets performed by
	* one of our IO threads...
	*/

   pBuffer->AddRef();

   pBuffer->SetUserData( IO_Write_Request );

   pSocket->AddRef();

   m_iocp.PostStatus( (ULONG_PTR)pSocket, thenShutdown, pBuffer );
}

void CSocketServer::OnError( const _tstring &message )
{
	Output( message );
}
  
/*
 * CSocketServer::Socket
 */
CSocketServer::Socket::Socket( CSocketServer &server, SOCKET theSocket )
				   :  m_server(server),
					  m_socket(theSocket),
					  m_ref(1)
{
	if ( INVALID_SOCKET == m_socket )
	{
		throw CException( _T("CSocketServer::Socket::Socket()"), _T("Invalid socket") );
	}
}

CSocketServer::Socket::~Socket()
{
}

void CSocketServer::Socket::Attach( SOCKET theSocket )
{
	if ( INVALID_SOCKET != m_socket )
	{
		throw CException( _T("CSocketServer::Socket::Attach()"), _T("Socket already attached") );
	}
	
	m_socket = theSocket;
	
	SetUserData( 0 );
}

void CSocketServer::Socket::AddRef()
{
	::InterlockedIncrement( &m_ref );
}

void CSocketServer::Socket::Release()
{
	if ( 0 == ::InterlockedDecrement( &m_ref ) )
	{
		m_server.ReleaseSocket( this );
	}
}

void CSocketServer::Socket::Shutdown( int how /* = SD_BOTH */ )
{
	Output( _T("CSocketServer::Socket::Shutdown() ") + ToString( how ) );
	
	if ( INVALID_SOCKET != m_socket )
	{
		if ( 0 != ::shutdown( m_socket, how ) )
		{
			m_server.OnError( _T("CSocketServer::Server::Shutdown() - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
		}
		
		Output( _T("shutdown initiated") );
	}
}

void CSocketServer::Socket::Close()
{
	CCriticalSection::Owner lock( m_server.m_listManipulationSection );
	
	if ( INVALID_SOCKET != m_socket )
	{
		if ( 0 != ::closesocket( m_socket ) )
		{
			m_server.OnError(_T("CSocketServer::Socket::Close() - closesocket - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
		}
		
		m_socket = INVALID_SOCKET;
		
		m_server.OnConnectionClosed( this );
		
		Release();
	}
}

void CSocketServer::Socket::AbortiveClose()
{
	m_server.PostAbortiveClose( this );
}

void CSocketServer::Socket::Read( CIOBuffer *pBuffer /* = 0 */ )
{
	m_server.Read( this, pBuffer );
}

void CSocketServer::Socket::Write(
			   const char *pData, 
			   size_t dataLength,
			   bool thenShutdown /* = false */)
{
	m_server.Write( this, pData, dataLength, thenShutdown );
}

void CSocketServer::Socket::Write(
			   CIOBuffer *pBuffer,
			   bool thenShutdown /* = false */)
{
	m_server.Write( this, pBuffer, thenShutdown );
}

/*
 * CSocketServer::WorkerThread
 */
CSocketServer::WorkerThread::WorkerThread( CIOCompletionPort &iocp )
	: m_iocp(iocp)
{
   /*
    * All work done in initialiser list
	*/
}

int CSocketServer::WorkerThread::Run()
{
	try
	{
		while ( true )
		{
			/*
			 * Continually loop to service io completion packets
			 */
			
			bool closeSocket = false;
			
			DWORD dwIoSize = 0;
			Socket *pSocket = 0;
			CIOBuffer *pBuffer = 0;
			
			try
			{
				m_iocp.GetStatus( (PDWORD_PTR)&pSocket, &dwIoSize, (OVERLAPPED **)&pBuffer );
			}
			catch (const CWin32Exception &e)
			{
				if ( e.GetError() != ERROR_NETNAME_DELETED &&
					e.GetError() != WSA_OPERATION_ABORTED )
				{
					throw;
				}
				
				Output( _T("IOCP error [client connection dropped] - ") +
					GetLastErrorMessage( ::WSAGetLastError() ) );
				
				closeSocket = true;
			}
			
			if ( !pSocket )
			{
				/*
				 * A completion key of 0 is posted to the iocp to request us to shut down...
				 */
				
				break;
			}
			
			/*
			 * Call to unqualified virtual function
			 */
			OnBeginProcessing();
			
			if ( pBuffer )
			{
				const IO_Operation operation = static_cast<IO_Operation>( pBuffer->GetUserData() );
				
				switch ( operation )
				{
				case IO_Read_Request:
					
					Read( pSocket, pBuffer );
					
					break;
					
				case IO_Read_Completed :
					
					if ( 0 != dwIoSize )
					{
						pBuffer->Use( dwIoSize );
						
						//DEBUG_ONLY( Output(_T("RX: ") + ToString(pBuffer) + _T("\n") + DumpData(reinterpret_cast<const BYTE*>( pBuffer->GetWSABUF()->buf), dwIoSize, 40) ) );
						
						/*
						 * Call to unqualified virtual function
						 */
						ReadCompleted( pSocket, pBuffer );
					}
					else
					{
						/*
						 * client connection dropped...
						 */
						
						Output( _T("ReadCompleted - 0 bytes - client connection dropped") );
						
						closeSocket = true;
					}
					
					pSocket->Release();
					pBuffer->Release();
					
					break;
					
				case IO_Write_Request :
					
					Write( pSocket, pBuffer );
					
					if ( dwIoSize != 0 )
					{
						/*
						 * final write, now shutdown send side of connection
						 */
						pSocket->Shutdown( SD_SEND );
					}
					
					break;
					
				case IO_Write_Completed :
					
					pBuffer->Use( dwIoSize );
					
					//DEBUG_ONLY( Output(_T("TX: ") + ToString(pBuffer) + _T("\n") + DumpData(reinterpret_cast<const BYTE*>( pBuffer->GetWSABUF()->buf), dwIoSize, 40) ) );
					
					/*
					 * Call to unqualified virtual function
					 */
					WriteCompleted( pSocket, pBuffer );
					
					pSocket->Release();
					pBuffer->Release();
					
					break;
					
				case IO_Close :
					
					AbortiveClose( pSocket );
					
					pSocket->Release();
					pBuffer->Release();
					
					break;
					
				default :
					/*
					 * all to unqualified virtual function
					 */
					OnError( _T("CSocketServer::WorkerThread::Run() - Unexpected operation") );
					break;
				} 
			}
			else
			{
				/*
				 * Call to unqualified virtual function
				 */
				OnError( _T("CSocketServer::WorkerThread::Run() - Unexpected - pBuffer is 0") );
			}
			
			if ( closeSocket )
			{
				pSocket->Close();
			}
			
			/*
			 * Call to unqualified virtual function
			 */
			OnEndProcessing();
      } 
   }
   catch(const CException &e)
   {
	   /*
	    * Call to unqualified virtual function
		*/
	   OnError( _T("CSocketServer::WorkerThread::Run() - Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
   }
   catch(...)
   {
	   /*
	    * Call to unqualified virtual function
		*/
	   OnError( _T("CSocketServer::WorkerThread::Run() - Unexpected exception") );
   }
   
   return 0;
}

void CSocketServer::WorkerThread::InitiateShutdown()
{
	m_iocp.PostStatus( 0 );
}

void CSocketServer::WorkerThread::WaitForShutdownToComplete()
{
	/*
     * If we havent already started a shut down, do so...
	 */
	
	InitiateShutdown();
	
	Wait();
}

void CSocketServer::WorkerThread::Read( Socket *pSocket, CIOBuffer *pBuffer ) const
{
	pBuffer->SetUserData( IO_Read_Completed );
	
	pBuffer->SetupRead();
	
	DWORD dwNumBytes = 0;
	DWORD dwFlags = 0;
	
	if ( SOCKET_ERROR == ::WSARecv(
				pSocket->m_socket, 
				pBuffer->GetWSABUF(), 
				1, 
				&dwNumBytes,
				&dwFlags,
				pBuffer, 
				NULL))
	{
		DWORD lastError = ::WSAGetLastError();
		
		if ( ERROR_IO_PENDING != lastError )
		{
			Output( _T("CSocketServer::Read() - WSARecv: ") + GetLastErrorMessage( lastError ) );
			
			if ( lastError == WSAECONNABORTED || 
				 lastError == WSAECONNRESET ||
				 lastError == WSAEDISCON)
			{
				pSocket->Close();
			}
			
			pSocket->Release();
			pBuffer->Release();
		}
	}
}

void CSocketServer::WorkerThread::Write( Socket *pSocket, CIOBuffer *pBuffer ) const
{
	pBuffer->SetUserData( IO_Write_Completed );
	
	pBuffer->SetupWrite();
	
	DWORD dwFlags = 0;
	DWORD dwSendNumBytes = 0;
	
	if ( SOCKET_ERROR == ::WSASend(
					pSocket->m_socket,
					pBuffer->GetWSABUF(), 
					1, 
					&dwSendNumBytes,
					dwFlags,
					pBuffer, 
					NULL) )
	{
		DWORD lastError = ::WSAGetLastError();
		
		if ( ERROR_IO_PENDING != lastError )
		{
			Output( _T("CSocketServer::Write() - WSASend: ") + GetLastErrorMessage( lastError ) );
			
			if ( lastError == WSAECONNABORTED || 
				 lastError == WSAECONNRESET ||
				 lastError == WSAEDISCON)
			{
				pSocket->Close();
			}
			
			pSocket->Release();
			pBuffer->Release();
		}
	}
}

void CSocketServer::WorkerThread::WriteCompleted( Socket * /*pSocket*/, CIOBuffer *pBuffer )
{
	if ( pBuffer->GetUsed() != pBuffer->GetWSABUF()->len )
	{
		/*
	     * Call to unqualified virtual function
		 */
		_ASSERT(_T("CSocketServer::WorkerThread::WriteCompleted - Socket write where not all data was written"));
	}

   /*
    * Pointer pBuffer could be declared const (but not in derived classes...)
	*/
}

void CSocketServer::WorkerThread::AbortiveClose( Socket *pSocket )
{
   /*
    * Force an abortive close.
	*/

	LINGER lingerStruct;
	
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	
	if ( SOCKET_ERROR == ::setsockopt( pSocket->m_socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct) ) )
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnError( _T("CSocketServer::Socket::AbortiveClose() - setsockopt(SO_LINGER) - ")  + GetLastErrorMessage( ::WSAGetLastError() ) );
	}
	
	pSocket->Close();
}

void CSocketServer::WorkerThread::OnError( const _tstring &message )
{
	Output( message );
}

} // End of namespace OnlineGameLib
} // End of namespace Win32