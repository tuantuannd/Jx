//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMusic.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMusic_H
#define KMusic_H
//---------------------------------------------------------------------------
#define NUMEVENTS	3
//---------------------------------------------------------------------------
#include "KThread.h"
//---------------------------------------------------------------------------
class ENGINE_API KMusic
{
protected:
	WAVEFORMATEX			m_WaveFormat;
	LPDIRECTSOUNDBUFFER 	m_pSoundBuffer;
	LPDIRECTSOUNDNOTIFY 	m_pSoundNotify;
	DSBPOSITIONNOTIFY		m_PosNotify[NUMEVENTS];
	HANDLE					m_hEvent[NUMEVENTS];
	DWORD					m_dwBufferSize;
	BOOL					m_bLoop;
	KThread					m_Thread;
public:
	KMusic();
	virtual ~KMusic();
	virtual BOOL			Open(LPSTR FileName);
	virtual void			Close();
	virtual void			Play(BOOL bLoop = FALSE);
	virtual void			Stop();
	virtual void			Rewind();
	virtual void			SetVolume(LONG lVolume);
	virtual BOOL			HandleNotify();
	virtual BOOL			IsPlaying();
protected:
	virtual BOOL			Init();
	virtual void			InitSoundFormat();
	virtual BOOL			InitSoundBuffer();
	virtual void			FreeSoundBuffer();
	virtual BOOL			InitSoundNotify();
	virtual void			FreeSoundNotify();
	virtual BOOL			InitEventHandle();
	virtual void			FreeEventHandle();
	virtual BOOL			FillBufferWithSound(DWORD dwPos);
	virtual BOOL			FillBufferWithSilence();
	virtual DWORD			ReadWaveData(LPBYTE lpBuf, DWORD dwLen);
	static	void			ThreadFunction(void* lpParam);
};
//---------------------------------------------------------------------------
#endif