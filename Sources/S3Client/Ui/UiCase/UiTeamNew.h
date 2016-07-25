/*****************************************************************************************
//	界面--新建队伍界面
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-2-26
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once

#include "../Elem/WndShowAnimate.h"
#include "../Elem/WndButton.h"
#include "../Elem/WndEdit.h"

class KUiTeamNew : protected KWndShowAnimate
{
public:
	static KUiTeamNew*	OpenWindow();		//打开窗口，返回唯一的一个类对象实例
	static KUiTeamNew*	GetIfVisible();		//如果窗口正被显示，则返回实例指针
	static void			CloseWindow();		//关闭窗口，同时可以选则是否删除对象实例
private:
	void	Initialize();					//初始化
	void	LoadScheme(const char* pScheme);//载入界面方案
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	void	OnOk();
	bool	CheckInput(char* pName);
private:
	static KUiTeamNew*		m_pSelf;
	KWndEdit32				m_TeamName;
	KWndButton				m_OkBtn, m_CancelBtn;
};