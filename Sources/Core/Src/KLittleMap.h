//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KLittleMap.h
// Date:	2003.05.08
// Code:	边城浪子
//---------------------------------------------------------------------------

#ifndef KLITTLEMAP_H
#define KLITTLEMAP_H

#ifndef _SERVER
#include	"../Represent/iRepresent/iRepresentShell.h"
#include "../../Represent/iRepresent/KRepresentUnit.h"

#define	LITTLE_MAP_POINT_NUM			500
#define	LITTLE_MAP_SHOW_REGION_WIDTH	4
#define	LITTLE_MAP_SHOW_REGION_HEIGHT	4

class	KLittleMap
{
private:
	RECT		m_sMapRect;				// 本地图的region范围(左上、右下)
	int			m_nMapWidth;			// 地图长(region, m_sMapRect.right - m_sMapRect.left + 1)
	int			m_nMapHeight;			// 地图宽(region, m_sMapRect.bottom - m_sMapRect.top + 1)
	BYTE		*m_pbyLoadFlag;			// 某个region的障碍数据载入与否
	BYTE		**m_ppbtBarrier;		// 所有region的障碍数据

	int			m_nScreenX;				// 屏幕显示左上角坐标
	int			m_nScreenY;				// 屏幕显示左上角坐标
	int			m_nShowWidth;			// 显示宽多少个region的障碍
	int			m_nShowHeight;			// 显示高多少个region的障碍

	KRColor		m_sBarrierColor;		// 障碍颜色
	KRColor		m_sSelfColor;			// 主角颜色
	KRColor		m_sTeammateColor;		// 队友颜色
	KRColor		m_sPlayerColor;			// 其他玩家颜色
	KRColor		m_sFightNpcColor;		// 战斗npc颜色
	KRColor		m_sNormalNpcColor;		// 普通npc颜色
	BOOL		m_bColorLoadFlag;		// 颜色设定文件是否已经载入
	BOOL		m_bTeammateShowFlag;	// 是否显示队友
	BOOL		m_bPlayerShowFlag;		// 是否显示其他玩家
	BOOL		m_bFightNpcShowFlag;	// 是否显示战斗npc
	BOOL		m_bNormalNpcShowFlag;	// 是否显示普通npc

	KRUPoint	m_sPoint[LITTLE_MAP_POINT_NUM];	// 用于绘制
	int			m_nPointPos;		// 用于绘制
	KRUShadow	m_sBigPoint;
public:
	int			m_nShowFlag;		// 是否显示小地图
public:
	KLittleMap();
	~KLittleMap();
	void	Release();
	void	Init(int nLeft, int nTop, int nRight, int nBottom);	// 初始化，分配内存空间
	BYTE	*GetBarrierBuf(int nX, int nY);	// 获得某个region的障碍数据的存储地址
	void	SetHaveLoad(int nX, int nY);	// 设定已载入某个region的障碍数据
	void	Draw(int nX, int nY);			// 绘制
	void	SetTeammateShow(BOOL bFlag);	// 队友显示开关
	void	SetPlayerShow(BOOL bFlag);		// 其它玩家显示开关
	void	SetFightNpcShow(BOOL bFlag);	// 战斗npc显示开关
	void	SetNormalNpcShow(BOOL bFlag);	// 普通npc显示开关
	void	SetScreenPos(int nX, int nY);	// 设定小地图在屏幕上的显示位置
	void	Show(int nFlag);				// 小地图显示开关
	void	SetShowSize(int nWidth, int nHeight);// 设定显示内容的大小
};



#endif

#endif
