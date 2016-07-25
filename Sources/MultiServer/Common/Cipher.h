/********************************************************************
	created:	2003/06/18
	file base:	Cipher
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_CIPHER_H__
#define __INCLUDE_CIPHER_H__

/*
 * ProtocolType same as s2c_accountbegin in the protocol.h file
 */
#define CIPHER_PROTOCOL_TYPE	0x20

#pragma pack( 1 )

typedef struct 
{
	BYTE			ProtocolType;
    BYTE            Mode;
    BYTE            Reserve1[6];
    unsigned        ServerKey;
    unsigned        ClientKey;
    BYTE            Reserve2[16];
} ACCOUNT_BEGIN;

#pragma pack()

#endif // __INCLUDE_CIPHER_H__