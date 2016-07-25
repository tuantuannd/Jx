//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpgFile.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Gif file read class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDDraw.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KPakFile.h"
#include "JpgLib.h"
#include "KJpgFile.h"
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	载入一个JPEG文件
// 参数:	FileName	文件名
//			lpBitmap	位图指针
//          uRGBMask16  指定 16 位色的MASK值，如果是-1, 表示使用从DirectDraw中取值
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KJpgFile::Load(LPSTR FileName, KBitmap16* lpBitmap, unsigned uRGBMask16)
{
	KPakFile	File;
	PBYTE		pJpg;
	BOOL		bRGB555;
	JPEG_INFO	JpegInfo;

	// open the file
	if (!File.Open(FileName))
		return FALSE;

	// allocate mem buffer for file
	if (!m_Buffer.Alloc(File.Size()))
		return FALSE;
	pJpg = (PBYTE)m_Buffer.GetMemPtr();
	
	// read file into mem buffer
	File.Read(pJpg, File.Size());

    if (uRGBMask16 == ((unsigned)-1))
    {
    	bRGB555 = (g_pDirectDraw->GetRGBBitMask16() == RGB_555) ? TRUE : FALSE;
    }
    else
    {
        bRGB555 = (uRGBMask16 == RGB_555) ? TRUE : FALSE;
    }

	// decode init
	if (!jpeg_decode_init(bRGB555, TRUE))
		return FALSE;

	// decode info
	if (!jpeg_decode_info(pJpg, &JpegInfo))
		return FALSE;

	// create bitmap
	if (!lpBitmap->Init(JpegInfo.width, JpegInfo.height))
		return FALSE;

	// decode frame
	if (!jpeg_decode_data((PWORD)lpBitmap->GetBitmap(), &JpegInfo))
		return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
