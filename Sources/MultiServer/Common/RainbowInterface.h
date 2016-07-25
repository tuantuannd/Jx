/********************************************************************
	created:	2003/03/24
	file base:	RainbowInterface
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_RAINBOWINTERFACE_H__
#define __INCLUDE_RAINBOWINTERFACE_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

enum enumServerConnectInfo
{
	enumServerConnectCreate = 0x100,
	enumServerConnectClose
};

typedef VOID ( CALLBACK *CALLBACK_CLIENT_EVENT )( 
				LPVOID lpParam,
				const unsigned long &ulnEventType 
				/* enumServerConnectCreate or enumServerConnectClose */ );

#endif // __INCLUDE_RAINBOWINTERFACE_H__