//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpgFile.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KJpgFile_H
#define KJpgFile_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KBitmap16.h"
//---------------------------------------------------------------------------
class ENGINE_API KJpgFile
{
private:
	KMemClass	m_Buffer;
public:
	BOOL		Load(LPSTR lpFileName, KBitmap16* lpBitmap, unsigned uRGBMask16 = ((unsigned)-1));
};
//---------------------------------------------------------------------------
#endif