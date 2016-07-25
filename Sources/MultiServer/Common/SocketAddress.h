/********************************************************************
	created:	2003/04/16
	file base:	SocketAddress
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_SOCKETADDRESS_H__
#define __INCLUDE_SOCKETADDRESS_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>

#include "tstring.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

class CSocketAddress 
{
public:
   
	explicit CSocketAddress( const sockaddr *pSockAddr );
      
	_tstring GetAsString() const;

private:

	const _tstring m_address;

    /*
	 * No copies do not implement
	 */
	CSocketAddress( const CSocketAddress &rhs );
	CSocketAddress &operator=( const CSocketAddress &rhs );

};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif // __INCLUDE_SOCKETADDRESS_H__