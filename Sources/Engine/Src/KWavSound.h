//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KWavSound.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KWavSound_H
#define KWavSound_H
//---------------------------------------------------------------------------
#define BUFFER_COUNT		3	// sound buffer count
//---------------------------------------------------------------------------
class ENGINE_API KWavSound
{
private:
	LPDIRECTSOUNDBUFFER 	m_Buffer[BUFFER_COUNT];
	int						GetFreeBuffer();
	int						GetPlayBuffer();
public:
	KWavSound();
	~KWavSound();
	BOOL					Load(LPSTR FileName);
	void					Free();
	void					Play(int nPan, int nVol, BOOL bLoop);
	void					Stop();
	void					SetPan(int nPan);
	void					SetVolume(int nVolume);
	BOOL					IsPlaying();
};
//---------------------------------------------------------------------------
#endif