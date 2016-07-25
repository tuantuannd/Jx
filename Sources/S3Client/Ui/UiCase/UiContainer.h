/* 
 * File:     UiContainer.h
 * Desc:     用户面板容器类
 * Author:   flying
 * Creation: 2003/7/4
 */
//-----------------------------------------------------------------------------
#pragma once

enum MSG_SWITCH_UI
{
	SWITCH_TO_MESSAGE = 0,
	SWITCH_TO_FRIEND,
	SWITCH_TO_EXCHANGE,
	SWITCH_TO_STATUS,
	SWITCH_TO_ITEM,
	SWITCH_TO_SKILL,
	SWITCH_TO_TEAM,
	SWITCH_TO_SIT,
	SWITCH_TO_RUN,
	SWITCH_COUNT,
};

class KUiContainer
{
public:
	KUiContainer();
	virtual ~KUiContainer();
	int DoSwitchUi(int nSwitch);
	int Initialize(KWndWindow* InitWnd);
	//int RegisterWnds(KWndWindow* pWnd, int nMessage);
private:
	KWndWindow* pCurrentWnd;
	int			nCurrentSwitch;
};