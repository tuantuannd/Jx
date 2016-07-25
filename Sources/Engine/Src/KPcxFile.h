//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPcxFile.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KPcxFile_H
#define KPcxFile_H
//---------------------------------------------------------------------------
#include "KBitmap.h"
//---------------------------------------------------------------------------
class ENGINE_API KPcxFile
{
private:
	long		m_nWidth;
	long		m_nHeight;

private:
	void		Decode(void* lpDes, void* lpSrc);
	void		Encode(void* lpDes, void* lpSrc);

public:
	BOOL		Load(LPSTR lpFileName, KBitmap* lpBitmap);
	BOOL		Save(LPSTR lpFileName, KBitmap* lpBitmap);
};
//---------------------------------------------------------------------------
#endif
