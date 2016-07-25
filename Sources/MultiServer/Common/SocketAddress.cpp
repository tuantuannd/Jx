#include "stdafx.h"
#include "SocketAddress.h"
#include "Utils.h"


/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * Static helper functions
 */
static _tstring DeblockSockAddr( const sockaddr *pSockAddr );

static _tstring DeblockSockAddr( const sockaddr *pSockAddr )
{
	USES_CONVERSION;
	
	if ( pSockAddr->sa_family != AF_INET )
	{
		return _T( "We only handle AF_INET address at present" );
	}
	
	const sockaddr_in *pSockAddrIn = reinterpret_cast< const sockaddr_in * >( pSockAddr );
	
	_tstring address( A2T( ::inet_ntoa( pSockAddrIn->sin_addr ) ) );
	
	return address + _T( " : " ) + ToString( ::htons( pSockAddrIn->sin_port ) );
}

/*
 * CSocketAddress
 */
CSocketAddress::CSocketAddress( const sockaddr *pSockAddr )
					: m_address( DeblockSockAddr( pSockAddr ) )
{   
}

_tstring CSocketAddress::GetAsString() const
{
   return m_address;
}

} // End of namespace OnlineGameLib
} // End of namespace Win32
