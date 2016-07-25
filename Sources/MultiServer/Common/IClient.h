/********************************************************************
	created:	2003/03/19
	file base:	IClient
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_INTERFACE_ICLIENT_H__
#define __INCLUDE_INTERFACE_ICLIENT_H__

#include "RainbowInterface.h"
#include <objbase.h>
/*
 * Antitype : IESClient Interface
 */
// {D38249A9-4565-4336-9F70-1374F1E158E7}
DEFINE_GUID( IID_IESClient, 
0xd38249a9, 0x4565, 0x4336, 0x9f, 0x70, 0x13, 0x74, 0xf1, 0xe1, 0x58, 0xe7);

/*
 * ATTENTION : PLEASE USE IT IN ANSI SETTING
 */

DECLARE_INTERFACE_( IClient, IUnknown )
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
	 * Connect to a specified server
	 */
	STDMETHOD( ConnectTo )
		(
			const char * const &pAddressToConnectServer,
			unsigned short usPortToConnectServer
		) PURE;

	/*
	 *
	 */
	STDMETHOD( RegisterMsgFilter )
		(
			LPVOID lpParam,
			CALLBACK_CLIENT_EVENT pfnEventNotify
		) PURE;
	/*
	 * Send some data to the current server
	 */
	STDMETHOD( SendPackToServer )
		(
			const void * const	pData,
			const size_t		&datalength
		) PURE;

	/*
	 * Get some data from the current server
	 */
	STDMETHOD_( const void *, GetPackFromServer )
		(
			size_t				&datalength			
		) PURE;
	
	/*
	 * Disconnect a specified connection
	 */
	STDMETHOD( Shutdown )
		(
		) PURE;

}; // Endof DECLARE_INTERFACE_( IClient ...

/*
 * Antitype : IClientFactory Interface
 */

// {9B2F150C-3D59-40db-A70B-E999D383D044}
DEFINE_GUID( IID_IClientFactory, 
0x9b2f150c, 0x3d59, 0x40db, 0xa7, 0xb, 0xe9, 0x99, 0xd3, 0x83, 0xd0, 0x44);

DECLARE_INTERFACE_( IClientFactory, IUnknown )
{
	STDMETHOD( SetEnvironment )
		(
			const size_t &bufferSize
		) PURE;

	STDMETHOD( CreateClientInterface )
		(
			REFIID riid, 
			void** ppv
		) PURE;

}; // IID_IClientFactory

/*
 *	Antetype:	CreateInterface
 *
 *	Return:		S_OK : success create object
 *				E_OUTOFMEMORY : create faild
 *
 *	Purpose:	Get client interface
 */
STDAPI CreateInterface
		(
			REFIID	riid,
			void	**ppv
		);

#endif // __INCLUDE_INTERFACE_ICLIENT_H__