#include "KCore.h"
#include "SkillDef.h"
#include "KSkillList.h"
#include "../../../lib/S3DBInterface.h"
#include "KSKills.h"
#include "crtdbg.h"

#ifndef _SERVER
#include "cOREsHELL.H"
#endif

KSkillList::KSkillList()
{
	ZeroMemory(m_Skills, sizeof(NPCSKILL) * MAX_NPCSKILL);
	m_nNpcIndex = 0;
}

KSkillList::~KSkillList()
{
}

int	KSkillList::FindSame(int nSkillID)
{
	if (!nSkillID)
		return 0;
	
	for (int i=1; i<MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId == nSkillID)
		{
			return i;
		}
	}
	return 0;	
}

int KSkillList::FindFree()
{
	for (int i=1; i<MAX_NPCSKILL; i++)
	{
		if (!m_Skills[i].SkillId)
		{
			return i;
		}
	}
	return 0;	
}

int KSkillList::GetCount()
{
	int nCount = 0;
	
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId)
		{
			nCount++;
		}
	}
	return nCount;
}

#ifndef _SERVER
void KSkillList::SetSkillLevel(int nId/*技能的ID号*/, int nLevel)
{
	int i = FindSame(nId);
	
	if (!i)
	{
		// for demo, need delete
		Add(nId, nLevel);
		return;		
	}
	else
		//m_Skills[i].SkillLevel = nLevel;
		SetLevel(i, nLevel);
}
#endif

#ifndef _SERVER
BOOL KSkillList::SetLevel(int nIndex/*技能列表的索引号*/, int nLevel)
{
	if (nIndex <= 0 || nIndex >= MAX_NPCSKILL)
		return FALSE;
	if (nLevel < 0 || nLevel >= MAX_SKILLLEVEL)
		return FALSE;
	
	int nInc = nLevel - m_Skills[nIndex].SkillLevel;
	IncreaseLevel(nIndex, nInc);
	//m_Skills[nIndex].SkillLevel = nLevel;
	//m_Skills[nIndex].CurrentSkillLevel += nInc;
	return TRUE;
}
#endif

BOOL KSkillList::IncreaseLevel(int nIdx, int nLvl)
{
	if (nIdx <= 0 || nIdx >= MAX_NPCSKILL)
		return FALSE;
	if (nLvl < 0)
		return FALSE;
	
	m_Skills[nIdx].SkillLevel += nLvl;
	m_Skills[nIdx].CurrentSkillLevel += nLvl;
	if (g_GetSkill(nIdx,m_Skills[nIdx].CurrentSkillLevel)->GetSkillStyle() == SKILL_SS_PassivityNpcState)
	{
		TSkillParam SkillParam;
		SkillParam.dwSkillParams = PARAM_LAUNCHER;
		SkillParam.tLauncher.tActorInfo.nActorType = Actor_Npc;
		SkillParam.tLauncher.tActorInfo.nActorIndex = m_nNpcIndex;
		SkillParam.dwSkillParams = PARAM_TARGET;
		SkillParam.tTarget.tActorInfo.nActorType = Actor_Npc;
		SkillParam.tTarget.tActorInfo.nActorIndex = m_nNpcIndex;
		g_GetSkill(nIdx,m_Skills[nIdx].CurrentSkillLevel)->Cast(&SkillParam);
	}
	return TRUE;
}

int KSkillList::Add(int nSkillID, int nLevel, int nMaxTimes, int RemainTimes)
{
	int i;
	
	if (nSkillID <= 0)
		return 0;
//	_ASSERT(nSkillID < 200 && nLevel < 64);
	i = FindSame(nSkillID);
	if (i)
	{
		if (nLevel > m_Skills[i].SkillLevel)
		{
			int nInc = nLevel - m_Skills[i].SkillLevel;
			IncreaseLevel(i, nInc);
		}
		m_Skills[i].MaxTimes = 0;
		m_Skills[i].RemainTimes = 0;
		
		//如果改技能属于被动辅助技能时，则设置Npc状态
		if (g_GetSkill(nSkillID,m_Skills[i].CurrentSkillLevel)->GetSkillStyle() == SKILL_SS_PassivityNpcState)
		{
			TSkillParam SkillParam;
			SkillParam.dwSkillParams = PARAM_LAUNCHER;
			SkillParam.tLauncher.tActorInfo.nActorType = Actor_Npc;
			SkillParam.tLauncher.tActorInfo.nActorIndex = m_nNpcIndex;
			SkillParam.dwSkillParams = PARAM_TARGET;
			SkillParam.tTarget.tActorInfo.nActorType = Actor_Npc;
			SkillParam.tTarget.tActorInfo.nActorIndex = m_nNpcIndex;
			g_GetSkill(nSkillID, m_Skills[i].CurrentSkillLevel)->Cast(&SkillParam);
		}
		return i;
	}
	i = FindFree();
	if (i)
	{
	//	g_GetSkill(nSkillID, nLevel)->CheckAndGenSkillData();
		m_Skills[i].SkillId = nSkillID;
		m_Skills[i].SkillLevel = nLevel;
		m_Skills[i].CurrentSkillLevel = nLevel;
		m_Skills[i].MaxTimes = 0;
		m_Skills[i].RemainTimes = 0;
		if (g_GetSkill(nSkillID, nLevel)->GetSkillStyle() == SKILL_SS_PassivityNpcState)
		{
			TSkillParam SkillParam;
			SkillParam.dwSkillParams = PARAM_LAUNCHER;
			SkillParam.tLauncher.tActorInfo.nActorType = Actor_Npc;
			SkillParam.tLauncher.tActorInfo.nActorIndex = m_nNpcIndex;
			SkillParam.dwSkillParams = PARAM_TARGET;
			SkillParam.tTarget.tActorInfo.nActorType = Actor_Npc;
			SkillParam.tTarget.tActorInfo.nActorIndex = m_nNpcIndex;
			g_GetSkill(nSkillID, nLevel)->Cast(&SkillParam);
		}
		return i;
	}
	
	return 0;
}

