#include "KCore.h"
#include "KPlayerFaction.h"
#include "KLadder.h"

KLadder	Ladder;

KLadder::KLadder()
{
	ZeroMemory(&GameStatData, sizeof(TGAME_STAT_DATA));
}

KLadder::~KLadder()
{
	ZeroMemory(&GameStatData, sizeof(TGAME_STAT_DATA));
}

BOOL KLadder::Init(void* pData, size_t uSize)
{
	if (uSize != sizeof(TGAME_STAT_DATA))
		return FALSE;

	memcpy(&GameStatData, pData, uSize);
	return TRUE;
}

const TRoleList* KLadder::TopTenFacMasterHand(int nFac)
{
	if (nFac < - 1 || nFac >= series_num * FACTIONS_PRR_SERIES)
		return NULL;
	return GameStatData.LevelStatBySect[nFac + 1];
}

const TRoleList* KLadder::TopTenFacRich(int nFac)
{
	if (nFac < - 1 || nFac >= series_num * FACTIONS_PRR_SERIES)
		return NULL;
	return GameStatData.MoneyStatBySect[nFac + 1];
}

const TRoleList* KLadder::TopTenKiller()
{
	return GameStatData.KillerStat;
}

const TRoleList* KLadder::TopTenMasterHand()
{
	return GameStatData.LevelStat;
}

const TRoleList* KLadder::TopTenRich()
{
	return GameStatData.MoneyStat;
}

int KLadder::GetFacMasterHandPercent(int nFac)
{
	if (nFac < - 1 || nFac >= series_num * FACTIONS_PRR_SERIES)
		return 0;
	return GameStatData.SectLevelMost[nFac + 1];
}

int KLadder::GetFacMoneyPercent(int nFac)
{
	if (nFac < - 1 || nFac >= series_num * FACTIONS_PRR_SERIES)
		return NULL;
	return GameStatData.SectMoneyMost[nFac + 1];
}

int KLadder::GetFacMemberCount(int nFac)
{
	if (nFac < - 1 || nFac >= series_num * FACTIONS_PRR_SERIES)
		return NULL;
	return GameStatData.SectPlayerNum[nFac + 1];
}

const TRoleList* KLadder::GetTopTen(DWORD dwLadderID)
{
	if (dwLadderID <= enumLadderBegin || dwLadderID >= enumLadderEnd)
	{
		return NULL;
	}

	if (dwLadderID == enumTopTenMasterHand)
	{
		return GameStatData.LevelStat;
	}
	else if (dwLadderID == enumTopTenRicher)
	{
		return GameStatData.MoneyStat;
	}
	else if (dwLadderID == enumTopTenKiller)
	{
		return GameStatData.KillerStat;
	}
	else if (dwLadderID < enumFacTopTenRicher)
	{
		return TopTenFacMasterHand(dwLadderID - enumFacTopTenMasterHand - 1);
	}
	else
	{
		return TopTenFacRich(dwLadderID - enumFacTopTenRicher - 1);
	}
}