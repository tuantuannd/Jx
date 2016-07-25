// -------------------------------------------------------------------------
//	文件名		：	KThread.h
//	创建者		：	万里
//	创建时间	：	2003-5-1 21:16:13
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __KTHREAD_H__
#define __KTHREAD_H__

#include "WTypes.h"

class KThread
{
public:
	KThread();
	virtual ~KThread();
	HANDLE m_hStop;	// Thread running signal
	HANDLE m_hProcessor;		// Thread handle

	virtual BOOL Stop();
	virtual HANDLE Start();

	static DWORD WINAPI ThreadFun(LPVOID lpParam);

	virtual DWORD Main(LPVOID lpParam) = 0;
};

class K_CS
{
public:
	K_CS();
	virtual ~K_CS();

	CRITICAL_SECTION m_sect;
	BOOL m_bAcquired;
	BOOL Lock();
	BOOL Unlock();
	BOOL IsLocked() {return m_bAcquired;}
};

class KThread_CS : public KThread, public K_CS
{
public:
	KThread_CS() {}
	~KThread_CS() {}
};


#endif // __KTHREAD_H__