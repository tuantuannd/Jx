//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2002 by Kingsoft
// File:	KFont.h
// Date:	2000.08.08
// Code:	Daniel Wang, Wooy
// Desc:	目前支持Chinese GBK and Chinese GB2132两种编码字符集
//---------------------------------------------------------------------------
#ifndef KFont_H
#define KFont_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KBitmap.h"
//---------------------------------------------------------------------------

//使用的编码字符集定义
enum CHARASET
{
	CHARASET_GBK	= 1,
	CHARASET_GB2132,
	//CHARASET_BIG5,
};

//#define CHARASET_GBK_MAXFONT		23940			//For GBK (0xFE-0x80)*(0xFE-0x3F-1)=23940
//#define CHARASET_GB2132_MAXFONT	94 * 94			//for GB2132 = 35 KB
//#define CHARASET_BIG5_MAXFONT		94 * 190		// for BIG5 = 70 KB

#define FONT_SOLID    0x00
#define FONT_ALPHA    0x01
      
//---------------------------------------------------------------------------
typedef struct _FontHead {
	char		Id[4];				// 标识
	DWORD		Size;				// 大小
	DWORD		Count;				// 数量
	WORD		Width;				// 宽度
	WORD		Height;				// 高度
} TFontHead;
//---------------------------------------------------------------------------
class ENGINE_API KFont
{
protected:
	KMemClass	m_Font;				// font data
	DWORD*		m_pdwOffs;			// offset table
	int			m_nOffsetSpace;		// offset buff 包含DWORD元素的数目
	DWORD		m_dwSize;			// size of font data
	int			m_nCount;			// count of word
	int			m_nFontW;			// width of font
	int			m_nFontH;			// height of font
	int			m_nLineW;			// width of line
	int			m_nLineH;			// height of line
	DWORD		m_dwColor;			// color of text
	int         m_TextStyle;        //文字风格
	CHARASET	m_CharaSet;
protected:
	int		GetFontRect(int nX, int nY, KBitmap* pBmp, PBYTE pBuf);
	int		GetFontLine(int nX, int nY, KBitmap* pBmp, PBYTE pBuf);
	BOOL	AddWord(int nSize, LPBYTE pBuf);
	void	DrawFont(int nIndex, int nX, int nY);
	void	DrawFontAlpha(int nIndex, int nX, int nY, int nAlpha);
public:
	KFont();
	~KFont();
	BOOL	Load(LPSTR FileName, CHARASET CharaSet);
	void	SetCharaset(CHARASET CharaSet);
	void	Clear();
	BOOL	Save(LPSTR FileName);	
	void	DrawWord(int nX, int nY, int nH, int nL);
	void	DrawText(int nX, int nY, LPSTR lpText);
	void	DrawTextAlpha(int nX, int nY, int nAlpha,LPSTR lpText);
	void	SetColor(BYTE Red, BYTE Green, BYTE Blue);
	void	SetLine(int nWidth, int nHeight);
	void	MakeText(LPSTR FileName);
	BOOL	MakeFont(LPSTR FileName, int nWidth, int nHeight);
	int		GetWidth(){ return m_nFontW; };
	int		GetHeight(){ return m_nFontH; };
    void	SetTextStyle(int nTextStyle) { m_TextStyle=nTextStyle; };
};
//---------------------------------------------------------------------------
#endif

