/*****************************************************************************************
//	界面--管理界面
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-2
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "../Elem/WndEdit.h"
#include "../Elem/WndImagePart.h"
#include "UiTeamManage.h"

// -------------------------------------------------------------------------
// ---> 门派管理
class KUiCliqueManage : public KWndPage
{
public:
	int		Init();									//初始化
	void	LoadScheme(const char* pScheme);		//载入界面方案	
protected:
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
};

// -------------------------------------------------------------------------
// ---> 帮会界面
class KUiConfraternityManage : public KWndPage
{
public:
	int		Init();									//初始化
	void	LoadScheme(const char* pScheme);		//载入界面方案	
protected:
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ---> 管理窗口
class KUiManage : protected KWndPageSet
{
public:
	static KUiManage*	OpenWindow();					//打开窗口，返回唯一的一个类对象实例
	static KUiManage*	GetIfVisible();					//如果窗口正被显示，则返回实例指针
	static void			CloseWindow(bool bDestroy);		//关闭窗口，同时可以选则是否删除对象实例
	static void			LoadScheme(const char* pScheme);		//载入界面方案

	void	UpdateLeaderData();
	void	UpdateTeamData(int nID);
	void	UpdateTeamChange(const KUiPlayerItem* playerItem,int bAdd);
	void	UpdateToJoinChange(const KUiPlayerItem* playerItem,int bAdd);

private:
	KUiManage(){}
	~KUiManage(){ m_pSelf = 0;}
	void	Initialize();								//初始化
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	
private:
	static KUiManage* m_pSelf;
private:
	// 三个属性页
	KUiTeamManage			m_TeamPad;
	KUiCliqueManage			m_CliquePad;
	KUiConfraternityManage	m_ConfraternityPad;
	// 与属性页相关联的按钮
	KWndButton				m_TeamPadBtn;
	KWndButton				m_CliquePadBtn;
	KWndButton				m_ConfraternityPadBtn;
	// 在属性集窗口上的控件
	KWndButton				m_Close;
	KWndText32				m_LeaderShipLevel;
	KWndImagePart			m_LeaderShipExperience;
};

// -------------------------------------------------------------------------

//extern KUiManage		g_UiManage;
