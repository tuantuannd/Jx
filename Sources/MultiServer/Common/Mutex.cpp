#include "stdafx.h"
#include "Mutex.h"
#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

static HANDLE Create(
				LPSECURITY_ATTRIBUTES lpMutexAttributes,
				BOOL bInitialOwner,
				BOOL bNeedOnlyOne,
				LPCTSTR lpName)
{
	HANDLE hMutex = ::CreateMutex( lpMutexAttributes, bInitialOwner, lpName );

	if ( NULL == hMutex )
	{
		throw CWin32Exception( _T("hMutex::Create()"), ::GetLastError() );
	}
	
	if ( bNeedOnlyOne && ERROR_ALREADY_EXISTS == ::GetLastError() )
	{
		throw CWin32Exception( _T("hMutex::Create()"), ::GetLastError() );
	}
	
	return hMutex;
}

CMutex::CMutex( LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner,
		BOOL bNeedOnlyOne,
		LPCTSTR lpName )
		: m_hMutex( Create( lpMutexAttributes, bInitialOwner, bNeedOnlyOne, lpName ) )
		, m_sName( lpName ? lpName : "" )
{

}

HANDLE CMutex::GetMutex() const
{
	return m_hMutex;
}

const char *CMutex::GetName() const
{
	return m_sName.c_str();
}

void CMutex::Wait() const
{
	if ( !Wait( INFINITE ) )
	{
		throw CException( _T("CMutex::Wait()"), _T("Unexpected timeout on infinite wait") );
	}
}

bool CMutex::Wait( DWORD timeoutMillis ) const
{
	bool ok;
	
	DWORD result = ::WaitForSingleObject( m_hMutex, timeoutMillis );
	
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
		throw CWin32Exception( _T("CMutex::Wait() - WaitForSingleObject"), ::GetLastError() );
	}
    
	return ok;
}

CMutex::~CMutex()
{
	::CloseHandle( m_hMutex );
}

} // End of namespace OnlineGameLib
} // End of namespace Win32