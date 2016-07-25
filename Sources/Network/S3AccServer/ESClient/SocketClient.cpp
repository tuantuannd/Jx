#include "SocketClient.h"
#include "Win32Exception.h"
#include "Utils.h"

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

void CSocketClient::ConnectTo(
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
	}
}

void CSocketClient::StartConnections()
{
	if ( INVALID_SOCKET == m_connectSocket )
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnStartConnections();
		
		/*
		 * call to unqualified virtual function
		 */
		m_connectSocket = CreateConnectionSocket( m_address, m_port );

		m_eventSelect.AssociateEvent( m_connectSocket, FD_CONNECT | FD_CLOSE | FD_READ );
		
		m_successConnectionsEvent.Set();
	}
}

void CSocketClient::StopConnections()
{
	if ( INVALID_SOCKET != m_connectSocket )
	{
		m_successConnectionsEvent.Reset();
		
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

void CSocketClient::InitiateShutdown()
{
	/*
	 * Signal that the dispatch thread should shut down all worker threads and then exit
	 */
	
	StopConnections();
	
	m_shutdownEvent.Set();
	
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
						 * A event to connect
						 */
						if ( m_eventSelect.IsConnect() )
						{
							OnConnect();						
						}

						/*
						 * A event to close
						 */
						if ( m_eventSelect.IsClose() )
						{
							OnClose();						
						}

						/*
						 * A event to read
						 */
						if ( m_eventSelect.IsRead() )
						{
							OnRead( pReadContext );
						}

						/*
						 * A event to write
						 */
						if ( m_eventSelect.IsWrite() )
						{
							OnWrite();
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
	catch( const CException &e )
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnError( _T("CSocketClient::Run() - Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
	}
	catch(...)
	{
		/*
		 * Call to unqualified virtual function
		 */
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
				pBuffer,
				NULL ) )
	{
		DWORD lastError = ::WSAGetLastError();
		
		if ( ERROR_IO_PENDING != lastError )
		{
			Output( _T("CSocketClient::OnRead() - WSARecv: ") + GetLastErrorMessage( lastError ) );
			
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

void CSocketClient::OnWrite()
{

}

void CSocketClient::Write( const char *pData, size_t dataLength )
{
	if ( INVALID_SOCKET != m_connectSocket &&
		dataLength > 0 &&
		pData )
	{
		CIOBuffer *pBuffer = Allocate();
		
		/*
		 * Call to unqualified virtual function
		 */
		PreWrite( pBuffer, pData, dataLength );

		pBuffer->AddData( pData, dataLength );

		/*
		 * Begin to send data
		 */
		pBuffer->SetupWrite();
		
		DWORD dwFlags = 0;
		DWORD dwSendNumBytes = 0;
		
		if ( SOCKET_ERROR == ::WSASend(
					m_connectSocket,
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
				Output( _T("CSocketClient::Write() - WSASend: ") + GetLastErrorMessage( lastError ) );
				
				if ( lastError == WSAECONNABORTED || 
					lastError == WSAECONNRESET ||
					lastError == WSAEDISCON)
				{
					StopConnections();
				}
			}
		}
		
		if ( pBuffer->GetUsed() != pBuffer->GetWSABUF()->len )
		{
			/*
			 * Call to unqualified virtual function
			 */
			//OnError(_T("CSocketClient::WriteCompleted - Socket write where not all data was written"));
		}

		pBuffer->Release();
	}
}

} // End of namespace OnlineGameLib
} // End of namespace Win32