int KSkillList::GetLevel(int nSkillID)
{
	int i;
	
	if (!nSkillID)
		return 0;
	
	i = FindSame(nSkillID);
	if (i)
	{
		_ASSERT(m_Skills[i].SkillLevel < 64);
		return m_Skills[i].SkillLevel;
	}
	return 0;
}

int KSkillList::GetCurrentLevel(int nSkillID)
{
	int i;
	
	if (!nSkillID)
		return 0;
	
	i = FindSame(nSkillID);
	if (i)
	{
		return m_Skills[i].CurrentSkillLevel;
	}
	return 0;
}

BOOL KSkillList::CanCast(int nSkillID, DWORD dwTime)
{
	//	return TRUE;	// for demo test, need delete
	if (!nSkillID)
		return FALSE;
	
	int i = FindSame(nSkillID);
	if (!i)
		return FALSE;
	
	if (m_Skills[i].NextCastTime > dwTime)
		return FALSE;
	
	return TRUE;
}

void KSkillList::SetNextCastTime(int nSkillID, DWORD dwTime)
{
//	m_Skills[nSkillID].NextCastTime = dwTime;
//	return;
	if (!nSkillID)
		return;;
	
	int i = FindSame(nSkillID);
	if (!i)
		return;
	
	m_Skills[i].NextCastTime = dwTime;
}	
#ifndef _SERVER
int	KSkillList::GetSkillSortList(KUiSkillData * pSkillList)
{
	//Ques
	if (!pSkillList) return 0;
	memset(pSkillList, 0, sizeof(KUiSkillData) * 50);
	int nCount = 0;
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId)
		{
			if (g_GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel)->IsPhysical())continue;
			KUiSkillData * pSkill = pSkillList + nCount;
			pSkill->uGenre = CGOG_SKILL_FIGHT;
			pSkill->uId = m_Skills[i].SkillId;
			pSkill->nLevel = m_Skills[i].SkillLevel;
			if ((++nCount) == 50)
			{
				g_DebugLog("Npc的技能数量超过了限制50！！！，请检查!!!");			
				break;
			}
		}
	}
	return nCount;
}

int KSkillList::GetLeftSkillSortList(KUiSkillData* pSkillList)
{
	if (!pSkillList) return 0;
	memset(pSkillList, 0, sizeof(KUiSkillData) * 65);
	int nCount = 1;
	
	pSkillList->uGenre = CGOG_SKILL_FIGHT;
	pSkillList->uId = -1;//物理技能置前!
	pSkillList->nData = 0;
	
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId && m_Skills[i].SkillLevel > 0)
		{
			if ((!g_GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel).IsPhysical()) 
				&& g_GetSkill(m_Skills[i].SkillId,m_Skills[i].SkillLevel).GetSkillLRInfo() == BothSkill 
				|| g_GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel).GetSkillLRInfo() == leftOnlySkill)
			{
				KUiSkillData * pSkill = pSkillList + nCount;
				pSkill->uGenre = CGOG_SKILL_FIGHT;
				pSkill->uId = m_Skills[i].SkillId;
				pSkill->nData = nCount / 8;
				nCount ++;
			}
		}
	}
	return nCount;
}

int KSkillList::GetRightSkillSortList(KUiSkillData* pSkillList)
{
	if (!pSkillList) return 0;
	memset(pSkillList, 0, sizeof(KUiSkillData) * 65);
	
	int nCount = 1;
	pSkillList->uGenre = CGOG_SKILL_FIGHT;
	pSkillList->uId = (unsigned int)-1;//物理技能置前!
	pSkillList->nData = 0;

	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId && m_Skills[i].SkillLevel > 0)
		{
			if ((!g_GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel)->IsPhysical()) 
				&& g_GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel)->GetSkillLRInfo() == BothSkill 
				|| g_GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel)->GetSkillLRInfo() == RightOnlySkill)
			{
				KUiSkillData * pSkill = pSkillList + nCount;
				pSkill->uGenre = CGOG_SKILL_FIGHT;
				pSkill->uId = m_Skills[i].SkillId;
				pSkill->nData = nCount / 8;
				nCount ++;
			}
		}
	}
	return nCount;
}

int KSkillList::GetSkillPosition(int nSkillId)//获得技能在技能界面的位置
{
	if (nSkillId <= 0) return -1;
	
	int nCount = 0;
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId)
		{
			
			if (m_Skills[i].SkillId == nSkillId)
			{
				return nCount;
			}
			nCount ++;
		}
	}
	return nCount;
}
#endif
#ifdef _SERVER
int	KSkillList::UpdateDBSkillList(BYTE * pSkillBuffer)
{
	if (!pSkillBuffer) return -1;
	int nCount = 0;
	char szFightSkill[100];
	char szFightSkillLevel[100];
	TDBSkillData * pSkillData = (TDBSkillData*) pSkillBuffer;
	for (int i = 0 ; i < MAX_NPCSKILL; i ++)
	{
		if (m_Skills[i].SkillId > 0)
		{
	//		_ASSERT(m_Skills[i].SkillId < 200);
			pSkillData->m_nSkillId = m_Skills[i].SkillId;
			pSkillData->m_nSkillLevel = m_Skills[i].SkillLevel;
			pSkillData ++;
			nCount ++;
		}
	}
	return nCount;
}
#endif