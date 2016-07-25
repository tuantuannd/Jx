//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KLMp3Music.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KLMp3Music_H
#define KLMp3Music_H
//---------------------------------------------------------------------------
#include "KLMusic.h"
#include "KLCFile.h"
//---------------------------------------------------------------------------
class KLMp3Music : public KLMusic
{
private:
	KLCFile			m_Mp3File;
protected:
	PBYTE			mp3_buffer;
	PBYTE			mp3_bufptr;
	int				mp3_trigger;
	int				mp3_bufbytes;
	int				mp3_frmbytes;
	BOOL			Mp3Init();
	BOOL			Mp3FillBuffer();
	DWORD			Mp3Decode(PBYTE lpPcmBuf, DWORD dwBufLen);
protected:
	virtual BOOL	Mp3FileOpen(LPSTR FileName);
	virtual DWORD	Mp3FileRead(PBYTE pBuf, DWORD dwLen);
	virtual DWORD	Mp3FileSeek(LONG lOffset);
public:
	virtual BOOL	Open(LPSTR FileName);
	virtual void	Close();
	virtual void	Seek(int nPercent);
	virtual void	Rewind();
protected:
	void			InitSoundFormat();
	DWORD			ReadWaveData(LPBYTE lpBuf, DWORD dwLen);
};
//---------------------------------------------------------------------------
#endif