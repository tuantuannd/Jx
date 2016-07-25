//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KWavSound.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Wav Sound Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KDSound.h"
#include "KDError.h"
#include "KWavFile.h"
#include "KWavSound.h"
//---------------------------------------------------------------------------
// 函数:	KWavSound
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KWavSound::KWavSound()
{
	g_MemZero(m_Buffer, sizeof(m_Buffer));
}
//---------------------------------------------------------------------------
// 函数:	~KWavSound
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KWavSound::~KWavSound()
{
	Free();
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	载入音效
// 参数:	FileName	文件名
// 返回:	音效索引
//---------------------------------------------------------------------------
BOOL KWavSound::Load(LPSTR FileName)
{
	LPDIRECTSOUND	pDirectSound;
	KWavFile		WavFile;
	DSBUFFERDESC	desc;
	HRESULT 		hres;
	WAVEFORMATEX	PcmWavFmt;

	// get direct sound
	pDirectSound = g_pDirectSound->GetDirectSound();
	if (pDirectSound == NULL)
		return FALSE;

	// open the file and get ready to read
	if (!WavFile.Open(FileName))
		return FALSE;

	// setup ddes structure
	g_MemZero(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | 
		DSBCAPS_LOCSOFTWARE;
	desc.dwBufferBytes = WavFile.GetWaveSize();
	WavFile.GetPcmWavFormat(&PcmWavFmt);
	desc.lpwfxFormat = &PcmWavFmt;

	// create the secondry sound buffer
	hres = pDirectSound->CreateSoundBuffer(&desc, &m_Buffer[0], NULL);
	if (hres != DS_OK)
		return FALSE;

	LPVOID	lpAudio;
	DWORD	dwBytes;
	// lock the sound buffer to get buffer pointer
	hres = m_Buffer[0]->Lock(0, WavFile.GetWaveSize(),
		&lpAudio, &dwBytes, NULL, NULL, 0);
	if (hres != DS_OK)
		return FALSE;

	// read the data from the file into the buffer
	WavFile.Read((PBYTE)lpAudio, dwBytes);

	// unlock the sound buffer
	hres = m_Buffer[0]->Unlock(lpAudio, dwBytes, NULL, 0);
	if (hres != DS_OK)
		return FALSE;

	// duplicate sound buffer
	for (int i = 1; i < BUFFER_COUNT; i++)
	{
		pDirectSound->DuplicateSoundBuffer(m_Buffer[0], &m_Buffer[i]);
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Free
// 功能:	释放音效
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KWavSound::Free()
{
	if (g_pDirectSound->GetDirectSound() == NULL)
		return;
	for (int i = BUFFER_COUNT - 1; i >= 0; i--)
	{
		SAFE_RELEASE(m_Buffer[i]);
	}
}
//---------------------------------------------------------------------------
// 函数:	GetFreeBuffer
// 功能:	取得空闲的SoundBuffer
// 参数:	void
// 返回:	Sound Buffer Index
//---------------------------------------------------------------------------
int KWavSound::GetFreeBuffer()
{
	DWORD	Status = 0;
	int 	i;

	for (i = 0; i < BUFFER_COUNT; i++)
	{
		if (m_Buffer[i] == NULL)
			continue;
		
		m_Buffer[i]->GetStatus(&Status);

		if (Status & DSBSTATUS_BUFFERLOST)
			//m_Buffer[i]->Restore();
			return BUFFER_COUNT;

		if ((Status & DSBSTATUS_PLAYING) == 0)
			break;
	}
	return i;
}
//---------------------------------------------------------------------------
// 函数:	GetPlayBuffer
// 功能:	取得正在播放的SoundBuffer
// 参数:	void
// 返回:	Sound Buffer Index
//---------------------------------------------------------------------------
int KWavSound::GetPlayBuffer()
{
	DWORD	Status = 0;
	int 	i;

	for (i = 0; i < BUFFER_COUNT; i++)
	{
		if (m_Buffer[i] == NULL)
			continue;
		
		m_Buffer[i]->GetStatus(&Status);

		if (Status & DSBSTATUS_BUFFERLOST)
			//m_Buffer[i]->Restore();
			return BUFFER_COUNT;

		if (Status & DSBSTATUS_PLAYING)
			break;
	}
	return i;
}
//---------------------------------------------------------------------------
// 函数:	Play
// 功能:	播放音效
// 参数:	nPan		声相 [-10000,10000]
//				 		SBPAN_LEFT = -10000
//						DSBPAN_CENTER = 0
//						DSBPAN_RIGHT = 10000
//			nVolume		音量 [0, -10000]
//					  	DSBVOLUME_MAX = 0
//						DSBVOLUME_MIN = -10000
//			bLoop		循环 [0,1]
//						0 : 不循环播放
//						1 : 循环播放
// 返回:	void
//---------------------------------------------------------------------------
void KWavSound::Play(int nPan, int nVolume, BOOL bLoop)
{
	int i = GetFreeBuffer();
	if (i >= BUFFER_COUNT)
		return;
	m_Buffer[i]->SetCurrentPosition(0);
	m_Buffer[i]->SetPan(nPan);
	m_Buffer[i]->SetVolume(nVolume);
	m_Buffer[i]->Play(0, 0, (bLoop)? DSBPLAY_LOOPING : 0);
}
//---------------------------------------------------------------------------
// 函数:	Stop
// 功能:	停止播放声效
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KWavSound::Stop()
{
	int i = GetPlayBuffer();
	if (i >= BUFFER_COUNT)
		return;
	m_Buffer[i]->Stop();
}
//---------------------------------------------------------------------------
// 函数:	SetPan
// 功能:	设置声相
// 参数:	nPan		声相 [-10000,10000]
//				 		SBPAN_LEFT = -10000
//						DSBPAN_CENTER = 0
//						DSBPAN_RIGHT = 10000
// 返回:	void
//---------------------------------------------------------------------------
void KWavSound::SetPan(int nPan)
{
	int i = GetPlayBuffer();
	if (i >= BUFFER_COUNT)
		return;
	m_Buffer[i]->SetPan(nPan);
}
//---------------------------------------------------------------------------
// 函数:	SetVolume
// 功能:	设置音量
// 参数:	nVolume		音量 [0, -10000]
//					  	DSBVOLUME_MAX = 0
//						DSBVOLUME_MIN = -10000
// 返回:	void
//---------------------------------------------------------------------------
void KWavSound::SetVolume(int nVolume)
{
	int i = GetPlayBuffer();
	if (i >= BUFFER_COUNT)
		return;
	m_Buffer[i]->SetVolume(nVolume);
}
//---------------------------------------------------------------------------
// 函数:	IsPlaying
// 功能:	是否在播放
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KWavSound::IsPlaying()
{
	int i = GetPlayBuffer();
	return (i < BUFFER_COUNT);
}
//---------------------------------------------------------------------------
