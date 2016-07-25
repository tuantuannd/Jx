//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSpriteCodec.h
// Date:	2001.09.18
// Code:	Wangwei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KSpriteCodec_H
#define KSpriteCodec_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KBitmap.h"
//---------------------------------------------------------------------------
class ENGINE_API KSpriteCodec
{
protected:
	KBitmap*	m_pBitmap;
	KBitmap*	m_pAlpha;
	KMemClass	m_Sprite;
	long		m_nWidth;
	long		m_nHeight;
	long		m_nLength;

protected:
	void		GetMiniRect(LPRECT lpRect);
	void		ConvertRect(LPRECT lpRect);
	LONG		ConvertLine(int nX, int nY, int nWidth, PBYTE pBuffer);
	BYTE		GetAlphaCount(int nX, int nY, int nWidth);

public:
	KSpriteCodec();
	BOOL		Convert(KBitmap* pBitmap, KBitmap* pAlpha);
	PVOID		GetSprite(){ return m_Sprite.GetMemPtr(); };
	LONG		GetLength(){ return m_nLength; };
};
//---------------------------------------------------------------------------
#endif
