//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerTask.cpp
// Date:	2002.10.05
// Code:	边城浪子
// Desc:	PlayerTask Class
//---------------------------------------------------------------------------

#include	"KCore.h"
#include	"KPlayerTask.h"

#ifdef _SERVER
int			g_TaskGlobalValue[TASKGLOBALVALUENUM]; //全局的变量，用于服务器脚本系统

KTabFile	g_WayPointTabFile;
KTabFile	g_StationTabFile;
KTabFile	g_DockTabFile;
KTabFile	g_StationPriceTabFile;
KTabFile	g_WayPointPriceTabFile;
KTabFile	g_DockPriceTabFile;

int	*		g_pStationPriceTab = NULL;
int	*		g_pWayPointPriceTab = NULL;
int	*		g_pDockPriceTab = NULL;

#endif
//---------------------------------------------------------------------------
//	功能：构造函数
//---------------------------------------------------------------------------
KPlayerTask::KPlayerTask()
{
	Release();
}

//---------------------------------------------------------------------------
//	功能：清空
//---------------------------------------------------------------------------
void	KPlayerTask::Release()
{
	memset(nSave, 0, sizeof(nSave));
	memset(nClear, 0, sizeof(nClear));
}

//---------------------------------------------------------------------------
//	功能：清除临时过程控制变量
//---------------------------------------------------------------------------
void	KPlayerTask::ClearTempVar()
{
	memset(nClear, 0, sizeof(nClear));
}

//---------------------------------------------------------------------------
//	功能：得到任务完成情况
//---------------------------------------------------------------------------
int		KPlayerTask::GetSaveVal(int nNo)
{
	if (nNo < 0 || nNo >= MAX_TASK)
		return 0;
	return nSave[nNo];
}

//---------------------------------------------------------------------------
//	功能：设定任务完成情况
//---------------------------------------------------------------------------
void	KPlayerTask::SetSaveVal(int nNo, BOOL bFlag)
{
	if (nNo < 0 || nNo >= MAX_TASK)
		return;
	nSave[nNo] = bFlag;
}

//---------------------------------------------------------------------------
//	功能：得到临时过程控制变量值
//---------------------------------------------------------------------------
int		KPlayerTask::GetClearVal(int nNo)
{
	if (nNo < 0 || nNo >= MAX_TEMP_TASK)
		return 0;
	return nClear[nNo];
}

//---------------------------------------------------------------------------
//	功能：设定临时过程控制变量值
//---------------------------------------------------------------------------
void	KPlayerTask::SetClearVal(int nNo, int nVal)
{
	if (nNo < 0 || nNo >= MAX_TEMP_TASK)
		return;
	nClear[nNo] = nVal;
}
