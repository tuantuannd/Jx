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

//#include <crtdbg.h>
#include "zport.h"

int KSG_DecodeEncode(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);
int KSG_DecodeEncode_ASM(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);

int KSG_DecodeBuf(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);
int KSG_EncodeBuf(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);


#endif  // _KSG_ENCODEDECODE_H
