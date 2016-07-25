/********************************************************************
	created:	2003/02/14
	file base:	SystemInfo
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for CSystemInfo class that 
				can get system information
*********************************************************************/
#ifndef __INCLUDE_SYSTEMINFO_H__
#define __INCLUDE_SYSTEMINFO_H__

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
 * CSystemInfo
 */
class CSystemInfo : public SYSTEM_INFO 
{
public:
	
	CSystemInfo()
	{ 
		::GetSystemInfo(this); 
	}
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif