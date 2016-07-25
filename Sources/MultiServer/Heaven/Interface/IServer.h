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

interface IMessageProcess
{
	virtual HRESULT OnMessage( VOID*, size_t ) = 0;
};


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
			LPVOID lpParam, 
			CALLBACK_SERVER_EVENT pfnEventNotify
		) PURE;

	/*
	 * Install a callback funtion is used to get message from server
	 */
	STDMETHOD( RegisterMsgFilter )
		(
			const unsigned long ulnClientID, 
			IMessageProcess* pfnMsgNotify
		) PURE;

	/*
	 * Control network engine
	 */
/*	STDMETHOD( Start )
		(
		) PURE;

	STDMETHOD( Stop )
		(
		) PURE;
*/
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
	 * Send all when ulnClientID is equal to -1
	 */
	STDMETHOD( SendPackToClient )
		(
			const unsigned long &ulnClientID /* -1 */
		) PURE;

	/*
	 * Send data to client immediately
	 */
	STDMETHOD( SendData )
		(
			const unsigned long &ulnClientID,
			const void * const	pData,
			const size_t		&datalength
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

	/*
	 * Get information from a specified client
	 */
	STDMETHOD_( const char *, GetClientInfo )
		(
			const unsigned long &ulnClientID
		) PURE;

}; // Endof DECLARE_INTERFACE_( IServer ...


/*
 * Antitype : IServerFactory Interface
 */
// {A4984B66-8CDE-4752-9BEE-DACC06A63548}
DEFINE_GUID( IID_IServerFactory, 
0xa4984b66, 0x8cde, 0x4752, 0x9b, 0xee, 0xda, 0xcc, 0x6, 0xa6, 0x35, 0x48);

DECLARE_INTERFACE_( IServerFactory, IUnknown )
{
	STDMETHOD( SetEnvironment )
		(
			const size_t &nPlayerMaxCount,
			const size_t &nPrecision,
			const size_t &maxFreeBuffers_Cache,
			const size_t &bufferSize_Cache
		) PURE;

	STDMETHOD( CreateServerInterface )
		(
			REFIID riid, 
			void** ppv
		) PURE;

}; // IID_IServerFactory

/*
 * Antetype:	CreateInterface
 *
 * Return:		S_OK : success create object
 * 				E_OUTOFMEMORY : create faild
 *
 * Purpose:		Get server interface
 */

STDAPI CreateInterface
		(
			REFIID	riid,
			void	**ppv
		);

#endif // __INCLUDE_INTERFACE_ISERVER_H__