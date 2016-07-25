//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPcxFile.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	256 color PCX file class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KPakFile.h"
#include "KPalette.h"
#include "KPcxFile.h"
//---------------------------------------------------------------------------
typedef struct {
	BYTE		manufacturer;
	BYTE		version;
	BYTE		encoding;
	BYTE		bits_per_pixel;
	WORD		xmin, ymin;
	WORD		xmax, ymax;
	WORD		hres;
	WORD		vres;
	BYTE		palette[48];
	BYTE		reserved;
	BYTE		color_planes;
	WORD		bytes_per_line;
	WORD		palette_type;
	BYTE		filles[58];
} PCXHEADER;
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	加载PCX文件
// 参数:	lpFileName	文件名
//			lpBitmap	8bit位图
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPcxFile::Load(LPSTR lpFileName, KBitmap* lpBitmap)
{
	KPakFile	File;
	KMemClass	Buffer;
	PCXHEADER	Header;
	KPAL24		Pal24[256];

	// open the file
	if (!File.Open(lpFileName))
		return FALSE;
	
	// read file head
	File.Read(&Header, sizeof(Header));

	// check manufacturer
	if (Header.manufacturer != 0x0a || Header.version != 5)
		return FALSE;

	// check bits_per_pixel	
	if (Header.bits_per_pixel != 8)
		return FALSE;

	// pcx width & height
	m_nWidth = (Header.xmax - Header.xmin) + 1;
	m_nHeight = (Header.ymax - Header.ymin) + 1;
	
	// alloc buffer for pcx 
	DWORD dwSize = File.Size() - 128 - 768;
	if (!Buffer.Alloc(dwSize))
		return FALSE;

	// read pcx data
	File.Read(Buffer.GetMemPtr(), dwSize);
	
	// read palette data
	File.Read(Pal24, 256 * sizeof(KPAL24));
	g_Pal24ToPal32(Pal24, lpBitmap->GetPal32(), 256);
	lpBitmap->MakePalette();

	// create bitmap
	if (!lpBitmap->Init(m_nWidth, m_nHeight, 256))
		return FALSE;

	// decode pcx data
	Decode(lpBitmap->GetBitmap(), Buffer.GetMemPtr());
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存PCX文件
// 参数:	lpFileName	文件名
//			lpBitmap	8bit位图
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPcxFile::Save(LPSTR lpFileName, KBitmap* lpBitmap)
{
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	解码
// 参数:	lpDes	目的
//			lpSrc	源
// 返回:	void
//---------------------------------------------------------------------------
void KPcxFile::Decode(void* lpDes, void* lpSrc)
{
	BYTE key, num;
	int  w, h;
	LPBYTE lpDesByte = (LPBYTE)lpDes;
	LPBYTE lpSrcByte = (LPBYTE)lpSrc;

	for (h = 0; h < m_nHeight; h++)
	{
		w = 0;
		while (w < m_nWidth)
		{
			key = *lpSrcByte++;
			num = 1;
			if ((key & 0xc0) == 0xc0)
			{
				num = key & 0x3f;
				key = *lpSrcByte++;
			}
			while (num > 0)
			{
				if (w < m_nWidth)
				{
					*lpDesByte++ = key;
					w++;
				}
				num--;
			}
		}
	}
}
//---------------------------------------------------------------------------
// 函数:	Encode
// 功能:	编码
// 参数:	lpDes	目的
//			lpSrc	源
// 返回:	void
//---------------------------------------------------------------------------
void KPcxFile::Encode(void* lpDes, void* lpSrc)
{
}
