#include "KCore.h"

#include "KNpcTemplate.h"
#define MAX_VALUE_LEN 300

#ifdef _SERVER
KItemDropRateTree g_ItemDropRateBinTree;
static KItemDropRate * g_GenItemDropRate(char * szDropIniFile)
{
	if ((!szDropIniFile)|| (!szDropIniFile[0]))
		return NULL;
	
	KIniFile IniFile;
	if (IniFile.Load(szDropIniFile))
	{
		KItemDropRate * pnewDrop = new KItemDropRate;
		IniFile.GetInteger("Main", "Count", 0, &pnewDrop->nCount);
		IniFile.GetInteger("Main", "RandRange", 0, &pnewDrop->nMaxRandRate);
		IniFile.GetInteger("Main", "MagicRate", 0, &pnewDrop->nMagicRate);
		IniFile.GetInteger("Main", "MoneyRate", 20, &pnewDrop->nMoneyRate);
		IniFile.GetInteger("Main", "MoneyScale", 50, &pnewDrop->nMoneyScale);
		IniFile.GetInteger("Main", "MinItemLevelScale", 20, &pnewDrop->nMinItemLevelScale);
		IniFile.GetInteger("Main", "MaxItemLevelScale", 10, &pnewDrop->nMaxItemLevelScale);
		IniFile.GetInteger("Main", "MaxItemLevel", 10, &pnewDrop->nMaxItemLevel);
		IniFile.GetInteger("Main", "MinItemLevel", 1, &pnewDrop->nMinItemLevel);

		if(pnewDrop->nCount <= 0)
		{
			delete pnewDrop;
			return NULL;
		}

		pnewDrop->pItemParam = new KItemDropRate::KItemParam[pnewDrop->nCount];
		char szSection[10];
		
		KItemDropRate::KItemParam * pItemParam = pnewDrop->pItemParam;
		for(int i = 0; i < pnewDrop->nCount; i ++, pItemParam ++)
		{
			sprintf(szSection, "%d", i + 1);
			IniFile.GetInteger(szSection, "Genre", 0, &(pItemParam->nGenre));
			IniFile.GetInteger(szSection, "Detail", 0, &(pItemParam->nDetailType));
			IniFile.GetInteger(szSection, "Particular", 0, &(pItemParam->nParticulType));
			IniFile.GetInteger(szSection, "RandRate", 0, &(pItemParam->nRate));
		}
		return pnewDrop;
	}
	else
	{
		printf("无法找到%s\n", szDropIniFile);
		return NULL;
	}

	return NULL;
}


int	operator<(KItemDropRateNode Left, KItemDropRateNode Right)
{
	return strcmp(Left.m_szFileName, Right.m_szFileName);
};

int operator==(KItemDropRateNode Left, KItemDropRateNode Right)
{
	return (strcmp(Left.m_szFileName, Right.m_szFileName) == 0);
};


#endif


