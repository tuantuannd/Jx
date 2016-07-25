#ifndef KLadderH
#define	KLadderH

#include "KProtocol.h"

#define	MAX_FAC series_num * FACTIONS_PRR_SERIES + 1
enum
{
	enumLadderBegin = 0,
	enumTopTenMasterHand,
	enumTopTenRicher,
	enumTopTenKiller,
	enumFacTopTenMasterHand,
	enumFacTopTenRicher = enumFacTopTenMasterHand + MAX_FAC,
	enumLadderEnd = enumFacTopTenRicher + MAX_FAC,
};

class KLadder
{
public:
	KLadder();
	~KLadder();
	const TRoleList*	TopTenRich();
	const TRoleList*	TopTenMasterHand();
	const TRoleList*	TopTenKiller();
	const TRoleList*	TopTenFacRich(int nFac);
	const TRoleList*	TopTenFacMasterHand(int nFac);
	const TRoleList*	GetTopTen(DWORD	dwLadderID);
	int					GetFacMemberCount(int nFac);
	int					GetFacMasterHandPercent(int nFac);
	int					GetFacMoneyPercent(int nFac);
	BOOL				Init(void* pData, size_t uSize);
private:
	TGAME_STAT_DATA	GameStatData;
};

extern KLadder	Ladder;
#endif