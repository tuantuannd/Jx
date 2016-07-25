/*****************************************************************************************
//	界面--申请队伍界面
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-2-26
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once

#include "../Elem/WndShowAnimate.h"
#include "../Elem/WndButton.h"
#include "../Elem/WndScrollBar.h"
#include "../Elem/WndList.h"

struct KUiTeamItem;

class KUiTeamApply : protected KWndShowAnimate
{
public:
	static KUiTeamApply* OpenWindow();			//打开窗口，返回唯一的一个类对象实例
	static KUiTeamApply* GetIfVisible();		//如果窗口正被显示，则返回实例指针
	static void			 CloseWindow();			//关闭窗口，同时可以选则是否删除对象实例
	static void			 LoadScheme(const char* pScheme);//载入界面方案
	static void			 UpdateData(KUiTeamItem* pList, int nCount);
private:
	KUiTeamApply();
	~KUiTeamApply();
	void	Initialize();						//初始化
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	void	OnApply();
	void	Clear();
	void	OnNewTeam();						//自己新建立队伍
private:
	static KUiTeamApply*	m_pSelf;
	KWndButton				m_RefuseBtn, m_ApplyBtn, m_NewBtn, m_CloseBtn, m_RefreshBtn;
	KWndList				m_TeamList;
	KWndScrollBar			m_ListScroll;
	KUiTeamItem*			m_pDataList;
	int						m_nCount;
};