#include "stdafx.h"
#include "Event.h"
#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * Static helper methods
 */
static HANDLE Create(
				LPSECURITY_ATTRIBUTES lpEventAttributes, 
				bool bManualReset, 
				bool bInitialState, 
				LPCTSTR lpName);	

static HANDLE Create(
				LPSECURITY_ATTRIBUTES lpEventAttributes, 
				bool bManualReset, 
				bool bInitialState, 
				LPCTSTR lpName)
{
	HANDLE hEvent = ::CreateEvent( lpEventAttributes, bManualReset, bInitialState, lpName );
	
	if ( NULL == hEvent )
	{
		throw CWin32Exception( _T("CEvent::Create()"), ::GetLastError() );
	}
	
	return hEvent;
}

CEvent::CEvent(
		   LPSECURITY_ATTRIBUTES lpEventAttributes, 
		   bool bManualReset, 
		   bool bInitialState)
		   : m_hEvent( Create( lpEventAttributes, bManualReset, bInitialState, NULL ) )
{
	
}

CEvent::CEvent(
		   LPSECURITY_ATTRIBUTES lpEventAttributes, 
		   bool bManualReset, 
		   bool bInitialState, 
		   const char *pEventName )
		   : m_hEvent( Create( lpEventAttributes, bManualReset, bInitialState, pEventName ) )
{
	
}

CEvent::~CEvent()
{
	::CloseHandle( m_hEvent );
}

void CEvent::Wait() const
{
	if ( !Wait( INFINITE ) )
	{
		throw CException( _T("CEvent::Wait()"), _T("Unexpected timeout on infinite wait") );
	}
}

bool CEvent::Wait( DWORD timeoutMillis ) const
{
	bool ok;
	
	DWORD result = ::WaitForSingleObject( m_hEvent, timeoutMillis );
	
	if ( result == WAIT_TIMEOUT )
	{
		ok = false;
	}
	else if ( result == WAIT_OBJECT_0 )
	{
		ok = true;
	}
	else
	{
		throw CWin32Exception( _T("CEvent::Wait() - WaitForSingleObject"), ::GetLastError() );
	}
    
	return ok;
}

void CEvent::Reset()
{
	if ( !::ResetEvent( m_hEvent ) )
	{
		throw CWin32Exception( _T("CEvent::Reset()"), ::GetLastError() );
	}
}

void CEvent::Set()
{
	if ( !::SetEvent( m_hEvent ) )
	{
		throw CWin32Exception( _T("CEvent::Set()"), ::GetLastError() );
	}
}

void CEvent::Pulse()
{
	if ( !::PulseEvent( m_hEvent ) )
	{
		throw CWin32Exception( _T("CEvent::Pulse()"), ::GetLastError() );
	}
}
	
} // End of namespace OnlineGameLib
} // End of namespace Win32