#ifndef KTASKFUNCS_H
#define KTASKFUNCS_H

#include "KSubWorldSet.h"
typedef int (* PF_TimerCallBackFun)(void * pOwner, char * szFileName);
#define TASKVALUE_SAVEWAYPOINT_BEGIN 201
#define	TASKVALUE_MAXWAYPOINT_COUNT 3

#define TASKVALUE_SAVESTATION_BEGIN  210
#define TASKVALUE_MAXSTATION_COUNT 32

#define TASKVALUE_TIMERTASK_TAST	220
#define TASKVALUE_TIMERTASK_RESTTIME 221
#define TIMERTASK_SETTINGFILE "\\settings\\TimerTask.txt"
#define TASKVALUE_REPUTE 100

class KPlayer;

class KTaskFun
{
public:
	virtual BOOL	SaveTask(KPlayer *) = 0;
	virtual BOOL	LoadTask(KPlayer *) = 0;
};

class KTimerTaskFun :public KTaskFun
{
public:
	KTimerTaskFun();
	
	KTimerTaskFun(void * pOwner)
	{
		m_pTimerOwner = pOwner;
	};

	void SetOwner(void * pOwner) { m_pTimerOwner = pOwner;};
	static BOOL	Init();

	BOOL	SaveTask(KPlayer *);
	BOOL	LoadTask(KPlayer *);
	void *  m_pTimerOwner;
	
	BOOL	SetTimer(size_t ulTime, unsigned short usTimerTaskId)
	{
		//如果间隔时间为0，表示中止Timer;
		if (ulTime == 0) 
		{
			CloseTimer();
			return FALSE;
		}

		m_dwIntervalTime = ulTime;
		m_dwTimeTaskTime = g_SubWorldSet.GetGameTime() + ulTime;
		m_dwTimerTaskId = usTimerTaskId;
		return TRUE;
	};
	
	void	CloseTimer()
	{
		m_dwTimeTaskTime = 0;
		m_dwTimerTaskId = 0;
		m_dwIntervalTime = 0;
	};

	unsigned long	GetRestTime() const
	{
		DWORD dwGameTime = g_SubWorldSet.GetGameTime();
		return (dwGameTime > m_dwTimeTaskTime) ? 0 : (m_dwTimeTaskTime - dwGameTime);
	};
	

	void	GetTimerTaskScript(char * szScriptFileName, unsigned short usTimerTaskId, size_t nScriptFileLen);
	BOOL	Activate(PF_TimerCallBackFun TimerCallBackFun);
	static KTabFile m_TimerTaskTab;
	DWORD GetTaskId(){return m_dwTimerTaskId;};
private:
	DWORD	m_dwTimeTaskTime;
	DWORD	m_dwTimerTaskId;
	DWORD	m_dwIntervalTime;
	
	
};


typedef KTaskFun KTask;

extern KTimerTaskFun g_TimerTask;
#endif
