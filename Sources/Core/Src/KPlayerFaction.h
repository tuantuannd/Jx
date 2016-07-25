//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerFaction.h
// Date:	2002.09.26
// Code:	边城浪子
// Desc:	PlayerFaction Class
//---------------------------------------------------------------------------

#ifndef KPLAYERFACTION_H
#define KPLAYERFACTION_H

#include	"KFaction.h"
#include	"KSkillList.h"

// Player 自带的有关门派的模块
class KPlayerFaction
{
public:
	int		m_nCurFaction;						// 当前加入门派 id ，如果为 -1 ，当前没有在门派中
	int		m_nFirstAddFaction;					// 第一次加入的门派 id
	int		m_nAddTimes;						// 当前加入门派的次数
//	struct	SSkillOpen
//	{
//		int		m_nID;							// 本系（五行）某个门派 id
//		int		m_nOpenLevel;					// 本系某个门派技能开放到第几级
//	}		m_sSkillOpen[FACTIONS_PRR_SERIES];	// 本系某个门派的技能开放度
public:
	KPlayerFaction();							// 构造函数
	void	Release();
	void	SetSeries(int nSeries);				// 根据玩家的五行属性确定玩家门派数据
	BOOL	AddFaction(int nSeries, char *lpszFactionName);// 加入门派
	BOOL	AddFaction(int nSeries, int nFactionID);// 加入门派
	void	LeaveFaction();						// 离开当前门派
	BOOL	OpenCurSkillLevel(int nLevel, KSkillList *pSkillList);// 开放当前门派某个等级的技能
	int		GetGurFactionCamp();				// 获得当前门派阵营
	int		GetCurFactionNo();					// 获得当前门派
	void	GetCurFactionName(char *lpszGetName);
};

/*
注：每个玩家一定会属于五行的某个系（金木水火土）。每个系会有两个门派，
玩家只可能加入这两个门派。每个门派都有一些技能可供学习。这些技能是分
等级的，只有这个等级对此玩家开放了，此玩家才能学习这个等级的技能。高
等级技能开放意味着比此等级低的其他等级也一并开放，所以只需纪录当前开
放到第几个等级就行了。玩家第一次加入的门派的等级技能开放条件和以后加
入的门派的开放条件是不一样的，所以必须纪录当前加入门派的次数。
*/
#endif
