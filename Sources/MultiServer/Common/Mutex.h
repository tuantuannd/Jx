/********************************************************************
	created:	2003/05/30
	file base:	Mutex
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_MUTEX_H__
#define __INCLUDE_MUTEX_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

#include "tstring.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * CMutex
 */
class CMutex
{
public:

	CMutex(
		LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner,
		BOOL bNeedOnlyOne,
		LPCTSTR lpName );

	HANDLE GetMutex() const;
	const char *GetName() const;

	void Wait() const;	
	bool Wait( DWORD timeoutMillis ) const;

	virtual ~CMutex();

private:
	
	HANDLE		m_hMutex;

	_tstring	m_sName;
	
	/*
	 * No copies do not implement
	 */
	CMutex( const CMutex &rhs );
	CMutex &operator=( const CMutex &rhs );
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif // #define __INCLUDE_MUTEX_H__