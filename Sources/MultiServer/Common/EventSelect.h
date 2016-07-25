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

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
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
	void DissociateEvent();

	bool WaitForEnumEvent( SOCKET s, DWORD dwTimeout );

	bool IsError() { return ( bool )( ( m_nErrorCode >> 1 ) & 0x1 ); };

	/*
	 * return value
	 *
	 * 0 : success
	 * 1 : fail
	 * 3 : exception
	 */
	enum enumExitValue
	{
		enumSuccess		= 0x0,
		enumFail		= 0x1,
		enumException	= 0x3
	};

	int Read();
	int Write();
	int Connect();
	int Close();

private:
	
	WSAEVENT	m_event;

	WSANETWORKEVENTS m_networkEvents;

	int			m_nErrorCode;

	/*
	 * No copies do not implement
	 */
	CEventSelect( const CEventSelect &rhs );
	CEventSelect &operator=( const CEventSelect &rhs );

};

inline bool CEventSelect::WaitForEnumEvent( SOCKET s, DWORD dwTimeout )
{
	/*
	 * Clear the older network events and wait to update
	 */
	m_nErrorCode = 0;

	memset( &m_networkEvents, 0, sizeof( WSANETWORKEVENTS ) );

	DWORD dwRet = ::WSAWaitForMultipleEvents(
						1,
						&m_event,
						FALSE,
						dwTimeout,
						FALSE );
	
	if ( WSA_WAIT_TIMEOUT == dwRet || WSA_WAIT_FAILED == dwRet )
	{
		return false;
	}

	::WSAEnumNetworkEvents(
					s,
					m_event,
					&m_networkEvents );

	return true;
}

inline int CEventSelect::Read()
{
	if ( m_networkEvents.lNetworkEvents & FD_READ )
	{
		if ( m_networkEvents.iErrorCode[FD_READ_BIT] != 0 )
		{
			/*
			 * throw CWin32Exception( _T("CEventSelect : FD_READ failed with error "), 
			 *	m_networkEvents.iErrorCode[FD_READ_BIT] );
			 */
			m_nErrorCode |= enumException;

			return enumException;
		}
		
		return enumSuccess;
	}

	return enumFail;
}

inline int CEventSelect::Write()
{
	if ( m_networkEvents.lNetworkEvents & FD_WRITE )
	{
		if ( m_networkEvents.iErrorCode[FD_WRITE_BIT] != 0 )
		{
			/*
			 * throw CWin32Exception( _T("CEventSelect : FD_WRITE failed with error "), 
			 *	m_networkEvents.iErrorCode[FD_WRITE_BIT] );
			 */

			m_nErrorCode |= enumException;

			return enumException;
		}
		
		return enumSuccess;
	}

	return enumFail;
}

inline int CEventSelect::Connect()
{
	if ( m_networkEvents.lNetworkEvents & FD_CONNECT )
	{
		if ( m_networkEvents.iErrorCode[FD_CONNECT_BIT] != 0 )
		{
			/*
			 * throw CWin32Exception( _T("CEventSelect : FD_CONNECT failed with error "), 
			 *	m_networkEvents.iErrorCode[FD_CONNECT_BIT] );
			 */
			m_nErrorCode |= enumException;

			return enumException;
		}
		
		return enumSuccess;
	}

	return enumFail;
}

inline int CEventSelect::Close()
{
	if ( m_networkEvents.lNetworkEvents & FD_CLOSE )
	{
		if ( m_networkEvents.iErrorCode[FD_CLOSE_BIT] != 0 )
		{
			/*
			 * throw CWin32Exception( _T("CEventSelect : FD_CLOSE failed with error "), 
			 *	m_networkEvents.iErrorCode[FD_CLOSE_BIT] );
			 */
			m_nErrorCode |= enumException;

			return enumException;
		}
		
		return enumSuccess;
	}

	return enumFail;
}
	
} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_EVENTSELECT_H__