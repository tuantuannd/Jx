/********************************************************************
	created:	2003/02/13
	file base:	CriticalSection
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for CRITICAL_SECTION class
*********************************************************************/
#ifndef __INCLUDE_CRITICALSECTION_H__
#define __INCLUDE_CRITICALSECTION_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

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
 * CCriticalSection
 */
class CCriticalSection 
{
public:

      class Owner
      {
	  public:
            explicit Owner( CCriticalSection &crit );

            ~Owner();
      
	  private:
            CCriticalSection &m_crit;

            /*
			 * No copies do not implement
			 */
            Owner( const Owner &rhs );
            Owner &operator=( const Owner &rhs );
      };

      CCriticalSection();      
      ~CCriticalSection();

      void Enter();
      void Leave();

private:
      CRITICAL_SECTION	m_crit;

      /*
	   * No copies do not implement
	   */
      CCriticalSection( const CCriticalSection &rhs );
      CCriticalSection &operator=( const CCriticalSection &rhs );
};	

} // End of namespace Win32 	
} // End of namespace OnlineGameLib


#endif //__INCLUDE_CRITICALSECTION_H__