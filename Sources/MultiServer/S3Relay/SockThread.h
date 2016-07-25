// SockThread.h: interface for the CSockThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKTHREAD_H__71A87FDA_23D2_46AF_B66C_92ECB2977F6C__INCLUDED_)
#define AFX_SOCKTHREAD_H__71A87FDA_23D2_46AF_B66C_92ECB2977F6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KThread.h"

class CSockThread : public KThread  
{
public:
	CSockThread();
	virtual ~CSockThread();

protected:
	virtual DWORD Main(LPVOID lpParam);

protected:
	virtual void EnterLoop() {}
	virtual void LeaveLoop() {}
	virtual void PrepareSock() {}
	virtual void UnprepareSock() {}

public:
	virtual BOOL Stop();
	virtual HANDLE Start();

private:
	enum {
		step_NONE,

		step_EnterLoop,

		step_PrepareSock,

		step_RelayCenter, 
		step_RelayServer,

		step_RootCenter,

		step_GatewayCenter,

		step_HostServer,
		step_TongServer,
		step_ChatServer,

		step_UnprepareSock,

		step_LeaveLoop,

		step_Sleep,
	} m_step;
	DWORD m_cntLoop;
	DWORD m_tickLoop;

public:
	BOOL TraceInfo();

};

#endif // !defined(AFX_SOCKTHREAD_H__71A87FDA_23D2_46AF_B66C_92ECB2977F6C__INCLUDED_)
