/*****************************************************************************************
//	界面--小地图
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-4-21
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "../Elem/WndImage.h"
#include "../Elem/WndButton.h"
#include "../Elem/WndText.h"
#include "../Elem/WndShadow.h"
#include "../Elem/PopupMenu.h"
#include "../../Core/Src/GameDataDef.h"

//小地图的显示内容模式
enum MINIMAP_MODE
{
	MINIMAP_M_BRIEF_PIC = 0,			//缩略图
	MINIMAP_M_BRIEF_PIC_BROWSE,			//缩略图浏览/放大模式
	MINIMAP_M_WORLD_MAP,				//世界地图

	//MINIMAP_M_OBSTACLE,				//障碍图模式
	MINIMAP_M_NONE,					//没有显示地图

	MINIMAP_M_COUNT,
};

struct KUiSceneTimeInfo;

class KUiMiniMap : public KWndImage
{
public:
	//----界面面板统一的接口函数----
	static KUiMiniMap*	OpenWindow();					//打开窗口，返回唯一的一个类对象实例
	static KUiMiniMap*	GetIfVisible();					//如果窗口正被显示，则返回实例指针
	static void			CloseWindow();					//关闭窗口

	static void			SetMode(MINIMAP_MODE eMode);	//设置显示内容模式
	static void			LoadScheme(const char* pScheme);//载入界面方案
	static void			UpdateSceneTimeInfo(KUiSceneTimeInfo* pInfo);

	static KUiMiniMap*	GetSelf() {return ms_pSelf;}

	void	Show();										//显示窗口
	void	Hide();										//隐藏窗口

private:
	KUiMiniMap();
	~KUiMiniMap() {}
	void	Initialize();
	void	PaintWindow();
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	void	LoadScheme(KIniFile* pIni);		//载入界面方案
	void	SetStatus(bool bSmall);			//切换状态
	void    MapScroll(int nbScrollScene);	//地图卷动
	void	StopScrollMap();				//停止拖动地图
	void	MapMoveBack();					//地图还原原始偏移
	virtual void	Breathe();

private:
	static KUiMiniMap*	ms_pSelf;
private:
	unsigned int	m_uShowElem;
	POINT			m_MapPos;
	SIZE			m_MapSize;
	KWndButton		m_SwitchBtn;
	KWndText32		m_SceneName;
	KWndText32		m_ScenePos;
	KWndShadow		m_Shadow;
	KWndButton		m_WorldMapBtn;

	POINT           m_OldPos;
	unsigned int	m_uLastScrollTime;             // 离最后一次滚动的时间(秒)
	int             m_nOffsetX;                    // 每次Loop，检测到跟上次鼠标位置的偏移X
	int             m_nOffsetY;                    // 每次Loop，检测到跟上次鼠标位置的偏移Y
};

void	MapToggleStatus();
void	MapSetMode(MINIMAP_MODE eMode);	//设置显示内容模式