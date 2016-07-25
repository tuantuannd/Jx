#ifndef	_KNPCTEMPLATE_H
#define	_KNPCTEMPLATE_H

#ifdef _SERVER
class KItemDropRateNode
{
public:
	KItemDropRate * m_pItemDropRate;
	char m_szFileName[200];
	KItemDropRateNode()
	{
		m_pItemDropRate = NULL;
	}
	~KItemDropRateNode()
	{
	//	if (m_pItemDropRate)
	//		delete m_pItemDropRate;
	//	m_pItemDropRate = NULL;
	}
};
#endif

#ifdef _SERVER
typedef	BinSTree<KItemDropRateNode> KItemDropRateTree;
extern KItemDropRateTree g_ItemDropRateBinTree;
#endif

#include "KCore.h"
#include "KNpc.h"

class KNpcTemplate
{
public:
private:
public:
	char	Name[32];
	DWORD	m_Kind;
	int		m_Camp;
	int		m_Series;
	int		m_HeadImage;
	int		m_bClientOnly;
	int		m_CorpseSettingIdx;
	int		m_DeathFrame;
	int		m_WalkFrame;
	int		m_StandFrame;
	int		m_StandFrame1;
	int		m_RunFrame;
	int		m_HurtFrame;
	int		m_WalkSpeed;
	int		m_AttackFrame;
	int		m_CastFrame;
	int		m_RunSpeed;
	int		m_LifeMax;

#ifdef _SERVER	
	DWORD	m_dwLevelSettingScript;
	int		m_Treasure;
	int		m_AiMode;
	int		m_AiParam[MAX_AI_PARAM - 1];
	int		m_FireResistMax;
	int		m_ColdResistMax;
	int		m_LightResistMax;
	int		m_PoisonResistMax;
	int		m_PhysicsResistMax;
	int		m_ActiveRadius;
	int		m_VisionRadius;
	BYTE	m_AIMAXTime;
	int		m_HitRecover;
	int		m_ReviveFrame;

	int		m_Experience;
	int		m_LifeReplenish;
	int		m_AttackRating;
	int		m_Defend;
	KMagicAttrib		m_PhysicsDamage;
	int		m_RedLum;
	int		m_GreenLum;
	int		m_BlueLum;
	int		m_FireResist;
	int		m_ColdResist;
	int		m_LightResist;
	int		m_PoisonResist;
	int		m_PhysicsResist;
	KSkillList m_SkillList;
	KItemDropRate *m_pItemDropRate;
	char	m_szDropRateFile[150];
	
#endif
	
#ifndef _SERVER	
	int		m_ArmorType;
	int		m_HelmType;
	int		m_WeaponType;
	int		m_HorseType;
	int		m_bRideHorse;
	char	ActionScript[32];
	char	m_szLevelSettingScript[100];
#endif
	
	int		m_NpcSettingIdx;
	int		m_nSkillID[4];
	int		m_nSkillLevel[4];
	BOOL	m_bHaveLoadedFromTemplate;
	int		m_nStature;	
	int		m_nLevel;
	
public:
	void	InitNpcBaseData(int nNpcTemplateId);
	void	InitNpcLevelData(KTabFile * pKindFile, int nNpcTemplateId, KLuaScript * pLevelScript, int nLevel);
	static int		GetNpcLevelDataFromScript(KLuaScript * pScript, char * szDataName, int nLevel, char * szParam);
	static int		GetNpcLevelDataFromScript(KLuaScript * pScript, char * szDataName, int nLevel, double nParam1, double nParam2, double nParam3);
	static int		SkillString2Id(char * szSkillString);
	KNpcTemplate(){	m_bHaveLoadedFromTemplate = FALSE;};
};

extern KNpcTemplate	* g_pNpcTemplate[MAX_NPCSTYLE][MAX_NPC_LEVEL]; //0,0ÎªÆðµã

#endif

