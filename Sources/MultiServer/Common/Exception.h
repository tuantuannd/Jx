/********************************************************************
	created:	2003/02/13
	file base:	Exception
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for CException class
*********************************************************************/
#ifndef __INCLUDE_EXCEPTION_H__
#define __INCLUDE_EXCEPTION_H__

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

#pragma warning(disable: 4201)

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * CException
	*/
class CException
{
public:
		
	CException( const _tstring &where, const _tstring &message );
		
	virtual ~CException() {}
		
	virtual _tstring GetWhere() const;

	virtual _tstring GetMessage() const; 

	void MessageBox( HWND hWnd = NULL ) const; 

protected:
      
	const _tstring m_where;
	const _tstring m_message;

};

} // End of namespace OnlineGameLib
} // End of namespace Win32	

#endif //__INCLUDE_EXCEPTION_H__