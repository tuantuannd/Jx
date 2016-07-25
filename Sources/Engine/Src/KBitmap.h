//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KBitmap.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KBitmap_H
#define KBitmap_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KPalette.h"
//---------------------------------------------------------------------------
typedef struct {
	char	Id[4];
	LONG	Width;
	LONG	Height;
	LONG	Colors;
} KBMPHEADER;
//---------------------------------------------------------------------------
class ENGINE_API KBitmap
{
private:
	KMemClass	m_Bitmap;
	KPAL32		m_Pal32[256];
	KPAL16		m_Pal16[256];
	int			m_nWidth;
	int			m_nHeight;
	int			m_nColors;
public:
	KBitmap();
	BOOL		Init(int nWidth, int nHeight, int nColors);
	BOOL		Load(LPSTR lpFileName);
	BOOL		Save(LPSTR lpFileName);
	void		Draw(int nX, int nY);
	void		Clear(BYTE byColor);
	void		MakePalette();
	void		PutPixel(int nX, int nY, BYTE byColor);
	BYTE		GetPixel(int nX, int nY);
public:
	void*		GetBitmap()	{ return m_Bitmap.GetMemPtr(); };
	KPAL32*		GetPal32()	{ return m_Pal32; };
	KPAL16*		GetPal16()	{ return m_Pal16; };
	int			GetWidth()	{ return m_nWidth; };
	int			GetHeight()	{ return m_nHeight; };
	int			GetColors()	{ return m_nColors; };
};
//---------------------------------------------------------------------------
#endif
