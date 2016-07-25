//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KLMusic.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Wave Stream Music Class
//---------------------------------------------------------------------------
#include <stdafx.h>
#include "KLMusic.h"
//---------------------------------------------------------------------------
// 函数:	KLMusic
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KLMusic::KLMusic()
{
	m_pDS		   = NULL;
	m_pSoundBuffer = NULL;
	m_pSoundNotify = NULL;
	m_hEvent[0]    = NULL;
	m_hEvent[1]    = NULL;
	m_hEvent[2]    = NULL;
	m_dwBufferSize = 0;
	m_bLoop        = FALSE;
	m_hWnd		   = NULL;
}
//---------------------------------------------------------------------------
// 函数:	~KLMusic
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KLMusic::~KLMusic()
{
	Close();
}
//---------------------------------------------------------------------------
// 函数:	ThreadFunction
// 功能:	播放线程
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLMusic::ThreadFunction(void* lpParam)
{
	KLMusic* pMusic = (KLMusic*)lpParam;
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
BOOL KLMusic::Open(LPSTR FileName)
{
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	初始化
// 参数:	void
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KLMusic::Init()
{
	if(!InitDSound())
		return FALSE;

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

	// 修改，先填一段声音数据，如果不这么做声音会延迟0.5秒
	FillBufferWithSound(1);

	// create play thread
	m_Thread.Create(ThreadFunction, this);

	return TRUE;
}

bool KLMusic::InitDSound()
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;
	
    SAFE_RELEASE( m_pDS );
	
    // Create IDirectSound using the primary sound device
    if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) )
    {
		return FALSE;
	}
	
    // Set DirectSound coop level 
    if( FAILED( hr = m_pDS->SetCooperativeLevel( m_hWnd, DSSCL_PRIORITY )))
    {
		return FALSE;
	}
	
    // Get the primary buffer 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
	
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) )
	{
		return FALSE;
	}
	
    if( FAILED( hr = pDSBPrimary->SetFormat(&m_WaveFormat) ) )
	{
		return FALSE;
	}
	
    SAFE_RELEASE( pDSBPrimary );
	
    return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	释放音乐
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLMusic::Close()
{
	// 必须先等线程结束，再释放内存，防止非法操作
	Stop();
	SetEvent(m_hEvent[2]);
	m_Thread.WaitForExit();
	FreeSoundNotify();
	FreeSoundBuffer();
	FreeEventHandle();

	SAFE_RELEASE( m_pDS ); 
}
//---------------------------------------------------------------------------
// 函数:	InitSoundFormat
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLMusic::InitSoundFormat()
{
}
//---------------------------------------------------------------------------
// 函数:	InitSoundBuffer
// 功能:	创建声音Buffer
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KLMusic::InitSoundBuffer()
{
	DSBUFFERDESC		dsbdesc;
	HRESULT				hres;
	
	// get direct sound
	if (m_pDS == NULL)
		return FALSE;
	
	// close any open file and release interfaces
	if (m_pSoundBuffer)
		FreeSoundBuffer();
	
	// set streaming sound buffer desc
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
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
	hres = m_pDS->CreateSoundBuffer(&dsbdesc, &m_pSoundBuffer, NULL);
	if (hres != DS_OK)
	{
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
void KLMusic::FreeSoundBuffer()
{
	if (m_pDS == NULL)
		return;
	SAFE_RELEASE(m_pSoundBuffer);
}
//---------------------------------------------------------------------------
// 函数:	InitEventHandle
// 功能:	创建事件句柄
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KLMusic::InitEventHandle()
{
	for (int i = 0; i < NUMEVENTS; i++)
	{
		if (m_hEvent[i] == NULL)
			m_hEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hEvent[i] == NULL)
		{
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
void KLMusic::FreeEventHandle()
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
BOOL KLMusic::InitSoundNotify()
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
		return FALSE;
	}
	
	// set notify position
	hres = m_pSoundNotify->SetNotificationPositions(2, m_PosNotify);
	if (hres != DS_OK)
	{
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
void KLMusic::FreeSoundNotify()
{
	if (m_pDS == NULL)
		return;
	SAFE_RELEASE(m_pSoundNotify);
}
//---------------------------------------------------------------------------
// 函数:	HandleNotify
// 功能:	处理音乐播放中的通告消息
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KLMusic::HandleNotify()
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
BOOL KLMusic::FillBufferWithSound(DWORD dwPos)
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
		memset((PBYTE)lpvPtr1 + dwBytesRead, 0,
			dwBytes1 - dwBytesRead);
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
BOOL KLMusic::FillBufferWithSilence()
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
	
	memset(pb1, 0, cb1);
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
DWORD KLMusic::ReadWaveData(LPBYTE lpBuf, DWORD dwLen)
{
	return 0;
}
//---------------------------------------------------------------------------
// 函数:	Play
// 功能:	播放音乐
// 参数:	bLoop		是否循环播放
// 返回:	void
//---------------------------------------------------------------------------
void KLMusic::Play(BOOL bLoop)
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
void KLMusic::Stop()
{
	if (m_pSoundBuffer)
	{
		m_pSoundBuffer->Stop();
	}
}
//---------------------------------------------------------------------------
// 函数:	SetVolume
// 功能:	设置音乐的音量
// 参数:	lVolume		音量 [0, -10000]
//					  	DSBVOLUME_MAX = 0
//						DSBVOLUME_MIN = -10000
// 返回:	void
//---------------------------------------------------------------------------
void KLMusic::SetVolume(LONG lVolume)
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
void KLMusic::Rewind()
{
}
//---------------------------------------------------------------------------
