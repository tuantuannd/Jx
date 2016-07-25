//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KFaction.h
// Date:	2002.09.26
// Code:	边城浪子
// Desc:	Faction Class
//---------------------------------------------------------------------------

#ifndef KFACTION_H
#define KFACTION_H

#include "GameDataDef.h"

#define		FACTIONS_PRR_SERIES				2			// 五行每个系的门派数
#define		MAX_FACTION						(FACTIONS_PRR_SERIES * series_num)	// 总的门派数

class KFaction
{
public:
	struct	SFactionAttirbute							// 每个门派的属性
	{
		int		m_nIndex;								// 编号
		int		m_nSeries;								// 五行属性
		int		m_nCamp;								// 阵营
		char	m_szName[64];							// 门派名
	}				m_sAttribute[MAX_FACTION];			// 所有门派

public:
	BOOL			Init();								// 初始化，载入门派说明文件
	int				GetID(int nSeries, int nNo);		// 根据五行属性和本属性第几个门派得到门派编号
	int				GetID(int nSeries, char *lpszName);	// 根据五行属性和门派名得到门派编号
	int				GetCamp(int nFactionID);			// 获得某个门派的阵营
};

extern	KFaction	g_Faction;
#endif
