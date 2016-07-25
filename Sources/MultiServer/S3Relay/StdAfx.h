// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WINVER                // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0403        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif                        

#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE            // Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0500    // Change this to the appropriate value to target IE 5.0 or later.
#endif

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers



// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Local Header Files

// TODO: reference additional headers your program requires here

#include <assert.h>
#include <tchar.h>
#include <winsock2.h>

#include "Lock.h"



template <class T>
static inline void _SafeAddRef(T*& rp)  { if (rp) {rp->AddRef();} }
template <class T>
static inline void _SafeRelease(T*& rp) { if (rp) {rp->Release(); rp = NULL;} }


#include <string>
namespace std { typedef basic_string< TCHAR, char_traits<TCHAR>, allocator<TCHAR> > _tstring; }

struct _tstring_less
{
	bool operator() (const std::_tstring& src1, const std::_tstring& src2) const
	{
		const size_t len1 = src1.size();
		const size_t len2 = src2.size();
		if (len1 < len2)
			return true;
		if (len1 > len2)
			return false;
		return _tcscmp(src1.c_str(), src2.c_str()) < 0;
	}
};
struct _tstring_iless
{
	bool operator() (const std::_tstring& src1, const std::_tstring& src2) const
	{
		const size_t len1 = src1.size();
		const size_t len2 = src2.size();
		if (len1 < len2)
			return true;
		if (len1 > len2)
			return false;
		return _tcsicmp(src1.c_str(), src2.c_str()) < 0;
	}
};

struct _tstring_equal
{
	bool operator() (const std::_tstring& src1, const std::_tstring& src2) const
	{
		const size_t len1 = src1.size();
		const size_t len2 = src2.size();
		if (len1 != len2)
			return false;
		return _tcscmp(src1.c_str(), src2.c_str()) == 0;
	}
};
struct _tstring_iequal
{
	bool operator() (const std::_tstring& src1, const std::_tstring& src2) const
	{
		const size_t len1 = src1.size();
		const size_t len2 = src2.size();
		if (len1 != len2)
			return false;
		return _tcsicmp(src1.c_str(), src2.c_str()) == 0;
	}
};




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
