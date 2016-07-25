//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KBmp2Spr.h
// Date:	2001.09.18
// Code:	Wangwei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KBmp2Spr_H
#define KBmp2Spr_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KBitmap.h"
//---------------------------------------------------------------------------
class ENGINE_API KBmp2Spr
{
protected:
	KBitmap*	m_pBitmap;
	KMemClass	m_Sprite;
	long		m_nWidth;
	long		m_nHeight;
	long		m_nLength;
	BOOL		m_bTransp;
	BYTE		m_ColorKey;

protected:
	void		GetMiniRect(LPRECT lpRect);
	long		ConvertRect(LPRECT lpRect, PBYTE pBuffer);
	long		ConvertLine(int nX, int nY, int nWidth, PBYTE pBuffer);
	long		GetTransCount(int nX, int nY, int nWidth);
	long		GetPixelCount(int nX, int nY, int nWidth);

public:
	KBmp2Spr();
	BOOL		Convert(KBitmap* pBitmap, BOOL bTransp);
	PVOID		GetSprite() { return m_Sprite.GetMemPtr(); };
	long		GetLength() { return m_nLength; };
};
//---------------------------------------------------------------------------
#endif