void	KNpcTemplate::InitNpcBaseData(int nNpcTemplateId)
{
	if (nNpcTemplateId < 0 ) return;
	int nNpcTempRow = nNpcTemplateId + 2;

	g_NpcSetting.GetString(nNpcTempRow, "Name", "", Name, sizeof(Name));
	g_NpcSetting.GetInteger(nNpcTempRow, "Kind", 0, (int *)&m_Kind);
	g_NpcSetting.GetInteger(nNpcTempRow, "Camp", 0, &m_Camp);
	g_NpcSetting.GetInteger(nNpcTempRow, "Series", 0, &m_Series);
	
	g_NpcSetting.GetInteger(nNpcTempRow, "HeadImage",	0, &m_HeadImage);
	g_NpcSetting.GetInteger(nNpcTempRow, "ClientOnly",	0, &m_bClientOnly);
	g_NpcSetting.GetInteger(nNpcTempRow, "CorpseIdx",	0, &m_CorpseSettingIdx);
	
	g_NpcSetting.GetInteger(nNpcTempRow, "DeathFrame",	12, &m_DeathFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "WalkFrame",	15, &m_WalkFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "RunFrame",	15, &m_RunFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "HurtFrame",	10, &m_HurtFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "WalkSpeed",	5, &m_WalkSpeed);
	g_NpcSetting.GetInteger(nNpcTempRow, "AttackSpeed",	20, &m_AttackFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "CastSpeed",	20, &m_CastFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "RunSpeed",	10, &m_RunSpeed);
	g_NpcSetting.GetInteger(nNpcTempRow, "StandFrame",	15, &m_StandFrame);
	g_NpcSetting.GetInteger(nNpcTempRow, "StandFrame1", 15, &m_StandFrame1);
	g_NpcSetting.GetInteger(nNpcTempRow, "Stature",		0,  &m_nStature);

#ifdef _SERVER	
	g_NpcSetting.GetInteger(nNpcTempRow, "Treasure",		0, &m_Treasure);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIMode",	0, &m_AiMode);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam1",	0, &m_AiParam[0]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam2",	0, &m_AiParam[1]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam3",	0, &m_AiParam[2]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam4",	0, &m_AiParam[3]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam5",	0, &m_AiParam[4]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam6",	0, &m_AiParam[5]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam7",	0, &m_AiParam[6]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam8",	0, &m_AiParam[7]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam9",	0, &m_AiParam[8]);
	g_NpcSetting.GetInteger(nNpcTempRow, "AIParam10",	5, &m_AiParam[9]);

	g_NpcSetting.GetInteger(nNpcTempRow, "FireResistMax",	0, &m_FireResistMax);
	g_NpcSetting.GetInteger(nNpcTempRow, "ColdResistMax",	0, &m_ColdResistMax);
	g_NpcSetting.GetInteger(nNpcTempRow, "LightResistMax",	0, &m_LightResistMax);
	g_NpcSetting.GetInteger(nNpcTempRow, "PoisonResistMax",	0, &m_PoisonResistMax);
	g_NpcSetting.GetInteger(nNpcTempRow, "PhysicsResistMax",	0, &m_PhysicsResistMax);
	g_NpcSetting.GetInteger(nNpcTempRow, "ActiveRadius", 30, &m_ActiveRadius);
	g_NpcSetting.GetInteger(nNpcTempRow, "VisionRadius", 40, &m_VisionRadius);
	
	char szDropFile[200];
	g_NpcSetting.GetString(nNpcTempRow, "DropRateFile", "", szDropFile, sizeof(szDropFile));
	strlwr(szDropFile);
	strcpy(m_szDropRateFile, szDropFile);
	KItemDropRateNode DropNode;
	strcpy(DropNode.m_szFileName, szDropFile);
	
	if (g_ItemDropRateBinTree.Find(DropNode))
	{
		m_pItemDropRate = DropNode.m_pItemDropRate;
	}
	else
	{
		KItemDropRateNode newDropNode;
		strcpy(newDropNode.m_szFileName,szDropFile);
		newDropNode.m_pItemDropRate = g_GenItemDropRate(szDropFile);
		g_ItemDropRateBinTree.Insert(newDropNode);
		m_pItemDropRate = newDropNode.m_pItemDropRate;
	}
	
	int nAIMaxTime = 0;
	g_NpcSetting.GetInteger(nNpcTempRow, "AIMaxTime", 25, (int*)&nAIMaxTime);
	m_AIMAXTime = (BYTE)nAIMaxTime;
	
	g_NpcSetting.GetInteger(nNpcTempRow, "HitRecover", 0, &m_HitRecover);
	g_NpcSetting.GetInteger(nNpcTempRow, "ReviveFrame", 2400, &m_ReviveFrame);
	
	char szLevelScript[MAX_PATH];
	g_NpcSetting.GetString(nNpcTempRow, "LevelScript", "", szLevelScript, MAX_PATH);
	if (!szLevelScript[0])
		m_dwLevelSettingScript = 0;
	else
	{
#ifdef WIN32
		_strlwr(szLevelScript);
#else
		for (int nl = 0; szLevelScript[nl]; nl++)
			if (szLevelScript[nl] >= 'A' && szLevelScript[nl] <= 'Z')
				szLevelScript[nl] += 'a' - 'A';
#endif
		m_dwLevelSettingScript = g_FileName2Id(szLevelScript);
	}

