/********************************************************************
	created:	2002/10/16
	file base:	global
	file ext:	h
	author:		liupeng
	
	purpose:	The file contains definitions, macros, and structures 
				to help me write gameengine

*********************************************************************/

#ifndef __INCLUDE_GAMEENGINE_GLOBAL_H__
#define __INCLUDE_GAMEENGINE_GLOBAL_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h> //For PutDebugString(...) API
	#undef WIN32_LEAN_AND_MEAN
#endif

/*
 * For you debug app
 */

#ifdef _DEBUG
//{
	void _trace(char *fmt, ...);

	#ifndef ASSERT
		#define ASSERT(x) { if ( !( x ) ) _asm{ int 0x03 } }
	#endif

	#ifndef VERIFY
	#define VERIFY(x) { if ( !( x ) ) _asm{ int 0x03 } }
	#endif
//}
#else
//{
	#ifndef ASSERT
		#define ASSERT(x)
	#endif

	#ifndef VERIFY
		#define VERIFY(x) x
	#endif
//}
#endif

#ifdef _DEBUG
//{
	#ifndef TRACE
		#define TRACE _trace
	#endif
//}
#else
//{
	inline void _trace(LPCTSTR fmt, ...) { }
	#ifndef TRACE
		#define TRACE  1 ? (void)0 : _trace
	#endif
//}
#endif


/*
 * Quickly and safely manage memory etc.
 */
#define INIT_PTR(x)				do{ (x) = NULL; }while(0);

#define SAFE_DELETE(x)			try{ if( (x) != NULL ) { delete (x); (x) = NULL; } } catch(...) { TRACE("SAFE_DELETE error\n"); }
#define SAFE_DELETE_ARRAY(x)	try{ if( (x) != NULL ) { delete[] (x); (x) = NULL; } } catch(...) { TRACE("SAFE_DELETE_ARRAY error\n"); }

#ifndef SAFE_FREE
	#undef SAFE_FREE
	#define SAFE_FREE(x)			try{ if( (x) != NULL ) { free(x); (x)=NULL; } } catch(...) { TRACE("SAFE_FREE error\n"); }
#endif

#ifndef SAFE_RELEASE
	#undef SAFE_RELEASE
	#define SAFE_RELEASE(x)			try{ if( (x) != NULL ) { (x)->Release(); (x) = NULL; } } catch(...) { TRACE("SAFE_RELEASE error\n"); }
#endif

#define SAFE_CLOSEHANDLE(x)		try{ if (x) { CloseHandle(x); (x) = NULL; } } catch(...) { TRACE("SAFE_CLOSEHANDLE error\n"); }
#define SAFE_FREELIB(x)			try{ if (x) { FreeLibrary(x); (x) = NULL; } } catch(...) { TRACE("SAFE_FREELIB error\n"); }

/*
 * Quick and safely convert number format
 */

#ifndef MAKEFOURCC
//{
	#define MAKEFOURCC(ch0, ch1, ch2, ch3)						\
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |		\
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
//}
#endif

#define COUNT_OF_ARRAY(array) ( sizeof( array ) / sizeof( array[0] ) )

/*
 * 'c' must be a char and not be a expression in the _private macro 
 */
#define _private_IS_NUM(c)     ((c) >= '0' && (c) <= '9')
#define _private_IS_SPACE(c)   ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t' || (c) == 'x')

#define IS_NUM(c)	_private_IS_NUM(c)
#define IS_SPACE(c)	_private_IS_SPACE(c)

#endif //#ifndef __INCLUDE_GAMEENGINE_GLOBAL_H__