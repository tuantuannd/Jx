#include "IOCompletionPort.h"
#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CIOCompletionPort::CIOCompletionPort( size_t maxConcurrency )
	: m_iocp( ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, maxConcurrency ) )
{
	if ( 0 == m_iocp )
	{
		throw CWin32Exception( _T("CIOCompletionPort::CIOCompletionPort() - CreateIoCompletionPort"), ::GetLastError() );
	}
}

CIOCompletionPort::~CIOCompletionPort() 
{ 
	::CloseHandle( m_iocp );
}

void CIOCompletionPort::AssociateDevice(
							HANDLE hDevice, 
							ULONG_PTR completionKey) 
{
	if ( m_iocp != ::CreateIoCompletionPort( hDevice, m_iocp, completionKey, 0 ) )
	{
		throw CWin32Exception(_T("CIOCompletionPort::AssociateDevice() - CreateIoCompletionPort"), ::GetLastError());
	}
}

void CIOCompletionPort::PostStatus(
				   ULONG_PTR completionKey, 
				   DWORD dwNumBytes /* = 0 */, 
				   OVERLAPPED *pOverlapped /* = 0 */) 
{
	if ( 0 == ::PostQueuedCompletionStatus( m_iocp, dwNumBytes, completionKey, pOverlapped ) )
	{
		throw CWin32Exception( _T("CIOCompletionPort::PostStatus() - PostQueuedCompletionStatus"), ::GetLastError() );
	}
}

void CIOCompletionPort::GetStatus(
			   ULONG_PTR *pCompletionKey, 
			   PDWORD pdwNumBytes,
			   OVERLAPPED **ppOverlapped)
{
	if ( 0 == ::GetQueuedCompletionStatus( m_iocp, pdwNumBytes, pCompletionKey, ppOverlapped, INFINITE ) )
	{
		throw CWin32Exception( _T("CIOCompletionPort::GetStatus() - GetQueuedCompletionStatus"), ::GetLastError() );
	}
}

bool CIOCompletionPort::GetStatus(
			   ULONG_PTR *pCompletionKey, 
			   PDWORD pdwNumBytes,
			   OVERLAPPED **ppOverlapped, 
			   DWORD dwMilliseconds)
{
	bool ok = true;
	
	if ( 0 == ::GetQueuedCompletionStatus( m_iocp, pdwNumBytes, pCompletionKey, ppOverlapped, dwMilliseconds ) )
	{
		DWORD lastError = ::GetLastError();
		
		if ( WAIT_TIMEOUT != lastError )
		{
			throw CWin32Exception( _T("CIOCompletionPort::GetStatus() - GetQueuedCompletionStatus"), lastError );
		}
		
		ok = false;
	}
	
	return ok;
}

} // End of namespace OnlineGameLib
} // End of namespace Win32