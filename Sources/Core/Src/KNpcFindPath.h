//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNpcFindPath.h
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Obj Class
//---------------------------------------------------------------------------

#ifndef KNPCFINDPATH_H
#define KNPCFINDPATH_H

class KNpcFindPath
{
public:
	int				m_NpcIdx;			// 这个寻路是属于哪个 npc 的
	int				m_nDestX;			// 目标点坐标 x
	int				m_nDestY;			// 目标点坐标 y
	int				m_nFindTimer;		// 找路，一段时间以后没找到就不找了
	int				m_nMaxTimeLong;		// 找多长时间的路
	int				m_nFindState;		// 当前是否处于找路状态
	int				m_nPathSide;		// 往哪个方向找
	int				m_nFindTimes;		// 朝一个目标点找路的次数
public:
	KNpcFindPath();
	//	功能：初始化	nNpc :这个寻路是属于哪个 npc 的
	void			Init(int nNpc);
	//	功能：传入当前坐标、方向、目标点坐标、速度，寻路找到下一步应该走的方向
	//	返回值；如果返回false：不能走了;true ，找到一个方向，方向值放在pnGetDir (按64方向)
	int				GetDir(int nXpos,int nYpos, int nDir, int nDestX, int nDestY, int nMoveSpeed, int *pnGetDir);
	// 64 方向转换为 8 方向
	int				Dir64To8(int nDir);
	// 8 方向转换为 64 方向
	int				Dir8To64(int nDir);
	// 判断两点间的直线距离是否大于或等于给定距离
	BOOL			CheckDistance(int x1, int y1, int x2, int y2, int nDistance);
	// 判断某个点是否是障碍
	int				CheckBarrier(int nChangeX, int nChangeY);
};
#endif
