//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPalette.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KPalette_H
#define KPalette_H
//---------------------------------------------------------------------------
typedef struct {
	BYTE		Blue;		// 兰色分量
	BYTE		Green;		// 绿色分量
	BYTE		Red;		// 红色分量
	BYTE		Alpha;		// 透明分量
} KPAL32;					// 32位色盘
//---------------------------------------------------------------------------
typedef struct {
	BYTE		Red;		// 红色分量
	BYTE		Green;		// 绿色分量
	BYTE		Blue;		// 兰色分量
} KPAL24;					// 24位色盘
//---------------------------------------------------------------------------
typedef WORD	KPAL16;		// 16位色盘
//---------------------------------------------------------------------------
void ENGINE_API	g_Pal32ToPal16(KPAL32* pPal32, KPAL16* pPal16, int nColors);
void ENGINE_API	g_Pal32ToPal24(KPAL32* pPal32, KPAL24* pPal24, int nColors);
void ENGINE_API	g_Pal32ToPal16Style(KPAL32* pPal32, KPAL16* pPal16, int nColors);
void ENGINE_API	g_Pal32ToPal16Lum(KPAL32* pPal32, KPAL16* pPal16, int nColors);
void ENGINE_API	g_Pal32ToPal16Gray(KPAL32* pPal32, KPAL16* pPal16, int nColors);
//---------------------------------------------------------------------------
void ENGINE_API	g_Pal24ToPal16(KPAL24* pPal24, KPAL16* pPal16, int nColors);
void ENGINE_API	g_Pal24ToPal32(KPAL24* pPal24, KPAL32* pPal32, int nColors);
void ENGINE_API	g_Pal24ToPal16Style(KPAL24* pPal24, KPAL16* pPal16, int nColors);
void ENGINE_API	g_Pal24ToPal16Lum(KPAL24* pPal24, KPAL16* pPal16, int nColors);
void ENGINE_API	g_Pal24ToPal16Red(KPAL24* pPal24, KPAL16* pPal16, int nColors);
//---------------------------------------------------------------------------
void ENGINE_API	g_Pal16ToPal24(KPAL16* pPal16, KPAL24* pPal24, int nColors);
void ENGINE_API	g_Pal16ToPal32(KPAL16* pPal16, KPAL32* pPal32, int nColors);
//---------------------------------------------------------------------------
void ENGINE_API	g_SetColorStyle(BYTE Red, BYTE Green, BYTE Blue);
//---------------------------------------------------------------------------
#endif
