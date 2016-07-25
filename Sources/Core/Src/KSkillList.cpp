 #include "KEngine.h"
#include "KCore.h"
#include "SkillDef.h"
#include "KSkillList.h"
#ifndef _STANDALONE
#include "../../../lib/S3DBInterface.h"
#include "crtdbg.h"
#else
#include "S3DBInterface.h"
#endif
#include "KSkills.h"
#include "KPlayer.h"

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


static int g_CallCount = 0;

int	KSkillList::FindSame(int nSkillID)
{
    g_CallCount++;

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
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (!m_Skills[i].SkillId)
		{
			return i;
		}
	}
	return 0;	
}

#ifdef _SERVER
void KSkillList::SetNpcSkill(int nSkillNo, int nSkillId, int nSkillLevel)
{
	if (nSkillNo <= 0 || nSkillLevel <= 0 || nSkillId <= 0)
		return ;
	if (nSkillNo >= MAX_NPCSKILL) 
		return ;
	
	m_Skills[nSkillNo].SkillId = nSkillId;
	m_Skills[nSkillNo].CurrentSkillLevel = nSkillLevel;
	m_Skills[nSkillNo].SkillLevel = nSkillLevel;
	m_Skills[nSkillNo].MaxTimes = 0;
	m_Skills[nSkillNo].RemainTimes = 0;

	_ASSERT(nSkillId > 0 && nSkillLevel > 0);
	KSkill * pOrdinSkill = (KSkill *) g_SkillManager.GetSkill(nSkillId, nSkillLevel);
	if (!pOrdinSkill) 
        return ;
	
    if (pOrdinSkill->GetSkillStyle() == SKILL_SS_PassivityNpcState)
	{
		pOrdinSkill->Cast(m_nNpcIndex, -1, m_nNpcIndex);
	}
}

void KSkillList::RemoveNpcSkill(int nSkillNo)
{
	if (nSkillNo <= 0 )
		return ;
	if (nSkillNo >= MAX_NPCSKILL) 
		return ;
	
	m_Skills[nSkillNo].SkillId = 0;
	m_Skills[nSkillNo].CurrentSkillLevel = 0;
	m_Skills[nSkillNo].SkillLevel = 0;
	m_Skills[nSkillNo].MaxTimes = 0;
	m_Skills[nSkillNo].RemainTimes = 0;
}

#endif

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
	return TRUE;
}
#endif

BOOL KSkillList::IncreaseLevel(int nIdx, int nLvl)
{
	if (nIdx <= 0 || nIdx >= MAX_NPCSKILL)
		return FALSE;
	if (nLvl <= 0)
		return FALSE;
	
	m_Skills[nIdx].SkillLevel += nLvl;
	m_Skills[nIdx].CurrentSkillLevel += nLvl;
	
	
	KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(m_Skills[nIdx].SkillId, m_Skills[nIdx].CurrentSkillLevel);
	if (!pOrdinSkill) 
        return FALSE;

	if (pOrdinSkill->GetSkillStyle() == SKILL_SS_PassivityNpcState)
	{
		pOrdinSkill->Cast(m_nNpcIndex, -1, m_nNpcIndex);
	}
	return TRUE;
}

