//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KEvent.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Implements a simple event object for thread synchronization
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KEvent.h"
//---------------------------------------------------------------------------
// 函数:	KEvent
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KEvent::KEvent()
{
    m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (!m_hEvent)
		g_DebugLog("KEvent::KEvent(): CreateEvent() failed!");
}
//---------------------------------------------------------------------------
// 函数:	~KEvent
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KEvent::~KEvent()
{
    CloseHandle(m_hEvent);
}
//---------------------------------------------------------------------------
// 函数:	Signal
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
inline void KEvent::Signal(void)
{
    SetEvent(m_hEvent);
}
//---------------------------------------------------------------------------
// 函数:	Wait
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
inline void KEvent::Wait(void)
{
    WaitForSingleObject(m_hEvent,INFINITE);
}
//---------------------------------------------------------------------------
// 函数:	TimedWait
// 功能:	
// 参数:	long ms
// 返回:	void
//---------------------------------------------------------------------------
inline bool KEvent::TimedWait(long ms)
{
    int r = WaitForSingleObject(m_hEvent, ms);
    return (WAIT_TIMEOUT == r) ? false : true;
}
//---------------------------------------------------------------------------
