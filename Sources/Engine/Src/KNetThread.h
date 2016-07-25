//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNetThread.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KNetThread_H
#define KNetThread_H
//---------------------------------------------------------------------------
#include "KMutex.h"
#include "KEvent.h"
#include "KMsgNode.h"
#include "KSafeList.h"
//---------------------------------------------------------------------------
#define THREADPROC __stdcall
//---------------------------------------------------------------------------
class ENGINE_API KNetThread
{
private:
    HANDLE m_hThread;
    int (THREADPROC *m_pThreadFunc)(KNetThread *);
    void (*m_pWakeupFunc)(KNetThread *);
    void* m_pUserData;
    bool m_bStopThread;
    bool m_bExistMsgList;
    bool m_bShutdown;
    KMutex m_Mutex;
    KEvent m_StartupEvent;
    KEvent m_ShutdownEvent;
    KSafeList* m_pMsgList;
	
public:
    KNetThread(void);
    virtual ~KNetThread(void);
    
    // thread management
    virtual void Init(int (THREADPROC *_thread_func)(KNetThread *),
		long stack_size,
		void (*_wakeup_func)(KNetThread *),
		KSafeList *_ext_msglist,
		void *_user_data);
    virtual void Exit(void);
    virtual void ThreadStart(void);
    virtual void ThreadSleep(long ms);
    virtual void ThreadStop(void);
    virtual bool ThreadStopRequested(void);
	
    // msg handling
    virtual void WaitMsg(void);
    virtual KMsgNode *GetMsg(void);
    virtual void ReplyMsg(KMsgNode *);
    virtual void PutMsg(void *buf, long buf_size);
    
    // access user data field
    virtual void *LockUserData(void);
    virtual void UnlockUserData(void);
};
//---------------------------------------------------------------------------
#endif
