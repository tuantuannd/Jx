// -------------------------------------------------------------------------
//	文件名		：	KThread.cpp
//	创建者		：	万里
//	创建时间	：	2003-5-1 21:15:58
//	功能描述	：	
//
// -------------------------------------------------------------------------

#include "StdAfx.h"
#include "KThread.h"
#include "process.h"

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
		m_hStop = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	unsigned ID = 0;
	m_hProcessor = (HANDLE)_beginthreadex( NULL, 0, KThread::ThreadFun, (void *)this, 0, &ID );

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

	SetEvent(m_hStop);

	DWORD dwResult = WaitForSingleObject(m_hProcessor, INFINITE);
	if (WAIT_FAILED == dwResult)
	{
		bRet = FALSE;
	}
	else if (WAIT_OBJECT_0 == dwResult)
	{
		bRet = TRUE;
	}
	else if (WAIT_TIMEOUT == dwResult)	// Time out.
	{
		if (TerminateThread(m_hProcessor, 0))
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
		CloseHandle(m_hStop);
		m_hStop = NULL;
		CloseHandle(m_hProcessor);
		m_hProcessor = NULL;
	}

	return bRet;
}

BOOL KThread::IsAskingStop()
{
	assert(m_hStop);
	return ::WaitForSingleObject(m_hStop, 0) != WAIT_TIMEOUT;
}


unsigned  __stdcall KThread::ThreadFun( void * lpParam )
{
	try
	{
		KThread* pThis = (KThread*)lpParam;
		assert(pThis);

		try
		{
			if (!pThis)
				throw -1;
			return pThis->Main(lpParam);
		}
		catch (...)
		{//try Clearup
			if (NULL != pThis->m_hProcessor)
			{
				::CloseHandle(pThis->m_hProcessor);
				pThis->m_hProcessor = NULL;
			}
			if (NULL != pThis->m_hStop)
			{
				::CloseHandle(pThis->m_hStop);
				pThis->m_hStop = NULL;
			}

			return -1;
		}
	}
	catch (...)
	{//fatal
		return -1;
	}
}

