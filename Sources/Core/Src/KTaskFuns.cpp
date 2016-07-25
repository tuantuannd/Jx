#include "KCore.h"
//#include "MyAssert.H"
#include "KPlayer.h"
#include "KPlayerTask.h"
#include "KTaskFuns.h"
#include "KNpc.h"
#include "KSubWorldSet.h"
#ifndef _SERVER
KTabFile g_StringResourseTabFile;

char* g_GetStringRes(int nStringID, char * szString, int nMaxLen)
{
	char szStringId[10];
	sprintf(szStringId, "%d", nStringID);
	g_StringResourseTabFile.GetString(szStringId, "STRING", "", szString, nMaxLen);
	return szString;
}
#endif


#ifdef _SERVER
KTabFile g_MissionTabFile;
int g_WayPointPriceUnit = 1;//WayPoint表格中价格的单位量，WayPoint价格 = 单位量 * 表格数值
int g_StationPriceUnit = 1;	//Station表格中价格的单位量，Station价格 = 单位量 * 表格数值
int g_DockPriceUnit	= 1;
int g_GetPriceToWayPoint(int nStationId, int nWayPoint)
{
	if (g_WayPointPriceTabFile.GetWidth() < nStationId)	
	{
		_ASSERT(0);
		return 0;
	}
	if (g_WayPointPriceTabFile.GetHeight() < nWayPoint)
	{
		_ASSERT(0);
		return 0;
	}
	if (*(g_pWayPointPriceTab + (nStationId - 1) * g_WayPointPriceTabFile.GetWidth() + nWayPoint - 1) < 0)
	{
		g_WayPointPriceTabFile.GetInteger(nWayPoint + 1, nStationId + 1, 0, (g_pWayPointPriceTab + (nStationId - 1) * g_WayPointPriceTabFile.GetWidth() + nWayPoint - 1));
	}
	return 	g_WayPointPriceUnit * (*(g_pWayPointPriceTab + (nStationId - 1) * g_WayPointPriceTabFile.GetWidth() + nWayPoint - 1));
}

int g_GetPriceToStation(int nStationId, int nNextStationId)
{
	if (nStationId == nNextStationId) return 0;
	if (g_StationPriceTabFile.GetWidth() < nStationId)	
	{
		_ASSERT(0);
		return 0;
	}
	if (g_StationTabFile.GetHeight() < nNextStationId)
	{
		_ASSERT(0);
		return 0;
	}
	if (*(g_pStationPriceTab + (nStationId - 1) * g_StationPriceTabFile.GetWidth() + nNextStationId - 1) < 0)
	{
		int t;
		g_StationPriceTabFile.GetInteger(nNextStationId + 1, nStationId + 1, 0, &t);
		*(g_pStationPriceTab + (nStationId - 1) * g_StationPriceTabFile.GetWidth() + nNextStationId - 1) = t;
	}
	return 	g_StationPriceUnit * (*(g_pStationPriceTab + (nStationId - 1) * g_StationPriceTabFile.GetWidth() + nNextStationId - 1));
}

int g_GetPriceToDock(int nCurDockId, int nNextDockId)
{
	if (nCurDockId == nNextDockId)
	{
		return 0;
	}
	
	if (g_DockPriceTabFile.GetWidth() < nCurDockId)	
	{
		_ASSERT(0);
		return 0;
	}

	if (g_DockTabFile.GetHeight()  - 1< nNextDockId)
	{
		_ASSERT(0);
		return 0;
	}
	
	if (*(g_pDockPriceTab + (nCurDockId - 1) * g_DockPriceTabFile.GetWidth() + nNextDockId - 1) < 0)
	{
		int t;
		g_DockPriceTabFile.GetInteger(nNextDockId + 1, nCurDockId + 1, 0, &t);
		*(g_pDockPriceTab + (nCurDockId - 1) * g_DockPriceTabFile.GetWidth() + nNextDockId - 1) = t;
	}
	return 	g_DockPriceUnit * (*(g_pDockPriceTab + (nCurDockId - 1) * g_DockPriceTabFile.GetWidth() + nNextDockId - 1));


}

