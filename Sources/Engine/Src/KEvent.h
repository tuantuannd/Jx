//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KEvent.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KEvent_H
#define KEvent_H
//---------------------------------------------------------------------------
class ENGINE_API KEvent
{
private:
	HANDLE m_hEvent;//用于多进程的线程同步
public:
    KEvent();
    ~KEvent();
    void Signal(void);
    void Wait(void);
    bool TimedWait(long ms);
};
//---------------------------------------------------------------------------
#endif
