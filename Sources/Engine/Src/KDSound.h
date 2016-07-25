//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDSound.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KDSound_H
#define KDSound_H
//---------------------------------------------------------------------------
class ENGINE_API KDirectSound
{
private:
	LPDIRECTSOUND			m_pDirectSound;
	LPDIRECTSOUNDBUFFER		m_pPrimarySoundBuffer;
public:
	KDirectSound();
	~KDirectSound();
	BOOL					Init();
	void					Exit();
	LPDIRECTSOUND			GetDirectSound(){return m_pDirectSound;};
private:
	BOOL					CreateDirectSound();
	BOOL					CreatePrimarySoundBuffer();
};
extern ENGINE_API KDirectSound* g_pDirectSound;
//---------------------------------------------------------------------------
#endif