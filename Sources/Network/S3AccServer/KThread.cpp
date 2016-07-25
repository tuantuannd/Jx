// -------------------------------------------------------------------------
//	文件名		：	KThread.cpp
//	创建者		：	万里
//	创建时间	：	2003-5-1 21:15:58
//	功能描述	：	
//
// -------------------------------------------------------------------------

#include "KStdAfx.h"
#include "GlobalFun.h"
#include "KThread.h"

KThread::KThread()
{
	m_hStop = NULL;
	m_hProcessor = NULL;
}

KThread::~KThread()
{
	assert(m_hStop == NULL);
	assert(m_hProcessor == NULL);
}

HANDLE KThread::Start()
{
	HANDLE hRet = NULL;
	if ( NULL != m_hProcessor )	// The thread has been running.
	{
		return hRet;
	}
	if (NULL == m_hStop)
	{
		m_hStop = KPICreateEvent(NULL, TRUE, FALSE, NULL);
	}

	m_hProcessor = KPICreateThread(KThread::ThreadFun, (LPVOID)this, NULL);

	hRet = m_hProcessor;

	return hRet;
}

BOOL KThread::Stop()
{
	BOOL bRet = FALSE;

	if (NULL == m_hProcessor
		|| NULL == m_hStop)
	{
		bRet = TRUE;
		return bRet;
	}

	KPISetEvent(m_hStop);

	DWORD dwResult = KPIWaitForSingleObject(m_hProcessor, INFINITE);
	if (0 == dwResult)
	{
		bRet = FALSE;
	}
	else if (1 == dwResult)
	{
		bRet = TRUE;
	}
	else if (2 == dwResult)	// Time out.
	{
		if (KPITerminateThread(m_hProcessor, 0))
		{
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}
	}

	if (bRet)
	{
		KPICloseHandle(m_hStop);
		m_hStop = NULL;
		KPICloseHandle(m_hProcessor);
		m_hProcessor = NULL;
	}

	return bRet;
}

DWORD WINAPI KThread::ThreadFun(LPVOID lpParam)
{
	assert(lpParam);
	KThread* pThis = (KThread*)lpParam;
	return pThis->Main(lpParam);
}

K_CS::K_CS()
{
	::InitializeCriticalSection(&m_sect);

	m_bAcquired = FALSE;
}

K_CS::~K_CS()
{
	Unlock();

	::DeleteCriticalSection(&m_sect);
}

BOOL K_CS::Lock()
{
	::EnterCriticalSection(&m_sect);
	m_bAcquired = TRUE;
	return m_bAcquired;
}

BOOL K_CS::Unlock()
{
	if (m_bAcquired)
	{
		::LeaveCriticalSection(&m_sect);
		m_bAcquired = FALSE;
	}

	// successfully unlocking means it isn't acquired
	return !m_bAcquired;
}
