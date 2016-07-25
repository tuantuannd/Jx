#include "stdafx.h"
#include "Thread.h"
#include "Macro.h"

#include <process.h>		//Thread define

#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CThread::CThread()
	: m_hThread( NULL )
{

}
      
CThread::~CThread()
{
	SAFE_CLOSEHANDLE( m_hThread );
}

HANDLE CThread::GetHandle() const
{
	return m_hThread;
}

void CThread::Start()
{
	if ( m_hThread == NULL )
	{
		unsigned int threadID = 0;

		m_hThread = (HANDLE)::_beginthreadex(0, 
			0, 
			ThreadFunction,
			( void * )this, 
			0, 
			&threadID );

		if ( m_hThread == NULL )
		{
			throw CWin32Exception(_T("CThread::Start() - _beginthreadex"), GetLastError());
		}
	}
	else
	{
		throw CException(_T("CThread::Start()"), _T("Thread already running - you can only call Start() once!"));
	}
}

void CThread::Wait() const
{
	if ( !Wait( INFINITE ) )
	{
		throw CException(_T("CThread::Wait()"), _T("Unexpected timeout on infinite wait"));
	}
}

bool CThread::Wait(DWORD timeoutMillis) const
{
	bool ok;

	if ( !m_hThread )
	{
		return true;
	}

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
		throw CWin32Exception( _T( "CThread::Wait() - WaitForSingleObject" ), ::GetLastError() );
	}
    
	return ok;
}

unsigned int __stdcall CThread::ThreadFunction( void *pV )
{
	int result = 0;

	CThread* pThis = ( CThread * )pV;
   
	if ( pThis )
	{
		try
		{
			result = pThis->Run();
		}
		catch( ... )
		{
			TRACE( "CThread::ThreadFunction exception!" );
		}
	}

	return result;
}

void CThread::Terminate( DWORD exitCode /* = 0 */ )
{
	if ( m_hThread && !::TerminateThread( m_hThread, exitCode ) )
	{
		TRACE( "CThread::Terminate error!" );
	}

	SAFE_CLOSEHANDLE( m_hThread );
}

} // End of namespace OnlineGameLib
} // End of namespace Win32