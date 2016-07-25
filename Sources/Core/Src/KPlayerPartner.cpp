#include "KCore.h"
/*#ifdef _PARTNER
#include "KPlayerPartner.h"
KPlayerPartner::KPlayerPartner()
{
	m_ulNpcIndex = m_ulCurLifeTime = 0;
	ZeroMemory(&m_Prop, sizeof(m_Prop));
}

BOOL KPlayerPartner::Init(int nNpcIndex, tPartnerProp Prop)
{
	if (nNpcIndex <= 0 
		|| nNpcIndex >= MAX_NPC 
		|| Npc[nNpcIndex].m_Index <= 0
		|| Prop.m_ulOwnerNpcIndex <= 0
		) 
		return FALSE;
	m_ulNpcIndex = nNpcIndex;
	if (!InitData())
		return FALSE;
	
	return TRUE;
	
}

BOOL KPlayerPartner::InitData()
{
	//如果当前为死亡则要求他复活
	
	//恢复全部的数值
	m_ulCurLifeTime = 0;	
	KNpc * pNpc = GetNpc();
	pNpc->ClearNpcState();
	pNpc->ClearNormalState();
	pNpc->RestoreNpcBaseInfo();
	pNpc->m_Camp = Npc[m_Prop.m_ulOwnerNpcIndex].m_Camp;
	pNpc->m_CurrentCamp = Npc[m_Prop.m_ulOwnerNpcIndex].m_Camp;
	pNpc->m_ActionScriptID = 0;
	pNpc->SendCommand(do_stand);
	return TRUE;
}

BOOL KPlayerPartner::Init(int nNpcTemplateId, int nLevel, int nSubWorld, int nRegion, int nMapX, int nMapY, int nOffX, int nOffY , tPartnerProp Prop)
{
	
	if (nNpcTemplateId < 0) 
		return FALSE;
	
	if (nLevel < 0 ) 
		return FALSE;
	
	int	nNpcIdxInfo = MAKELONG(nLevel, nNpcTemplateId);//(nNpcTemplateId << 7) + nLevel;
	int nNpcIdx = NpcSet.Add(nNpcIdxInfo, nSubWorld, nRegion, nMapX, nMapY, nOffX, nOffY);
	if (nNpcIdx <= 0)
		return FALSE;
	
	m_ulNpcIndex = nNpcIdx;
	if (!InitData())
		return FALSE;
	
	return TRUE;
}

BOOL KPlayerPartner::Init(int nNpcTemplateId, int nLevel, int nSubWorld, int nMpsX, int nMpsY, tPartnerProp Prop)
{
	if (nNpcTemplateId < 0) 
		return FALSE;
	
	if (nLevel < 0 ) 
		return FALSE;
	
	int	nNpcIdxInfo = MAKELONG(nLevel, nNpcTemplateId);//(nNpcTemplateId << 7) + nLevel;
	
	int nNpcIdx = NpcSet.Add(nNpcIdxInfo, nSubWorld, nMpsX, nMpsY);
	if (nNpcIdx <= 0)
		return FALSE;
	
	m_ulNpcIndex = nNpcIdx;
	
	if (!InitData())
		return FALSE;
	
	return TRUE;
}

int	 KPlayerPartner::Activate()
{
	if (m_Prop.m_ulTimeOutVanish && m_ulCurLifeTime >= m_Prop.m_ulTimeOutVanish)
		GetNpc()->SendCommand(do_death);
	m_ulCurLifeTime ++;
	return TRUE;
}
#endif*/