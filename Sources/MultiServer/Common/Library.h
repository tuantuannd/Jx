/********************************************************************
	created:	2003/04/21
	file base:	Library
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_LIBRARY_H__
#define __INCLUDE_LIBRARY_H__

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

class CLibrary
{
public:
	
	CLibrary( const char * const lpFileName );
	CLibrary();

	virtual ~CLibrary();

	bool Load( const char * const lpFileName );
	bool Free();
	
	FARPROC GetProcAddress( LPCSTR lpProcName ) { return ::GetProcAddress( m_hInst, lpProcName ); };

private:

	_tstring m_sFileName;

	HMODULE	m_hInst;
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif // __INCLUDE_LIBRARY_H__