#else
	g_NpcSetting.GetInteger(nNpcTempRow, "ArmorType", 0, &m_ArmorType);
	g_NpcSetting.GetInteger(nNpcTempRow, "HelmType", 0, &m_HelmType);
	g_NpcSetting.GetInteger(nNpcTempRow, "WeaponType", 0, &m_WeaponType);
	g_NpcSetting.GetInteger(nNpcTempRow, "HorseType", -1, &m_HorseType);
	g_NpcSetting.GetInteger(nNpcTempRow, "RideHorse",0, &m_bRideHorse);
	g_NpcSetting.GetString(nNpcTempRow, "ActionScript", "", ActionScript,sizeof(ActionScript));
	g_NpcSetting.GetString(nNpcTempRow, "LevelScript", "", m_szLevelSettingScript, 100);
#endif

	
}

void KNpcTemplate::InitNpcLevelData(KTabFile * pKindFile, int nNpcTemplateId, KLuaScript * pLevelScript, int nLevel)
{
	if (nNpcTemplateId < 0 || nLevel <= 0 || (!pLevelScript)) return;
	int nNpcTempRow = nNpcTemplateId + 2;
	int	 nTopIndex = 0;
	m_nLevel = nLevel;

	pLevelScript->SafeCallBegin(&nTopIndex);
	{
		m_NpcSettingIdx = nNpcTemplateId;
#ifdef _SERVER				
		//技能
		char szValue1[MAX_VALUE_LEN];
		char szValue2[MAX_VALUE_LEN];
		g_NpcSetting.GetString(nNpcTempRow, "Skill1",	"", szValue1, MAX_VALUE_LEN);
		g_NpcSetting.GetString(nNpcTempRow, "Level1", "", szValue2, MAX_VALUE_LEN);
		if (szValue1[0] && szValue2[0])
			m_SkillList.SetNpcSkill(1, SkillString2Id(szValue1), GetNpcLevelDataFromScript(pLevelScript, "Level1", nLevel, szValue2));
		
		g_NpcSetting.GetString(nNpcTempRow, "Skill2",	"", szValue1, MAX_VALUE_LEN);
		g_NpcSetting.GetString(nNpcTempRow, "Level2", "", szValue2, MAX_VALUE_LEN);
		if (szValue1[0] && szValue2[0])
			m_SkillList.SetNpcSkill(2, SkillString2Id(szValue1), GetNpcLevelDataFromScript(pLevelScript, "Level2", nLevel, szValue2));

		
		g_NpcSetting.GetString(nNpcTempRow, "Skill3",	"", szValue1, MAX_VALUE_LEN);
		g_NpcSetting.GetString(nNpcTempRow, "Level3", "", szValue2, MAX_VALUE_LEN);
		if (szValue1[0] && szValue2[0])
			m_SkillList.SetNpcSkill(3, SkillString2Id(szValue1), GetNpcLevelDataFromScript(pLevelScript, "Level3", nLevel, szValue2));


		g_NpcSetting.GetString(nNpcTempRow, "Skill4",	"", szValue1, MAX_VALUE_LEN);
		g_NpcSetting.GetString(nNpcTempRow, "Level4", "", szValue2, MAX_VALUE_LEN);
		if (szValue1[0] && szValue2[0])
			m_SkillList.SetNpcSkill(4, SkillString2Id(szValue1), GetNpcLevelDataFromScript(pLevelScript, "Level4", nLevel, szValue2));
		//Question  Change as 1Level , only debug version


		float nParam1 = 0;
		float nParam2 = 0;
		float nParam3 = 0;
		float nParam = 1;
		
		g_NpcSetting.GetFloat(nNpcTempRow, "ExpParam", 1, &nParam);
		g_NpcSetting.GetFloat(nNpcTempRow, "ExpParam1", 0, &nParam1);
		g_NpcSetting.GetFloat(nNpcTempRow, "ExpParam2", 0, &nParam2);
		g_NpcSetting.GetFloat(nNpcTempRow, "ExpParam3", 0, &nParam3);
		m_Experience = nParam * GetNpcLevelDataFromScript(pLevelScript, "Exp", nLevel, nParam1, nParam2, nParam3) / 100;
		
		g_NpcSetting.GetFloat(nNpcTempRow, "LifeParam", 1, &nParam);
		g_NpcSetting.GetFloat(nNpcTempRow, "LifeParam1", 0, &nParam1);
		g_NpcSetting.GetFloat(nNpcTempRow, "LifeParam2", 0, &nParam2);
		g_NpcSetting.GetFloat(nNpcTempRow, "LifeParam3", 0, &nParam3);
		m_LifeMax = nParam *  GetNpcLevelDataFromScript(pLevelScript, "Life", nLevel, nParam1, nParam2, nParam3) / 100;
		if (m_LifeMax == 0) m_LifeMax = 100;
		

		g_NpcSetting.GetString(nNpcTempRow, "LifeReplenish", "", szValue1, MAX_VALUE_LEN);
		m_LifeReplenish = GetNpcLevelDataFromScript(pLevelScript, "LifeReplenish", nLevel, szValue1);



		g_NpcSetting.GetFloat(nNpcTempRow, "ARParam", 1, &nParam);
		g_NpcSetting.GetFloat(nNpcTempRow, "ARParam1", 0, &nParam1);
		g_NpcSetting.GetFloat(nNpcTempRow, "ARParam2", 0, &nParam2);
		g_NpcSetting.GetFloat(nNpcTempRow, "ARParam3", 0, &nParam3);
		m_AttackRating = nParam * GetNpcLevelDataFromScript(pLevelScript, "AR", nLevel, nParam1, nParam2, nParam3) / 100;
		if (m_AttackRating == 0) m_AttackRating = 100;


		g_NpcSetting.GetFloat(nNpcTempRow, "DefenseParam", 1, &nParam);
		g_NpcSetting.GetFloat(nNpcTempRow, "DefenseParam1", 0, &nParam1);
		g_NpcSetting.GetFloat(nNpcTempRow, "DefenseParam2", 0, &nParam2);
		g_NpcSetting.GetFloat(nNpcTempRow, "DefenseParam3", 0, &nParam3);
		m_Defend = nParam *GetNpcLevelDataFromScript(pLevelScript, "Defense", nLevel, nParam1, nParam2, nParam3) / 100;


		g_NpcSetting.GetFloat(nNpcTempRow, "MinDamageParam", 1, &nParam);
		g_NpcSetting.GetFloat(nNpcTempRow, "MinDamageParam1", 0, &nParam1);
		g_NpcSetting.GetFloat(nNpcTempRow, "MinDamageParam2", 0, &nParam2);
		g_NpcSetting.GetFloat(nNpcTempRow, "MinDamageParam3", 0, &nParam3);
		m_PhysicsDamage.nValue[0] = nParam * GetNpcLevelDataFromScript(pLevelScript, "MinDamage", nLevel, nParam1, nParam2, nParam3) / 100;

		g_NpcSetting.GetFloat(nNpcTempRow, "MaxDamageParam", 1, &nParam);
		g_NpcSetting.GetFloat(nNpcTempRow, "MaxDamageParam1", 0, &nParam1);
		g_NpcSetting.GetFloat(nNpcTempRow, "MaxDamageParam2", 0, &nParam2);
		g_NpcSetting.GetFloat(nNpcTempRow, "MaxDamageParam3", 0, &nParam3);
		m_PhysicsDamage.nValue[2] = nParam * GetNpcLevelDataFromScript(pLevelScript, "MaxDamage", nLevel, nParam1, nParam2, nParam3) / 100;

		
		g_NpcSetting.GetString(nNpcTempRow, "RedLum", "", szValue1, MAX_VALUE_LEN);
		m_RedLum = GetNpcLevelDataFromScript(pLevelScript, "RedLum", nLevel, szValue1);
		
		g_NpcSetting.GetString(nNpcTempRow, "GreenLum", "", szValue1, MAX_VALUE_LEN);
		m_GreenLum = GetNpcLevelDataFromScript(pLevelScript, "GreenLum", nLevel, szValue1);
		
		g_NpcSetting.GetString(nNpcTempRow, "BlueLum", "", szValue1, MAX_VALUE_LEN);
		m_BlueLum = GetNpcLevelDataFromScript(pLevelScript, "BlueLum", nLevel, szValue1);
		
	
		g_NpcSetting.GetString(nNpcTempRow, "FireResist", "", szValue1, MAX_VALUE_LEN);
		m_FireResist = GetNpcLevelDataFromScript(pLevelScript, "FireResist", nLevel, szValue1);

		g_NpcSetting.GetString(nNpcTempRow, "ColdResist", "", szValue1, MAX_VALUE_LEN);
		m_ColdResist = GetNpcLevelDataFromScript(pLevelScript, "ColdResist", nLevel, szValue1);

		g_NpcSetting.GetString(nNpcTempRow, "LightResist", "", szValue1, MAX_VALUE_LEN);
		m_LightResist = GetNpcLevelDataFromScript(pLevelScript, "LightResist", nLevel, szValue1);
		
		g_NpcSetting.GetString(nNpcTempRow, "PoisonResist", "", szValue1, MAX_VALUE_LEN);
		m_PoisonResist = GetNpcLevelDataFromScript(pLevelScript, "PoisonResist", nLevel, szValue1);

		g_NpcSetting.GetString(nNpcTempRow, "PhysicsResist", "", szValue1, MAX_VALUE_LEN);
		m_PhysicsResist = GetNpcLevelDataFromScript(pLevelScript, "PhysicsResist", nLevel, szValue1);
#endif
#ifndef _SERVER
		int nParam1 = 0;
		int nParam2 = 0;
		int nParam3 = 0;
		int nParam = 1;
		g_NpcSetting.GetInteger(nNpcTempRow, "LifeParam", 1, &nParam);
		g_NpcSetting.GetInteger(nNpcTempRow, "LifeParam1", 0, &nParam1);
		g_NpcSetting.GetInteger(nNpcTempRow, "LifeParam2", 0, &nParam2);
		g_NpcSetting.GetInteger(nNpcTempRow, "LifeParam3", 0, &nParam3);
		m_LifeMax = nParam *  GetNpcLevelDataFromScript(pLevelScript, "Life", nLevel, nParam1, nParam2, nParam3) / 100;
		if (m_LifeMax == 0) m_LifeMax = 100;
#endif
	}
	pLevelScript->SafeCallEnd(nTopIndex);
}

