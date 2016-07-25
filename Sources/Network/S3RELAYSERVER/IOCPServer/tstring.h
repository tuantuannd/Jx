/********************************************************************
	created:	2003/02/13
	file base:	tstring
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for define variable that 
				can be used ANSI routines or UNICODE routines
*********************************************************************/
#ifndef __INCLUDE_TSTRING_H__
#define __INCLUDE_TSTRING_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include <string>
#include <tchar.h>

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * Typedefs
 */
#ifdef _UNICODE
	typedef std::wstring _tstring;
#else
	typedef std::string _tstring;
#endif

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_TSTRING_H__