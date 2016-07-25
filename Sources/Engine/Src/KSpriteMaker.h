//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSpriteMaker.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KSpriteMaker_H
#define KSpriteMaker_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KSprite.h"
#include "KBitmap.h"
#include "KNode.h"
#include "KList.h"
//---------------------------------------------------------------------------
class KSprNode : public KNode
{
public:
	long		m_nSize;
	WORD		m_Sprite[4];
};
//---------------------------------------------------------------------------
class ENGINE_API KSpriteMaker
{
public:
	KMemClass	m_Offset;
	KList		m_FrameList;
	KPAL24		m_Pal24[256];
	KPAL16		m_Pal16[256];
	int			m_nWidth;
	int			m_nHeight;
	int			m_nCenterX;
	int			m_nCenterY;
	int			m_nFrames;
	int			m_nColors;
	int			m_nDirections;
	int			m_nInterval;
public:
	KSpriteMaker();
	~KSpriteMaker();
	BOOL		Load(LPSTR FileName);
	BOOL		Save(LPSTR FileName);
	void		Free();
	void		Draw(int nX, int nY, int nFrame);
	BOOL		AddFrame(KBitmap* pBitmap, BOOL bTrans);
	BOOL		AddAlphaFrame(KBitmap* pBitmap, KBitmap* pAlpha);
	BOOL		DelFrame(int nFrame);
	KSprNode*	GetFrame(int nFrame);
	void		MakeOffset();
};
//---------------------------------------------------------------------------
#endif
