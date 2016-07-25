/*****************************************************************************************
//	界面--快速输入消息界面
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-12-11
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once

#include "../Elem/WndButton.h"
#include "../Elem/WndEdit.h"
#include "../Elem/PopupMenu.h"
#include "../../../Represent/iRepresent/KRepresentUnit.h"
#include "../../../Core/src/GameDataDef.h"

class KUiFastInputMsg : protected KWndImage
{
public:
	//----界面面板统一的接口函数----
	static KUiFastInputMsg* OpenWindow();					//打开窗口，返回唯一的一个类对象实例
	static KUiFastInputMsg* GetIfVisible();					//如果窗口正被显示，则返回实例指针
	static void				CloseWindow(bool bDestroy);		//关闭窗口，同时可以选则是否删除对象实例
	static void				LoadScheme(const char* pScheme);//载入界面方案
	static void				Clear();
	static void				UpdateCurrentChannel();
private:
	KUiFastInputMsg();
	~KUiFastInputMsg();
	void	Show();									//显示窗口
	void	Hide();									//隐藏窗口
	void	OnSend();
	void	Initialize();							//初始化
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	void	SwitchLockMode();
	void	PaintWindow();
	void	PopupChannelMenu();
	void	PopupColorMenu();
	void	CancelMenu();
	void	SelectedColor(int x, int y);
	void	LoadScheme(class KIniFile* pIni);	//载入界面方案
	void	InputRecentMsg(bool bPrior);
private:
	static KUiFastInputMsg*	m_pSelf;
private:
	enum { UI_INPUT_MSG_MAX_COLOR = 12 };
	KWndButton	m_ChannelBtn;
	KWndButton	m_ColorBtn;
	KWndButton	m_SendBtn;
	KWndEdit512	m_InputEdit;
	KWndText32	m_ChannelName;
	unsigned int m_ColorList[UI_INPUT_MSG_MAX_COLOR];
	KRColor		m_CurColor;
	bool		m_bLocked;
	char		m_cNumColor;
	char		m_cPreMsgCounter;
	char		m_cLatestMsgIndex;
	bool		m_bChannelMenu;
	bool		m_bColorMenu;
#define	MAX_RECENT_MSG_COUNT	8
	char		m_RecentMsg[MAX_RECENT_MSG_COUNT][512];
	KPopupMenuData* m_pMenuData;
	KUiChatChannel* m_pChannelData;
};
