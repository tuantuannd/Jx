//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2002 by Kingsoft
//
// File:	KGraphics.cpp
// Date:	2002.03.06
// Code:	Spe
// Desc:	不同的KBitmap之间的处理
//---------------------------------------------------------------------------
#include "kwin32.h"
#include "KEngine.h"
#include "KBitmap.h"
#include "KGraphics.h"
//---------------------------------------------------------------------------
// 函数:	g_RemoveSamePixel
// 功能:	去除目标图中与源图相同的点
// 参数:	*DesBitmap, *SrcBitmap
// 返回:	是否成功处理
//---------------------------------------------------------------------------
BOOL g_RemoveSamePixel(KBitmap *DesBitmap, KBitmap *SrcBitmap)
{
	BYTE	*pDesBuffer, *pSrcBuffer;
	int		nHeight, nWidth, i, j;
	// 源图与目标图宽高不同，无法比较去除
	if (DesBitmap->GetHeight() != SrcBitmap->GetHeight()
		|| DesBitmap->GetWidth() != SrcBitmap->GetWidth())
		return FALSE;

	nHeight	= DesBitmap->GetHeight();
	nWidth	= DesBitmap->GetWidth();
	pDesBuffer = (BYTE *)DesBitmap->GetBitmap();
	pSrcBuffer = (BYTE *)SrcBitmap->GetBitmap();

	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			if (pDesBuffer[i * nWidth + j] == pSrcBuffer[i * nWidth + j])
				pDesBuffer[i * nWidth + j] = 0;
		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	g_RemoveDiffPixel
// 功能:	去除目标图中与源图不同的点
// 参数:	*DesBitmap, *SrcBitmap
// 返回:	是否成功处理
//---------------------------------------------------------------------------
BOOL g_RemoveDiffPixel(KBitmap *DesBitmap, KBitmap *SrcBitmap)
{
	BYTE	*pDesBuffer, *pSrcBuffer;
	int		nHeight, nWidth, i, j;
	// 源图与目标图宽高不同，无法比较去除
	if (DesBitmap->GetHeight() != SrcBitmap->GetHeight()
		|| DesBitmap->GetWidth() != SrcBitmap->GetWidth())
		return FALSE;
	
	nHeight	= DesBitmap->GetHeight();
	nWidth	= DesBitmap->GetWidth();
	pDesBuffer = (BYTE *)DesBitmap->GetBitmap();
	pSrcBuffer = (BYTE *)SrcBitmap->GetBitmap();
	
	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			if (pDesBuffer[i * nWidth + j] != pSrcBuffer[i * nWidth + j])
				pDesBuffer[i * nWidth + j] = 0;
		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	g_RemoveNoneZeroPixel
// 功能:	去除目标图中源图位置为非零点的点
// 参数:	*DesBitmap, *SrcBitmap
// 返回:	是否成功处理
//---------------------------------------------------------------------------
BOOL g_RemoveNoneZeroPixel(KBitmap *DesBitmap, KBitmap *SrcBitmap)
{
	BYTE	*pDesBuffer, *pSrcBuffer;
	int		nHeight, nWidth, i, j;
	KPAL32	*pPal32;
	// 源图与目标图宽高不同，无法比较去除
	if (DesBitmap->GetHeight() != SrcBitmap->GetHeight()
		|| DesBitmap->GetWidth() != SrcBitmap->GetWidth())
		return FALSE;
	
	nHeight	= DesBitmap->GetHeight();
	nWidth	= DesBitmap->GetWidth();
	pDesBuffer = (BYTE *)DesBitmap->GetBitmap();
	pSrcBuffer = (BYTE *)SrcBitmap->GetBitmap();
	pPal32 = SrcBitmap->GetPal32();

	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			if (pPal32[pSrcBuffer[i * nWidth + j]].Red != 0
				|| pPal32[pSrcBuffer[i * nWidth + j]].Green != 0
				|| pPal32[pSrcBuffer[i * nWidth + j]].Blue != 0)
				pDesBuffer[i * nWidth + j] = 0;
		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	g_RemoveZeroPixel
// 功能:	去除目标图中源图位置为零点的点
// 参数:	*DesBitmap, *SrcBitmap
// 返回:	是否成功处理
//---------------------------------------------------------------------------
BOOL g_RemoveZeroPixel(KBitmap *DesBitmap, KBitmap *SrcBitmap)
{
	BYTE	*pDesBuffer, *pSrcBuffer;
	int		nHeight, nWidth, i, j;
	KPAL32	*pPal32;
	// 源图与目标图宽高不同，无法比较去除
	if (DesBitmap->GetHeight() != SrcBitmap->GetHeight()
		|| DesBitmap->GetWidth() != SrcBitmap->GetWidth())
		return FALSE;
	
	nHeight	= DesBitmap->GetHeight();
	nWidth	= DesBitmap->GetWidth();
	pDesBuffer = (BYTE *)DesBitmap->GetBitmap();
	pSrcBuffer = (BYTE *)SrcBitmap->GetBitmap();
	pPal32 = SrcBitmap->GetPal32();
	
	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			if (pPal32[pSrcBuffer[i * nWidth + j]].Red == 0
				&& pPal32[pSrcBuffer[i * nWidth + j]].Green == 0
				&& pPal32[pSrcBuffer[i * nWidth + j]].Blue == 0)
				pDesBuffer[i * nWidth + j] = 0;
		}
	}
	return TRUE;
}
