//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KLMusic.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KLMusic_H
#define KLMusic_H
//---------------------------------------------------------------------------
#define NUMEVENTS			3
//---------------------------------------------------------------------------
#include "KLThread.h"
//---------------------------------------------------------------------------
class KLMusic
{
protected:
	LPDIRECTSOUND8			m_pDS;
	WAVEFORMATEX			m_WaveFormat;
	LPDIRECTSOUNDBUFFER 	m_pSoundBuffer;
	LPDIRECTSOUNDNOTIFY 	m_pSoundNotify;
	DSBPOSITIONNOTIFY		m_PosNotify[NUMEVENTS];
	HANDLE					m_hEvent[NUMEVENTS];
	DWORD					m_dwBufferSize;
	BOOL					m_bLoop;
	KLThread				m_Thread;
	HWND					m_hWnd;
public:
	KLMusic();
	~KLMusic();
	virtual BOOL			Open(LPSTR FileName);
	virtual void			Close();
	virtual void			Play(BOOL bLoop = FALSE);
	virtual void			Stop();
	virtual void			Rewind();
	virtual void			SetVolume(LONG lVolume);
	virtual BOOL			HandleNotify();
	void					SetWinHandle(HWND h){m_hWnd = h;}
protected:
	virtual BOOL			Init();
	bool					InitDSound();
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