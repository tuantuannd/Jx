//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSprite.h
// Date:	2000.09.18
// Code:	WangWei(Daphnis),Wooy
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KSprite_H
#define KSprite_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KPalette.h"
//---------------------------------------------------------------------------
typedef struct
{
	BYTE	Comment[4];	// 注释文字(SPR\0)
	WORD	Width;		// 图片宽度
	WORD	Height;		// 图片高度
	WORD	CenterX;	// 重心的水平位移
	WORD	CenterY;	// 重心的垂直位移
	WORD	Frames;		// 总帧数
	WORD	Colors;		// 颜色数
	WORD	Directions;	// 方向数
	WORD	Interval;	// 每帧间隔（以游戏帧为单位）
	WORD	Reserved[6];// 保留字段（到以后使用）
} SPRHEAD;

#define	SPR_COMMENT_FLAG				0x525053	//'SPR'

//---------------------------------------------------------------------------
typedef struct
{
	DWORD	Offset;		// 每一帧的偏移
	DWORD	Length;		// 每一帧的长度
} SPROFFS;
//---------------------------------------------------------------------------
typedef struct
{
	WORD	Width;		// 帧最小宽度
	WORD	Height;		// 帧最小高度
	WORD	OffsetX;	// 水平位移（相对于原图左上角）
	WORD	OffsetY;	// 垂直位移（相对于原图左上角）
	BYTE	Sprite[1];	// RLE压缩图形数据
} SPRFRAME;
//---------------------------------------------------------------------------
class ENGINE_API KSprite
{
private:
	KMemClass 	m_Buffer;
	KMemClass 	m_Palette;
	KPAL24*		m_pPal24;
	KPAL16*		m_pPal16;
	SPROFFS* 	m_pOffset;
	PBYTE		m_pSprite;
	int			m_nWidth;
	int			m_nHeight;
	int			m_nCenterX;
	int			m_nCenterY;
	int			m_nFrames;
	int			m_nColors;
	int			m_nDirections;
	int			m_nInterval;
	int			m_nColorStyle;
	int			m_nLum;
public:
	KSprite();
	BOOL		Load(LPSTR FileName);
	BOOL		LoadFor3D(LPSTR FileName);
	void		Free();
	void		MakePalette();
	void		Make4444Palette();
	void		MakePaletteLum32();
	void		SetColorStyle(int nStyle, int nLum);
	void		SetStyle(int nStyle);
	void		Draw(int nX, int nY, int nFrame);
	void        DrawCenter(int nX, int nY, int nFrame); 
	void		DrawMixColor(int nX, int nY, int nFrame,int nColor,int nAlpha);
	void		DrawWithColor(int nX, int nY, int nFrame,int nColor,int nAlpha);
	void		DrawAlpha(int nX, int nY, int nFrame, int nExAlpha = 32);
    void		DrawAlphaCenter(int nX, int nY,int nFrame, int nExAlpha = 32);
	void		DrawTrans(int nX, int nY, int nFrame);
	void		DrawBorder(int nX, int nY, int nFrame, int nColor);
	int			NextFrame(int nFrame);
	int			GetWidth(){ return m_nWidth; };
	int			GetHeight(){ return m_nHeight; };
	int			GetCenterX(){ return m_nCenterX; };
	int			GetCenterY(){ return m_nCenterY; };
	int			GetFrames(){ return m_nFrames; };
	PVOID		GetFrame(int nFrame);
	int			GetDirections(){ return m_nDirections; };
	int			GetInterval(){ return m_nInterval; };
	int			GetPixelAlpha(int nFrame, int x, int y);
	PVOID		GetPalette(){return m_Palette.GetMemPtr();}	;
	PVOID		Get24Palette(){return this->m_pPal24;};
};
//---------------------------------------------------------------------------
#endif
