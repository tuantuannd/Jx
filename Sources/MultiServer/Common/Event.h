/********************************************************************
	created:	2003/02/13
	file base:	Event
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for CEvent class
*********************************************************************/
#ifndef __INCLUDE_EVENT_H__
#define __INCLUDE_EVENT_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * CEvent
 */
class CEvent 
{
public:
	
	CEvent(
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		bool manualReset,
		bool initialState);
	
	CEvent(
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		bool manualReset,
		bool initialState,
		const char *pEventName );
	
	virtual ~CEvent();
	
	HANDLE GetEvent() const { return m_hEvent; };	
	void Wait() const;	
	bool Wait( DWORD timeoutMillis ) const;
	
	void Reset();	
	void Set();	
	void Pulse();
	
private:
	
	HANDLE m_hEvent;
	
	/*
	 * No copies do not implement
	 */
	CEvent( const CEvent &rhs );
	CEvent &operator=( const CEvent &rhs );
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_EVENT_H__