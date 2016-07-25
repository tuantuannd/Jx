/********************************************************************
	created:	2003/02/13
	file base:	Utils
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for common routines
*********************************************************************/
#ifndef __INCLUDE_UTILS_H__
#define __INCLUDE_UTILS_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#pragma warning(disable: 4201)   // nameless struct/union

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

#include "tstring.h"

#include <strstream>
#include <atlbase.h>       // USES_CONVERSION

#pragma warning(default: 4201)

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * Debugging defines...
 */

#ifndef DEBUG_ONLY
	#ifdef _DEBUG
		#define DEBUG_ONLY(x)   x
	#else
		#define DEBUG_ONLY(x)
	#endif
#endif

/*
 * Functions defined in this file...
 */

/*
 * Converts a type to a _tstring.
 * Convert a type to a string by streaming it. Requires that there's an ostream
 * inserter available for type T.
 *
 * parameter num could be declared as a pointer to const
 */

template <class T>
_tstring ToString( T num )
{
	_tstring strNum = _T("");

   {
      std::strstream buf;

	   buf << num << std::ends;

#ifdef _UNICODE
      std::string temp = buf.str();

      USES_CONVERSION;

      strNum = A2W(temp.c_str());
#else 
	   strNum = buf.str();
#endif
	   buf.freeze(false);
   }

   return strNum;
}

template <class T>
bool ToBool( const T &value )
{
   return ( 0 != value );
}

inline bool BOOL_to_bool(const BOOL bResult)
{
   /*
    * Convert a make believe BOOL into a real bool.
    * Removes warning C4800...
	*/

   return (TRUE == bResult);
}

_tstring HexToString( const BYTE *pBuffer, size_t iBytes );
void StringToHex( const _tstring &str, BYTE *pBuffer, size_t nBytes );

_tstring GetLastErrorMessage( DWORD last_error );

_tstring GetCurrentDirectory();

_tstring GetDateStamp();
_tstring GetTimeStamp();

void SetLogFileName( const _tstring &name );

void Output( const _tstring &message );
void OutPutInfo( const _tstring &message );
void Trace2File( const _tstring &message );

_tstring ToHex( BYTE c );

_tstring DumpData( const BYTE * const pData, size_t dataLength, size_t lineLength = 0 );

_tstring GetComputerName();
_tstring GetModuleFileName( HINSTANCE hModule = 0 );
_tstring GetUserName();

_tstring GetFileVersion();

_tstring StripLeading( const _tstring &source, const char toStrip );
_tstring StripTrailing( const _tstring &source, const char toStrip );

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_UTILS_H__