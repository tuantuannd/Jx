/********************************************************************
	created:	2003/02/19
	file base:	EventSelect
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_EVENTSELECT_H__
#define __INCLUDE_EVENTSELECT_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include <winsock2.h>

#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

class CEventSelect
{
public:
	
	CEventSelect();
	~CEventSelect();

	void AssociateEvent( SOCKET s, long lNetworkEvents );

	bool WaitForEnumEvent( SOCKET s, DWORD dwTimeout );

	bool IsRead();
	bool IsWrite();
	bool IsConnect();
	bool IsClose();

private:
	
	WSAEVENT	m_event;

	WSANETWORKEVENTS m_networkEvents;

	/*
	 * No copies do not implement
	 */
	CEventSelect( const CEventSelect &rhs );
	CEventSelect &operator=( const CEventSelect &rhs );

};
	
} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_EVENTSELECT_H__