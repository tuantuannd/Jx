//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSG_EncodeDecode.cpp
//  Version     :   1.0
//  Creater     :   
//  Date        :   2003-6-3 10:28:57
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KSG_EncodeDecode.h"

int KSG_DecodeEncode(size_t uSize, unsigned char *pbyBuf, unsigned *puKey)
{
    unsigned *puBuf = (unsigned *)pbyBuf;
    unsigned uKey = *puKey;
    unsigned uRemainSize = uSize % 4;
    uSize /= 4;

    while (uSize-- > 0)
        *puBuf++ ^= uKey;

    pbyBuf = (unsigned char *)puBuf;
    
    while (uRemainSize-- > 0)
    {
        *pbyBuf++ ^= (unsigned char)(uKey & 0xff);
        uKey >>= 8;
    }

    *puKey = (*puKey * 31) + 134775813L;

    return true;
}


int KSG_DecodeEncode_ASM(size_t uSize, unsigned char *pbyBuf, unsigned *puKey)
{
    __asm mov eax, [uSize]
    __asm mov esi, [puKey]
    __asm mov edi, [pbyBuf]
    __asm mov ecx, eax
    __asm mov edx, [esi]        // key

    __asm shr eax, 2
    __asm jz  Next1
   
    Loop1:
    __asm mov ebx, [edi]
    __asm add edi, 4
    __asm xor ebx, edx
    __asm dec eax
    __asm mov [edi - 4], ebx
    __asm jnz Loop1


    Next1:
    __asm and ecx, 3
    __asm mov eax, edx  // Key
    __asm jz Next2

    Loop2:
    __asm mov bl, [edi]
    __asm inc edi
    __asm xor bl, al
    __asm shr eax, 8
    __asm mov [edi - 1], bl
    __asm dec ecx
    __asm jnz Loop2
    Next2:

    __asm mov eax, edx      // NewKey = OldKey * 31 +  134775813L
    __asm shl edx, 5
    __asm sub edx, eax
    __asm add edx, 134775813    // 08088405H
    __asm mov [esi], edx

    return true;
}
