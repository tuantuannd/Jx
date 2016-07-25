#include "precompile.h"
#include "d3d_utils.h"

void d3d_GetColorMasks(D3DFORMAT iD3DFormat, uint32& iBitCount, uint32& iAlphaMask, uint32& iRedMask, uint32& iGreenMask, uint32& iBlueMask)
{
	switch (iD3DFormat) {
	case D3DFMT_R8G8B8 :
		iBitCount = 24; iAlphaMask = 0x00000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_X8R8G8B8 :
		iBitCount = 32; iAlphaMask = 0x00000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_A8R8G8B8 :
		iBitCount = 32; iAlphaMask = 0xFF000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_R5G6B5 :
		iBitCount = 16; iAlphaMask = 0x00000000; iRedMask = 0x0000F800; iGreenMask = 0x000007E0; iBlueMask = 0x0000001F; break;
	case D3DFMT_A1R5G5B5 :
		iBitCount = 16; iAlphaMask = 0x00008000; iRedMask = 0x00007C00; iGreenMask = 0x000003E0; iBlueMask = 0x0000001F; break;
	case D3DFMT_X1R5G5B5 :
		iBitCount = 16; iAlphaMask = 0x00000000; iRedMask = 0x00007C00; iGreenMask = 0x000003E0; iBlueMask = 0x0000001F; break;
	case D3DFMT_A4R4G4B4 :
		iBitCount = 16; iAlphaMask = 0x0000F000; iRedMask = 0x00000F00; iGreenMask = 0x000000F0; iBlueMask = 0x0000000F; break;
	case D3DFMT_P8 :
		iBitCount = 8;  iAlphaMask = 0xFF000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	default : assert(0 && "Unknown Format"); }
}

int32 FitTextureSize(int32 nSize)
{
	if(nSize < 0 || nSize > MAX_TEXTURE_SIZE)
		return 0;

	for(int32 i=MIN_TEXTURE_SIZE; i<=MAX_TEXTURE_SIZE; i*= 2)
	{
		if(nSize <= i)
			return i;
	}
	return 0;
}