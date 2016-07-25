//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNetThread.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	BG2132 Font Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include <process.h>
#include "KDebug.h"
#include "KMemBase.h"
#include "KNetThread.h"
//---------------------------------------------------------------------------
#define DEFAULT_STACKSIZE 4096
//---------------------------------------------------------------------------
// 函数:	KNetThread
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KNetThread::KNetThread(void)
{
    m_hThread = NULL;
    m_pThreadFunc = NULL;
    m_pWakeupFunc = NULL;
    m_pUserData = NULL;
    m_bStopThread = FALSE;
    m_bExistMsgList = FALSE;
    m_bShutdown = FALSE;
    m_pMsgList = NULL;
}
//---------------------------------------------------------------------------
// 函数:	~KNetThread
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KNetThread::~KNetThread(void)
{
	Exit();
}

//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetThread::Init(int (THREADPROC *thread_func)(KNetThread *),
					  long stack_size,
					  void (*_wakeup_func)(KNetThread *),
					  KSafeList *_ext_msglist,
					  void *_user_data)
{
    KASSERT(thread_func);
    if (!stack_size)
		stack_size = DEFAULT_STACKSIZE;
    if (_ext_msglist)
	{
        m_pMsgList = _ext_msglist;
        m_bExistMsgList = TRUE;
    }
	else
	{
        m_pMsgList = new KSafeList;
        m_bExistMsgList = FALSE;
    }
    m_pThreadFunc = thread_func;
    m_pWakeupFunc = _wakeup_func; 
    m_pUserData   = _user_data;
    m_bStopThread = FALSE;
    m_bShutdown   = FALSE;
    
    // launch thread
    // we are using _beginthreadx() instead of CreateThread(),
    // because we want to use c runtime functions from within the thread
    unsigned int thrdaddr;
    m_hThread = (HANDLE) _beginthreadex(
		NULL,    // security
		stack_size,
		(unsigned (__stdcall *)(void *))thread_func,
		this,    // arglist
		0,       // init_flags
		&thrdaddr);
    if (!m_hThread)
	{
        g_DebugLog("KNetThread::KNetThread(): _begiKNetThreadx() failed!\n");
    }
	
	// wait until the thread has started
	m_StartupEvent.Wait();
}

//---------------------------------------------------------------------------
// 函数:	Exit
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetThread::Exit(void)
{
    // wake up thread, if a wakeup func is defined
    m_bStopThread = TRUE;
    if (m_pWakeupFunc)
		m_pWakeupFunc(this);
    
	// signal the thread that it may terminate now
    m_ShutdownEvent.Signal();
	
    // wait until the thread has indeed terminated
    // (do nothing under Win32, because _endthreadex()
    // will be called at the end of the thread, which
    // does the CloseHandle() stuff itself
    WaitForSingleObject(m_hThread,500);
    CloseHandle(m_hThread);
    m_hThread = 0;
    
    // flush msg list (all remaining messages will be lost)
    if (!(m_bExistMsgList))
	{
        KMsgNode *nd;
        m_pMsgList->Lock();
        while ((nd = (KMsgNode *) m_pMsgList->RemoveHead()))
			delete nd;
        m_pMsgList->Unlock();
        delete m_pMsgList;
    }
}

//-------------------------------------------------------------------
//  KNetThread::ThreadStart()
//  20-Oct-98   floh    created
//  27-Apr-99   floh    + Support fuer __NEBULA_NO_THREADS__
//-------------------------------------------------------------------
void KNetThread::ThreadStart(void)
{
	m_StartupEvent.Signal();
}

//-------------------------------------------------------------------
//  KNetThread::ThreadStop()
//  20-Oct-98   floh    created
//	30-Oct-98	floh	wartet jetzt auf den Destruktor
//  27-Apr-99   floh    + Support fuer __NEBULA_NO_THREADS__
//  20-Feb-00   floh    + rewritten to _endthreadex()
//-------------------------------------------------------------------
void KNetThread::ThreadStop(void)
{
	// synchronize with destructor
    m_ShutdownEvent.Wait();
    m_bShutdown = TRUE;
    _endthreadex(0);
}

//-------------------------------------------------------------------
//  KNetThread::ThreadStopRequested()
//  20-Oct-98   floh    created
//  27-Apr-99   floh    + Support fuer __NEBULA_NO_THREADS__
//-------------------------------------------------------------------
bool KNetThread::ThreadStopRequested(void)
{
    return m_bStopThread;
}

//-------------------------------------------------------------------
//  KNetThread::ThreadSleep()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
void KNetThread::ThreadSleep(long ms)
{
    Sleep(ms);
}

//-------------------------------------------------------------------
//  KNetThread::GetMsg()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
KMsgNode *KNetThread::GetMsg(void)
{
    KMsgNode *nd;
    m_pMsgList->Lock();
    nd = (KMsgNode *) m_pMsgList->RemoveHead();
    m_pMsgList->Unlock();
	return nd;
}

//-------------------------------------------------------------------
//  KNetThread::ReplyMsg()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
void KNetThread::ReplyMsg(KMsgNode *nd)
{
    delete nd;
}

//-------------------------------------------------------------------
//  KNetThread::WaitMsg()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
void KNetThread::WaitMsg(void)
{
    m_pMsgList->WaitEvent();
}

//-------------------------------------------------------------------
//  KNetThread::PutMsg()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
void KNetThread::PutMsg(void *buf, long size)
{
    KASSERT(buf);
    KASSERT(size > 0);
    KMsgNode *nd = new KMsgNode();
	nd->SetMsg(buf, size);
    m_pMsgList->Lock();
    m_pMsgList->AddTail(nd);
    m_pMsgList->Unlock();
    m_pMsgList->SignalEvent();
}
 
//-------------------------------------------------------------------
//  KNetThread::LockUserData()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
void *KNetThread::LockUserData(void)
{
    if (m_pUserData)
		m_Mutex.Lock();
    return m_pUserData;
}

//-------------------------------------------------------------------
//  KNetThread::UnlockUserData()
//  20-Oct-98   floh    created
//-------------------------------------------------------------------
void KNetThread::UnlockUserData(void)
{
    m_Mutex.Unlock();
}
//---------------------------------------------------------------------------
