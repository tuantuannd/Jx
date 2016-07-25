#include "stdafx.h"
#include "EventSelect.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CEventSelect::CEventSelect()
		: m_event( WSA_INVALID_EVENT )
		, m_nErrorCode( 0 )
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

void CEventSelect::DissociateEvent()
{
	if ( WSA_INVALID_EVENT != m_event )
	{
		::WSACloseEvent( m_event );
		m_event = WSA_INVALID_EVENT;
	}
}

} // End of namespace OnlineGameLib
} // End of namespace Win32