#ifndef	KSkillListH
#define	KSkillListH
//#include "Windows.h"
#include "GameDataDef.h"

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
	NPCSKILL	m_Skills[MAX_NPCSKILL];//下标从1开始，0为无效index
	int			m_nNpcIndex;
public:
	KSkillList();
	~KSkillList();
	int			Add(int nSkillID, int nLevel = 1, int nMaxTimes = 0, int RemainTimes = 0);
	void		Remove(int nSkillID)
	{
		if (nSkillID <= 0 || nSkillID >= MAX_SKILL) 
			return ;
		int i = FindSame(nSkillID);
		if (i)
		{
			m_Skills[i].CurrentSkillLevel = 0;
			m_Skills[i].SkillId = 0;
			m_Skills[i].SkillLevel = 0;
		}

	};//Question :需要加！

	int			GetSkillId(unsigned long ulSkillIdx) const
	{
		if (ulSkillIdx >= MAX_NPCSKILL || ulSkillIdx == 0) 
			return 0;
		else 
			return m_Skills[ulSkillIdx].SkillId;
	};


	int	SetSkillLevelDirectlyUsingIndex(unsigned long ulSkillIdx, unsigned long ulLevel)/*请慎用该函数，因为目前未加任何被动技能降级对数值的影响*/
	{
		if (ulSkillIdx >= MAX_NPCSKILL ||  ulSkillIdx == 0 || ulLevel >= MAX_SKILLLEVEL)
			return 0;
		
		if (m_Skills[ulSkillIdx].SkillId)
		{
			m_Skills[ulSkillIdx].SkillLevel = ulLevel;
			m_Skills[ulSkillIdx].CurrentSkillLevel = ulLevel;
			return 1;
		}	
		return 0;
	}

	int	SetSkillLevelDirectlyUsingId(unsigned long ulSkillId, unsigned long ulLevel)/*请慎用该函数，因为目前未加任何被动技能降级对数值的影响*/
	{
		if (ulSkillId >= MAX_SKILL || ulSkillId == 0 ||  ulLevel >= MAX_SKILLLEVEL)
			return 0;
		unsigned long  ulSkillIdx = FindSame(ulSkillId);
		if (ulSkillIdx)
		{
			if (m_Skills[ulSkillIdx].SkillId)
			{
				m_Skills[ulSkillIdx].SkillLevel = ulLevel;
				m_Skills[ulSkillIdx].CurrentSkillLevel = ulLevel;
				return 1;
			}	
		}
		return 0;
	}

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
	void	SetNpcSkill(int nSkillNo, int nSkillId, int nSkillLevel);//sometime , npc's AI SkillList may have same skills.IT ONLY BE USED FOR NPC(NO PLAYER)
	void	RemoveNpcSkill(int nSkillNo);
	int		ClearSkillLevel(bool bClearPhysicalLevel);
#endif
};
#endif
