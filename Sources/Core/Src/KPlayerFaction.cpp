//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerFaction.cpp
// Date:	2002.09.26
// Code:	边城浪子
// Desc:	PlayerFaction Class
//---------------------------------------------------------------------------

#include	"KCore.h"
#include	"MyAssert.H"
#include	"GameDataDef.h"
#include	"KFaction.h"
#include	"KPlayerFaction.h"

#define		FACTION_NEW			"初入江湖"
#define		FACTION_OLD			"江湖游侠"

//---------------------------------------------------------------------------
//	功能：构造函数
//---------------------------------------------------------------------------
KPlayerFaction::KPlayerFaction()
{
	Release();
}

//---------------------------------------------------------------------------
//	功能：清空
//---------------------------------------------------------------------------
void	KPlayerFaction::Release()
{
	m_nCurFaction = -1;
	m_nFirstAddFaction = -1;
	m_nAddTimes = 0;
}

//---------------------------------------------------------------------------
//	功能：根据玩家的五行属性确定玩家门派数据
//---------------------------------------------------------------------------
void	KPlayerFaction::SetSeries(int nSeries)
{
	return;
/*
	if (nSeries < series_metal || nSeries >= series_num)
		return;
	int		nID;
	for (int i = 0; i < FACTIONS_PRR_SERIES; i++)
	{
		nID = g_Faction.GetID(nSeries, i);
		_ASSERT(nID >= 0);
		if (m_sSkillOpen[i].m_nID != nID)
		{
			m_sSkillOpen[i].m_nID = nID;
			m_sSkillOpen[i].m_nOpenLevel = 0;
		}
	}
*/
}

//---------------------------------------------------------------------------
//	功能：加入门派
//---------------------------------------------------------------------------
BOOL	KPlayerFaction::AddFaction(int nSeries, char *lpszFactionName)
{
	int		nID;

	nID = g_Faction.GetID(nSeries, lpszFactionName);
	if (nID < 0)
		return FALSE;
	return AddFaction(nSeries, nID);
}

//---------------------------------------------------------------------------
//	功能：加入门派
//---------------------------------------------------------------------------
BOOL	KPlayerFaction::AddFaction(int nSeries, int nFactionID)
{
	if (nSeries < series_metal || nSeries >= series_num)
		return FALSE;
	if (nFactionID < nSeries * FACTIONS_PRR_SERIES || nFactionID >= (nSeries + 1) * FACTIONS_PRR_SERIES)
		return FALSE;

	SetSeries(nSeries);
	m_nCurFaction = nFactionID;
	m_nAddTimes++;
	if (m_nAddTimes == 1)
		m_nFirstAddFaction = nFactionID;

	return TRUE;
}

//---------------------------------------------------------------------------
//	功能：离开当前门派
//---------------------------------------------------------------------------
void	KPlayerFaction::LeaveFaction()
{
	m_nCurFaction = -1;
}

//---------------------------------------------------------------------------
//	功能：开放当前门派某个等级的技能
//---------------------------------------------------------------------------
BOOL	KPlayerFaction::OpenCurSkillLevel(int nLevel, KSkillList *pSkillList)
{
	return TRUE;
/*
	if (nLevel < 0 || nLevel >= FACTION_SKILL_LEVEL || !pSkillList)
		return FALSE;
	if (m_nCurFaction < 0)
		return FALSE;

	int		i, j;

	// 设定当前开放等级
	for (i = 0; i < FACTIONS_PRR_SERIES; i++)
	{
		if (m_nCurFaction == m_sSkillOpen[i].m_nID)
		{
			m_sSkillOpen[i].m_nOpenLevel = nLevel;
			break;
		}
	}

	// 设定玩家技能
	for (i = 0; i <= nLevel; i++)
	{
		for (j = 0; j < FACTION_SKILLS_PER_LEVEL; j++)
		{
			if (g_Faction.m_sAttribute[m_nCurFaction].m_nSkill[i][j] <= 0)
				continue;
			pSkillList->Add(g_Faction.m_sAttribute[m_nCurFaction].m_nSkill[i][j], 0);
		}
	}

	return TRUE;
*/
}

//---------------------------------------------------------------------------
//	功能：获得当前门派阵营
//---------------------------------------------------------------------------
int		KPlayerFaction::GetGurFactionCamp()
{
	if (m_nCurFaction < 0)
	{
		if (m_nAddTimes == 0)
			return camp_begin;
		else
			return camp_free;
	}
	else
	{
		if (g_Faction.GetCamp(m_nCurFaction) >= 0)
			return g_Faction.GetCamp(m_nCurFaction);
		else
			return camp_begin;
	}
}

//---------------------------------------------------------------------------
//	功能：获得当前门派
//---------------------------------------------------------------------------
int		KPlayerFaction::GetCurFactionNo()
{
	return m_nCurFaction;
}

//---------------------------------------------------------------------------
//	功能：获得当前门派
//---------------------------------------------------------------------------
void	KPlayerFaction::GetCurFactionName(char *lpszGetName)
{
	if (!lpszGetName)
		return;

	if (this->m_nCurFaction == -1)
	{
		if (this->m_nAddTimes == 0)
		{
			lpszGetName[0] = 0;
		}
		else
		{
			strcpy(lpszGetName, FACTION_NEW);
		}
	}
	else
	{
		strcpy(lpszGetName, g_Faction.m_sAttribute[m_nCurFaction].m_szName);
	}

	return;
}



