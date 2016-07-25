/*****************************************************************************************
//	遊戲窗口的接口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-18
------------------------------------------------------------------------------------------
	不可忘却的9-18国耻！忘记过去意味着背叛！
*****************************************************************************************/
#pragma once

#include "../Elem/WndGameSpace.h"
#include "../Elem/WndWindow.h"

class KUiGameSpace : public KWndWindow
{
public:
	//输入处理消息以及一些特定窗口消息的响应
	int	WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	//绘制游戏世界
	void PaintWindow();

	void OnMouseMoveCursor(int x,int y);

	KUiPlayerItem m_SelectPlayer;
};

extern KUiGameSpace	g_WndGameSpace;

enum
{
	ACTION_CHAT = 0,					//要用Name
	ACTION_MAKEFRIEND,					//要用Name
	ACTION_TRADE,						//pDest->nData == PLAYER_MENU_STATE_TRADEOPEN && pDest->nIndex != -1
	ACTION_JOINTEAM,					//pDest->nData == PLAYER_MENU_STATE_TEAMOPEN && pDest->nIndex != -1
	ACTION_INVITETEAM,					//pDest->uId != 0
	ACTION_FOLLOW,						//pDest->nIndex != -1
	ACTION_REVENGE,						//pDest->uId != 0
	ACTION_VIEWITEM,					//pDest->uId != 0
	ACTION_BLACKLIST,					//要用Name
	ACTION_TONG,                        //要用Name
};

//pDest的Name必须填, 然后其他根据具体情况填写
void ProcessPeople(KUiPlayerItem* pDest, int nAction);
void ProcessEmote(char* szDest, char *szDestChannel = "", int nEmoteIndex = 0);

void PopUpContextPeopleMenu(const KUiPlayerItem& SelectPlayer, int x, int y);
void PopUpContextEmoteMenu(char* szDest, int x, int y);