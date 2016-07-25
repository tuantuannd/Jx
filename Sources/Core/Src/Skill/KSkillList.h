#ifndef	KSkillListH
#define	KSkillListH
#include "gamedatadef.h"

#define	MAX_NPCSKILL	80
#define	MAX_TRAPNUM		5

typedef struct tagNpcSkill
{
	int		SkillId;
	int		SkillLevel;
	int		MaxTimes;
	int		RemainTimes;
	DWORD	NextCastTime;		//下次可发送时的最小时间
	int		CurrentSkillLevel;
} NPCSKILL, *LPNPCSKILL;
struct TDBPlayerData;
#ifndef TOOLVERSION
class KSkillList
#else 
class CORE_API KSkillList
#endif
{
public:
	int			m_nTrapNpcID[5];
	NPCSKILL	m_Skills[MAX_NPCSKILL];
	int			m_nNpcIndex;
public:
	KSkillList();
	~KSkillList();
	int			Add(int nSkillID, int nLevel = 1, int nMaxTimes = 0, int RemainTimes = 0);
	void		Remove(int nSkillID){};//Question :需要加！
	void		Clear(){memset(m_Skills, 0, sizeof(m_Skills));};
#ifndef _SERVER
	void		SetSkillLevel(int nId, int nLevel);
	BOOL		SetLevel(int nIndex, int nLevel);		// 把某个编号技能设为某一级
	int			GetSkillSortList(KUiSkillData *);		// 获得角色当前所有技能排序列表
	int			GetSkillPosition(int nSkillId);
	int			GetLeftSkillSortList(KUiSkillData*);	// 获得角色当前左键技能排序列表
	int			GetRightSkillSortList(KUiSkillData*);	// 获得角色当前右键技能排序列表
#endif
	BOOL		IncreaseLevel(int nIdx, int nLvl);
	int			GetLevel(int nSkillID);
	int			GetSkillIdxLevel(int nIdx) 
	{
		if (nIdx <= 0) return 0;
		return m_Skills[nIdx].SkillLevel;
	};
	int			GetCurrentLevel(int nSkillID);
	int			GetCount();
	int			FindSame(int nSkillID);
	int			FindFree();
	BOOL		CanCast(int nSkillID, DWORD dwTime);
	void		SetNextCastTime(int nSkillID, DWORD dwTime);
	
	int			GetSkillId(int nListIndex)
	{
		if (nListIndex < MAX_NPCSKILL && nListIndex > 0)
			return m_Skills[nListIndex].SkillId;
		else 
			return -1;
	};
	
#ifdef _SERVER
	int		UpdateDBSkillList(BYTE *);
#endif
};
#endif