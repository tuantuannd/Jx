#include "EventSelect.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CEventSelect::CEventSelect()
		: m_event(WSA_INVALID_EVENT)
{
	memset( &m_networkEvents, 0, sizeof( WSANETWORKEVENTS ) );
}

CEventSelect::~CEventSelect()
{
	if ( WSA_INVALID_EVENT != m_event )
	{
		::WSACloseEvent( m_event );
		m_event = WSA_INVALID_EVENT;
	}
}

void CEventSelect::AssociateEvent( SOCKET s, long lNetworkEvents )
{
	if ( WSA_INVALID_EVENT == ( m_event = ::WSACreateEvent() ) )
	{
		throw CWin32Exception( _T("CEventSelect::AssociateEvent() - WSACreateEvent"), ::WSAGetLastError() );
	}
	
	if ( SOCKET_ERROR == ::WSAEventSelect( s, m_event, lNetworkEvents ) )
	{
		throw CWin32Exception( _T("CEventSelect::AssociateEvent() - WSAEventSelect"), ::WSAGetLastError() );
	}
}

bool CEventSelect::WaitForEnumEvent( SOCKET s, DWORD dwTimeout )
{
	/*
	 * Clear the older network events and wait to update
	 */
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

bool CEventSelect::IsRead()
{
	if ( m_networkEvents.lNetworkEvents & FD_READ )
	{
		if ( m_networkEvents.iErrorCode[FD_READ_BIT] != 0 )
		{
			throw CWin32Exception( _T("CEventSelect : FD_READ failed with error "), 
				m_networkEvents.iErrorCode[FD_READ_BIT] );
		}
		
		return true;
	}

	return false;
}

bool CEventSelect::IsWrite()
{
	if ( m_networkEvents.lNetworkEvents & FD_WRITE )
	{
		if ( m_networkEvents.iErrorCode[FD_WRITE_BIT] != 0 )
		{
			throw CWin32Exception( _T("CEventSelect : FD_WRITE failed with error "), 
				m_networkEvents.iErrorCode[FD_WRITE_BIT] );
		}
		
		return true;
	}

	return false;
}

bool CEventSelect::IsConnect()
{
	if ( m_networkEvents.lNetworkEvents & FD_CONNECT )
	{
		if ( m_networkEvents.iErrorCode[FD_CONNECT_BIT] != 0 )
		{
			throw CWin32Exception( _T("CEventSelect : FD_CONNECT failed with error "), 
				m_networkEvents.iErrorCode[FD_CONNECT_BIT] );
		}
		
		return true;
	}

	return false;
}

bool CEventSelect::IsClose()
{
	if ( m_networkEvents.lNetworkEvents & FD_CLOSE )
	{
		if ( m_networkEvents.iErrorCode[FD_CLOSE_BIT] != 0 )
		{
			throw CWin32Exception( _T("CEventSelect : FD_CLOSE failed with error "), 
				m_networkEvents.iErrorCode[FD_CLOSE_BIT] );
		}
		
		return true;
	}

	return false;
}

} // End of namespace OnlineGameLib
} // End of namespace Win32