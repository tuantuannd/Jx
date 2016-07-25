//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMusic.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Wave Stream Music Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KDSound.h"
#include "KMusic.h"
//---------------------------------------------------------------------------
// 函数:	KMusic
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMusic::KMusic()
{
	m_pSoundBuffer = NULL;
	m_pSoundNotify = NULL;
	m_hEvent[0]    = NULL;
	m_hEvent[1]    = NULL;
	m_hEvent[2]    = NULL;
	m_dwBufferSize = 0;
	m_bLoop        = FALSE;
}
//---------------------------------------------------------------------------
// 函数:	~KMusic
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMusic::~KMusic()
{
	Close();
}
//---------------------------------------------------------------------------
// 函数:	ThreadFunction
// 功能:	播放线程
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::ThreadFunction(void* lpParam)
{
	KMusic* pMusic = (KMusic*)lpParam;
	while (TRUE)
	{
		if (!pMusic->HandleNotify())
			break;
	}
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开音乐文件
// 参数:	FileName	WAV文件名
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::Open(LPSTR FileName)
{
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	初始化
// 参数:	void
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::Init()
{
	// init sound format
	InitSoundFormat();

	// init sound buffer
	if (!InitSoundBuffer())
		return FALSE;

	// create event handle
	if (!InitEventHandle())
		return FALSE;

	// create sound notify
	if (!InitSoundNotify())
		return FALSE;

	// fill buffer with silence
	FillBufferWithSilence();

	// set buffer play position
	m_pSoundBuffer->SetCurrentPosition(0);

	// create play thread
	m_Thread.Create(ThreadFunction, this);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	释放音乐
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::Close()
{
	// 必须先等线程结束，再释放内存，防止非法操作
	Stop();
	SetEvent(m_hEvent[2]);
	m_Thread.WaitForExit();
	FreeSoundNotify();
	FreeSoundBuffer();
	FreeEventHandle();
}
//---------------------------------------------------------------------------
// 函数:	InitSoundFormat
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::InitSoundFormat()
{
}
//---------------------------------------------------------------------------
// 函数:	InitSoundBuffer
// 功能:	创建声音Buffer
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::InitSoundBuffer()
{
	DSBUFFERDESC		dsbdesc;
	HRESULT				hres;
	LPDIRECTSOUND		pDirectSound;
	
	// get direct sound
	pDirectSound = g_pDirectSound->GetDirectSound();
	if (pDirectSound == NULL)
		return FALSE;
	
	// close any open file and release interfaces
	if (m_pSoundBuffer)
		FreeSoundBuffer();
	
	// set streaming sound buffer desc
	g_MemZero(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY // 播放位置通知
			| DSBCAPS_GETCURRENTPOSITION2 // 可以取得当前位置
			| DSBCAPS_CTRLVOLUME // 使音量可以控制
			| DSBCAPS_LOCSOFTWARE // 使用软件混音
			| DSBCAPS_STICKYFOCUS; // 可以在后台播放
	
	// The size of the buffer is arbitrary, but should be at least
	// two seconds, to keep data writes well ahead of the play position.
	dsbdesc.dwBufferBytes = m_dwBufferSize;
	dsbdesc.lpwfxFormat = &m_WaveFormat;
	
	// create streaming sound buffer
	hres = pDirectSound->CreateSoundBuffer(&dsbdesc, &m_pSoundBuffer, NULL);
	if (hres != DS_OK)
	{
		g_MessageBox("KWavMusic : Create sound buffer failed");
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FreeSoundBuffer
// 功能:	释放音乐Buffer
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::FreeSoundBuffer()
{
	if (g_pDirectSound->GetDirectSound() == NULL)
		return;
	SAFE_RELEASE(m_pSoundBuffer);
}
//---------------------------------------------------------------------------
// 函数:	InitEventHandle
// 功能:	创建事件句柄
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::InitEventHandle()
{
	for (int i = 0; i < NUMEVENTS; i++)
	{
		if (m_hEvent[i] == NULL)
			m_hEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hEvent[i] == NULL)
		{
			g_MessageBox("KMusic : Create event failed");
			return FALSE;
		}
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FreeEventHandle
// 功能:	创建事件句柄
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
void KMusic::FreeEventHandle()
{
	for (int i = 0; i < NUMEVENTS; i++)
	{
		if (m_hEvent[i])
		{
			CloseHandle(m_hEvent[i]);
			m_hEvent[i] = NULL;
		}
	}
}
//---------------------------------------------------------------------------
// 函数:	InitSoundNotify
// 功能:	创建SoundNotify
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::InitSoundNotify()
{
	HRESULT		hres;
	
	// check sound buffer
	if (m_pSoundBuffer == NULL)
		return FALSE;
	
	// setup pos notify
	m_PosNotify[0].dwOffset		= 0;
	m_PosNotify[0].hEventNotify	= m_hEvent[0];
	m_PosNotify[1].dwOffset		= m_dwBufferSize / 2;
	m_PosNotify[1].hEventNotify	= m_hEvent[1];
	
	// query interface
	hres = m_pSoundBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pSoundNotify);
	if (hres != S_OK)
	{
		g_MessageBox("KMusic : QueryInterface failed");
		return FALSE;
	}
	
	// set notify position
	hres = m_pSoundNotify->SetNotificationPositions(2, m_PosNotify);
	if (hres != DS_OK)
	{
		g_MessageBox("KMusic : Set notification positions failed");
		return FALSE;
	}
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FreeSoundNotify
// 功能:	释放播放通告
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::FreeSoundNotify()
{
	if (g_pDirectSound->GetDirectSound() == NULL)
		return;
	SAFE_RELEASE(m_pSoundNotify);
}
//---------------------------------------------------------------------------
// 函数:	HandleNotify
// 功能:	处理音乐播放中的通告消息
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KMusic::HandleNotify()
{
	DWORD dwEvent;
	
	// wait for event[0..3] to be signaled
	dwEvent = WaitForMultipleObjects(
		NUMEVENTS,		// How many possible events
		m_hEvent,		// Location of handles
		FALSE,			// Wait for all?
		INFINITE);		// How long to wait
	
    // WAIT_OBJECT_0 == 0 but is properly treated as an arbitrary
    // index value assigned to the first event, therefore we subtract
    // it from dwEvent to get the zero-based index of the event.
	dwEvent -= WAIT_OBJECT_0;
	
	// If the event was set by the buffer,
	// there's input to process.
	if ((dwEvent == 0) || (dwEvent == 1))
	{
		FillBufferWithSound(dwEvent);
	}

	// If the event[2] was set, exit thread
	return (dwEvent != 2);
}
//---------------------------------------------------------------------------
// 函数:	FillBufferWithSound
// 功能:	填充音乐Buffer
// 参数:	dwPos	位置
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::FillBufferWithSound(DWORD dwPos)
{
	DWORD		dwStartOfs;
	LONG		lNumToWrite;
	PVOID		lpvPtr1;
	PVOID		lpvPtr2;
	DWORD		dwBytes1;
	DWORD		dwBytes2;
	DWORD		dwBytesRead;
	HRESULT		hres;

	// check sound buffer
	if (m_pSoundBuffer == NULL)
		return FALSE;

	// decide start offset in sound buffer
	if (dwPos == 0)
		dwStartOfs = m_PosNotify[1].dwOffset;
	else
		dwStartOfs = m_PosNotify[0].dwOffset;

	// decide number bytes to write to buffer
	lNumToWrite = m_dwBufferSize / 2;

	// lock buffer to obtain buffer pointer
	hres = m_pSoundBuffer->Lock(
			dwStartOfs,       // Offset of lock start
			lNumToWrite,      // Number of bytes to lock
			&lpvPtr1,         // Address of lock start
			&dwBytes1,        // Count of bytes locked
			&lpvPtr2,         // Address of wrap around
			&dwBytes2,        // Count of wrap around bytes
			0);               // Flags
	
	// check return value
	if (hres != DS_OK)
		return FALSE;

	// Read wave data from the file into the buffer
	dwBytesRead = ReadWaveData((PBYTE)lpvPtr1, dwBytes1);

	// if reached end of file
	if (dwBytesRead < dwBytes1)
	{
		g_MemFill((PBYTE)lpvPtr1 + dwBytesRead,
			dwBytes1 - dwBytesRead,	(BYTE)0);
	}
	
	// unlock sound buffer
	m_pSoundBuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	
	// if reached end of file
	if (dwBytesRead < dwBytes1)
	{
		Rewind();
		if (!m_bLoop)
			Stop();
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FillMusicBufferWithSilence
// 功能:	用空白填充音乐Buffer
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KMusic::FillBufferWithSilence()
{
	PVOID   pb1, pb2;
	DWORD   cb1, cb2;
	HRESULT	hres;
	
	// check sound buffer
	if (m_pSoundBuffer == NULL)
		return FALSE;
	
	hres = m_pSoundBuffer->Lock(0, m_dwBufferSize, &pb1, &cb1, &pb2, &cb2, 0);
	if (hres != DS_OK)
		return FALSE;
	
	g_MemFill(pb1, cb1, (BYTE)0);
	m_pSoundBuffer->Unlock(pb1, cb1, pb2, cb2);
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	ReadWaveData
// 功能:	读取波形数据
// 参数:	lpBuf	缓存
//			dwLen	长度	
// 返回:	DWORD	实际读取的长度
//---------------------------------------------------------------------------
DWORD KMusic::ReadWaveData(LPBYTE lpBuf, DWORD dwLen)
{
	return 0;
}
//---------------------------------------------------------------------------
// 函数:	Play
// 功能:	播放音乐
// 参数:	bLoop		是否循环播放
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::Play(BOOL bLoop)
{
	if (m_pSoundBuffer)
	{
		m_pSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
		m_bLoop = bLoop;
	}
}
//---------------------------------------------------------------------------
// 函数:	Stop
// 功能:	停止音乐播放
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::Stop()
{
	if (m_pSoundBuffer)
	{
		m_pSoundBuffer->Stop();
	}
}

//---------------------------------------------------------------------------
// 函数:	IsPlaying		zroc add 不知道正不正确
// 功能:	判断音乐是否正在播放
// 参数:	void
// 返回:	TRUE 正在播放 FALSE 播完了
//---------------------------------------------------------------------------
BOOL KMusic::IsPlaying()
{
	if (m_pSoundBuffer == NULL)
		return FALSE;

	DWORD	Status = 0;
	m_pSoundBuffer->GetStatus(&Status);
	if (Status & DSBSTATUS_BUFFERLOST)
		return FALSE;
	if (Status & DSBSTATUS_PLAYING)
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
// 函数:	SetVolume
// 功能:	设置音乐的音量
// 参数:	lVolume		音量 [0, -10000]
//					  	DSBVOLUME_MAX = 0
//						DSBVOLUME_MIN = -10000
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::SetVolume(LONG lVolume)
{
	if (m_pSoundBuffer)
	{
		m_pSoundBuffer->SetVolume(lVolume);
	}
}
//---------------------------------------------------------------------------
// 函数:	Rewind
// 功能:	从头播放音乐
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMusic::Rewind()
{
}
//---------------------------------------------------------------------------
