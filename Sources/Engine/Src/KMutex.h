//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMutex.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMutex_H
#define KMutex_H
//---------------------------------------------------------------------------
#define SINGLE_PROCESS
//---------------------------------------------------------------------------
#ifdef WIN32
class ENGINE_API KMutex
#else
class KMutex
#endif
{
private:
#ifdef WIN32
#ifdef SINGLE_PROCESS
	CRITICAL_SECTION m_CriticalSection;//用于单进程的线程同步
#else
	HANDLE m_hMutex;//用于多进程的线程同步
#endif
#else
    pthread_mutex_t mutex;
#endif
public:
    KMutex();
    ~KMutex();
    void Lock(void);
    void Unlock(void);
};
//---------------------------------------------------------------------------
#endif
