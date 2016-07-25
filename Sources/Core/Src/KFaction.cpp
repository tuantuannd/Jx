//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KFaction.cpp
// Date:	2002.09.26
// Code:	边城浪子
// Desc:	Faction Class
//---------------------------------------------------------------------------

#include	"KCore.h"
//#include	"MyAssert.h"
#include	"KIniFile.h"
#include	"KSkills.h"
#include	"KFaction.h"
#include	"CoreUseNameDef.h"


KFaction	g_Faction;


//---------------------------------------------------------------------------
//	功能：初始化，载入门派说明文件
//---------------------------------------------------------------------------
BOOL	KFaction::Init()
{
	KIniFile	Ini;
	char		szSection[80], szBuffer[32];
	char		szSeries[series_num][16] = {"金", "木", "水", "火", "土"};
	char		szCamp[camp_num][16] = {"新手", "正派", "邪派", "中立", "杀手", "野兽", "路人"};
	int			i, j, k, nArrayPos;

	// 清空
	for (i = 0; i < MAX_FACTION; i++)
	{
		m_sAttribute[i].m_nIndex = i;
		m_sAttribute[i].m_nSeries = series_metal;
		m_sAttribute[i].m_nCamp = camp_justice;
		m_sAttribute[i].m_szName[0] = 0;
	}

	if ( !Ini.Load(FACTION_FILE) )
		return FALSE;

	// 读入每一个门派数据
	for (i = 0; i < MAX_FACTION; i++)
	{
		sprintf(szSection, "%d", i);
		Ini.GetString(szSection, "Series", "金", szBuffer, sizeof(szBuffer));
		// 找到阵营数组中的相应位置
		for (j = 0; j < series_num; j++)
		{
			if (strcmp(szBuffer, szSeries[j]) != 0)
				continue;
			for (k = 0; k < FACTIONS_PRR_SERIES; k++)
			{
				if (m_sAttribute[j * FACTIONS_PRR_SERIES + k].m_szName[0] == 0)
				{
					nArrayPos = j * FACTIONS_PRR_SERIES + k;
					m_sAttribute[nArrayPos].m_nSeries = j;
					break;
				}
			}
			break;
		}
		_ASSERT(j < series_num);
		Ini.GetString(szSection, "Name", "少林派", m_sAttribute[nArrayPos].m_szName, sizeof(m_sAttribute[nArrayPos].m_szName));
		Ini.GetString(szSection, "Camp", "正义", szBuffer, sizeof(szBuffer));
		for (j = 0; j < camp_num; j++)
		{
			if (strcmp(szBuffer, szCamp[j]) == 0)
			{
				m_sAttribute[nArrayPos].m_nCamp = j;
				break;
			}
		}
		_ASSERT(j < camp_num);
	}

	return TRUE;
}

//---------------------------------------------------------------------------
//	功能：根据五行属性和本属性第几个门派得到门派编号
//---------------------------------------------------------------------------
int		KFaction::GetID(int nSeries, int nNo)
{
	if (nSeries < series_metal || nSeries >= series_num || nNo < 0 || nNo >= FACTIONS_PRR_SERIES)
		return -1;
	return nSeries * FACTIONS_PRR_SERIES + nNo;
}

//---------------------------------------------------------------------------
//	功能：根据五行属性和门派名得到门派编号
//---------------------------------------------------------------------------
int		KFaction::GetID(int nSeries, char *lpszName)
{
	if (nSeries < series_metal || nSeries >= series_num)
		return -1;
	if ( !lpszName || !lpszName[0])
		return -1;
	for (int i = nSeries * FACTIONS_PRR_SERIES; i < (nSeries + 1) * FACTIONS_PRR_SERIES; i++)
	{
		if (strcmp(lpszName, m_sAttribute[i].m_szName) == 0)
			return i;
	}
	return -1;
}

//---------------------------------------------------------------------------
//	功能：获得某个门派的阵营
//---------------------------------------------------------------------------
int		KFaction::GetCamp(int nFactionID)
{
	if (nFactionID < 0 || nFactionID >= MAX_FACTION)
		return -1;
	return m_sAttribute[nFactionID].m_nCamp;
}
