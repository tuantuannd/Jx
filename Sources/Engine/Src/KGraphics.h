//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2002 by Kingsoft
//
// File:	KGraphics.h
// Date:	2002.03.06
// Code:	Spe
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KGraphics_H
#define KGraphics_H

BOOL	ENGINE_API g_RemoveSamePixel(KBitmap *DesBitmap, KBitmap *SrcBitmap);
BOOL	ENGINE_API g_RemoveDiffPixel(KBitmap *DesBitmap, KBitmap *SrcBitmap);
BOOL	ENGINE_API g_RemoveNoneZeroPixel(KBitmap *DesBitmap, KBitmap *SrcBitmap);
BOOL	ENGINE_API g_RemoveZeroPixel(KBitmap *DesBitmap, KBitmap *SrcBitmap);
#endif