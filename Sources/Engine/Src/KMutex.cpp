//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMutex.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Implements a simple mutex object for thread synchronization
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMutex.h"

KMutex::KMutex()
{
#ifdef WIN32
#ifdef SINGLE_PROCESS
	InitializeCriticalSection(&m_CriticalSection);
#else
	m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (!m_hMutex)
		g_MessageBox("KMutex::KMutex() CreateMutex() failed!");
#endif
#else
     int rc = pthread_mutex_init(&mutex, NULL);
#endif
}
//---------------------------------------------------------------------------
// 函数:	~KMutex
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMutex::~KMutex()
{
#ifdef WIN32
#ifdef SINGLE_PROCESS
	DeleteCriticalSection(&m_CriticalSection);
#else
	CloseHandle(m_hMutex);
#endif
#else
     int rc = pthread_mutex_destroy(&mutex);
     printf("create mutex%d\n", rc);
#endif
}
//---------------------------------------------------------------------------
// 函数:	Lock
// 功能:	锁定
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMutex::Lock(void)
{
#ifdef WIN32
#ifdef SINGLE_PROCESS
	EnterCriticalSection(&m_CriticalSection);
#else
	WaitForSingleObject(m_hMutex, INFINITE);
#endif
#else
     int rc = pthread_mutex_lock(&mutex);
#endif
}
//---------------------------------------------------------------------------
// 函数:	Unlock
// 功能:	解开
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMutex::Unlock(void)
{
#ifdef WIN32
#ifdef SINGLE_PROCESS
	LeaveCriticalSection(&m_CriticalSection);
#else
	ReleaseMutex(m_hMutex);
#endif
#else
     int rc = pthread_mutex_unlock(&mutex);
#endif
}
