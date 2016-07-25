#include "Thread.h"

#include <process.h>		//Thread define

#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CThread::CThread()
	: m_hThread( INVALID_HANDLE_VALUE )
{
	
}

CThread::~CThread()
{
	if ( INVALID_HANDLE_VALUE != m_hThread )
	{
		::CloseHandle( m_hThread );
		m_hThread = INVALID_HANDLE_VALUE;
	}
}

/*
 * member indirectly modifies object
 */
HANDLE CThread::GetHandle() const
{
	return m_hThread;
}

void CThread::Start()
{
	if ( INVALID_HANDLE_VALUE == m_hThread )
	{
		unsigned int threadID = 0;
		
		m_hThread = (HANDLE)::_beginthreadex( 0, 0, ThreadFunction, (void*)this, 0, &threadID );
		
		if ( INVALID_HANDLE_VALUE == m_hThread )
		{
			throw CWin32Exception( _T("CThread::Start() - _beginthreadex"), GetLastError() );
		}
	}
	else
	{
		throw CException( _T("CThread::Start()"), _T("Thread already running - you can only call Start() once!") );
	}
}

void CThread::Wait() const
{
	if ( !Wait( INFINITE ) )
	{
		throw CException( _T("CThread::Wait()"), _T("Unexpected timeout on infinite wait") );
	}
}

bool CThread::Wait( DWORD timeoutMillis ) const
{	
	bool ok;
	
	DWORD result = ::WaitForSingleObject( m_hThread, timeoutMillis );
	
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
		throw CWin32Exception( _T("CThread::Wait() - WaitForSingleObject"), ::GetLastError() );
	}
	
	return ok;
}

unsigned int __stdcall CThread::ThreadFunction( void *pV )
{
	int result = 0;
	
	CThread* pThis = (CThread *)pV;
	
	if ( pThis )
	{
		try
		{
			result = pThis->Run();
		}
		catch(...)
		{
		}
	}
	
	return result;
}

void CThread::Terminate( DWORD exitCode /* = 0 */ )
{
	if ( !::TerminateThread( m_hThread, exitCode ) )
	{
		// We could throw an exception here...
	}
}

} // End of namespace OnlineGameLib
} // End of namespace Win32