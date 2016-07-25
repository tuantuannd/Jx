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

#define SERVER_MSG_BASE			0x100

#define SERVER_CONNECT_CREATE	SERVER_MSG_BASE + 1
#define SERVER_CONNECT_CLOSE	SERVER_MSG_BASE + 2

typedef VOID ( CALLBACK *CALLBACK_CLIENT_EVENT )( 
				const unsigned long &ulnEventType 
				/*SERVER_CONNECT_CREATE | SERVER_CONNECT_CLOSE*/ );

#endif // __INCLUDE_RAINBOWINTERFACE_H__