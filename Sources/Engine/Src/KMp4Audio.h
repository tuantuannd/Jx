//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp4Audio.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMp4Audio_H
#define KMp4Audio_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KAviFile.h"
#include "KMp3Music.h"
//---------------------------------------------------------------------------
class ENGINE_API KMp4Audio : public KMp3Music
{
private:
	KAviFile		m_AviFile;
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