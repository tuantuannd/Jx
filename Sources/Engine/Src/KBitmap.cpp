//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KBitmap.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	256 Color bitmap file class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KPakFile.h"
#include "KCanvas.h"
#include "KBitmap.h"
//---------------------------------------------------------------------------
// 函数:	KBitmap
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KBitmap::KBitmap()
{
	m_nWidth  = 0;
	m_nHeight = 0;
	m_nColors = 0;
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	创建新位图
// 参数:	nWidth		宽度
//			nHeight		高度
//			nColors		颜色数
// 返回:	void
//---------------------------------------------------------------------------
BOOL KBitmap::Init(int nWidth, int nHeight, int nColors)
{
	m_nWidth  = nWidth;
	m_nHeight = nHeight;
	m_nColors = nColors;
	m_Bitmap.Alloc(m_nWidth * m_nHeight);
	return (m_Bitmap.GetMemPtr() != NULL);
}
//---------------------------------------------------------------------------
// 函数:	Draw
// 功能:	绘制位图
// 参数:	nX		X坐标
//			nY		Y坐标
// 返回:	void
//---------------------------------------------------------------------------
void KBitmap::Draw(int nX, int nY)
{
	LPVOID lpBitmap = m_Bitmap.GetMemPtr();
	if (lpBitmap)
	{
		g_pCanvas->DrawBitmap(nX, nY, m_nWidth, m_nHeight, lpBitmap, m_Pal16);
	}
}
//---------------------------------------------------------------------------
// 函数:	Clear
// 功能:	填充位图
// 参数:	byColor	颜色
// 返回:	void
//---------------------------------------------------------------------------
void KBitmap::Clear(BYTE byColor)
{
	m_Bitmap.Fill(byColor);
}
//---------------------------------------------------------------------------
// 函数:	MakePalette
// 功能:	制作调色板
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KBitmap::MakePalette()
{
	g_Pal32ToPal16(m_Pal32, m_Pal16, m_nColors);
}
//---------------------------------------------------------------------------
// 函数:	PutPixel
// 功能:	画点
// 参数:	nX		X 坐标
//			nY		Y 坐标
//			byColor	颜色值
// 返回:	void
//---------------------------------------------------------------------------
void KBitmap::PutPixel(int nX, int nY, BYTE byColor)
{
//	if ((nX < 0) || (nY < 0) || (nX >= m_nWidth) || (nY >= m_nHeight))
//		return;
	PBYTE pPixel = (PBYTE)m_Bitmap.GetMemPtr();
	pPixel[nY * m_nWidth + nX] = byColor;
}
//---------------------------------------------------------------------------
// 函数:	GetPixel
// 功能:	读点
// 参数:	nX		X 坐标
//			nY		Y 坐标
// 返回:	颜色值
//---------------------------------------------------------------------------
BYTE KBitmap::GetPixel(int nX, int nY)
{
//	if ((nX < 0) || (nY < 0) || (nX >= m_nWidth) || (nY >= m_nHeight))
//		return 0;
	PBYTE pPixel = (PBYTE)m_Bitmap.GetMemPtr();
	return pPixel[nY * m_nWidth + nX];
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	载入位图
// 参数:	lpFileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KBitmap::Load(LPSTR lpFileName)
{
	KPakFile	File;
	KBMPHEADER	Header;

	File.Open(lpFileName);
	File.Read(&Header, sizeof(Header));

	if (!g_MemComp(Header.Id, "BM08", 4))
		return FALSE;
	
	if (!Init(Header.Width, Header.Height, Header.Colors))
		return FALSE;
	
	File.Read(m_Pal32, m_nColors * sizeof(KPAL32));
	File.Read(m_Bitmap.GetMemPtr(), m_nWidth * m_nHeight);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存位图
// 参数:	lpFileName	文件名
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KBitmap::Save(LPSTR lpFileName)
{
	KFile		File;
	KBMPHEADER	Header;

	File.Create(lpFileName);
	
	g_MemZero(&Header, sizeof(Header));
	g_MemCopy(Header.Id, "BM08", 4);
	Header.Width  = m_nWidth;
	Header.Height = m_nHeight;
	Header.Colors = m_nColors;

	File.Write(&Header, sizeof(Header));
	File.Write(m_Pal32, m_nColors * sizeof(KPAL32));
	File.Write(m_Bitmap.GetMemPtr(), m_nWidth * m_nHeight);

	return TRUE;
}
//---------------------------------------------------------------------------
