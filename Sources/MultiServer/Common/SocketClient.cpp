#include "stdafx.h"
#include "SocketClient.h"
#include "Win32Exception.h"
#include "Utils.h"
#include "Socket.h"
#include "Macro.h"

#include <vector>

#pragma comment(lib, "ws2_32.lib")

/*
 * Link options and warning
 */
#pragma message( "NOTE : --------------------OnlineGameLib [Client] : Announcement--------------------" )
#pragma message( "NOTE : The lib be used for socket communicate that it is base on" )
#pragma message( "NOTE : Windows 95 and Windows NT 3.51 and later" )
#pragma message( "NOTE :" )
#pragma message( "NOTE : liupeng xishanju.zhuhai.china 2003.1" )
#pragma message( "NOTE : -----------------------------------------------------------------------------" )

using std::vector;

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CSocketClient::CSocketClient(
		const _tstring &addressToConnectServer,
		unsigned short portToConnectServer,
		size_t maxFreeBuffers,
		size_t bufferSize /* = 1024 */
		)
	  : CIOBuffer::Allocator(bufferSize, maxFreeBuffers),
		m_address(addressToConnectServer),
		m_port(portToConnectServer),
		m_connectSocket(INVALID_SOCKET)
{

}

CSocketClient::CSocketClient(
		size_t maxFreeBuffers,
		size_t bufferSize /* = 1024 */
		)
	: CIOBuffer::Allocator(bufferSize, maxFreeBuffers),
	m_port(0),
	m_connectSocket(INVALID_SOCKET)
{

}

void CSocketClient::Connect(
			   const _tstring &addressToConnectServer,
			   unsigned short portToConnectServer
			   )
{
	m_address = addressToConnectServer;
	m_port = portToConnectServer;
}

CSocketClient::~CSocketClient()
{
	try
	{
		StopConnections();
	}
	catch(...)
	{	
		TRACE( "CSocketClient::~CSocketClient() exception!" );
	}
}

bool CSocketClient::StartConnections()
{
	CCriticalSection::Owner lock( m_criticalSection );
    
	if ( INVALID_SOCKET == m_connectSocket )
	{
		/*
		 * Call to unqualified virtual function
		 */
		//OnStartConnections();
		
		/*
		 * call to unqualified virtual function
		 */
		m_connectSocket = CreateConnectionSocket( m_address, m_port );

		if ( !WaitAndVerifyCipher() )
		{
			return false;
		}

		m_eventSelect.AssociateEvent( m_connectSocket, FD_CONNECT | FD_CLOSE | FD_READ );

		m_successConnectionsEvent.Set();

		return true;
	}

	return false;
}

