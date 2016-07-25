#include "KStateFightSkill.h"

BOOL KStateFightSkill::Cast(TSkillParam * pSkillParam)
{
	if (GetSkillStyle() == SKILL_SS_InitiativeNpcState)
		CastInitiativeSkill(pSkillParam);
	else if (GetSkillStyle() == SKILL_SS_PassivityNpcState)
		CastPassivitySkill(pSkillParam);
	else 
		return FALSE;
	return TRUE;
}

//服务器端需要获得并通知客户端
BOOL	KStateFightSkill::GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow)
{
	if (!pSkillsSettingFile || nRow < 0) return FALSE;
	pSkillsSettingFile->GetInteger(nRow, "StateSpecialId",  0, &m_nStateSpecialId, TRUE);
	return TRUE;
}
/*!*****************************************************************************
// Function		: KSkill::CastInitiativeSkill
// Purpose		: 主动辅助技能
// Return		: BOOL 
// Argumant		: int nLauncher
// Argumant		: int nParam1
// Argumant		: int nParam2
// Argumant		: int nWaitTime
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL KStateFightSkill::CastInitiativeSkill(TSkillParam * pSkillParam)
{
#ifdef _SERVER
	//对自已的主动辅助技能
	/*if (!m_bTargetOnly && !m_bTargetAlly && !m_bTargetEnemy)
	{
		nParam1 = -1;
		nParam2 = nLauncher;
	}
*/
	if (!((pSkillParam->dwSkillParams & PARAM_LAUNCHER) &&  (pSkillParam->tLauncher.tActorInfo.nActorType == Actor_Npc))) return FALSE;
	KMissleMagicAttribsData * pAttribsData = NULL;
	if (pSkillParam->dwSkillParams & PARAM_LAUNCHER)
	{
		pAttribsData = CreateMissleMagicAttribsData(pSkillParam->tLauncher.tActorInfo);	
	}
	else
		return FALSE;

	if (pAttribsData) 
	{
		Npc[pSkillParam->tTarget.tActorInfo.nActorIndex].ReceiveDamage(pSkillParam->tLauncher.tActorInfo.nActorIndex, 0, pAttribsData->m_pDamageMagicAttribs);
		
		if (pAttribsData->m_nStateMagicAttribsNum > 0)
			Npc[pSkillParam->tTarget.tActorInfo.nActorIndex].SetStateSkillEffect(pSkillParam->tLauncher.tActorInfo.nActorIndex, m_nId, m_nLevel, pAttribsData->m_pStateMagicAttribs, pAttribsData->m_nStateMagicAttribsNum, pAttribsData->m_pStateMagicAttribs[0].nValue[1]);
		
		if (pAttribsData->m_nImmediateMagicAttribsNum > 0)
			Npc[pSkillParam->tTarget.tActorInfo.nActorIndex].SetImmediatelySkillEffect(pSkillParam->tLauncher.tActorInfo.nActorIndex, pAttribsData->m_pImmediateAttribs, pAttribsData->m_nImmediateMagicAttribsNum);
		return TRUE;
	}
	if (pAttribsData->DelRef() == 0)
		delete pAttribsData;
#endif //_SERVER
	return TRUE;
}


/*!*****************************************************************************
// Function		: KSkill::CastPassivitySkill
// Purpose		: 被动
// Return		: BOOL 
// Argumant		: int nLauncher
// Argumant		: int nParam1
// Argumant		: int nParam2
// Argumant		: int nWaitTime
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL KStateFightSkill::CastPassivitySkill(TSkillParam * pSkillParam)
{
#ifdef _SERVER
	if ((pSkillParam->dwSkillParams & PARAM_LAUNCHER) &&  (pSkillParam->tLauncher.tActorInfo.nActorType == Actor_Npc))
	{
		//是被动技能时，是否还需要生成MissleMagicAttribs?
		KMissleMagicAttribsData * pAttribsData = (KMissleMagicAttribsData*)m_StateAttribs;
		if (m_nStateAttribsNum > 0);
		{
			Npc[pSkillParam->tTarget.tActorInfo.nActorIndex].SetStateSkillEffect(pSkillParam->tLauncher.tActorInfo.nActorIndex, m_nId, m_nLevel, m_StateAttribs, m_nStateAttribsNum, -1);
		}
		return TRUE;
	}
#endif
	return FALSE;
}