int KSkillList::Add(int nSkillID, int nLevel, int nMaxTimes, int RemainTimes)
{

	int i;
	
	if (nSkillID <= 0 || nLevel < 0)
		return 0;
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
		
		if (nLevel  > 0)
		{
			ISkill * pSkill = g_SkillManager.GetSkill(nSkillID, m_Skills[i].CurrentSkillLevel);
			if (!pSkill) 
				return 0;
			
			//如果改技能属于被动辅助技能时，则设置Npc状态
			if (pSkill->GetSkillStyle() == SKILL_SS_PassivityNpcState)
			{
				((KSkill *)pSkill)->Cast(m_nNpcIndex, -1, m_nNpcIndex);
			}
		}

		return i;
	}

	i = FindFree();
	if (i)
	{
		m_Skills[i].SkillId = nSkillID;
		m_Skills[i].SkillLevel = nLevel;
		m_Skills[i].CurrentSkillLevel = nLevel;
		m_Skills[i].MaxTimes = 0;
		m_Skills[i].RemainTimes = 0;

		if (nLevel > 0)
		{
			ISkill * pSkill =  g_SkillManager.GetSkill(nSkillID, nLevel);
			if (!pSkill) 
				return 0 ;
			
			if (pSkill->GetSkillStyle() == SKILL_SS_PassivityNpcState)
			{
				((KSkill*)pSkill)->Cast(m_nNpcIndex, -1, m_nNpcIndex);
			}
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
		_ASSERT(m_Skills[i].SkillLevel < MAX_SKILLLEVEL);
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
	
	if (!nSkillID)
		return FALSE;
	
	int i = FindSame(nSkillID);
	if (!i)
	{
		return FALSE;
	}

	if (m_Skills[i].CurrentSkillLevel <= 0 || m_Skills[i].NextCastTime > dwTime)
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
	ISkill* pSkill = NULL;
	KSkill * pOrdinSkill = NULL;
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId)
		{
			_ASSERT(m_Skills[i].SkillLevel >= 0);
			
			if (!m_Skills[i].SkillLevel)
			{
				pSkill =  g_SkillManager.GetSkill(m_Skills[i].SkillId, 1);
			}
			else 
			{
				pSkill =  g_SkillManager.GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel);
			}
			
			if (!pSkill)
				continue;
			eSkillStyle eStyle = (eSkillStyle)pSkill->GetSkillStyle();
			
			switch(eStyle)
			{
			case SKILL_SS_Missles:			//	子弹类		本技能用于发送子弹类
			case SKILL_SS_Melee:
			case SKILL_SS_InitiativeNpcState:	//	主动类		本技能用于改变当前Npc的主动状态
			case SKILL_SS_PassivityNpcState:		//	被动类		本技能用于改变Npc的被动状态
				{
					pOrdinSkill = (KSkill * ) pSkill;
					if (pOrdinSkill->IsPhysical())
						continue;

				}break;
			case SKILL_SS_Thief:					//	偷窃类
				{
					
				}
				break;
			default:
				continue;
			}
			
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
	
	pSkillList->uGenre = CGOG_SKILL_SHORTCUT;
	pSkillList->uId = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].GetCurActiveWeaponSkill();//物理技能置前!
	pSkillList->nData = 0;
	
	ISkill * pISkill = NULL;
	KSkill * pOrdinSkill = NULL;
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId && m_Skills[i].SkillLevel > 0)
		{
			_ASSERT(m_Skills[i].SkillId > 0);
			pISkill = g_SkillManager.GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel);
			if (!pISkill)
				continue;
			
			eSkillStyle eStyle = (eSkillStyle)pISkill->GetSkillStyle();
			switch(eStyle)
			{
			case SKILL_SS_Missles:			//	子弹类		本技能用于发送子弹类
			case SKILL_SS_Melee:
			case SKILL_SS_InitiativeNpcState:	//	主动类		本技能用于改变当前Npc的主动状态
			case SKILL_SS_PassivityNpcState:		//	被动类		本技能用于改变Npc的被动状态
				{
					pOrdinSkill = (KSkill * ) pISkill;
					if ((!pOrdinSkill->IsPhysical()) &&	
						(pOrdinSkill->GetSkillLRInfo() == BothSkill) || 
						(pOrdinSkill->GetSkillLRInfo() == leftOnlySkill)
						)
					{
						
					}
					else 
						continue;
					
				}
				break;
				
			case SKILL_SS_Thief:
				{
					
					continue;
					
				}break;
			}
			
			KUiSkillData * pSkill = pSkillList + nCount;
			pSkill->uGenre = CGOG_SKILL_SHORTCUT;
			pSkill->uId = m_Skills[i].SkillId;
			pSkill->nData = nCount / 8;
			nCount ++;
			if (nCount >= 65)
				break;
			
		}
	}
	return nCount;
}

