//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KBmpFile.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	256c BMP file class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KPakFile.h"
#include "KPalette.h"
#include "KBmpFile.h"
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	打开BMP文件
// 参数:	lpFileName	文件名
//			lpBitmap	8bit位图
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KBmpFile::Load(LPSTR lpFileName, KBitmap* lpBitmap)
{
	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;
	KPakFile			File;
	KMemClass			Buffer;

	// open the file
	if (!File.Open(lpFileName))
		return FALSE;
	
	// read file head
	File.Read(&FileHeader, sizeof(FileHeader));
	
	// check BMP flag "BM"
	if (FileHeader.bfType != 0x4d42)
		return FALSE;
	
	// read info head
	File.Read(&InfoHeader, sizeof(InfoHeader));
	
	// check compression
	if (InfoHeader.biCompression != 0)
		return FALSE;

	// check bit count
	if (InfoHeader.biBitCount != 8)
		return FALSE;
	
	// setup bitmap width,height
	m_nWidth  = InfoHeader.biWidth;
	m_nHeight = InfoHeader.biHeight;
	
	// byte per line % 4 must = 0
	m_nBytesPerLine = InfoHeader.biWidth;
	if ((m_nBytesPerLine % 4) != 0)
		m_nBytesPerLine = m_nBytesPerLine + 4 - (m_nBytesPerLine % 4);
	
	// get color used by bitmap
	m_nColors = InfoHeader.biClrUsed;
	if (m_nColors == 0)
		m_nColors = 256;

	// init bitmap
	if (!lpBitmap->Init(m_nWidth, m_nHeight, m_nColors))
		return FALSE;

	// read 32 bit palette data
	File.Read(lpBitmap->GetPal32(), m_nColors * sizeof(KPAL32));
	lpBitmap->MakePalette();

	// allocate buffer for bitmap
	if (!Buffer.Alloc(m_nBytesPerLine * m_nHeight))
		return FALSE;

	// set file pointer to data begin
	File.Seek(FileHeader.bfOffBits, FILE_BEGIN);

	// read bitmap bits data
	File.Read(Buffer.GetMemPtr(), m_nBytesPerLine * m_nHeight);

	// decode bitmap
	Decode(lpBitmap->GetBitmap(), Buffer.GetMemPtr());
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存BMP文件
// 参数:	lpFileName	文件名
//			lpBitmap	8bit位图
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KBmpFile::Save(LPSTR lpFileName, KBitmap* lpBitmap)
{
	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;
	KFile				File;
	DWORD				OffBits;
	KMemClass			Buffer;

	// get width and height
	m_nWidth  = lpBitmap->GetWidth();
	m_nHeight = lpBitmap->GetHeight();
	m_nColors = lpBitmap->GetColors();

	// byte per line % 4 must = 0
	m_nBytesPerLine = m_nWidth;
	if ((m_nBytesPerLine % 4) != 0)
		m_nBytesPerLine = m_nBytesPerLine + 4 - (m_nBytesPerLine % 4);

	// offset in bytes
	OffBits = sizeof(BITMAPFILEHEADER)
			+ sizeof(BITMAPINFOHEADER)
			+ sizeof(RGBQUAD) * m_nColors;
	
	// file header
	FileHeader.bfType          = 0x4d42; // "BM"
	FileHeader.bfSize          = m_nBytesPerLine * m_nHeight + OffBits;
	FileHeader.bfReserved1     = 0;
	FileHeader.bfReserved2     = 0;
	FileHeader.bfOffBits       = OffBits;
	
	// info header
	InfoHeader.biSize          = sizeof(BITMAPINFOHEADER);
	InfoHeader.biWidth         = m_nWidth;
	InfoHeader.biHeight        = m_nHeight;
	InfoHeader.biPlanes        = 1;
	InfoHeader.biBitCount      = 8;
	InfoHeader.biCompression   = 0;
	InfoHeader.biSizeImage     = 0;
	InfoHeader.biXPelsPerMeter = 0xb40;
	InfoHeader.biYPelsPerMeter = 0xb40;
	InfoHeader.biClrUsed       = m_nColors;
	InfoHeader.biClrImportant  = 0;
	
	// allocate buffer for bitmap
	if (!Buffer.Alloc(m_nBytesPerLine * m_nHeight))
		return FALSE;

	// Encode bitmap
	Encode(Buffer.GetMemPtr(), lpBitmap->GetBitmap());

	// crete the file
	if (!File.Create(lpFileName))
		return FALSE;

	// write file head
	File.Write(&FileHeader, sizeof(FileHeader));
	
	// write info head
	File.Write(&InfoHeader, sizeof(InfoHeader));

	// write palette
	File.Write(lpBitmap->GetPal32(), m_nColors * sizeof(KPAL32));

	// write bitmap bits data
	File.Write(Buffer.GetMemPtr(), m_nBytesPerLine * m_nHeight);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	解码
// 参数:	lpDes	目的
//			lpSrc	源
// 返回:	void
//---------------------------------------------------------------------------
void KBmpFile::Decode(void* lpDes, void* lpSrc)
{
	LPBYTE lpDesByte = (LPBYTE)lpDes;
	LPBYTE lpSrcByte = (LPBYTE)lpSrc;

	lpSrcByte += (m_nHeight - 1) * m_nBytesPerLine;
	for (int i = 0; i < m_nHeight; i++)
	{
		g_MemCopy(lpDesByte, lpSrcByte, m_nWidth);
		lpSrcByte -= m_nBytesPerLine;
		lpDesByte += m_nWidth;
	}
}
//---------------------------------------------------------------------------
// 函数:	Encode
// 功能:	编码
// 参数:	lpDes	目的
//			lpSrc	源
// 返回:	void
//---------------------------------------------------------------------------
void KBmpFile::Encode(void* lpDes, void* lpSrc)
{
	LPBYTE lpDesByte = (LPBYTE)lpDes;
	LPBYTE lpSrcByte = (LPBYTE)lpSrc;

	lpDesByte += (m_nHeight - 1) * m_nBytesPerLine;
	for (int i = 0; i < m_nHeight; i++)
	{
		g_MemCopy(lpDesByte, lpSrcByte, m_nWidth);
		lpDesByte -= m_nBytesPerLine;
		lpSrcByte += m_nWidth;
	}
}
//---------------------------------------------------------------------------
