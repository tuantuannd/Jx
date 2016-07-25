//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KVideo.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Video Stream Play Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KVideo.h"
//---------------------------------------------------------------------------
// 函数:	KVideo
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KVideo::KVideo()
{
	m_hEvent[0]		= 0;
	m_hEvent[1]		= 0;
	m_nX			= 0;
	m_nY			= 0;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_pVideo		= NULL;
	m_pFrame		= NULL;
	m_nFrameSize	= 0;
	m_nFrameRate	= 20;
	m_nFrameTime	= 50;
	m_uTimeID		= 0;
	m_bZoomToWin	= FALSE;
}
//---------------------------------------------------------------------------
// 函数:	~KVideo
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KVideo::~KVideo()
{
	// Close();
}
//---------------------------------------------------------------------------
// 函数:	ThreadFunction
// 功能:	播放线程
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KVideo::ThreadFunction(void* lpParam)
{
	KVideo* pVideo = (KVideo*)lpParam;
	while (TRUE)
	{
		if (!pVideo->HandleNotify())
			break;
	}
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开视频文件
// 参数:	FileName	文件名
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KVideo::Open(LPSTR FileName)
{
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	初始化
// 参数:	
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KVideo::Init()
{
	// create event handle
	if (!InitEventHandle())
		return FALSE;

	// create play thread
	m_Thread.Create(ThreadFunction, this);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭文件，释放资源
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KVideo::Close()
{
	// 必须先等线程结束，再释放内存，防止非法操作
	Stop();
	SetEvent(m_hEvent[1]);
	m_Thread.WaitForExit();
	FreeEventHandle();
}
//---------------------------------------------------------------------------
// 函数:	InitEventHandle
// 功能:	创建事件句柄
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KVideo::InitEventHandle()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_hEvent[i] == NULL)
			m_hEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hEvent[i] == NULL)
		{
			g_MessageBox("KVideo : Create event failed");
			return FALSE;
		}
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FreeEventHandle
// 功能:	释放事件句柄
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
void KVideo::FreeEventHandle()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_hEvent[i])
		{
			CloseHandle(m_hEvent[i]);
			m_hEvent[i] = NULL;
		}
	}
}
//---------------------------------------------------------------------------
// 函数:	HandleNotify
// 功能:	处理音乐播放中的通告消息
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KVideo::HandleNotify()
{
	DWORD dwEvent;
	
	// wait for event[0..3] to be signaled
	dwEvent = WaitForMultipleObjects(
		2,				// How many possible events
		m_hEvent,		// Location of handles
		FALSE,			// Wait for all?
		INFINITE);		// How long to wait
	
    // WAIT_OBJECT_0 == 0 but is properly treated as an arbitrary
    // index value assigned to the first event, therefore we subtract
    // it from dwEvent to get the zero-based index of the event.
	dwEvent -= WAIT_OBJECT_0;
	
	// If event[0] was set then goto next frame
	if (dwEvent == 0)
		return NextFrame();

	// If the event[1] was set then exit thread
	return (dwEvent != 1);
}
//---------------------------------------------------------------------------
// 函数:	NextFrame
// 功能:	下一帧
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KVideo::NextFrame()
{
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Play
// 功能:	播放
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KVideo::Play(int nX, int nY, int nZoom)
{
	if (m_pVideo == NULL)
		return;
	if (m_uTimeID == 0)
	{
		m_uTimeID = timeSetEvent(
			m_nFrameTime,	// delay time in ms
			1,	// resolution
			(LPTIMECALLBACK)m_hEvent[0],
			0,	// user data
			TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
		m_Timer.Start();
		m_nX = nX;
		m_nY = nY;
		m_bZoomToWin = nZoom;
	}
}
//---------------------------------------------------------------------------
// 函数:	Stop
// 功能:	停止
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KVideo::Stop()
{
	if (m_uTimeID)
	{
		timeKillEvent(m_uTimeID);
		m_uTimeID = 0;
		m_Timer.Stop();
	}
}
//---------------------------------------------------------------------------
// 函数:	IsPlaying
// 功能:	是否在播放
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KVideo::IsPlaying()
{
	return (m_uTimeID != 0);
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	移动当前帧
// 参数:	nPercent	百分比0-100
// 返回:	void
//---------------------------------------------------------------------------
void KVideo::Seek(int nPercent)
{
}
//---------------------------------------------------------------------------
// 函数:	Rewind
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KVideo::Rewind()
{
}
//---------------------------------------------------------------------------
