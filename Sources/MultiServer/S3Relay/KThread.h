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

	virtual BOOL IsAskingStop();

	static unsigned  __stdcall ThreadFun( void * lpParam );

	virtual DWORD Main(LPVOID lpParam) = 0;
};


#endif // __KTHREAD_H__
