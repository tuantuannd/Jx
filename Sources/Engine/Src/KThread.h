//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KThread.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KThread_H
#define KThread_H
//---------------------------------------------------------------------------
typedef void (* TThreadFunc)(void* arg);

#ifndef WIN32
#include <pthread.h>
#endif
//---------------------------------------------------------------------------
#ifdef WIN32
class ENGINE_API KThread
#else
class KThread
#endif
{
private:
#ifdef WIN32
	HANDLE			m_ThreadHandle;
	DWORD			m_ThreadId;
#else
     pthread_t  p_thread;
#endif
	TThreadFunc 	m_ThreadFunc;
	LPVOID			m_ThreadParam;
public:
	KThread();
	~KThread();
	BOOL			Create(TThreadFunc lpFunc, void* lpParam);
	void			Destroy();
	void			Suspend();
	void			Resume();
	BOOL			IsRunning();
	void			WaitForExit();
	int				GetPriority();
	BOOL			SetPriority(int priority);
//private:
	DWORD			ThreadFunction();
#ifdef WIN32
//	static			DWORD __stdcall ThreadProc(LPVOID lpParam);
#else
//	void * ThreadProc(LPVOID lpParam);
#endif
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif
