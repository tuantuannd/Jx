//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPalette.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	256 Color bitmap file class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KColors.h"
#include "KPalette.h"
#include "KDDraw.h"
//---------------------------------------------------------------------------
static KPAL24 m_ColorStyle = {255, 255, 255};
//---------------------------------------------------------------------------
// 函数:	SetColorStyle
// 功能:	设置颜色风格
// 参数:	Red			红色 0-255
//			Green		绿色 0-255
//			Blue		蓝色 0-255
// 返回:	void
//---------------------------------------------------------------------------
void g_SetColorStyle(BYTE Red, BYTE Green, BYTE Blue)
{
	m_ColorStyle.Red   = Red;
	m_ColorStyle.Green = Green;
	m_ColorStyle.Blue  = Blue;
}
//---------------------------------------------------------------------------
// 函数:	Pal24ToPal16
// 功能:	24位调色板转化位16位调色板
// 参数:	pPal24		KPAL24指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal24ToPal16(KPAL24* pPal24, KPAL16* pPal16, int nColors)
{
	if (pPal24 && pPal16 && nColors > 0)
	{
		//	_ASSERT(pPal24 && pPal16 && g_pDirectDraw && count > 0);
		if (g_pDirectDraw->GetRGBBitMask16() == RGB_565)
		{
			_asm
			{
				mov		ecx, nColors
				mov		esi, pPal24
				mov		edi, pPal16
Start_Convert_565:
				{
					xor		ebx, ebx		//ebx清0
					mov		dx, [esi + 1]	//读如GB
					mov		al, [esi]		//读入R
					mov		bl, dl			//把G移动到[bl]
					shr		eax, 3
					shr		ebx, 2
					shl		eax, 11			//目标r生成了
					shl		ebx, 5
					add		esi, 3
					add		eax, ebx		//目标RG都合成到ax了
					xor		ebx, ebx		//把ebx清0
					mov		bl, dh			//把B移动[bl]
					shr		ebx, 3
					add		eax, ebx		//把目标
					mov		[edi], ax
					add		edi, 2
					dec		ecx				//减少count记数
					jg		Start_Convert_565
				}
			}
		}
		else
		{
			_asm
			{
				mov		ecx, nColors
				mov		esi, pPal24
				mov		edi, pPal16
Start_Convert_555:
				{
					//ax用于保存目标结果，假设第15bit对结果无影响
					xor		ebx, ebx		//ebx清0
					mov		dx, [esi + 1]	//读如GB
					mov		al, [esi]		//读入R
					mov		bl, dl			//把G移动到[bl]
					shr		eax, 3
					shr		ebx, 3
					shl		eax, 10			//目标r生成了
					shl		ebx, 5
					add		esi, 3
					add		eax, ebx		//目标RG都合成到ax了
					xor		ebx, ebx		//把ebx清0
					mov		bl, dh			//把B移动[bl]
					shr		ebx, 3
					add		eax, ebx		//把目标
					mov		[edi], ax
					add		edi, 2
					dec		ecx				//减少count记数
					jg		Start_Convert_555
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
// 功能:	16位调色板转化位16位调色板
//---------------------------------------------------------------------------
void g_Pal16ToPal24(KPAL16* pPal16, KPAL24* pPal24, int nColors)
{
	if (pPal24 && pPal16 && nColors > 0)
	{
		//	_ASSERT(pPal24 && pPal16 && g_pDirectDraw && count > 0);
		if (g_pDirectDraw->GetRGBBitMask16() == RGB_565)
		{
			_asm
			{
				mov		ecx, nColors
				mov		esi, pPal16
				mov		edi, pPal24
Start_Convert_565:
				{
					mov		ax, [esi]		//读入16bit色
					add		esi, 2
					mov		bx, ax			//复制16bit色
					and		ax, 0xF814		//得到r0b
					add		edi, 3
					mov		dl, al			//dl = b
					sub		bx, ax			//bx = 0g0
					shl		dl, 3			//dl = B
					shr		bx, 3			//bl = G
					mov		[edi - 1], dl
					mov		[edi - 2], bl
					mov		[edi - 3], ah
					dec		ecx				//减少count记数
					jg		Start_Convert_565
				}
			}
		}
		else
		{
			_asm
			{
				mov		ecx, nColors
				mov		esi, pPal16
				mov		edi, pPal24
Start_Convert_555:
				{
					mov		ax, [esi]		//读入16bit色
					add		esi, 2
					mov		bx, ax			//复制16bit色
					and		ax, 0x7C14		//得到r0b
					add		edi, 3
					mov		dl, al			//dl = b
					sub		bx, ax			//bx = 0g0
					shl		dx, 11			//dx = B0
					shr		bx, 2			//bl = G
					add		ax, ax
					mov		dl, bl			//dx = BG
					mov		[edi - 3], ah
					mov		[edi - 2], dx
					dec		ecx				//减少count记数
					jg		Start_Convert_555
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
// 函数:	Pal32ToPal16
// 功能:	32位调色板转化位16位调色板
// 参数:	pPal32		KPAL32指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal32ToPal16(KPAL32* pPal32, KPAL16* pPal16, int nColors)
{
	while (nColors-- > 0)
	{
		*pPal16 = g_RGB(pPal32->Red, pPal32->Green, pPal32->Blue);
		pPal16++;
		pPal32++;
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal24ToPal16Style
// 功能:	24位调色板转化位16位调色板
// 参数:	pPal24		KPAL24指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal24ToPal16Style(KPAL24* pPal24, KPAL16* pPal16, int nColors)
{
	while (nColors-- > 0)
	{
		*pPal16 = g_RGB(
			pPal24->Red * m_ColorStyle.Red / 255,
			pPal24->Green * m_ColorStyle.Green / 255,
			pPal24->Blue * m_ColorStyle.Blue / 255);
		pPal16++;
		pPal24++;
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal32ToPal16Style
// 功能:	32位调色板转化位16位调色板
// 参数:	pPal32		KPAL32指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal32ToPal16Style(KPAL32* pPal32, KPAL16* pPal16, int nColors)
{
	while (nColors-- > 0)
	{
		*pPal16 = g_RGB(
			pPal32->Red * m_ColorStyle.Red / 255,
			pPal32->Green * m_ColorStyle.Green / 255,
			pPal32->Blue * m_ColorStyle.Blue / 255);
		pPal16++;
		pPal32++;
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal24ToPal16Gray
// 功能:	24位调色板转化位16位调色板(灰度)
// 参数:	pPal24		KPAL24指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal24ToPal16Gray(KPAL24* pPal24, KPAL16* pPal16, int nColors)
{
	int nGray;
	while (nColors-- > 0)
	{
		nGray = (pPal24->Red * 30 + pPal24->Green * 59 + pPal24->Blue * 11) / 100;
		*pPal16 = g_RGB(nGray, nGray, nGray);
		pPal16++;
		pPal24++;
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal32ToPal16Gray
// 功能:	32位调色板转化位16位调色板(灰度)
// 参数:	pPal32		KPAL32指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal32ToPal16Gray(KPAL32* pPal32, KPAL16* pPal16, int nColors)
{
	int nGray;
	while (nColors-- > 0)
	{
		nGray = (pPal32->Red * 30 + pPal32->Green * 59 + pPal32->Blue * 11) / 100;
		*pPal16 = g_RGB(nGray, nGray, nGray);
		pPal16++;
		pPal32++;
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal24ToPal16Lum
// 功能:	24位调色板转化位16位调色板
// 参数:	pPal24		KPAL24指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal24ToPal16Lum(KPAL24* pPal24, KPAL16* pPal16, int nColors)
{
	int lum, i;
	for (lum = 32; lum > 0; lum--)
	{
		for (i = 0; i < nColors; i++)
		{
			pPal16[i] = g_RGB(
				pPal24[i].Red * lum / 32,
				pPal24[i].Green * lum / 32,
				pPal24[i].Blue * lum / 32);
		}
		pPal16 += nColors * sizeof(KPAL16);
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal32ToPal16Lum
// 功能:	32位调色板转化位16位调色板
// 参数:	pPal32		KPAL32指针
//			pPal16		KPAL16指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal32ToPal16Lum(KPAL32* pPal32, KPAL16* pPal16, int nColors)
{
	int lum, i;
	for (lum = 32; lum > 0; lum--)
	{
		for (i = 0; i < nColors; i++)
		{
			pPal16[i] = g_RGB(
				pPal32[i].Red * lum / 32,
				pPal32[i].Green * lum / 32,
				pPal32[i].Blue * lum / 32);
		}
		pPal16 += nColors * sizeof(KPAL16);
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal24ToPal32
// 功能:	24位调色板转化位32位调色板
// 参数:	pPal24		KPAL24指针
//			pPal32		KPAL32指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal24ToPal32(KPAL24* pPal24, KPAL32* pPal32, int nColors)
{
	for (int i = 0; i < nColors; i++)
	{
		pPal32[i].Red   = pPal24[i].Red;
		pPal32[i].Green = pPal24[i].Green;
		pPal32[i].Blue  = pPal24[i].Blue;
		pPal32[i].Alpha = 0;
	}
}
//---------------------------------------------------------------------------
// 函数:	Pal32ToPal24
// 功能:	32位调色板转化位24位调色板
// 参数:	pPal32		KPAL32指针
//			pPal24		KPAL24指针
//			nClolors	颜色数(0 - 256)
// 返回:	void
//---------------------------------------------------------------------------
void g_Pal32ToPal24(KPAL32* pPal32, KPAL24* pPal24, int nColors)
{
	for (int i = 0; i < nColors; i++)
	{
		pPal24[i].Red   = pPal32[i].Red;
		pPal24[i].Green = pPal32[i].Green;
		pPal24[i].Blue  = pPal32[i].Blue;
	}
}
//---------------------------------------------------------------------------
