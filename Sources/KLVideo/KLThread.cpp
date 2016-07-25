//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KLThread.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	BG2132 Font Class
//---------------------------------------------------------------------------
#include <stdafx.h>
#include "KLThread.h"
//---------------------------------------------------------------------------
// 函数:	KLThread
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KLThread::KLThread()
{
	m_ThreadHandle	= NULL;
	m_ThreadId		= 0;
	m_ThreadFunc	= NULL;
	m_ThreadParam	= NULL;
}
//---------------------------------------------------------------------------
// 函数:	~KLThread
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KLThread::~KLThread()
{
	if (m_ThreadHandle)
	{
		CloseHandle(m_ThreadHandle);
		m_ThreadHandle	= NULL;
	}
}
//---------------------------------------------------------------------------
// 函数:	ThreadProc
// 功能:	线程过程
// 参数:	lpParam		参数
// 返回:	void
//---------------------------------------------------------------------------
DWORD WINAPI KLThread::ThreadProc(LPVOID lpParam)
{
	KLThread* pThread = (KLThread*)lpParam;
	return pThread->ThreadFunction();
}
//---------------------------------------------------------------------------
// 函数:	ThreadFunction
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
DWORD KLThread::ThreadFunction()
{
	m_ThreadFunc(m_ThreadParam);
	return 0;
}
//---------------------------------------------------------------------------
// 函数:	Create
// 功能:	创建线程
// 参数:	func		线程函数
//			arg 		线程参数
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KLThread::Create(TThreadFunc lpFunc, void* lpParam)
{
	m_ThreadFunc   = lpFunc;
	m_ThreadParam  = lpParam;
	m_ThreadHandle = CreateThread(
		NULL,			// SD
		0,				// initial stack size
		ThreadProc,		// thread function
		this,			// thread argument
		0,				// creation option
		&m_ThreadId);	// thread identifier
	if(m_ThreadHandle)
		SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL); 
	return (m_ThreadHandle != NULL);
}
//---------------------------------------------------------------------------
// 函数:	Destroy
// 功能:	销毁线程
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLThread::Destroy()
{
	TerminateThread(m_ThreadHandle, 0);
}
//---------------------------------------------------------------------------
// 函数:	Suspend
// 功能:	挂起线程
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLThread::Suspend()
{
	 SuspendThread(m_ThreadHandle);
}
//---------------------------------------------------------------------------
// 函数:	Resume
// 功能:	唤醒线程
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLThread::Resume()
{
	ResumeThread(m_ThreadHandle);
}
//---------------------------------------------------------------------------
// 函数:	IsRunning
// 功能:	判断线程是否在运行
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KLThread::IsRunning()
{
	DWORD dwResult = WaitForSingleObject(m_ThreadHandle, 0);
	return (dwResult == WAIT_OBJECT_0);
}
//---------------------------------------------------------------------------
// 函数:	WaitForExit
// 功能:	等待线程结束
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KLThread::WaitForExit()
{
	WaitForSingleObject(m_ThreadHandle, INFINITE);
}
//---------------------------------------------------------------------------
// 函数:	GetPriority
// 功能:	取得线程优先级
// 参数:	void
// 返回:	int
//---------------------------------------------------------------------------
int KLThread::GetPriority()
{
	return GetThreadPriority(m_ThreadHandle);
}
//---------------------------------------------------------------------------
// 函数:	SetPriority
// 功能:	设置线程优先级
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KLThread::SetPriority(int priority)
{
	return SetThreadPriority(m_ThreadHandle, priority);
}
//---------------------------------------------------------------------------
