//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp4Audio.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef __KMp4Audio_H__
#define __KMp4Audio_H__
//---------------------------------------------------------------------------
#include "KLAviFile.h"
#include "KLMp3Music.h"
//---------------------------------------------------------------------------
class KLMp4Audio : public KLMp3Music
{
private:
	KLAviFile		m_AviFile;
public:
	void			Seek(int nPercent);
	void			Rewind();
protected:
	BOOL			Mp3FileOpen(LPSTR FileName);
	DWORD			Mp3FileRead(PBYTE pBuf, DWORD dwLen);
	DWORD			Mp3FileSeek(LONG lOffset);
};
//---------------------------------------------------------------------------
#endif