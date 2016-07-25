/********************************************************************
	created:	2003/02/16
	file base:	protocol
	file ext:	h
	author:		liupeng
	
	purpose:	Define protocol for we can transmit some correct data
*********************************************************************/
#ifndef __INCLUDE_PROTOCOL_H__
#define __INCLUDE_PROTOCOL_H__

//[ Only for debug, delete it if you release the lib
//#define NETWORK_DEBUG
//]

#pragma pack(1)

typedef struct tagPackHeader
{
	BYTE	cPackBeginFlag;	// 0xBF

	WORD	wDataLen;		// [PACK_HEADER] [DATA] /* wDataLen == sizeof(DATA) */
	WORD	wCRCData;		// [cPackBeginFlag | cPackEndFlag] | [wDataLen]

	BYTE	cPackEndFlag;	// 0xEF
	
}PACK_HEADER, NEAR *PPACK_HEADER, FAR *LPPACK_HEADER;

#define PACK_BEGIN_FLAG		0xBF
#define PACK_END_FLAG		0xEF

#define _private_make_crc_data( x, y, z ) ( ( x << 8 | y ) | z )
#define MAKE_CRC_DATE( x, y, z ) _private_make_crc_data( x, y, z )

#define PACK_HEADER_LEN			sizeof( PACK_HEADER )

#pragma pack()

#endif //__INCLUDE_PROTOCOL_H__
