/********************************************************************
	created:	2003/02/14
	file base:	IOCompletionPort
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for CIOCompletionPort routines
*********************************************************************/
#ifndef __INCLUDE_IOCOMPLETIONPORT_H__
#define __INCLUDE_IOCOMPLETIONPORT_H__

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
 * CIOCompletionPort
 */
class CIOCompletionPort
{
public:
	
	explicit CIOCompletionPort( size_t maxConcurrency );
	
	~CIOCompletionPort();
	
	void AssociateDevice( HANDLE hDevice, ULONG_PTR completionKey );
	
	void PostStatus( 
			ULONG_PTR completionKey, 
			DWORD dwNumBytes = 0, 
			OVERLAPPED *pOverlapped = 0 );
	
	void GetStatus(
			ULONG_PTR *pCompletionKey, 
			PDWORD pdwNumBytes,
			OVERLAPPED **ppOverlapped);
	
	bool GetStatus(
			ULONG_PTR *pCompletionKey, 
			PDWORD pdwNumBytes,
			OVERLAPPED **ppOverlapped, 
			DWORD dwMilliseconds);
	
private:
	
	HANDLE m_iocp;
	
	/*
	 * No copies do not implement
	 */
	CIOCompletionPort( const CIOCompletionPort &rhs );
	CIOCompletionPort &operator=( const CIOCompletionPort &rhs );
};


} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_IOCOMPLETIONPORT_H__