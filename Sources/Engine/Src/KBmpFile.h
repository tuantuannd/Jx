//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KBmpFile.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KBmpFile_H
#define KBmpFile_H
//---------------------------------------------------------------------------
#include "KBitmap.h"
//---------------------------------------------------------------------------
class ENGINE_API KBmpFile
{
private:
	int			m_nWidth;
	int			m_nHeight;
	int			m_nColors;
	int			m_nBytesPerLine;

private:
	void		Decode(void* lpDes, void* lpSrc);
	void		Encode(void* lpDes, void* lpSrc);

public:
	BOOL		Load(LPSTR lpFileName, KBitmap* lpBitmap);
	BOOL		Save(LPSTR lpFileName, KBitmap* lpBitmap);
};
//---------------------------------------------------------------------------
#endif