int KSkillList::GetRightSkillSortList(KUiSkillData* pSkillList)
{
	if (!pSkillList) return 0;
	memset(pSkillList, 0, sizeof(KUiSkillData) * 65);
	
	int nCount = 1;
	pSkillList->uGenre = CGOG_SKILL_SHORTCUT;
	//pSkillList->uId = (unsigned int)-1;//物理技能置前!
	pSkillList->uId = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].GetCurActiveWeaponSkill();//物理技能置前!;	 
	pSkillList->nData = 0;
	ISkill * pISkill = NULL;
	KSkill * pOrdinSkill = NULL;
	
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		if (m_Skills[i].SkillId && m_Skills[i].SkillLevel > 0)
		{
			_ASSERT(m_Skills[i].SkillId > 0);
			pISkill  =  g_SkillManager.GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel);
			if(!pISkill)
				continue;

			eSkillStyle eStyle = ( eSkillStyle ) pISkill->GetSkillStyle();
			
			switch(eStyle)
			{
			case SKILL_SS_Missles:			//	子弹类		本技能用于发送子弹类
			case SKILL_SS_Melee:
			case SKILL_SS_InitiativeNpcState:	//	主动类		本技能用于改变当前Npc的主动状态
			case SKILL_SS_PassivityNpcState:		//	被动类		本技能用于改变Npc的被动状态
				{
					pOrdinSkill = (KSkill * ) pISkill;
					if (
						(!pOrdinSkill->IsPhysical()) && 
						(pOrdinSkill->GetSkillLRInfo() == BothSkill) || 
						(pOrdinSkill->GetSkillLRInfo() == RightOnlySkill)
						)
					{
						
					}
					else 
						continue;
				}
				break;
				
			case SKILL_SS_Thief:
				{
					
				}break;
			}
			
			
			
			KUiSkillData * pSkill = pSkillList + nCount;
			pSkill->uGenre = CGOG_SKILL_SHORTCUT;
			pSkill->uId = m_Skills[i].SkillId;
			pSkill->nData = nCount / 8;
			nCount ++;
			if (nCount >= 65)
				break;
			
		}
	}
	return nCount;
}




int KSkillList::GetSkillPosition(int nSkillId)//获得技能在技能界面的位置
{
	if (nSkillId <= 0) return -1;
	KSkill * pOrdinSkill = NULL;
	
	ISkill * pISkill = NULL;
	int nCount = 0;
	for (int i = 1; i < MAX_NPCSKILL; i++)
	{
		_ASSERT(m_Skills[i].SkillLevel >= 0);
		if (m_Skills[i].SkillId<= 0) continue;
		
		if (m_Skills[i].SkillLevel == 0)
		{
			pISkill = g_SkillManager.GetSkill(m_Skills[i].SkillId, 1);
		}
		else
		{
			pISkill =  g_SkillManager.GetSkill(m_Skills[i].SkillId, m_Skills[i].SkillLevel);
		}

		if (!pISkill) 
        	continue ;


		eSkillStyle eStyle = ( eSkillStyle ) pISkill->GetSkillStyle();
		switch(eStyle)
		{
		case SKILL_SS_Missles:			//	子弹类		本技能用于发送子弹类
		case SKILL_SS_Melee:
		case SKILL_SS_InitiativeNpcState:	//	主动类		本技能用于改变当前Npc的主动状态
		case SKILL_SS_PassivityNpcState:		//	被动类		本技能用于改变Npc的被动状态
			{
				pOrdinSkill = (KSkill * ) pISkill;
				if (
					(!m_Skills[i].SkillId) || 
					(pOrdinSkill->IsPhysical())
					)
				{
					continue;
				}
			}
			break;
			
		case SKILL_SS_Thief:
			{
				
				
			}break;
		}
		
		
		if (m_Skills[i].SkillId == nSkillId)
		{
			return nCount;
		}
		nCount ++;
		
	}
	return nCount;
}
#endif

#ifdef _SERVER
int	KSkillList::UpdateDBSkillList(BYTE * pSkillBuffer)
{
	if (!pSkillBuffer)
		return -1;
	
	int nCount = 0;

	TDBSkillData * pSkillData = (TDBSkillData *) pSkillBuffer;
	for (int i = 0 ; i < MAX_NPCSKILL; i ++)
	{
		if (m_Skills[i].SkillId > 0)
		{
			pSkillData->m_nSkillId = m_Skills[i].SkillId;
			pSkillData->m_nSkillLevel = m_Skills[i].SkillLevel;
			pSkillData++;
			nCount ++;
		}
	}
	return nCount;
}

int		KSkillList::ClearSkillLevel(bool bClearPhysicalLevel)
{
	int nTotalSkillPoint = 0;

	for (int i = 0 ; i < MAX_NPCSKILL; i ++)
	{
		if (m_Skills[i].SkillId > 0)
		{
			ISkill * pSkill = g_SkillManager.GetSkill(m_Skills[i].SkillId, 1);
			if (pSkill)
			{
				if (pSkill->IsPhysical())
					continue;
				nTotalSkillPoint += m_Skills[i].SkillLevel;
				m_Skills[i].SkillLevel = 0;
				m_Skills[i].CurrentSkillLevel = 0;
			}
		}
	}
	return nTotalSkillPoint;
}
#endif
