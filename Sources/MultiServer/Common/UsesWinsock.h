/********************************************************************
	created:	2003/02/14
	file base:	UsesWinsock
	file ext:	h
	author:		liupeng
	
	purpose:	Create an environment for socket
*********************************************************************/
#ifndef __INCLUDE_USESWINSOCK_H__
#define __INCLUDE_USESWINSOCK_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

class CUsesWinsock
{
public:
	
	CUsesWinsock();	
	~CUsesWinsock();
	
private:
	
	WSADATA m_data;
	
	/*
	 * No copies do not implement
	 */
	CUsesWinsock( const CUsesWinsock &rhs );
	CUsesWinsock &operator=( const CUsesWinsock &rhs );
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_USESWINSOCK_H__