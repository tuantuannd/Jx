//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSafeList.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KSafeList_H
#define	KSafeList_H
//---------------------------------------------------------------------------
#include "KList.h"
#include "KMutex.h"
#include "KEvent.h"
//---------------------------------------------------------------------------
class ENGINE_API KSafeList : public KList
{
protected:
	KMutex	m_Mutex;
	KEvent	m_Event;
public:
    void SignalEvent(void)
	{
		m_Event.Signal();
	};
    void WaitEvent(void)
	{
		m_Event.Wait();
	};
    void TimedWaitEvent(long ms)
	{
		m_Event.TimedWait(ms);
	};
    void Lock(void)
	{
		m_Mutex.Lock();
	};
    void Unlock(void)
	{
		m_Mutex.Unlock();
	};
};
//---------------------------------------------------------------------------
#endif