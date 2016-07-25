/********************************************************************
	created:	2003/03/19
	file base:	Toolkit
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_TOOLKIT_H__
#define __INCLUDE_TOOLKIT_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

inline unsigned long g_ProgID2Key( const char * const pProgID )
{
	unsigned long ulnID = 0;

	for ( int i = 0; pProgID[i]; i++ )
	{
		ulnID = ( ulnID + (i + 1) * pProgID[i] ) % 0x8000000B * 0xFFFFFFEF;
	}

	return ( ulnID ^ 0x12345678 );
}

#endif // __INCLUDE_TOOLKIT_H__
