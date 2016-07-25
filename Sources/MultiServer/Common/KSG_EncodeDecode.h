//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSG_EncodeDecode.h
//  Version     :   1.0
//  Creater     :   
//  Date        :   2003-6-3 10:29:43
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _KSG_ENCODEDECODE_H
#define _KSG_ENCODEDECODE_H 1

#include <crtdbg.h>

int KSG_DecodeEncode(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);
int KSG_DecodeEncode_ASM(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);

inline int KSG_DecodeBuf(size_t uSize, unsigned char *pbyBuf, unsigned *puKey)
{
    _ASSERT(uSize);
    _ASSERT(pbyBuf);
    _ASSERT(puKey);
	
	unsigned uKey = *puKey;
    //return KSG_DecodeEncode(uSize, pbyBuf, puKey);
    return KSG_DecodeEncode_ASM(uSize, pbyBuf, &uKey);
}

inline int KSG_EncodeBuf(size_t uSize, unsigned char *pbyBuf, unsigned *puKey)
{
    _ASSERT(uSize);
    _ASSERT(pbyBuf);
    _ASSERT(puKey);

	unsigned uKey = *puKey;
    //return KSG_DecodeEncode(uSize, pbyBuf, puKey);
    return KSG_DecodeEncode_ASM(uSize, pbyBuf, &uKey);
}



#endif  // _KSG_ENCODEDECODE_H