KTimerTaskFun::KTimerTaskFun()
{
	m_dwTimeTaskTime = 0;
	m_dwTimerTaskId = 0;
	m_dwIntervalTime = 0;
}

KTabFile KTimerTaskFun::m_TimerTaskTab;
BOOL	KTimerTaskFun::Init()
{
	m_TimerTaskTab.Clear();
	if (!m_TimerTaskTab.Load(TIMERTASK_SETTINGFILE))
	{
		g_DebugLog("[TASK]Can Not Load %s,ERRRROR!", TIMERTASK_SETTINGFILE);
		return FALSE;
	}
	return TRUE;
}


BOOL	KTimerTaskFun::Activate(PF_TimerCallBackFun TimerCallBackFun)
{
	if (m_dwTimeTaskTime && m_dwTimeTaskTime <=  g_SubWorldSet.GetGameTime())
	{
		//自动设置下一个周期时间
		m_dwTimeTaskTime = g_SubWorldSet.GetGameTime() + m_dwIntervalTime; 
		
		if (m_dwTimerTaskId)
		{
			char szTimerScript[MAX_PATH];
			g_TimerTask.GetTimerTaskScript(szTimerScript, m_dwTimerTaskId, MAX_PATH);
			TimerCallBackFun(m_pTimerOwner, szTimerScript);
		}
	}
	return TRUE;
}

BOOL	KTimerTaskFun::SaveTask(KPlayer * pPlayer)
{
	if (!pPlayer)
	{
		return FALSE;
	}
	pPlayer->m_cTask.SetSaveVal(TASKVALUE_TIMERTASK_TAST, 0);
	pPlayer->m_cTask.SetSaveVal(TASKVALUE_TIMERTASK_RESTTIME, 0);
	
	if (m_dwTimerTaskId)
	{
		int nRestTime = m_dwTimeTaskTime - g_SubWorldSet.GetGameTime();

		if (nRestTime > 0)
		{
			pPlayer->m_cTask.SetSaveVal(TASKVALUE_TIMERTASK_TAST, m_dwTimerTaskId);
			pPlayer->m_cTask.SetSaveVal(TASKVALUE_TIMERTASK_RESTTIME, nRestTime);
		}
	}
	return TRUE;
}

BOOL	KTimerTaskFun::LoadTask(KPlayer * pPlayer)
{
	if (!pPlayer) 
	{
		return FALSE;
	}
	
	int nTimerTaskId = pPlayer->m_cTask.GetSaveVal(TASKVALUE_TIMERTASK_TAST);
	int nRestTime = pPlayer->m_cTask.GetSaveVal(TASKVALUE_TIMERTASK_RESTTIME);
	m_dwTimerTaskId = 0;
	m_dwTimeTaskTime = 0;
	
	if (nRestTime > 0)
	{
		int nAtTime = nRestTime + g_SubWorldSet.GetGameTime();
		m_dwTimerTaskId = nTimerTaskId;
		m_dwTimeTaskTime = nAtTime;
	}
	return TRUE;
}

void	KTimerTaskFun::GetTimerTaskScript(char * szScriptFileName, unsigned short usTimerTaskId, size_t nScriptFileLen)
{
	if (!szScriptFileName)
		return ;
	
	szScriptFileName[0] = 0;
	char szTaskId[20];
	sprintf(szTaskId, "%d", usTimerTaskId);
	m_TimerTaskTab.GetString(szTaskId, "SCRIPT", "", szScriptFileName, nScriptFileLen);
}
KTimerTaskFun g_TimerTask;

int g_PlayerTimerCallBackFun(void * pOwner, char * szScriptFileName)
{
	if (!pOwner)
		return 0;
	KPlayer * pPlayer = (KPlayer * )pOwner;
	pPlayer->ExecuteScript(szScriptFileName, "OnTimer", 0);
	return 1;
};

#endif 
