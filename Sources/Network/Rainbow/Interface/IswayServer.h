/********************************************************************
	created:	2003/03/17
	file base:	IswayServer
	file ext:	h
	author:		liupeng
	
	purpose:	This interface for the single-way server
*********************************************************************/
#ifndef __INCLUDE_INTERFACE_ISWAYSERVER_H__
#define __INCLUDE_INTERFACE_ISWAYSERVER_H__

#include "RainbowInterface.h"

/*
 * Antitype : IswayServer Interface
 */
// {2972B126-12B2-46c8-AB9B-C8103F369C92}
DEFINE_GUID(IID_ISWAYSERVER, 
0x2972b126, 0x12b2, 0x46c8, 0xab, 0x9b, 0xc8, 0x10, 0x3f, 0x36, 0x9c, 0x92);

DECLARE_INTERFACE_( IswayServer, IUnknown )
{

};

#endif // __INCLUDE_INTERFACE_ISWAYSERVER_H__