int KNpcTemplate::GetNpcLevelDataFromScript(KLuaScript * pScript, char * szDataName, int nLevel, char * szParam)
{
	int nTopIndex = 0;
	int nReturn = 0;
	if (szParam == NULL|| szParam[0] == 0 || strlen(szParam) < 3)
	{
		return 0;
	}
	pScript->SafeCallBegin(&nTopIndex);
	pScript->CallFunction("GetNpcLevelData", 1, "dss", nLevel, szDataName, szParam);
	nTopIndex = Lua_GetTopIndex(pScript->m_LuaState);
	nReturn = (int) Lua_ValueToNumber(pScript->m_LuaState, nTopIndex);
	pScript->SafeCallEnd(nTopIndex);
	return nReturn;
}

int KNpcTemplate::GetNpcLevelDataFromScript(KLuaScript * pScript, char * szDataName, int nLevel, double nParam1, double nParam2, double nParam3)
{
	int nTopIndex = 0;
	int nReturn = 0;
	pScript->SafeCallBegin(&nTopIndex);
	pScript->CallFunction("GetNpcKeyData", 1, "dsnnn", nLevel, szDataName, nParam1, nParam2, nParam3);
	nTopIndex = Lua_GetTopIndex(pScript->m_LuaState);
	nReturn = (int) Lua_ValueToNumber(pScript->m_LuaState, nTopIndex);
	pScript->SafeCallEnd(nTopIndex);
	return nReturn;
}


int KNpcTemplate::SkillString2Id(char * szSkillString)
{
	if (!szSkillString[0]) return 0;
	int nSkillNum = g_OrdinSkillsSetting.GetHeight() - 1;
	char szSkillName[100];
	for (int i = 0 ;  i < nSkillNum; i ++)
	{
		g_OrdinSkillsSetting.GetString(i + 2, "SkillName", "", szSkillName, sizeof(szSkillName));
		if (g_StrCmp(szSkillString, szSkillName))
		{
			int nSkillId = 0;
			g_OrdinSkillsSetting.GetInteger(i + 2, "SkillId", 0, &nSkillId);
			return nSkillId;
		}
	}
	g_DebugLog("无找到该[%s]魔法的Id,请检查！", szSkillString);
	return 0;
}


