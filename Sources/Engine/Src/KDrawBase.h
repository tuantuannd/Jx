//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDrawBase.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KDrawBase_H
#define KDrawBase_H
//---------------------------------------------------------------------------
void	g_DrawPixel(void* node, void* canvas);
void	g_DrawPixelAlpha(void* node, void* canvas);
void	g_DrawLine(void* node, void* canvas);
void	g_DrawLineAlpha(void* node, void* canvas);
void	g_Clear(void* node, void* canvas);
void	g_DotClear(void* node, void* canvas);
void	g_ClearAlpha(void* node, void* canvas);
//---------------------------------------------------------------------------
#endif
