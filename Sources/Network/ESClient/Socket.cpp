#include "Socket.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CSocket::CSocket()
	: m_socket( INVALID_SOCKET )
{
}

CSocket::CSocket( SOCKET theSocket )
	: m_socket( theSocket )
{
   if ( INVALID_SOCKET == m_socket )
   {
      throw Exception( _T("CSocket::CSocket()"),  WSAENOTSOCK );
   }
}
      
CSocket::~CSocket()
{
	if ( INVALID_SOCKET != m_socket )
	{
		try
		{
			AbortiveClose();
		}
		catch(...)
		{
		}
	}
}

void CSocket::Attach( SOCKET theSocket )
{
	AbortiveClose();
	
	m_socket = theSocket;
}

SOCKET CSocket::Detatch()
{
	SOCKET theSocket = m_socket;
	
	m_socket = INVALID_SOCKET;
	
	return theSocket;
}

void CSocket::Close()
{
	if ( 0 != ::closesocket( m_socket ) )
	{
		throw Exception( _T("CSocket::Close()"), ::WSAGetLastError() );
	}
}

void CSocket::AbortiveClose()
{
	LINGER lingerStruct;
	
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	
	if ( SOCKET_ERROR == ::setsockopt( m_socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct) ) )
	{
		throw Exception(_T("CSocket::AbortiveClose()"), ::WSAGetLastError());
	}
	
	Close();
}

void CSocket::Shutdown( int how )
{
	if ( 0 != ::shutdown( m_socket, how ) )
	{
		throw Exception( _T("CSocket::Shutdown()"), ::WSAGetLastError() );
	}
}

void CSocket::Connect( const SOCKADDR_IN &address )
{
	if ( SOCKET_ERROR == ::connect( m_socket, reinterpret_cast<struct sockaddr *>( const_cast<SOCKADDR_IN *>( &address ) ), sizeof(SOCKADDR_IN) ) )
	{
		throw Exception( _T("CSocket::Connect()"), ::WSAGetLastError() );
	}
}

void CSocket::Connect( const struct sockaddr &address, size_t addressLength )
{
	/*
	 * Loss of precision (arg. no. 3) (unsigned int to int)
	 */
	if ( SOCKET_ERROR == ::connect( m_socket, const_cast<struct sockaddr *>( &address ), addressLength ) )
	{
		throw Exception( _T("CSocket::Connect()"), ::WSAGetLastError() );
	}
}

/*
 * CSocket::InternetAddress
 */

CSocket::InternetAddress::InternetAddress(
				const _tstring &addressToConnectServer,
				unsigned short port  )
{
	sin_family = AF_INET;
	sin_port = htons( port );
	sin_addr.s_addr = inet_addr( addressToConnectServer.c_str() ); 
	
	/*
	 * member 'sockaddr_in::sin_zero not initialised
	 */
}

/*
 * CSocket::Exception
 */
CSocket::Exception::Exception( const _tstring &where, DWORD error )
	: CWin32Exception(where, error)
{
}

} // End of namespace OnlineGameLib
} // End of namespace Win32