//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerTask.h
// Date:	2002.10.05
// Code:	边城浪子
// Desc:	PlayerTask Class
//---------------------------------------------------------------------------

#ifndef KPLAYERTASK_H
#define KPLAYERTASK_H

#define		MAX_TASK		256
#define		MAX_TEMP_TASK	2560



#ifdef _SERVER
#define TASKGLOBALVALUENUM 5000
extern int		g_TaskGlobalValue[TASKGLOBALVALUENUM];

extern KTabFile g_WayPointTabFile;
extern KTabFile g_StationTabFile;
extern KTabFile g_DockTabFile;

extern KTabFile g_StationPriceTabFile;
extern KTabFile g_WayPointPriceTabFile;
extern KTabFile g_DockPriceTabFile;

extern int	*g_pStationPriceTab;
extern int  *g_pWayPointPriceTab;
extern int  *g_pDockPriceTab;

#endif


class KPlayerTask
{
public:
	int			nSave[MAX_TASK];					// 用于记录任务是否完成，须保存到数据库
	int			nClear[MAX_TEMP_TASK];				// 用于记录任务过程中的中间步骤的完成情况，不保存到数据库，玩家下线后次数据清空
public:
	KPlayerTask();									// 构造函数
	void		Release();							// 清空
	void		ClearTempVar();						// 清除临时过程控制变量
	int			GetSaveVal(int nNo);				// 得到任务完成情况
	void		SetSaveVal(int nNo, BOOL bFlag);	// 设定任务完成情况
	int			GetClearVal(int nNo);				// 得到临时过程控制变量值
	void		SetClearVal(int nNo, int nVal);		// 设定临时过程控制变量值
};

#endif
