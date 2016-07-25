#ifndef __D3D_UTILS_H__
#define __D3D_UTILS_H__

#include <d3d9types.h>

#define MASK_RED_555	0x00007C00
#define MASK_GREEN_555	0x000003E0
#define MASK_BLUE_555	0x0000001F
#define MASK_RED_565	0x0000F800
#define MASK_GREEN_565	0x000007E0
#define MASK_BLUE_565	0x0000001F

#define ARGBTO4444(a,r,g,b) (((((WORD)(a))<<8)&0xf000) + ((((WORD)(r))<<4)&0x0f00) + (((WORD)(g))&0x00f0) + (((WORD)(b))>>4))

void d3d_GetColorMasks(D3DFORMAT iD3DFormat, uint32& iBitCount, uint32& iAlphaMask, uint32& iRedMask, uint32& iGreenMask, uint32& iBlueMask);
int32 FitTextureSize(int32 nSize);

#endif