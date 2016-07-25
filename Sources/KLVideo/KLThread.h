//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KLThread.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef __KThread_H__
#define __KThread_H__
//---------------------------------------------------------------------------
typedef void (* TThreadFunc)(void* arg);
//---------------------------------------------------------------------------
class KLThread
{
private:
	HANDLE			m_ThreadHandle;
	DWORD			m_ThreadId;
	TThreadFunc 	m_ThreadFunc;
	LPVOID			m_ThreadParam;
public:
	KLThread();
	~KLThread();
	BOOL			Create(TThreadFunc lpFunc, void* lpParam);
	void			Destroy();
	void			Suspend();
	void			Resume();
	BOOL			IsRunning();
	void			WaitForExit();
	int				GetPriority();
	BOOL			SetPriority(int priority);
private:
	DWORD			ThreadFunction();
	static			DWORD WINAPI ThreadProc(LPVOID lpParam);
};
//---------------------------------------------------------------------------
#endif
