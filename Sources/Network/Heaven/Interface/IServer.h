/********************************************************************
	created:	2003/03/19
	file base:	IServer
	file ext:	h
	author:		liupeng
	
	purpose:	Server interface definition
*********************************************************************/
#ifndef __INCLUDE_INTERFACE_ISERVER_H__
#define __INCLUDE_INTERFACE_ISERVER_H__

#include "HeavenInterface.h"
#include <objbase.h>

/*
 * Antitype : IOCPServer Interface
 */
// {FC6B562E-89C2-4fbc-BF5F-553C09AE93D8}
DEFINE_GUID( IID_IIOCPServer, 
0xfc6b562e, 0x89c2, 0x4fbc, 0xbf, 0x5f, 0x55, 0x3c, 0x9, 0xae, 0x93, 0xd8);

/*
 * ATTENTION : PLEASE USE IT IN ANSI SETTING
 */

DECLARE_INTERFACE_( IServer, IUnknown )
{
	/*
	 * Initialize server object and start up it
	 */
	STDMETHOD( Startup )
		(
		) PURE;

	/*
	 * Stop this object and destroy it
	 */
	STDMETHOD( Cleanup )
		(			
		) PURE;

	/*
	 *	Afford service base on an IP.PORT
	 */
	STDMETHOD( OpenService )
		(
			const unsigned long		&ulnAddressToListenOn,
			const unsigned short	&usnPortToListenOn
		) PURE;

	/*
	 * Close the current connection
	 */
	STDMETHOD( CloseService )
		(
		) PURE;

	/*
	 * Install a callback funtion is used to get event from server
	 */
	STDMETHOD( RegisterMsgFilter )
		(
			CALLBACK_SERVER_EVENT pfnEventNotify
		) PURE;

	/*
	 * prepare to send 
	 */
	STDMETHOD( PreparePackSink )
		(
		) PURE;

	/*
	 * Add data into client sink but don't to send immediately
	 */
	STDMETHOD( PackDataToClient )
		(
			const unsigned long &ulnClientID,
			const void * const	pData,
			const size_t		&datalength
		) PURE;

	/*
	 * Begin to send
	 */
	STDMETHOD( SendPackToClient )
		(
		) PURE;

	/*
	 * Get some data from a specified client
	 */
	STDMETHOD_( const void *, GetPackFromClient )
		(
			const unsigned long &ulnClientID,
			size_t				&datalength			
		) PURE;

	/*
	 * Disconnect a specified connection
	 */
	STDMETHOD( ShutdownClient )
		(
			const unsigned long &ulnClientID			
		) PURE;

	/*
	 * Get current client count
	 */
	STDMETHOD_( size_t, GetClientCount )
		(
		) PURE;

}; // Endof DECLARE_INTERFACE_( IServer ...



#endif // __INCLUDE_INTERFACE_ISERVER_H__