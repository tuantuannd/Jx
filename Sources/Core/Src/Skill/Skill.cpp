#include "KEngine.h"
#include "KNode.h"
#include "Skill.h"
#include "KSubWorld.h"
#include "KMissleSkill.h"
unsigned long g_ulSkillCount = 0;
int	g_CastSkill(TSkillParam * pSkillParam)
{
	if (pSkillParam->usSkillID > g_ulSkillCount || pSkillParam->usSkillLevel > MAX_SKILLLEVEL)	return FALSE;
	if (!g_pSkill[pSkillParam->usSkillID][pSkillParam->usSkillLevel])
	{
		if (!g_GenSkillData(pSkillParam->usSkillID, pSkillParam->usSkillLevel)) return FALSE;
	}

	return g_pSkill[pSkillParam->usSkillID][pSkillParam->usSkillLevel]->Cast(pSkillParam);
}

BOOL	g_InstanceSkill(eSKillStyle eStyle, unsigned long ulSkillID, unsigned long ulSkillLevel)
{
	
	if (!g_pSkill[ulSkillID][ulSkillLevel])
	{
		switch(eStyle)
		{
		case SKILL_SS_Missles:			//	子弹类		本技能用于发送子弹类
			{
				g_pSkill[ulSkillID][ulSkillLevel] = new KMissleSkill;
			}break;
		case SKILL_SS_Melee:
			{
				
			}break;
		case SKILL_SS_InitiativeNpcState:	//	主动类		本技能用于改变当前Npc的主动状态
		case SKILL_SS_PassivityNpcState:		//	被动类		本技能用于改变Npc的被动状态
			{
				g_pSkill[ulSkillID][ulSkillLevel] = new KStateFightSkill;
			}break;
		default:
				g_pSkill[ulSkillID][ulSkillLevel] = new ISkill;
		}	

	}
	return TRUE;
}

ISkill * g_GetSkill(unsigned long ulSkillID, unsigned long ulSkillLevel)
{
	_ASSERT(usSkillID < g_ulSkillCount || usSkillLevel < MAX_SKILLLEVEL);

	if (!g_pSkill[ulSkillID][ulSkillLevel])
	{
		_ASSERT(ulSkillLevel != 0);

		if (!g_InstanceSkill(g_pSkill[ulSkillID][0].GetSkillStyle(), ulSkillID, ulSkillLevel)) return NULL;
		g_pSkill[ulSkillID][ulSkillLevel]->SetRowIdInSkillTabFile(g_pSkill[ulSkillID][0]->GetRowIdInSkillTabFile());
		*g_pSkill[ulSkillID][ulSkillLevel] = *g_pSkill[ulSkillID][0];
		g_pSkill[ulSkillID][ulSkillLevel]->SetSkillLevelData(ulSkillLevel);	
	}
		return g_pSkill[ulSkillID][ulSkillLevel];
}

void TGameActorInfo::GetMps(int &nPX, int &nPY)
{
	
	switch(nActorType)
	{
	case Actor_Npc:
		{
			if (Npc[nActorIndex].IsMatch(dwActorID))
				Npc[nActorIndex].GetMpsPos(&nPX, &nPY);
		}break;
	case Actor_Obj:
		{
			//if (Object[nActorIndex].m_nID == dwActorID)
			//	Object[nActorIndex].GetMpsPos(&nPX, &nPY);
		}break;
	case Actor_Missle:
		{
			if (Missle[nActorIndex].m_dwID == dwActorID)
				Missle[nActorIndex].GetMpsPos(&nPX, &nPY);
		}break;
	}
}

int TSkillParam::GetSubWorldIndex()
{
	if (dwSkillParams & PARAM_LAUNCHER)
	{
		switch(tLauncher.tActorInfo.nActorType)
		{
		case Actor_Npc:
			{
				if (Npc[tLauncher.tActorInfo.nActorIndex].IsMatch(tLauncher.tActorInfo.dwActorID))
					return Npc[tLauncher.tActorInfo.nActorIndex].m_SubWorldIndex;
			}break;
		case Actor_Obj:
			{
				if (Object[tLauncher.tActorInfo.nActorIndex].m_nID == tLauncher.tActorInfo.dwActorID)
					return Object[tLauncher.tActorInfo.nActorIndex].m_nSubWorldID;
			}break;
		case Actor_Missle:
			{
				if (Missle[tLauncher.tActorInfo.nActorIndex].m_dwID == tLauncher.tActorInfo.dwActorID)
					return Missle[tLauncher.tActorInfo.nActorIndex].m_nSubWorldId;
			}break;
		default:
			return  -1;
		}
	}
	else
	{
		return tLauncher.tPoint.nSubWorldIndex;
	}
	return -1;
};

inline int TSkillParam::GetCellWidth()
{
	return SubWorld[GetSubWorldIndex()].m_nCellWidth;
};
inline int TSkillParam::GetCellHeight()
{
	return SubWorld[GetSubWorldIndex()].m_nCellHeight;
};

int ISkill::GetSkillIdFromName(char * szSkillName)
{
	//	CheckAndGenSkillData();
	if (!szSkillName || !szSkillName[0]) return -1;
	
	for (int i = 0; i < MAX_SKILL; i ++)
	{
		if (!strcmp(Skill[i][1].m_szName, szSkillName))		return i;
	}
	return -1;
}
