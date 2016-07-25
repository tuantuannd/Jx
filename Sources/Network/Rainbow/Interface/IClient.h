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
	Antetype:	QueryClientInterface	

	Return:		S_OK : success create object
				E_OUTOFMEMORY : create faild

	Purpose:	Get client interface

				CoInitialize( NULL ); 
				...
				QueryClientInterface( ... );
				...
				CoUninitialize();

	Created:	2003/03/19
*/

STDAPI QueryClientInterface
		(
			REFIID	riid,
			void	**ppv
		);

#endif // __INCLUDE_INTERFACE_ICLIENT_H__