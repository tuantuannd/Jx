//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMpgMusic.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMpgMusic_H
#define KMpgMusic_H
//---------------------------------------------------------------------------
#include "KMusic.h"
#include "KMemClass.h"
#include "KPakFile.h"
//---------------------------------------------------------------------------
class ENGINE_API KMpgMusic : public KMusic
{
private:
	KPakFile	m_Mp3File;
	KMemClass	m_Mp3Struct;
	KMemClass	m_Mp3Buffer;
	PBYTE	mp3_buffer;
	PBYTE	mp3_bufptr;
	long	mp3_bufbytes;
	BOOL	Mp3Init();
	BOOL	Mp3FillBuffer();
	DWORD	Mp3Decode(PBYTE lpPcmBuf, DWORD dwBufLen);
public:
	virtual BOOL	Open(LPSTR FileName);
	virtual void	Seek(int nPercent);
	virtual void	Rewind();
protected:
	virtual BOOL	Mp3FileOpen(LPSTR FileName);
	virtual DWORD	Mp3FileRead(PBYTE pBuf, DWORD dwLen);
	virtual DWORD	Mp3FileSeek(LONG lOffset);
	virtual void	InitSoundFormat();
	virtual DWORD	ReadWaveData(LPBYTE lpBuf, DWORD dwLen);
};
//---------------------------------------------------------------------------
#endif