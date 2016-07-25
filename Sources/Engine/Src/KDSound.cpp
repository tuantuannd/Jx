//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDSound.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Direct Sound Related Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KWin32Wnd.h"
#include "KDError.h"
#include "KDSound.h"
//---------------------------------------------------------------------------
ENGINE_API KDirectSound* g_pDirectSound = NULL;
//---------------------------------------------------------------------------
// 函数:	KDirectSound
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KDirectSound::KDirectSound()
{
	g_pDirectSound			= this;
	m_pDirectSound			= NULL;
	m_pPrimarySoundBuffer	= NULL;
}
//---------------------------------------------------------------------------
// 函数:	~KDirectSound
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KDirectSound::~KDirectSound()
{
	Exit();
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	初始化DirectSound
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectSound::Init()
{
	// free prior object
	if (m_pDirectSound != NULL)
		Exit();

	// create direct sound
	if (!CreateDirectSound())
		return FALSE;

	// create primary sound buffer
	if (!CreatePrimarySoundBuffer())
		return FALSE;

	g_DebugLog("DirectSound init ok");
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Exit
// 功能:	释放DirectSound
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectSound::Exit()
{
	SAFE_RELEASE(m_pPrimarySoundBuffer);
	SAFE_RELEASE(m_pDirectSound);
	g_DebugLog("DirectSound release ok");
}
//---------------------------------------------------------------------------
// 函数:	CreateDirectSound
// 功能:	创建DirectSound
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectSound::CreateDirectSound()
{
	HRESULT		hres;

	// create direct sound
	hres = DirectSoundCreate(NULL, &m_pDirectSound, NULL);
	if (hres != DS_OK)
	{
		g_DebugLog("DirectSound : DirectSoundCreate() Failed!");
		return FALSE;
	}
	
	// set cooperative level
	hres = m_pDirectSound->SetCooperativeLevel(g_GetMainHWnd(), DSSCL_PRIORITY);
	if (hres != DS_OK)
	{
		g_DebugLog("DirectSound : SetCooperativeLevel() Failed!");
		return FALSE;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	CreatePrimarySoundBuffer
// 功能:	创建主声音缓冲
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectSound::CreatePrimarySoundBuffer()
{
	HRESULT			hres;
	DSBUFFERDESC	desc;
	WAVEFORMATEX	wfx;
	
	// setup primary sound buffer desc
	g_MemZero(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

	// create primary sound buffer
	hres = m_pDirectSound->CreateSoundBuffer(&desc, &m_pPrimarySoundBuffer, NULL);
	if (hres != DS_OK)
	{
		g_MessageBox("DirectSound : Create Primary Sound Buffer Failed");
		return FALSE;
	}
	
	// set wave format structure
	g_MemZero(&wfx, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 22050;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (WORD)(wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	// set primary sound buffer format
	hres = m_pPrimarySoundBuffer->SetFormat(&wfx);
	if (hres != DS_OK)
	{
		g_MessageBox("DirectSound : Set Primary Sound Buffer Format Failed");
		return FALSE;
	}

	// play primary sound buffer
	// m_pPrimarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

	return TRUE;
}
//---------------------------------------------------------------------------