void CSocketClient::StopConnections()
{
	CCriticalSection::Owner lock( m_criticalSection );

	if ( INVALID_SOCKET != m_connectSocket )
	{
		/*
		 * Change the sockH to be blocking
		 */
		/*
		DWORD dwIOCtlInfo = 0;
		if ( WSAIoctl( m_connectSocket, FIONBIO, &dwIOCtlInfo, sizeof( DWORD ),
			          NULL, 0, NULL, NULL, NULL ) )
		{
			OnError( _T("CSocketClient::StopAcceptingConnections() - WSAIoctl - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
		}
		*/
		
		/*
		 * Force an abortive close.
		 */	
		LINGER lingerStruct;
		
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;
		
		if ( SOCKET_ERROR == ::setsockopt( m_connectSocket, 
								SOL_SOCKET, 
								SO_LINGER, 
								( char * )&lingerStruct, 
								sizeof( lingerStruct ) ) )
		{
			OnError( _T("CSocketClient::setsockopt( SO_LINGER ) - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
		}

		m_successConnectionsEvent.Reset();

		m_eventSelect.DissociateEvent();
		
		if ( 0 != ::closesocket( m_connectSocket ) )
		{
			/*
			 * Call to unqualified virtual function
			 */
			OnError( _T("CSocketClient::StopAcceptingConnections() - closesocket - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
		}
		
		m_connectSocket = INVALID_SOCKET;
		
		/*
		 * Call to unqualified virtual function
		 */
		OnStopConnections();
	}
}

SOCKET CSocketClient::CreateConnectionSocket( 
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

void CSocketClient::InitiateShutdown()
{
	/*
	 * Signal that the dispatch thread should shut down all worker threads and then exit
	 */
	
	m_shutdownEvent.Set();

	StopConnections();
	
	/*
	 * Call to unqualified virtual function
	 */
	OnShutdownInitiated();
}

void CSocketClient::WaitForShutdownToComplete()
{
	/*
	 * If we havent already started a shut down, do so...
	 */
	
	InitiateShutdown();
	
	Wait();
}

int CSocketClient::Run()
{
	try
	{		
		HANDLE handlesToWaitFor[2];
		
		handlesToWaitFor[0] = m_shutdownEvent.GetEvent();
		handlesToWaitFor[1] = m_successConnectionsEvent.GetEvent();
		
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
				 * Allocate a buffer for required read
				 */
				CIOBuffer *pReadContext = Allocate();

				while ( !m_shutdownEvent.Wait( 0 ) && m_successConnectionsEvent.Wait( 0 ) )
				{
					if ( m_eventSelect.WaitForEnumEvent( m_connectSocket, 1000 ) )
					{
						/*
						 * Find some events and process it
						 */

						/*
						 * A event to close
						 */
						if ( !m_eventSelect.Close() )
						{
							/*
							 * Call to unqualified virtual function
							 */

							StopConnections();
							//OnClose();						
						}

						/*
						 * A event to read
						 */
						if ( !m_eventSelect.Read() )
						{
							OnRead( pReadContext );
						}
					
						/*
						 * A event to connect
						 */
						if ( !m_eventSelect.Connect() )
						{
							/*
							 * Call to unqualified virtual function
							 */
							
							OnStartConnections();
							//OnConnect();						
						}

						/*
						 * A event to write
						 */
						/*
						if ( !m_eventSelect.Write() )
						{
							OnWrite();
						}*/

						if ( m_eventSelect.IsError() )
						{
							m_shutdownEvent.Set();
							
							StopConnections();
							
							/*
							 * Exit this thread
							 */
							break;
						}
					}

				} // while (...

				pReadContext->Release();
			}
			else
			{
				/*
				 * Call to unqualified virtual function
				 */
				OnError( _T("CSocketClient::Run() - WaitForMultipleObjects: ") + GetLastErrorMessage( ::GetLastError() ) );
			}
			
		} // while ( ... 		
	}
	catch( const CWin32Exception &e )
	{
		/*
		 * Call to unqualified virtual function
		 */
		StopConnections();

		OnError( _T("CSocketClient::Run() - Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );

		_tstring sErrorInfo = e.GetMessage();

		//DEBUG_ONLY( Message( sErrorInfo.c_str() ) );
	}
	catch(...)
	{
		/*
		 * Call to unqualified virtual function
		 */
		StopConnections();
		
		OnError( _T("CSocketClient::Run() - Unexpected exception") );
	}
	
	/*
	 * Call to unqualified virtual function
	 */
	OnShutdownComplete();
	
	return 0;
}

void CSocketClient::ReleaseBuffers()
{
	Flush();
}

void CSocketClient::OnError( const _tstring &message )
{
	Output( message );
}

void CSocketClient::OnRead( CIOBuffer *pBuffer )
{
	/*
	 * Determine the amount of data that can be read atomically from socket s
	 *
	 * ::WSAIoctl( s, FIONREAD, ...
	 */

	DWORD dwNumBytes = 0;
	DWORD dwFlags = 0;
	
	pBuffer->SetupRead();
	
	if ( SOCKET_ERROR == ::WSARecv(
				m_connectSocket,
				pBuffer->GetWSABUF(), 
				1,
				&dwNumBytes, 
				&dwFlags,
				NULL,
				NULL ) )
	{
		DWORD lastError = ::WSAGetLastError();
		
		if ( ERROR_IO_PENDING != lastError )
		{
			_tstring sErrorInfo = GetLastErrorMessage( lastError );

			Output( _T("CSocketClient::OnRead() - WSARecv: ") + sErrorInfo );
			
			if ( lastError == WSAECONNABORTED || 
				lastError == WSAECONNRESET ||
				lastError == WSAEDISCON)
			{
				StopConnections();
			}
		}
	}
	else
	{	
		pBuffer->Use( dwNumBytes );
		
		ReadCompleted( pBuffer );
	}
}

void CSocketClient::Write( const char *pData, size_t dataLength )
{
	if ( INVALID_SOCKET != m_connectSocket &&
		dataLength > 0 &&
		pData )
	{
		CIOBuffer *pBuffer = Allocate();
		
		pBuffer->AddData( pData, dataLength );

		Write( pBuffer );
		
		pBuffer->Release();
	}
}

static struct timeval gs_CheckRW_timeout = { 5, 0 };

void CSocketClient::Write( CIOBuffer *pBuffer )
{
	size_t uDataLength = 0;

	if ( NULL == pBuffer ||
		( 0 == ( uDataLength = pBuffer->GetUsed() ) ) ||
		INVALID_SOCKET == m_connectSocket )
	{
		return;
	}

	/*
	 * Begin to send data
	 */

	int nError = 0;
	DWORD lastError= 0;
	
	DWORD dwFlags = 0;
	DWORD dwSendNumBytes = 0;
	
	pBuffer->SetupWrite();

	WSABUF &wsa = *( pBuffer->GetWSABUF() );
	
	do
	{
		if ( SOCKET_ERROR != nError )
		{
			wsa.len		-= dwSendNumBytes;
			wsa.buf		+= dwSendNumBytes;
			uDataLength	-= dwSendNumBytes;
			
			nError = ::WSASend(
						m_connectSocket,
						&wsa, 
						1, 
						&dwSendNumBytes,
						dwFlags,
						NULL, 
						NULL
						);
			
			if ( SOCKET_ERROR != nError && dwSendNumBytes >= uDataLength )
			{
				return;
			}
		}
		
		/*
		 * Continue to send buffer when 'dwSendNumBytes < uDataLength'
		 */
		if ( SOCKET_ERROR != nError )
		{
			continue;
		}

		/*
		 * Get error
		 */
		lastError = ::WSAGetLastError();
		
		if ( lastError == WSAECONNABORTED || 
			 lastError == WSAECONNRESET ||
			 lastError == WSAEDISCON )
		{
			_tstring sErrorInfo = GetLastErrorMessage( lastError );
			
			Output( _T("CSocketClient::Write() - WSASend: ") + sErrorInfo );
			
			StopConnections();

			return;
		}
		
		/*
		 * The other error what we don't process it
		 */
		if ( ( ERROR_IO_PENDING != lastError ) && 
			 ( WSAEWOULDBLOCK != lastError) )
		{
			_tstring sErrorInfo = GetLastErrorMessage( lastError );
			
			Output( _T("CSocketClient::Write() - WSASend: ") + sErrorInfo );
			
			return;
		}
		
		fd_set writefds;
		memset( &writefds, 0, sizeof( writefds ) );
		
		writefds.fd_count = 1;
		writefds.fd_array[0] = m_connectSocket;
		
		do
		{
			/*
			 * Check socket status
			 */
			nError = select( 1, NULL, &writefds, NULL, &gs_CheckRW_timeout );
			
			if ( SOCKET_ERROR == nError )
			{
				_tstring sErrorInfo = GetLastErrorMessage( lastError );
				
				Output( _T("CSocketClient::Write() - WSASend: ") + sErrorInfo );
				
				StopConnections();

				return;
			}
			
			/*
			 * It isn't time out
			 */
			if ( nError > 0 )
			{
				break;
			}
			
			/*
			 * If timeout
			 */			
			if ( m_shutdownEvent.Wait( 0 ) )
			{
				return;
			}

		} while ( true );

	} while ( true );	
}

static const DWORD g_dwTimeout = 1000;

bool CSocketClient::WaitAndVerifyCipher()
{
	fd_set	fdRead  = { 0 };

	TIMEVAL	stTime;
	TIMEVAL	*pstTime = NULL;

	if ( INFINITE != g_dwTimeout ) 
	{
		stTime.tv_sec = g_dwTimeout;
		stTime.tv_usec = 0;

		pstTime = &stTime;
	}
	
	/*
	 * Select function set read timeout
	 */

	SOCKET s = ( SOCKET )m_connectSocket;

	DWORD dwTotalLength = 0;

	while ( true )
	{
		DWORD dwBytesRead = 0L;

		/*
		 * Set Descriptor
		 */
		FD_ZERO( &fdRead );
		FD_SET( s, &fdRead );

		int res = select( 0, &fdRead, NULL, NULL, pstTime );
		if ( res > 0)
		{
			res = recv( s, ( LPSTR )&m_theSendAccountBegin + dwTotalLength, sizeof( m_theSendAccountBegin ) - dwTotalLength, 0 );
			
			if ( res > 0 )
			{
				dwBytesRead = res;
				
				dwTotalLength += dwBytesRead;			
			}
		}
		
		if ( res <= 0 )
		{
			/*
			 * Timeout and exit
			 */
			break;
		}
		
		if ( dwTotalLength == sizeof( m_theSendAccountBegin ) )
		{
			ACCOUNT_BEGIN *pAccountBegin = ( ACCOUNT_BEGIN * )( &m_theSendAccountBegin.AccountBegin );
			
			if ( pAccountBegin->ProtocolType == CIPHER_PROTOCOL_TYPE )
			{
				if ( pAccountBegin->Mode != 0 )
				{
				/*
				* Write this message and then shutdown the sending side of the socket.
					*/
					Output( "Server send cliper mode error!" );
					
					if ( 0 != ::closesocket( m_connectSocket ) )
					{
					/*
					* Call to unqualified virtual function
						*/
						OnError( _T("CSocketClient::StopAcceptingConnections() - closesocket - ") + GetLastErrorMessage( ::WSAGetLastError() ) );
					}
					
					m_connectSocket = INVALID_SOCKET;
					
					return false;
				}
				
				m_uKeyMode = pAccountBegin->Mode; 
				
				m_uServerKey = ~(pAccountBegin->ServerKey);
				m_uClientKey = ~(pAccountBegin->ClientKey);
				
				return true;
			}
		}
	}

	return false;
}

} // End of namespace OnlineGameLib
} // End of namespace Win32