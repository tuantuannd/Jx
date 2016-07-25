//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp3Music.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMp3Music_H
#define KMp3Music_H
//---------------------------------------------------------------------------
#include "KMusic.h"
#include "KMemClass.h"
#include "KPakFile.h"
//---------------------------------------------------------------------------
class ENGINE_API KMp3Music : public KMusic
{
private:
	KPakFile  m_Mp3File;
	KMemClass m_Mp3Buffer;
protected:
	PBYTE	mp3_buffer;
	PBYTE	mp3_bufptr;
	int		mp3_bufbytes;
	int		mp3_frmbytes;
	virtual BOOL  Mp3Init();
	virtual BOOL  Mp3FillBuffer();
	virtual DWORD Mp3Decode(PBYTE lpPcmBuf, DWORD dwBufLen);
	virtual BOOL  Mp3FileOpen(LPSTR FileName);
	virtual DWORD Mp3FileRead(PBYTE pBuf, DWORD dwLen);
	virtual DWORD Mp3FileSeek(LONG lOffset);
	virtual void  InitSoundFormat();
	virtual DWORD ReadWaveData(LPBYTE lpBuf, DWORD dwLen);
public:
	virtual BOOL Open(LPSTR FileName);
	virtual void Seek(int nPercent);
	virtual void Rewind();
};
//---------------------------------------------------------------------------
#endif