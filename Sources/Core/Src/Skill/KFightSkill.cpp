#include "KFightSkill.h"
extern const char * g_MagicID2String(int nAttrib);
/*!*****************************************************************************
// Function		: KFightSkill::GetInfoFromTabFile
// Purpose		: 从TabFile中获得该技能的常规属性
// Return		: 
// Argumant		: int nCol
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL	KFightSkill::GetInfoFromTabFile(int nRow)
{
	KITabFile * pITabFile = &g_SkillsSetting;
	return GetInfoFromTabFile(&g_SkillsSetting, nRow);
}


BOOL	KFightSkill::GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow)
{
	
	if (!pSkillsSettingFile || nRow < 0) return FALSE;
	pSkillsSettingFile->GetString(nRow, "SkillName",		"", m_szName, sizeof(m_szName) ,TRUE);
	pSkillsSettingFile->GetInteger(nRow, "SkillId",			0, (int *)&m_nId,TRUE);

	
	//pSkillsSettingFile->GetInteger(nRow, "CharClass",		0, &m_nCharClass, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "TargetOnly",		0, &m_bTargetOnly, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "TargetEnemy",		0, &m_bTargetEnemy, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "TargetAlly",		0, &m_bTargetAlly, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "TargetObj",		0, &m_bTargetObj, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "SkillCostType",	0, (int *)&m_nSkillCostType, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "CostValue",		0, &m_nCost, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "TimePerCast",		0, &m_nMinTimePerCast, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "ChildSkillId",	0, &m_nChildSkillId, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "ChildSkillLevel", 0, &m_nChildSkillLevel, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "AttackRadius",	50, &m_nAttackRadius, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "WaitTime",		0, &m_nWaitTime, TRUE);

	pSkillsSettingFile->GetInteger(nRow, "TargetSelf",		0, &m_bTargetSelf, TRUE);
	
#ifndef _SERVER
	pSkillsSettingFile->GetString(nRow, "SkillIcon","\\spr\\skill\\图标\\通用.spr",	m_szSkillIcon, 80);
	if (!m_szSkillIcon[0])	strcpy(m_szSkillIcon, "\\spr\\skill\\图标\\通用.spr");
	pSkillsSettingFile->GetInteger(nRow, "LRSkill",		0, (int*)&m_eLRSkillInfo);
	pSkillsSettingFile->GetString(nRow, "PreCastSpr", "", m_szPreCastEffectFile, 100);
#endif
	return TRUE;
}

/*!*****************************************************************************
// Function		: KFightSkill::ParseString2MagicAttrib
// Purpose		: 解析通过脚本运算获得的技能数据
// Return		: 
// Argumant		: char * szMagicAttribName
// Argumant		: char * szValue
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL	KFightSkill::ParseString2MagicAttrib(MAGIC_ATTRIB MagicAttrib, int nValue1, int nValue2, int nValue3)
{
	if (MagicAttrib > magic_skill_begin && MagicAttrib < magic_skill_end)
	{
		switch(MagicAttrib)
		{
		case magic_skill_cost_v:				// 消耗MANA
			{
				m_nCost = nValue1;
			}
			break;
			
		case magic_skill_costtype_v:
			{
				m_nSkillCostType = (NPCATTRIB)nValue1;
			}break;
			
		case magic_skill_mintimepercast_v: 		// 每次发魔法的间隔时间
			{
				m_nMinTimePerCast = nValue1;
			}break;
			
		}
		return TRUE;
	}
				
	if (MagicAttrib > magic_damage_begin && MagicAttrib < magic_damage_end)
	{
		switch(MagicAttrib)
		{
		case magic_attackrating_v:
		case magic_attackrating_p:
			m_DamageAttribs[0].nAttribType = MagicAttrib;
			m_DamageAttribs[0].nValue[0] = nValue1;
			m_DamageAttribs[0].nValue[1] = nValue2;
			m_DamageAttribs[0].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_ignoredefense_p:
			m_DamageAttribs[1].nAttribType = MagicAttrib;
			m_DamageAttribs[1].nValue[0] = nValue1;
			m_DamageAttribs[1].nValue[1] = nValue2;
			m_DamageAttribs[1].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_physicsdamage_v:
		case magic_physicsenhance_p:
			m_DamageAttribs[2].nAttribType = MagicAttrib;
			m_DamageAttribs[2].nValue[0] = nValue1;
			m_DamageAttribs[2].nValue[1] = nValue2;
			m_DamageAttribs[2].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_colddamage_v:
		case magic_coldenhance_p:
			m_DamageAttribs[3].nAttribType = MagicAttrib;
			m_DamageAttribs[3].nValue[0] = nValue1;
			m_DamageAttribs[3].nValue[1] = nValue2;
			m_DamageAttribs[3].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_firedamage_v:
		case magic_fireenhance_p:
			m_DamageAttribs[4].nAttribType = MagicAttrib;
			m_DamageAttribs[4].nValue[0] = nValue1;
			m_DamageAttribs[4].nValue[1] = nValue2;
			m_DamageAttribs[4].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_lightingdamage_v:
		case magic_lightingenhance_p:
			m_DamageAttribs[5].nAttribType = MagicAttrib;
			m_DamageAttribs[5].nValue[0] = nValue1;
			m_DamageAttribs[5].nValue[1] = nValue2;
			m_DamageAttribs[5].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_poisondamage_v:
		case magic_poisonenhance_p:
			m_DamageAttribs[6].nAttribType = MagicAttrib;
			m_DamageAttribs[6].nValue[0] = nValue1;
			m_DamageAttribs[6].nValue[1] = nValue2;
			m_DamageAttribs[6].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		case magic_magicdamage_v:
		case magic_magicenhance_p:
			m_DamageAttribs[7].nAttribType = MagicAttrib;
			m_DamageAttribs[7].nValue[0] = nValue1;
			m_DamageAttribs[7].nValue[1] = nValue2;
			m_DamageAttribs[7].nValue[2] = nValue3;
			m_nDamageAttribsNum ++;
			break;
		}
		return TRUE;
	}
				
	//剩下的为数据计算时的数据参数
	//根据nValue2值决定状态参数还是非状态参数
	if (MagicAttrib >magic_normal_begin)
	{
		if (nValue2 == 0) 
		{
			m_ImmediateAttribs[m_nImmediateAttribsNum].nAttribType = MagicAttrib;
			m_ImmediateAttribs[m_nImmediateAttribsNum].nValue[0] = nValue1;
			m_ImmediateAttribs[m_nImmediateAttribsNum].nValue[1] = nValue2;
			m_ImmediateAttribs[m_nImmediateAttribsNum].nValue[2] = nValue3;
			m_nImmediateAttribsNum ++;
			return TRUE;
			
		}
		else
		{
			m_StateAttribs[m_nStateAttribsNum].nAttribType = MagicAttrib;
			m_StateAttribs[m_nStateAttribsNum].nValue[0] = nValue1;
			m_StateAttribs[m_nStateAttribsNum].nValue[1] = nValue2;
			m_StateAttribs[m_nStateAttribsNum].nValue[2] = nValue3;
			m_nStateAttribsNum ++;
			return TRUE;
		}
	}
	return FALSE;
				
}


const char * KFightSkill::MagicAttrib2String(int MagicAttrib) 
{
	return 	g_MagicID2String(MagicAttrib);
}
#ifndef _SERVER
void	KFightSkill::DrawSkillIcon(int x, int y, int Width, int Height)
{
	if (!m_szSkillIcon[0]) return ;
	
	m_RUIconImage.nType = ISI_T_SPR;
	m_RUIconImage.Alpha = 32;
	m_RUIconImage.bRenderStyle = IMAGE_RENDER_STYLE_ALPHA;
	m_RUIconImage.uImage = 0;
	m_RUIconImage.nISPosition = 0;
	m_RUIconImage.bMatchReferenceSpot = 0;
	m_RUIconImage.bFrameDraw = FALSE;
	strcpy(m_RUIconImage.szImage, m_szSkillIcon);
	m_RUIconImage.oPosition.nX = x;
	m_RUIconImage.oPosition.nY = y;
	m_RUIconImage.oPosition.nZ = 0;
	m_RUIconImage.nFrame = 0;
	g_pRepresent->DrawPrimitives(1, &m_RUIconImage, RU_T_IMAGE, 1);
}
#endif

#ifdef _SERVER
KMissleMagicAttribsData* KFightSkill::CreateMissleMagicAttribsData(TGameActorInfo GameActor)
{
	if (GameActor.nActorIndex <= 0 || m_bClientSend) return NULL; 
	
	KMissleMagicAttribsData* pMissleMagicAttribsData = new KMissleMagicAttribsData;
	
	pMissleMagicAttribsData->m_pStateMagicAttribs = m_StateAttribs;
	pMissleMagicAttribsData->m_nStateMagicAttribsNum = m_nStateAttribsNum;
	
	pMissleMagicAttribsData->m_pImmediateAttribs = m_ImmediateAttribs;
	pMissleMagicAttribsData->m_nImmediateMagicAttribsNum = m_nImmediateAttribsNum;
	
	KMagicAttrib * pDamageAttribs =  new KMagicAttrib[MAX_MISSLE_DAMAGEATTRIB];
	pMissleMagicAttribsData->m_nDamageMagicAttribsNum = m_nDamageAttribsNum;
	
	//根据玩家的基本属性，确定子弹的伤害
	if (GameActor.nActorType == Actor_Npc)
		Npc[GameActor.nActorIndex].AppendSkillEffect(m_DamageAttribs, pDamageAttribs);
	else
		memcpy(pDamageAttribs, m_DamageAttribs, sizeof(m_DamageAttribs));
	pMissleMagicAttribsData->m_pDamageMagicAttribs = pDamageAttribs;
	return pMissleMagicAttribsData;
}


KMissleMagicAttribsData* KFightSkill::CreateMissleMagicAttribsData(TSkillParam * pSkillParam)
{
	if (!pSkillParam) return NULL;
	if (!(pSkillParam->dwSkillParams & PARAM_LAUNCHER)) 
	{
		return CreateMissleMagicAttribsData(pSkillParam->tLauncher.tActorInfo);
	}
	return NULL;
}
#endif

void		KFightSkill::SetSkillLevelData(int nLevel /* =0*/)
{
	if (nLevel == 0) nLevel = m_nLevel;
	else
		m_nLevel = nLevel;
	
	char szSettingScriptName[MAX_PATH];
	char szSettingNameValue[100];
	char szSettingDataValue[100];
	char szResult[300];
	//Question 必须顺序一至才行
	g_SkillsSetting.GetString(GetRowIdInSkillTabFile(), "LvlSetScript", "", szSettingScriptName, MAX_PATH );
	if (!szSettingScriptName[0]) return;
	g_SetFilePath("\\");
	//加载Lua脚本
	KLuaScript Script;
	Script.Init();
	if (!Script.Load(szSettingScriptName)) 
	{
		g_DebugLog("无法读取该脚本文件，请确认是否文件存在或脚本语法有误！");
		return;
	}
	
	int nSafeIndex = 1;
	Script.SafeCallBegin(&nSafeIndex);
	
	for(int i = 0 ;  i  < MAXSKILLLEVELSETTINGNUM ; i ++)
	{
		char szSettingName[40];
		char szSettingData[40];
		sprintf(szSettingName, "LvlSetting%d", i + 1);
		sprintf(szSettingData, "LvlData%d", i + 1);
		
		g_SkillsSetting.GetString(GetRowIdInSkillTabFile(), szSettingName, "", szSettingNameValue, 100);
		g_SkillsSetting.GetString(GetRowIdInSkillTabFile(), szSettingData, "", szSettingDataValue, 100);
		
		if (szSettingNameValue[0] == 0 || szSettingScriptName[0] == '0') continue;
		{  
			Script.CallFunction("GetSkillLevelData", 1, "ssd", szSettingNameValue, szSettingDataValue, nLevel);
			const char * szType = lua_typename(Script.m_LuaState, Lua_GetTopIndex(Script.m_LuaState));
			if (Lua_IsNumber(Script.m_LuaState, Lua_GetTopIndex(Script.m_LuaState)) == 1)
			{
				int nResult = (int)Lua_ValueToNumber(Script.m_LuaState, Lua_GetTopIndex(Script.m_LuaState));
				sprintf(szResult, "%d", nResult);
			}
			else if (Lua_IsString(Script.m_LuaState, Lua_GetTopIndex(Script.m_LuaState)) == 1)
			{
				strcpy(szResult , (char *)Lua_ValueToString(Script.m_LuaState, Lua_GetTopIndex(Script.m_LuaState)));
			}
			else
			{
				char szMsg[300];
				sprintf(szMsg, "当获得该技能等级为%d（%s,%s）时，出错，请检查脚本!,",nLevel, szSettingNameValue, szSettingDataValue);
				g_DebugLog(szMsg);
				break;
			}
			ParseString2MagicAttrib(szSettingNameValue, szResult);
		}
	}
	Script.SafeCallEnd(nSafeIndex);
}

BOOL	KFightSkill::ParseString2MagicAttrib(char * szMagicAttribName, char * szValue)
{
	int nValue1 = 0;
	int nValue2 = 0;
	int nValue3 = 0;
	if ((!szMagicAttribName) || (!szMagicAttribName[0])) return FALSE;
	
	//nValue2 当值为-1时为永久性状态，0为非状态，其它值为有时效性状态魔法效果
	//需要将状态数据与非状态数据分离出来，放入相应的数组内，并记录总数量
	for (int i  = 0 ; i <= magic_normal_end; i ++)
	{
		if (!strcmp(szMagicAttribName, MagicAttrib2String(i)))
		{
			sscanf(szValue, "%d,%d,%d", &nValue1, &nValue2, &nValue3);
			if (!ParseString2MagicAttrib((MAGIC_ATTRIB)i, nValue1, nValue2, nValue3))
			{
				g_DebugLog("[Skill]Can Not Find Magic Attrib %d",i);
				return FALSE;
			}
			else
				return TRUE;
		}
	}
			
	return FALSE;
}