//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSpriteMaker.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Sprite Make Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KMemManager.h"
#include "KFile.h"
#include "KBmp2Spr.h"
#include "KCanvas.h"
#include "KSpriteCodec.h"
#include "KSpriteMaker.h"
//---------------------------------------------------------------------------
// 函数:	KSpriteMaker
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KSpriteMaker::KSpriteMaker()
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_nCenterX = 0;
	m_nCenterY = 0;
	m_nFrames = 0;
	m_nColors = 0;
	m_nDirections = 1;
	m_nInterval = 1;
}
//---------------------------------------------------------------------------
// 函数:	~KSpriteMaker
// 功能:	析构函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KSpriteMaker::~KSpriteMaker()
{
	Free();
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	载入SPR文件
// 参数:	FileName	文件名
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KSpriteMaker::Load(LPSTR FileName)
{
	KFile	File;
	SPRHEAD	Header;

	// open the file
	if (!File.Open(FileName))
		return FALSE;

	// read header
	File.Read(&Header, sizeof(Header));

	// check file header
	if (!g_MemComp(Header.Comment, "SPR\0", 4))
		return FALSE;

	// setup class member
	m_nWidth      = Header.Width;
	m_nHeight     = Header.Height;
	m_nCenterX    = Header.CenterX;
	m_nCenterY    = Header.CenterY;
	m_nFrames     = Header.Frames;
	m_nColors     = Header.Colors;
	m_nDirections = Header.Directions;
	m_nInterval   = Header.Interval;

	// read palette table
	File.Read(m_Pal24, m_nColors * sizeof(KPAL24));
	g_Pal24ToPal16(m_Pal24, m_Pal16, m_nColors);

	// read offset table
	m_Offset.Alloc(m_nFrames * sizeof(SPROFFS));
	SPROFFS* pOffset = (SPROFFS*)m_Offset.GetMemPtr();
	File.Read(pOffset, m_nFrames * sizeof(SPROFFS));

	// read frame data
	KSprNode* pNode;
	for (int i = 0; i < m_nFrames; i++)
	{
		pNode = (KSprNode*)g_MemManager.Calloc(sizeof(KSprNode) + pOffset[i].Length);
		pNode->m_nSize = pOffset[i].Length;
		File.Read(pNode->m_Sprite, pNode->m_nSize);
		m_FrameList.AddTail(pNode);
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存SFF文件
// 参数:	FileName	文件名
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KSpriteMaker::Save(LPSTR FileName)
{
	KFile	File;
	SPRHEAD	Header;

	// create the file
	if (!File.Create(FileName))
		return FALSE;

	// setup file header
	g_MemZero(&Header, sizeof(Header));
	g_MemCopy(Header.Comment, "SPR\0", 4);
	Header.Width = m_nWidth;
	Header.Height = m_nHeight;
	Header.CenterX = m_nCenterX;
	Header.CenterY = m_nCenterY;
	Header.Frames = m_nFrames;
	Header.Colors = m_nColors;
	Header.Directions = m_nDirections;
	Header.Interval = m_nInterval;

	// write file header
	File.Write(&Header, sizeof(Header));

	// write palette table
	File.Write(m_Pal24, m_nColors * sizeof(KPAL24));

	// write offset table
	MakeOffset();
	File.Write(m_Offset.GetMemPtr(), m_nFrames * sizeof(SPROFFS));

	// write frame data
	KSprNode* pNode = (KSprNode*)m_FrameList.GetHead();
	for (int i = 0; i < m_nFrames; i++)
	{
		File.Write(pNode->m_Sprite, pNode->m_nSize);
		pNode = (KSprNode*)pNode->GetNext();
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Free
// 功能:	释放资源
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KSpriteMaker::Free()
{
	KSprNode* pNode;
	while (pNode = (KSprNode*)m_FrameList.GetHead())
	{
		pNode->Remove();
		g_MemManager.Free(pNode);
	}
	m_nWidth = 0;
	m_nHeight = 0;
	m_nCenterX = 0;
	m_nCenterY = 0;
	m_nFrames = 0;
	m_nColors = 0;
	m_nDirections = 1;
	m_nInterval = 1;
}
//---------------------------------------------------------------------------
// 函数:	Draw
// 功能:	绘制图象
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSpriteMaker::Draw(int nX, int nY, int nFrame)
{
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	
	// goto frame
	KSprNode* pNode = (KSprNode*)m_FrameList.GetHead();
	for (int i = 0; i < nFrame; i++)
		pNode = (KSprNode*)pNode->GetNext();
	SPRFRAME* pFrame = (SPRFRAME*)pNode->m_Sprite;
	
	// draw the frame
	g_pCanvas->DrawSprite(
		nX + pFrame->OffsetX,
		nY + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_Pal16);
}
//---------------------------------------------------------------------------
// 函数:	AddFrame
// 功能:	追加图象帧
// 参数:	pBitmap		追加位图
//			bTrans		是否透明
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KSpriteMaker::AddFrame(KBitmap* pBitmap, BOOL bTrans)
{
	KBmp2Spr Bmp2Spr;

	// convert bitmap
	if (!Bmp2Spr.Convert(pBitmap, bTrans))
		return FALSE;

	// copy palette data
	if (m_FrameList.IsEmpty())
	{
		m_nWidth = pBitmap->GetWidth();
		m_nHeight = pBitmap->GetHeight();
		m_nColors = pBitmap->GetColors();
		g_Pal32ToPal24(pBitmap->GetPal32(), m_Pal24, m_nColors);
		g_Pal24ToPal16(m_Pal24, m_Pal16, m_nColors);
	}
	else
	{
		if (m_nWidth < pBitmap->GetWidth())
			m_nWidth = pBitmap->GetWidth();
		if (m_nHeight < pBitmap->GetHeight())
			m_nHeight = pBitmap->GetHeight();
		if (m_nColors < pBitmap->GetColors())
			return FALSE;
	}
	// copy frame data
	KSprNode* pNode = (KSprNode*)g_MemManager.Calloc(sizeof(KSprNode) + Bmp2Spr.GetLength());
	pNode->m_nSize = Bmp2Spr.GetLength();
	g_MemCopy(pNode->m_Sprite, Bmp2Spr.GetSprite(), Bmp2Spr.GetLength());
	m_FrameList.AddTail(pNode);
	
	// inc frame number
	m_nFrames++;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	AddFrame
// 功能:	追加图象帧
// 参数:	pBitmap		追加位图
//			bTrans		是否透明
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KSpriteMaker::AddAlphaFrame(KBitmap* pBitmap, KBitmap* pAlpha)
{
	KSpriteCodec SpriteCodec;
	
	// convert bitmap
	if (!SpriteCodec.Convert(pBitmap, pAlpha))
		return FALSE;
	
	// copy palette data
	if (m_FrameList.IsEmpty())
	{
		m_nWidth = pBitmap->GetWidth();
		m_nHeight = pBitmap->GetHeight();
		m_nColors = pBitmap->GetColors();
		g_Pal32ToPal24(pBitmap->GetPal32(), m_Pal24, m_nColors);
		g_Pal24ToPal16(m_Pal24, m_Pal16, m_nColors);
	}
	else
	{
		if (m_nWidth < pBitmap->GetWidth())
			m_nWidth = pBitmap->GetWidth();
		if (m_nHeight < pBitmap->GetHeight())
			m_nHeight = pBitmap->GetHeight();
		if (m_nColors < pBitmap->GetColors())
			return FALSE;
	}
	
	// copy frame data
	KSprNode* pNode = (KSprNode*)g_MemManager.Calloc(sizeof(KSprNode)
		+ SpriteCodec.GetLength());
	pNode->m_nSize = SpriteCodec.GetLength();
	g_MemCopy(pNode->m_Sprite, SpriteCodec.GetSprite(), SpriteCodec.GetLength());
	m_FrameList.AddTail(pNode);
	
	// inc frame number
	m_nFrames++;
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Del Frame
// 功能:	删除图象帧
// 参数:	nFrame	帧
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KSpriteMaker::DelFrame(int nFrame)
{
	KSprNode* pNode = (KSprNode*)m_FrameList.GetHead();
	while (pNode && nFrame > 0)
	{
		pNode = (KSprNode*)pNode->GetNext();
		nFrame--;
	}
	if (pNode)
	{
		pNode->Remove();
		g_MemManager.Free(pNode);
		m_nFrames--;
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Get Frame
// 功能:	取得图象帧
// 参数:	nFrame	帧
// 返回:	KSprNode*
//---------------------------------------------------------------------------
KSprNode* KSpriteMaker::GetFrame(int nFrame)
{
	KSprNode* pNode = (KSprNode*)m_FrameList.GetHead();
	while (pNode && nFrame > 0)
	{
		pNode = (KSprNode*)pNode->GetNext();
		nFrame--;
	}
	return pNode;
}
//---------------------------------------------------------------------------
// 函数:	Make Offset
// 功能:	生产偏移表
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KSpriteMaker::MakeOffset()
{
	m_Offset.Alloc(m_nFrames * sizeof(SPROFFS));
	long nOffset = 0;
	SPROFFS* pOffset = (SPROFFS*)m_Offset.GetMemPtr();
	KSprNode* pNode = (KSprNode*)m_FrameList.GetHead();
	for (int  i = 0; i < m_nFrames; i++)
	{
		pOffset[i].Offset = nOffset;
		pOffset[i].Length = pNode->m_nSize;
		nOffset += pNode->m_nSize;
		pNode = (KSprNode*)pNode->GetNext();
	}
}
//---------------------------------------------------------------------------
