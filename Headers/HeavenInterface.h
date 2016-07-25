/********************************************************************
	created:	2003/03/17
	file base:	HeavenInterface
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_INTERFACE_HEAVENINTERFACE_H__
#define __INCLUDE_INTERFACE_HEAVENINTERFACE_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

enum enumClientConnectInfo
{
	enumClientConnectCreate = 0x100,
	enumClientConnectClose
};

/*
 * If you install this function, You can get some event that 
 * it happend on all whole client. example: a connect come from client,
 * a client close the current link, etc.
 */
typedef VOID ( CALLBACK *CALLBACK_SERVER_EVENT )( 
				LPVOID lpParam,
				const unsigned long &ulnID,
				const unsigned long &ulnEventType 
				/* enumClientConnectCreate or enumClientConnectClose */ );

#endif // __INCLUDE_INTERFACE_HEAVENINTERFACE_H__